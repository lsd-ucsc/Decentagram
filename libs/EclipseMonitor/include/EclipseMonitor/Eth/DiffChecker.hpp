// Copyright (c) 2022 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <memory>

#include "../MonitorReport.hpp"

#include "CheckpointMgr.hpp"
#include "DAA.hpp"
#include "DataTypes.hpp"
#include "HeaderMgr.hpp"
#include "Params.hpp"

namespace EclipseMonitor
{
namespace Eth
{


class DiffCheckerBase
{
public:
	DiffCheckerBase() = default;

	// LCOV_EXCL_START
	virtual ~DiffCheckerBase() = default;
	// LCOV_EXCL_STOP

	virtual void OnChkptUpd(const CheckpointMgr& chkpt) = 0;

	/**
	 * @brief Check the difficulty value (or anything equivalent) of the
	 *        current header
	 *
	 * @param parentHdr
	 * @param currentHdr
	 * @return true if the difficulty value is OK, otherwise false
	 */
	virtual bool CheckDifficulty(
		const HeaderMgr& parentHdr,
		const HeaderMgr& currentHdr) const = 0;

	/**
	 * @brief Check the estimated difficulty value (or anything equivalent)
	 *        based on the current time
	 *
	 * @param parentHdr
	 * @param currentHdr
	 * @return true if the difficulty value is OK, otherwise false
	 */
	virtual bool CheckEstDifficulty(
		const HeaderMgr& parentHdr,
		uint64_t currentTime) const = 0;

}; // class DiffCheckerBase


class PoWDiffChecker : public DiffCheckerBase
{
public: // static members:
	using Self = PoWDiffChecker;
	using Base = DiffCheckerBase;

public:
	PoWDiffChecker(
		const MonitorConfig& mConf,
		std::unique_ptr<DAABase> diffEstimator
	) :
		DiffCheckerBase(),
		m_minDiffPercent(mConf.get_minDiffPercent().GetVal()),
		m_maxWaitTime(mConf.get_maxWaitTime().GetVal()),
		m_diffEstimator(std::move(diffEstimator))
	{}

	// LCOV_EXCL_START
	virtual ~PoWDiffChecker() = default;
	// LCOV_EXCL_STOP

	virtual void OnChkptUpd(const CheckpointMgr& chkpt) override
	{
		m_minDiff = (chkpt.GetDiffMedian() >> 7) * m_minDiffPercent;
	}

	virtual bool CheckDifficulty(
		const HeaderMgr& parentHdr,
		const HeaderMgr& currentHdr
	) const override
	{
		return
			// current header is received after the parent header
			(currentHdr.GetTrustedTime() > parentHdr.GetTrustedTime()) &&
			// current header is received within the max wait time
			((currentHdr.GetTrustedTime() - parentHdr.GetTrustedTime()) <=
				m_maxWaitTime) &&
			// current header's difficulty is greater than the min difficulty
			(currentHdr.GetDiff() >= m_minDiff);
	}

	virtual bool CheckEstDifficulty(
		const HeaderMgr& parentHdr,
		uint64_t currentTime
	) const override
	{
		HeaderMgr estNextHdr;
		estNextHdr.SetNumber(parentHdr.GetNumber() + 1);
		estNextHdr.SetTime(currentTime);

		auto estDiff = (*m_diffEstimator)(parentHdr, estNextHdr);

		auto deltaTime = currentTime - parentHdr.GetTrustedTime();

		return
			// current header is received within the max wait time
			(deltaTime <= m_maxWaitTime) &&
			// estimated difficulty is greater than the min difficulty
			(estDiff >= m_minDiff);
	}


private:
	uint8_t m_minDiffPercent;
	Difficulty m_minDiff;
	uint64_t m_maxWaitTime;

	std::unique_ptr<DAABase> m_diffEstimator;

}; // class PoWDiffChecker


class PoSDiffChecker : public DiffCheckerBase
{
public: // static members:
	using Self = PoSDiffChecker;
	using Base = DiffCheckerBase;

public:
	PoSDiffChecker(
		const MonitorConfig& /* mConf */
	) :
		Base()
	{}

	// LCOV_EXCL_START
	virtual ~PoSDiffChecker() = default;
	// LCOV_EXCL_STOP

	virtual void OnChkptUpd(
		const CheckpointMgr& /* chkpt */
	) override
	{}

	virtual bool CheckDifficulty(
		const HeaderMgr& /* parentHdr */,
		const HeaderMgr& /* currentHdr */
	) const override
	{
		// At this moment, we don't check anything for Proof-of-Stake
		return true;
	}

	virtual bool CheckEstDifficulty(
		const HeaderMgr& /* parentHdr */,
		uint64_t         /* currentTime */
	) const override
	{
		// At this moment, we don't check anything for Proof-of-Stake
		return true;
	}


private:

}; // class PoSDiffChecker


template<typename _NetConfig>
class GenericDiffCheckerImpl : public DiffCheckerBase
{
public: // static members:
	using Self = GenericDiffCheckerImpl<_NetConfig>;
	using Base = DiffCheckerBase;

public:
	GenericDiffCheckerImpl(
		const MonitorConfig& mConf,
		std::unique_ptr<DAABase> diffEstimator
	) :
		Base(),
		m_powChecker(mConf, std::move(diffEstimator)),
		m_posChecker(mConf)
	{}

	// LCOV_EXCL_START
	virtual ~GenericDiffCheckerImpl() = default;
	// LCOV_EXCL_STOP

	virtual void OnChkptUpd(const CheckpointMgr& chkpt) override
	{
		auto blkNumRange = chkpt.GetCheckpointBlkNumRange();
		if (_NetConfig::IsBlockOfParis(blkNumRange.second))
		{
			return m_posChecker.OnChkptUpd(chkpt);
		}
		else
		{
			return m_powChecker.OnChkptUpd(chkpt);
		}
	}

	virtual bool CheckDifficulty(
		const HeaderMgr& parentHdr,
		const HeaderMgr& currentHdr
	) const override
	{
		if (_NetConfig::IsBlockOfParis(currentHdr.GetNumber()))
		{
			return m_posChecker.CheckDifficulty(parentHdr, currentHdr);
		}
		else
		{
			return m_powChecker.CheckDifficulty(parentHdr, currentHdr);
		}
	}

	virtual bool CheckEstDifficulty(
		const HeaderMgr& parentHdr,
		uint64_t currentTime
	) const override
	{
		if (_NetConfig::IsBlockOfParis(parentHdr.GetNumber() + 1))
		{
			return m_posChecker.CheckEstDifficulty(parentHdr, currentTime);
		}
		else
		{
			return m_powChecker.CheckEstDifficulty(parentHdr, currentTime);
		}
	}

private:

	PoWDiffChecker m_powChecker;
	PoSDiffChecker m_posChecker;
}; // class GenericDiffCheckerImpl


using DiffCheckerMainNet =
	GenericDiffCheckerImpl<MainnetConfig>;


} // namespace Eth
} // namespace EclipseMonitor
