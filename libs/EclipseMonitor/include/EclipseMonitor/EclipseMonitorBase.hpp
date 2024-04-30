// Copyright (c) 2022 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <cstdint>

#include <memory>
#include <string>
#include <vector>

#include "Config.hpp"
#include "DataTypes.hpp"
#include "Logging.hpp"
#include "MonitorReport.hpp"
#include "PlatformInterfaces.hpp"

namespace EclipseMonitor
{

enum class Phases
{
	BootstrapI,
	BootstrapII,
	Sync,
	Runtime,
}; // enum class Phases


class EclipseMonitorBase
{
public: // Static members:

	using TimestamperType = std::unique_ptr<TimestamperBase>;
	using RandomGeneratorType = std::unique_ptr<RandomGeneratorBase>;

public:

	EclipseMonitorBase(
		const MonitorConfig& conf,
		TimestamperType timestamper,
		RandomGeneratorType randGen
	) :
		m_mConfig(conf),
		m_mId(),
		m_mSecState(),
		m_phase(Phases::BootstrapI),
		m_timestamper(std::move(timestamper)),
		m_randGen(std::move(randGen)),
		m_logger(LoggerFactory::GetLogger("EclipseMonitorBase"))
	{
		m_mId.get_sessionID().resize(std::tuple_size<SessionID>::value);
#ifdef ECLIPSEMONITOR_DEV_USE_DEV_SESSION_ID
		// use development session ID
		// sessionID = 0x52fdfc072182654f163f5f0f9a621d72
		static constexpr SessionID sk_devSessId = {
			0X52U, 0XFDU, 0XFCU, 0X07U, 0X21U, 0X82U, 0X65U, 0X4FU,
			0X16U, 0X3FU, 0X5FU, 0X0FU, 0X9AU, 0X62U, 0X1DU, 0X72U,
		};
		std::copy(
			sk_devSessId.begin(),
			sk_devSessId.end(),
			m_mId.get_sessionID().begin()
		);
#else // ECLIPSEMONITOR_DEV_USE_DEV_SESSION_ID
		// generate a random session ID
		m_randGen->GenerateRandomBytes(
			&(m_mId.get_sessionID()[0]),
			m_mId.get_sessionID().size()
		);
#endif // ECLIPSEMONITOR_DEV_USE_DEV_SESSION_ID
	}

	virtual ~EclipseMonitorBase() = default;

	/**
	 * @brief Re-generate the sync nonce and the timestamp that marks
	 *        the beginning of the sync process
	 *        NOTE: a call to this function is needed at the beginning of EACH
	 *        sync process, even the first one, otherwise, the beginning
	 *        timestamp will be zero and the sync process will be failed
	 *        eventually.
	 *
	 */
	// virtual std::vector<uint8_t> ResetSyncNonce()
	// {
	// 	m_syncStartTime = m_timestamper.NowInSec();

	// 	// TODO: generate nonce
	// }

	const MonitorConfig& GetMonitorConfig() const
	{
		return m_mConfig;
	}

	const MonitorId& GetMonitorId() const
	{
		return m_mId;
	}

	const MonitorSecState& GetMonitorSecState() const
	{
		return m_mSecState;
	}

	const TimestamperBase& GetTimestamper() const
	{
		return *m_timestamper;
	}

	const RandomGeneratorBase& GetRandomGenerator() const
	{
		return *m_randGen;
	}

	virtual void Update(const std::vector<uint8_t>& hdrBinary) = 0;

	virtual void EndBootstrapI()
	{
		m_phase = Phases::BootstrapII;
		m_logger.Debug("Bootstrap I phase ended");
	}

	virtual void EndBootstrapII()
	{
		m_phase = Phases::Sync;
		m_logger.Debug("Bootstrap II phase ended");
	}

	virtual void EndSync()
	{
		m_phase = Phases::Runtime;
		m_logger.Debug("Entering runtime phase");
	}

	Phases GetPhase() const
	{
		return m_phase;
	}

protected:

	TimestamperBase& GetTimestamper()
	{
		return *m_timestamper;
	}

	RandomGeneratorBase& GetRandomGenerator()
	{
		return *m_randGen;
	}

	MonitorSecState& GetMonitorSecState()
	{
		return m_mSecState;
	}

	Logger& GetLogger()
	{
		return m_logger;
	}

	const Logger& GetLogger() const
	{
		return m_logger;
	}

private:

	MonitorConfig       m_mConfig;
	MonitorId           m_mId;
	MonitorSecState     m_mSecState;
	Phases              m_phase;
	TimestamperType     m_timestamper;
	RandomGeneratorType m_randGen;
	Logger              m_logger;

}; // class EclipseMonitorBase

} // namespace EclipseMonitor
