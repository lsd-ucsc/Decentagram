// Copyright (c) 2022 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <cstdint>

#include <array>
#include <atomic>
#include <memory>

#include "Config.hpp"
#include "DataTypes.hpp"
#include "Exceptions.hpp"
#include "Logging.hpp"
#include "MonitorReport.hpp"
#include "PlatformInterfaces.hpp"

namespace EclipseMonitor
{

class SyncState
{
public: // static members

	/**
	 * @brief The fixed nonce for development use
	 *        0x9566c74d10037c4d7bbb0407d1e2c64981855ad8681d0d86d1e91e00167939cb
	 *        NOTE: only use it for development purpose
	 *
	 */
	static constexpr SyncNonce GetDevNonce()
	{
		return SyncNonce({
			0X95U, 0X66U, 0XC7U, 0X4DU, 0X10U, 0X03U, 0X7CU, 0X4DU,
			0X7BU, 0XBBU, 0X04U, 0X07U, 0XD1U, 0XE2U, 0XC6U, 0X49U,
			0X81U, 0X85U, 0X5AU, 0XD8U, 0X68U, 0X1DU, 0X0DU, 0X86U,
			0XD1U, 0XE9U, 0X1EU, 0X00U, 0X16U, 0X79U, 0X39U, 0XCBU,
		});
	};

	/**
	 * @brief Get the Sync State for development use containing a fixed nonce
	 *        NOTE: only use it for development purpose
	 *
	 * @return SyncState
	 */
	static SyncState GetDevSyncState()
	{
		return SyncState(
			std::numeric_limits<TrustedTimestamp>::max(),
			TrustedTimestamp(),
			GetDevNonce(),
			true
		);
	}

public:

#ifdef ECLIPSEMONITOR_DEV_USE_DEV_SYNC_NONCE
	SyncState(
		TrustedTimestamp maxWaitTime,
		const TimestamperBase&,
		const RandomGeneratorBase&
	) :
		m_maxWaitTime(maxWaitTime),
		m_genTime(),
		m_nonce(GetDevNonce()),
		m_isSynced(false),
		m_logger(LoggerFactory::GetLogger("SyncState"))
	{}
#else // ECLIPSEMONITOR_DEV_USE_DEV_SYNC_NONCE
	SyncState(
		TrustedTimestamp maxWaitTime,
		const TimestamperBase& timestamper,
		const RandomGeneratorBase& randGen
	) :
		m_maxWaitTime(maxWaitTime),
		m_genTime(),
		m_nonce(),
		m_isSynced(false),
		m_logger(LoggerFactory::GetLogger("SyncState"))
	{
		// generate a timestamp for this nonce
		m_genTime = timestamper.NowInSec();
		// generate a random nonce
		randGen.GenerateRandomBytes(m_nonce.data(), m_nonce.size());
	}
#endif // ECLIPSEMONITOR_DEV_USE_DEV_SYNC_NONCE

	SyncState(SyncState&& other) :
		m_maxWaitTime(std::move(other.m_maxWaitTime)),
		m_genTime(std::move(other.m_genTime)),
		m_nonce(std::move(other.m_nonce)),
		m_isSynced(other.m_isSynced.load()),
		m_logger(std::move(other.m_logger))
	{}

	~SyncState() = default;

	void SetSynced(TrustedTimestamp recvTime)
	{
		TrustedTimestamp deltaT = recvTime - m_genTime;
		// ensure the sync msg is received within the max wait time
		if (deltaT <= m_maxWaitTime)
		{
			m_isSynced.store(true);
			m_logger.Info(
				"Synced after " + std::to_string(deltaT) +
				" ; @ " + std::to_string(recvTime)
			);
		}
	}

	bool IsSynced() const
	{
		return m_isSynced.load();
	}

	const SyncNonce& GetNonce() const
	{
		return m_nonce;
	}

private:

	SyncState(
		TrustedTimestamp maxWaitTime,
		TrustedTimestamp genTime,
		SyncNonce nonce,
		bool isSynced
	) :
		m_maxWaitTime(maxWaitTime),
		m_genTime(genTime),
		m_nonce(nonce),
		m_isSynced(isSynced),
		m_logger(LoggerFactory::GetLogger("SyncState"))
	{}

	const TrustedTimestamp m_maxWaitTime;
	TrustedTimestamp m_genTime;
	SyncNonce m_nonce;
	std::atomic_bool m_isSynced;
	Logger           m_logger;
}; // class SyncState

class SyncMsgMgrBase
{
public: // static members:

#if defined(__cpp_lib_atomic_shared_ptr) && __cpp_lib_atomic_shared_ptr != 0
	using AtomicSyncStateType = std::atomic<std::shared_ptr<SyncState> >;
#else // defined(__cpp_lib_atomic_shared_ptr) && __cpp_lib_atomic_shared_ptr != 0
	using AtomicSyncStateType = std::shared_ptr<SyncState>;
#endif // defined(__cpp_lib_atomic_shared_ptr) && __cpp_lib_atomic_shared_ptr != 0

public:

	/**
	 * @brief Construct a new sync message manager object
	 *
	 * @param mId    reference to the monitor ID
	 * @param mConf  Configuration of the monitor
	 * @param timestamper reference to the timestamper
	 * @param randGen     reference to the random generator
	 */
	SyncMsgMgrBase(
		const MonitorId& mId,
		const MonitorConfig& mConf,
		const TimestamperBase& timestamper,
		const RandomGeneratorBase& randGen
	) :
		m_maxWaitTime(mConf.get_syncMaxWaitTime().GetVal()),
		m_sessId(GetSessIdFromVec(mId.get_sessionID().GetVal())),
		m_lastSyncState(BuildSyncState(timestamper, randGen))
	{}

	// LCOV_EXCL_START
	virtual ~SyncMsgMgrBase() = default;
	// LCOV_EXCL_STOP

	std::shared_ptr<SyncState> GetLastSyncState() const
	{
		return AtomicGetSyncState();
	}

	const SessionID& GetSessionID() const
	{
		return m_sessId;
	}

	virtual std::shared_ptr<SyncState> NewSyncState(
		const TimestamperBase& timestamper,
		const RandomGeneratorBase& randGen
	) = 0;

protected:

	std::shared_ptr<SyncState> BuildSyncState(
		const TimestamperBase& timestamper,
		const RandomGeneratorBase& randGen
	)
	{
		return std::make_shared<SyncState>(
			m_maxWaitTime,
			timestamper,
			randGen
		);
	}

	std::shared_ptr<SyncState> AtomicGetSyncState() const
	{
#if defined(__cpp_lib_atomic_shared_ptr) && __cpp_lib_atomic_shared_ptr != 0
		return m_lastSyncState.load();
#else // defined(__cpp_lib_atomic_shared_ptr) && __cpp_lib_atomic_shared_ptr != 0
		return std::atomic_load(&m_lastSyncState);
#endif // defined(__cpp_lib_atomic_shared_ptr) && __cpp_lib_atomic_shared_ptr != 0
	}

	void AtomicSetSyncState(std::shared_ptr<SyncState> syncState)
	{
#if defined(__cpp_lib_atomic_shared_ptr) && __cpp_lib_atomic_shared_ptr != 0
		m_lastSyncState.store(syncState);
#else // defined(__cpp_lib_atomic_shared_ptr) && __cpp_lib_atomic_shared_ptr != 0
		std::atomic_store(&m_lastSyncState, syncState);
#endif // defined(__cpp_lib_atomic_shared_ptr) && __cpp_lib_atomic_shared_ptr != 0
	}

private: // helper functions:

	static SessionID GetSessIdFromVec(
		const std::vector<uint8_t>& sessIdVec
	)
	{
		if (sessIdVec.size() != std::tuple_size<SessionID>::value)
		{
			throw Exception("Invalid session ID size");
		}
		SessionID sessId;
		std::copy(
			sessIdVec.begin(),
			sessIdVec.end(),
			sessId.begin()
		);
		return sessId;
	}

private:

	const TrustedTimestamp m_maxWaitTime;
	const SessionID m_sessId;
	AtomicSyncStateType m_lastSyncState;
}; // class SyncMsgMgrBase

} // namespace EclipseMonitor
