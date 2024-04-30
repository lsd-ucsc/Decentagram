// Copyright (c) 2023 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <memory>

#include <DecentEnclave/Common/Logging.hpp>
#include <EclipseMonitor/Eth/DiffChecker.hpp>
#include <EclipseMonitor/Eth/EclipseMonitor.hpp>
#include <EclipseMonitor/Eth/Keccak256.hpp>
#include <EclipseMonitor/Eth/Validator.hpp>
#include <SimpleObjects/Codec/Hex.hpp>
#include <SimpleObjects/Internal/make_unique.hpp>

#include "HostBlockService.hpp"
#include "Pubsub/SubscriberService.hpp"
#include "RandomGenerator.hpp"
#include "Timestamper.hpp"


namespace EthereumClt
{
namespace Trusted
{


template<typename _NetConfig>
class BlockchainMgr
{
public: // static members:

	using NetConfig = _NetConfig;

	using DAAType    =
		typename EclipseMonitor::Eth::DAASelector<NetConfig>::Calculator;
	using DAAEstType =
		typename EclipseMonitor::Eth::DAASelector<NetConfig>::Estimator;

	using ValidatorType   =
		EclipseMonitor::Eth::Validator<NetConfig>;
	using DiffCheckerType =
		EclipseMonitor::Eth::GenericDiffCheckerImpl<NetConfig>;

	using EclipseMonitorType = EclipseMonitor::Eth::EclipseMonitor;


	static std::unique_ptr<ValidatorType> MakeValidator()
	{
		return
			SimpleObjects::Internal::make_unique<ValidatorType>(
				SimpleObjects::Internal::make_unique<DAAType>()
			);
	}

	static std::unique_ptr<DiffCheckerType> MakeDiffChecker(
		const EclipseMonitor::MonitorConfig& mConfig
	)
	{
		return SimpleObjects::Internal::make_unique<DiffCheckerType>(
			mConfig,
			SimpleObjects::Internal::make_unique<DAAEstType>()
		);
	}

public:

	BlockchainMgr(
		const EclipseMonitor::MonitorConfig& mConfig,
		uint64_t startBlockNum,
		const EclipseMonitor::Eth::ContractAddr& syncContractAddr,
		const std::string& syncEventSign,
		std::unique_ptr<Pubsub::SubscriberService> subSvc,
		std::unique_ptr<HostBlockService> hostBlkSvc
	) :
		m_logger(
			DecentEnclave::Common::LoggerFactory::GetLogger("BlockchainMgr")
		),
		m_monitorConfig(mConfig),
		m_monitorMutex(),
		m_monitor(
			SimpleObjects::Internal::make_unique<EclipseMonitorType>(
				m_monitorConfig,
				SimpleObjects::Internal::make_unique<Timestamper>(),
				SimpleObjects::Internal::make_unique<RandomGenerator>(),
				[this](const EclipseMonitor::Eth::HeaderMgr& hdr) {
					this->OnHeaderValidated(hdr);
				},
				[this](const EclipseMonitor::Eth::HeaderMgr& hdr) {
					this->OnHeaderConfirmed(hdr);
				},
				MakeValidator(),
				MakeDiffChecker(m_monitorConfig),
				syncContractAddr,
				EclipseMonitor::Eth::Keccak256(syncEventSign)
			)
		),
		m_lastChkptIter(0),
		m_subSvc(std::move(subSvc)),
		m_hostBlkSvc(std::move(hostBlkSvc)),
		m_lastValidatedBlkNum()
	{
		const auto latestBlkNum = m_hostBlkSvc->GetLatestBlockNum();
		m_monitor->RefreshBootstrapPlan(latestBlkNum, &startBlockNum);

		m_subSvc->Start(m_monitor->GetEventManager());
	}


	~BlockchainMgr() = default;


	void AppendBlock(const std::vector<uint8_t>& headerRlp)
	{
		std::lock_guard<std::mutex> lock(m_monitorMutex);
		m_monitor->Update(headerRlp);
	}

	const Pubsub::SubscriberService& GetSubscriberService() const
	{
		return *m_subSvc;
	}

	EclipseMonitor::MonitorSecState GetMonitorSecState() const
	{
		std::lock_guard<std::mutex> lock(m_monitorMutex);
		const auto& monitor = *m_monitor;
		return monitor.GetMonitorSecState();
	}

	SimpleObjects::Bytes GetLastValidatedBlkNum() const
	{
		std::lock_guard<std::mutex> lock(m_monitorMutex);
		return m_lastValidatedBlkNum;
	}

	const EclipseMonitor::Eth::EventManager& GetEventManager() const
	{
		return *m_monitor->GetEventManager();
	}

	EclipseMonitor::Eth::EventManager& GetEventManager()
	{
		return *m_monitor->GetEventManager();
	}


private:

	void OnHeaderValidated(const EclipseMonitor::Eth::HeaderMgr& hdr)
	{
		m_lastValidatedBlkNum = hdr.GetRawHeader().get_Number();

		auto receiptsMgrGetter =
			[this](EclipseMonitor::Eth::BlockNumber blkNum)
				-> EclipseMonitor::Eth::ReceiptsMgr
			{
				return EclipseMonitor::Eth::ReceiptsMgr(
					m_hostBlkSvc->GetReceiptsRlpByNum(blkNum).AsList()
				);
			};

		m_monitor->GetEventManager()->CheckEvents(
			hdr,
			receiptsMgrGetter
		);

		const auto phase = m_monitor->GetPhase();
		switch (phase)
		{
		case EclipseMonitor::Phases::BootstrapI:
			if (hdr.GetNumber() == m_monitor->GetBootstrapIEndBlkNum())
			{
				// Refresh bootstrap plan first
				// We may have a new bootstrap plan at this point
				const auto latestBlkNum = m_hostBlkSvc->GetLatestBlockNum();
				m_monitor->RefreshBootstrapPlan(latestBlkNum);
			}
			break;

		case EclipseMonitor::Phases::BootstrapII:
			// we need to refresh bootstrap plan on every block
			{
				const auto latestBlkNum = m_hostBlkSvc->GetLatestBlockNum();
				m_monitor->RefreshBootstrapPlan(latestBlkNum);
			}
			break;

		default:
			break;
		}
	}

	void OnHeaderConfirmed(const EclipseMonitor::Eth::HeaderMgr& hdr)
	{
		(void)hdr;

		const auto& monitor = *m_monitor;
		const auto& secState = monitor.GetMonitorSecState();
		const auto chkptIter = secState.get_checkpointIter().GetVal();

		bool logStatus = false;

		if (m_lastChkptIter != chkptIter)
		{
			// We have entered the next checkpoint iteration
			m_lastChkptIter = chkptIter;

			const auto phase = m_monitor->GetPhase();
			switch (phase)
			{
			// during bootstrap I, log status every 3 checkpoint iterations
			case EclipseMonitor::Phases::BootstrapI:
				logStatus = (chkptIter % 3 == 0);
				break;
			// In other cases, log status at every checkpoint
			default:
				logStatus = true;
				break;
			}
		}

		if (logStatus)
		{
			LogMonitorStatus();
		}
	}

	void LogMonitorStatus() const
	{
		const auto phase = m_monitor->GetPhase();
		std::string phaseStr =
			phase == EclipseMonitor::Phases::BootstrapI ?  "BootstrapII" :
			phase == EclipseMonitor::Phases::BootstrapII ? "BootstrapII" :
			phase == EclipseMonitor::Phases::Sync ?        "Sync" :
			"Runtime";

		const auto& monitor = *m_monitor;
		const auto& secState = monitor.GetMonitorSecState();
		const std::string genesisHash =
			SimpleObjects::Codec::Hex::Encode<std::string>(
				secState.get_genesisHash().GetVal()
			);
		const std::string chkptHash =
			SimpleObjects::Codec::Hex::Encode<std::string>(
				secState.get_checkpointHash().GetVal()
			);
		const auto chkptIter = secState.get_checkpointIter().GetVal();

		m_logger.Info(
			std::string("Current Eclipse Monitor Status:\n") +
			"\tPhase:                " + phaseStr    + ";\n" +
			"\tGenesis Hash:         " + genesisHash + ";\n" +
			"\tCheckpoint Hash:      " + chkptHash   + ";\n" +
			"\tCheckpoint Iteration: " + std::to_string(chkptIter) + ";\n"
		);
	}

private:

	DecentEnclave::Common::Logger m_logger;
	EclipseMonitor::MonitorConfig m_monitorConfig;
	mutable std::mutex m_monitorMutex;
	std::unique_ptr<EclipseMonitorType> m_monitor;
	uint64_t m_lastChkptIter;
	std::unique_ptr<Pubsub::SubscriberService> m_subSvc;
	std::unique_ptr<HostBlockService> m_hostBlkSvc;
	SimpleObjects::Bytes m_lastValidatedBlkNum;
};


} // namespace Trusted
} // namespace EthereumClt
