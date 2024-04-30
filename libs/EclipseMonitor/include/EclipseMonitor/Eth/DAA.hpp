// Copyright (c) 2022 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "../Exceptions.hpp"

#include "DataTypes.hpp"
#include "HeaderMgr.hpp"
#include "Params.hpp"

namespace EclipseMonitor
{
namespace Eth
{


// Reference: https://github.com/ethereum/go-ethereum/blob/master/consensus/ethash/consensus.go


class DAABase
{
public: // static members:

	using BlkNumType = BlockNumber;
	using TimeType   = Timestamp;
	using DiffType   = Difficulty;

public:

	DAABase() = default;

	// LCOV_EXCL_START
	virtual ~DAABase() = default;
	// LCOV_EXCL_STOP

	virtual DiffType operator()(
		const HeaderMgr& parent,
		const HeaderMgr& current
	) const = 0;

protected:

	static DiffType CalcBomb(
		const BlkNumType& periodCount,
		const BlkNumType& blkNBig2,
		const DiffType&   diffBig1,
		const DiffType&   diffBig2)
	{
		if (periodCount <= blkNBig2)
		{
			// reference: https://pkg.go.dev/math/big#Int.Exp
			return diffBig1;
		}
		else
		{
			BlkNumType exp = periodCount - blkNBig2;
			DiffType y = diffBig1;
			for (BlkNumType i = 0; i < exp; i++)
			{
				y *= diffBig2;
			}
			return y;
		}
	}
}; // class DAABase


class EthashDAACalculator : public DAABase
{
public: // static members:
	using Self = EthashDAACalculator;
	using Base = DAABase;

	using BlkNumType = typename Base::BlkNumType;
	using TimeType   = typename Base::TimeType;
	using DiffType   = typename Base::DiffType;

	static const EthashDAACalculator& GetEip5133()
	{
		static const EthashDAACalculator inst(
			/* considerUncle */     true,
			/* deltaDivisor */      DiffType(9),
			/* hasMaxCheck */       true,
			/* hasBombDelay */      true,
			/* bombDelay */         BlkNumType(11400000U) );
		return inst;
	}

	static const EthashDAACalculator& GetEip5133Estimated()
	{
		static const EthashDAACalculator inst(
			/* considerUncle */     true,
			/* deltaDivisor */      DiffType(9),
			/* hasMaxCheck */       false,
			/* hasBombDelay */      true,
			/* bombDelay */         BlkNumType(11400000U) );
		return inst;
	}

	static const EthashDAACalculator& GetEip4345()
	{
		static const EthashDAACalculator inst(
			/* considerUncle */     true,
			/* deltaDivisor */      DiffType(9),
			/* hasMaxCheck */       true,
			/* hasBombDelay */      true,
			/* bombDelay */         BlkNumType(10700000U));
		return inst;
	}

	static const EthashDAACalculator& GetEip3554()
	{
		static const EthashDAACalculator inst(
			/* considerUncle */     true,
			/* deltaDivisor */      DiffType(9),
			/* hasMaxCheck */       true,
			/* hasBombDelay */      true,
			/* bombDelay */         BlkNumType(9700000U));
		return inst;
	}

	static const EthashDAACalculator& GetEip2384()
	{
		static const EthashDAACalculator inst(
			/* considerUncle */     true,
			/* deltaDivisor */      DiffType(9),
			/* hasMaxCheck */       true,
			/* hasBombDelay */      true,
			/* bombDelay */         BlkNumType(9000000U));
		return inst;
	}

	static const EthashDAACalculator& GetConstantinople()
	{
		static const EthashDAACalculator inst(
			/* considerUncle */     true,
			/* deltaDivisor */      DiffType(9),
			/* hasMaxCheck */       true,
			/* hasBombDelay */      true,
			/* bombDelay */         BlkNumType(5000000U));
		return inst;
	}

	static const EthashDAACalculator& GetByzantium()
	{
		static const EthashDAACalculator inst(
			/* considerUncle */     true,
			/* deltaDivisor */      DiffType(9),
			/* hasMaxCheck */       true,
			/* hasBombDelay */      true,
			/* bombDelay */         BlkNumType(3000000U));
		return inst;
	}

	static const EthashDAACalculator& GetHomestead()
	{
		static const EthashDAACalculator inst(
			/* considerUncle */     false,
			/* deltaDivisor */      DiffType(10),
			/* hasMaxCheck */       true,
			/* hasBombDelay */      false,
			/* bombDelay */         BlkNumType(0));
		return inst;
	}

public:
	EthashDAACalculator(
		bool considerUncle,
		const DiffType& deltaDivisor,
		bool hasMaxCheck,
		bool hasBombDelay,
		const BlkNumType& bombDelay) :
		DAABase(),
		m_considerUncle(considerUncle),
		m_deltaDivisor(deltaDivisor),
		m_hasMaxCheck(hasMaxCheck),
		m_hasBombDelay(hasBombDelay),
		m_bombDelay(bombDelay),
		m_bombDelayFromParent(m_bombDelay - BlkNumType(1))
	{}

	// LCOV_EXCL_START
	virtual ~EthashDAACalculator() = default;
	// LCOV_EXCL_STOP

	virtual DiffType operator()(
		const HeaderMgr& parent,
		const HeaderMgr& current
	) const override
	{
		return Self::operator()(
			parent.GetNumber(),
			parent.GetTime(),
			parent.GetDiff(),
			parent.HasUncle(),
			current.GetNumber(),
			current.GetTime()
		);
	}

	DiffType operator()(
		const BlkNumType& parentBlkNum,
		const TimeType& parentTime,
		const DiffType& parentDiff,
		bool parentHasUncle,
		const BlkNumType& /* currBlkNum */,
		const TimeType& currTime) const
	{
		// Reference: https://github.com/ethereum/go-ethereum/blob/master/consensus/ethash/consensus.go

		static const DiffType sk_diffBig1  = DiffType(1);
		static const DiffType sk_diffBig2  = DiffType(2);
		static const DiffType sk_diffBig99 = DiffType(99);
		static const DiffType sk_minDiff   =
			DiffType(Params::GetMinimumDifficulty());

		static const BlkNumType sk_blkNBig0      = BlkNumType(0);
		static const BlkNumType sk_blkNBig1      = BlkNumType(1);
		static const BlkNumType sk_blkNBig2      = BlkNumType(2);
		static const BlkNumType sk_expDiffPeriod = BlkNumType(100000);

		bool isReducing = false;

		// Homestead algorithm:
		// diff = (parent_diff +
		//         (parent_diff / 2048 *
		//            max(1 - (block_timestamp - parent_timestamp) // 10, -99))
		//        ) + 2^(periodCount - 2)
		// Byzantium algorithm:
		// diff = (parent_diff +
		//         (parent_diff / 2048 *
		//            max((2 if len(parent.uncles) else 1) -
		//                   ((timestamp - parent.timestamp) // 9), -99))
		//        ) + 2^(periodCount - 2)


		// (block_timestamp - parent_timestamp) // 9
		// assuming currTime > parentTime is checked during validation
		DiffType x = DiffType(currTime - parentTime);
		x = x / m_deltaDivisor;

		// (2 if len(parent_uncles) else 1) - (block_timestamp - parent_timestamp) // 9
		if (m_considerUncle && parentHasUncle)
		{
			// 2 - x ==> if x > 2; -1 * (x - 2); else; (2 - x)
			if (x > sk_diffBig2)
			{
				isReducing = true;
				x -= sk_diffBig2;
			}
			else
			{
				isReducing = false;
				x = sk_diffBig2 - x;
			}
		}
		else
		{
			// 1 - x ==> if x > 1; -1 * (x - 1); else; (1 - x)
			if (x > sk_diffBig1)
			{
				isReducing = true;
				x -= sk_diffBig1;
			}
			else
			{
				isReducing = false;
				x = sk_diffBig1 - x;
			}
		}

		// max((2 if len(parent.uncles) else 1) - ((timestamp - parent.timestamp) // 9), -99)
		if (m_hasMaxCheck && isReducing && (x > sk_diffBig99))
		{
			x = sk_diffBig99;
		}

		// parent_diff / 2048
		DiffType y = parentDiff >> Params::GetDifficultyBoundDivisorBitShift();

		// (parent_diff / 2048) *
		//     max((2 if len(parent.uncles) else 1) - ((timestamp - parent.timestamp) // 9), -99)
		x = y * x;

		// parent_diff +
		//     (parent_diff / 2048 * max((2 if len(parent.uncles) else 1) - ((timestamp - parent.timestamp) // 9), -99))
		if (!m_hasMaxCheck && isReducing && (x > parentDiff))
		{
			x = sk_minDiff;
		}
		else if (isReducing)
		{
			x = parentDiff - x;
		}
		else
		{
			x = parentDiff + x;
		}

		// minimum difficulty cap
		if (x < sk_minDiff)
		{
			x = sk_minDiff;
		}

		// calculate delay bomb for the ice-age delay
		BlkNumType fakeBlockNumber = sk_blkNBig0;
		if (m_hasBombDelay)
		{
			if (parentBlkNum >= m_bombDelayFromParent)
			{
				fakeBlockNumber = parentBlkNum - m_bombDelayFromParent;
			}
		}
		else
		{
			fakeBlockNumber = parentBlkNum + sk_blkNBig1;
		}

		BlkNumType periodCount = fakeBlockNumber / sk_expDiffPeriod;
		if (periodCount > sk_blkNBig1)
		{
			y = CalcBomb(periodCount, sk_blkNBig2, sk_diffBig1, sk_diffBig2);
			x += y;
		}

		return x;
	}

private:
	bool m_considerUncle;
	DiffType m_deltaDivisor;
	bool m_hasMaxCheck;
	bool m_hasBombDelay;
	BlkNumType m_bombDelay;
	BlkNumType m_bombDelayFromParent;
}; // class EthashDAACalculator


class EthashDAACalculatorFrontier : public DAABase
{
public: // static members:
	using Self = EthashDAACalculatorFrontier;
	using Base = DAABase;

	using BlkNumType = typename Base::BlkNumType;
	using TimeType   = typename Base::TimeType;
	using DiffType   = typename Base::DiffType;

	static const EthashDAACalculatorFrontier& GetInstance()
	{
		static const EthashDAACalculatorFrontier inst;
		return inst;
	}

public:
	EthashDAACalculatorFrontier() = default;

	// LCOV_EXCL_START
	virtual ~EthashDAACalculatorFrontier() = default;
	// LCOV_EXCL_STOP

	virtual DiffType operator()(
		const HeaderMgr& parent,
		const HeaderMgr& current
	) const override
	{
		return Self::operator()(
			parent.GetNumber(),
			parent.GetTime(),
			parent.GetDiff(),
			parent.HasUncle(),
			current.GetNumber(),
			current.GetTime()
		);
	}

	DiffType operator()(
		const BlkNumType& parentBlkNum,
		const TimeType& parentTime,
		const DiffType& parentDiff,
		bool /* parentHasUncle */,
		const BlkNumType& /* currBlkNum */,
		const TimeType& currTime
	) const
	{
		static const DiffType sk_diffBig1  = DiffType(1);
		static const DiffType sk_diffBig2  = DiffType(2);
		static const DiffType sk_minDiff   =
			DiffType(Params::GetMinimumDifficulty());

		static const BlkNumType sk_blkNBig1      = BlkNumType(1);
		static const BlkNumType sk_blkNBig2      = BlkNumType(2);
		static const BlkNumType sk_expDiffPeriod = BlkNumType(100000);

		// Frontier algorithm:
		// diff =
		//    (parent_diff + (parent_diff / 2048))
		//        if (timestamp - parent_timestamp) < DurationLimit else
		//            (parent_diff - (parent_diff / 2048)) +
		//                2^(periodCount - 2)

		DiffType adjust = parentDiff >> Params::GetDifficultyBoundDivisorBitShift();

		TimeType delta = currTime - parentTime;

		DiffType diff = parentDiff;
		if (delta < Params::GetDurationLimit())
		{
			diff += adjust;
		}
		else
		{
			diff -= adjust;
		}

		if (diff < sk_minDiff)
		{
			diff = sk_minDiff;
		}

		BlkNumType fakeBlockNumber = parentBlkNum + sk_blkNBig1;

		BlkNumType periodCount = fakeBlockNumber / sk_expDiffPeriod;
		if (periodCount > sk_blkNBig1)
		{
			DiffType bombVal =
				CalcBomb(periodCount, sk_blkNBig2, sk_diffBig1, sk_diffBig2);
			diff += bombVal;
		}

		return diff;
	}

}; // class EthashDAACalculatorFrontier


template<typename _ChainConfig>
class EthashDAAImpl : public DAABase
{
public: // static members:
	using Self = EthashDAAImpl<_ChainConfig>;
	using Base = DAABase;

	using ChainConfig = _ChainConfig;

	using BlkNumType = typename Base::BlkNumType;
	using TimeType   = typename Base::TimeType;
	using DiffType   = typename Base::DiffType;

	static const Base& GetCalculator(const BlkNumType& blkNum)
	{
		if (ChainConfig::IsBlockOfParis(blkNum))
		{
			throw Exception("Blocks since Paris fork no longer use DAA");
		}
		else if (ChainConfig::IsBlockOfGrayGlacier(blkNum))
		{
			return EthashDAACalculator::GetEip5133();
		}
		else if (ChainConfig::IsBlockOfArrowGlacier(blkNum))
		{
			return EthashDAACalculator::GetEip4345();
		}
		else if (ChainConfig::IsBlockOfLondon(blkNum))
		{
			return EthashDAACalculator::GetEip3554();
		}
		else if (ChainConfig::IsBlockOfMuirGlacier(blkNum))
		{
			return EthashDAACalculator::GetEip2384();
		}
		else if (ChainConfig::IsBlockOfConstantinople(blkNum))
		{
			return EthashDAACalculator::GetConstantinople();
		}
		else if (ChainConfig::IsBlockOfByzantium(blkNum))
		{
			return EthashDAACalculator::GetByzantium();
		}
		else if (ChainConfig::IsBlockOfHomestead(blkNum))
		{
			return EthashDAACalculator::GetHomestead();
		}
		else
		{
			return EthashDAACalculatorFrontier::GetInstance();
		}
	}

public:

	EthashDAAImpl() = default;

	// LCOV_EXCL_START
	virtual ~EthashDAAImpl() = default;
	// LCOV_EXCL_STOP

	virtual DiffType operator()(
		const HeaderMgr& parent,
		const HeaderMgr& current
	) const override
	{
		const DAABase& calculator = GetCalculator(current.GetNumber());
		return calculator(parent, current);
	}

}; // class EthashDAAImpl


template<typename _ChainConfig>
class EthashDAAEstImpl : public DAABase
{
public: // static members:
	using Self = EthashDAAEstImpl<_ChainConfig>;
	using Base = DAABase;

	using ChainConfig = _ChainConfig;

	using BlkNumType = typename Base::BlkNumType;
	using TimeType   = typename Base::TimeType;
	using DiffType   = typename Base::DiffType;

	static const Base& GetCalculator(const BlkNumType& blkNum)
	{
		if (ChainConfig::IsBlockOfParis(blkNum))
		{
			throw Exception("Blocks since Paris fork no longer use DAA");
		}
		else if (ChainConfig::IsBlockOfGrayGlacier(blkNum))
		{
			return EthashDAACalculator::GetEip5133Estimated();
		}
		else
		{
			throw Exception("Estimating the difficulty value before EIP-5133 "
				"is not allowed.");
		}
	}

public:

	EthashDAAEstImpl() = default;

	// LCOV_EXCL_START
	virtual ~EthashDAAEstImpl() = default;
	// LCOV_EXCL_STOP

	virtual DiffType operator()(
		const HeaderMgr& parent,
		const HeaderMgr& current
	) const override
	{
		const DAABase& calculator = GetCalculator(current.GetNumber());
		return calculator(parent, current);
	}

}; // class EthashDAAEstImpl


template<typename _ChainConfig>
class TestnetDAACalculator : public DAABase
{
public: // static members:
	using Self = TestnetDAACalculator;
	using Base = DAABase;

	using ChainConfig = _ChainConfig;

	using DiffType   = typename Base::DiffType;

public:
	TestnetDAACalculator() = default;

	// LCOV_EXCL_START
	virtual ~TestnetDAACalculator() = default;
	// LCOV_EXCL_STOP

	virtual DiffType operator()(
		const HeaderMgr&,
		const HeaderMgr& current
	) const override
	{
		if (ChainConfig::IsBlockOfParis(current.GetNumber()))
		{
			throw Exception("Blocks since Paris fork no longer use DAA");
		}
		else
		{
			// The testnet is using the Clique consensus protocol, where
			// the difficulty is calculated differently.
			// At the moment, we don't have a plan to calculate the actual value
			return current.GetDiff();
		}
	}
}; // class TestnetDAACalculator


template<typename _ChainConfig>
class TestnetDAAEstCalculator : public DAABase
{
public: // static members:
	using Self = TestnetDAAEstCalculator;
	using Base = DAABase;

	using ChainConfig = _ChainConfig;

	using DiffType   = typename Base::DiffType;

public:
	TestnetDAAEstCalculator() = default;

	// LCOV_EXCL_START
	virtual ~TestnetDAAEstCalculator() = default;
	// LCOV_EXCL_STOP

	virtual DiffType operator()(
		const HeaderMgr&,
		const HeaderMgr& current
	) const override
	{
		if (ChainConfig::IsBlockOfParis(current.GetNumber()))
		{
			throw Exception("Blocks since Paris fork no longer use DAA");
		}
		else
		{
			// The testnet is using the Clique consensus protocol, where
			// the difficulty is calculated differently.
			// At the moment, we don't have a plan to calculate the actual value
			// The possible values are either 1 or 2.
			// cite: `diffInTurn` or `diffNoTurn`
			//     from github.com/ethereum/go-ethereum/consensus/clique/clique.go
			return 2;
		}
	}
}; // class TestnetDAAEstCalculator


template<typename _ChainConfig>
struct DAASelector;

template<>
struct DAASelector<MainnetConfig>
{
	using Calculator = EthashDAAImpl<MainnetConfig>;
	using Estimator  = EthashDAAEstImpl<MainnetConfig>;
}; // struct DAASelector<MainnetConfig>

template<>
struct DAASelector<GoerliConfig>
{
	using Calculator = TestnetDAACalculator<GoerliConfig>;
	using Estimator  = TestnetDAAEstCalculator<GoerliConfig>;
}; // struct DAASelector<GoerliConfig>


using MainnetDAA          = typename DAASelector<MainnetConfig>::Calculator;
using MainnetDAAEstimator = typename DAASelector<MainnetConfig>::Estimator;

using GoerliDAA           = typename DAASelector<GoerliConfig>::Calculator;
using GoerliDAAEstimator  = typename DAASelector<GoerliConfig>::Estimator;


} // namespace Eth
} // namespace EclipseMonitor
