// Copyright (c) 2022 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include <SimpleObjects/Codec/Hex.hpp>

#include "../EclipseMonitorBase.hpp"
#include "../Internal/SimpleObj.hpp"

#include "CheckpointMgr.hpp"
#include "DiffChecker.hpp"
#include "EventManager.hpp"
#include "HeaderMgr.hpp"
#include "SyncMsgMgr.hpp"
#include "Validator.hpp"

namespace EclipseMonitor
{
namespace Eth
{


class EclipseMonitor : public EclipseMonitorBase
{
public: // Static members

	using Self = EclipseMonitor;
	using Base = EclipseMonitorBase;

	using OnHeaderConfCallback = std::function<void(const HeaderMgr&)>;
	using NodeLookUpMap =
		std::unordered_map<Internal::Obj::Bytes, HeaderNode*>;

public:

	EclipseMonitor(
		const MonitorConfig& conf,
		TimestamperType timestamper,
		RandomGeneratorType randGen,
		OnHeaderConfCallback onHeaderValidated,
		OnHeaderConfCallback onHeaderConfirmed,
		std::unique_ptr<ValidatorBase> validator,
		std::unique_ptr<DiffCheckerBase> diffChecker,
		const ContractAddr& syncContractAddr,
		const EventTopic& syncEventSign
	) :
		EclipseMonitorBase(conf, std::move(timestamper), std::move(randGen)),

		m_onHeaderValidated(onHeaderValidated),
		m_onHeaderConfirmed(onHeaderConfirmed),

		m_checkpoint(conf, [this](){
			this->OnCheckpointComplete();
		}),
		m_validator(std::move(validator)),
		m_diffChecker(std::move(diffChecker)),

		m_eventManager(std::make_shared<EventManager>()),
		m_syncMsgMgr(
			Base::GetMonitorId(),
			Base::GetMonitorConfig(),
			Base::GetTimestamper(),
			Base::GetRandomGenerator(),
			syncContractAddr,
			syncEventSign,
			m_eventManager
		),

		m_offlineNodes(),
		m_activeNodes(),

		m_startBlockNum(0),
		m_bootstrapIEndBlkNum(-1),
		m_planedSyncBlkNum(-1)
	{}

	virtual ~EclipseMonitor()
	{}

	virtual void Update(const std::vector<uint8_t>& hdrBinary) override
	{
		BlockNumber blkNum = 0;
		// 1. check current phase
		if (Base::GetPhase() == Phases::BootstrapI)
		{
			blkNum = UpdateOnBootstrapI(hdrBinary);
		}
		// all other phase will be treated like the runtime phase
		else
		{
			blkNum = UpdateOnRuntime(hdrBinary);
		}

		PhaseChangeCheck(blkNum);

		if (Base::GetPhase() != Phases::BootstrapI)
		{
			RuntimeMaintenance();
		}
	}

	virtual void EndBootstrapI() override
	{
		auto syncState = m_syncMsgMgr.GetLastSyncState();

		// 1. notify checkpoint manager so it will create the dummy node
		//    for the last header
		m_checkpoint.EndBootstrapPhase(syncState);

		// 2. update active nodes so we will use it as the starting
		//    point to add the following children
		auto lastNodePtr = m_checkpoint.GetLastNodePtr();
		const auto& lastHeader = lastNodePtr->GetHeader();
		m_offlineNodes[lastHeader.GetHashObj()] = lastNodePtr;

		// 3. notify the base class that we're entering the next phase
		Base::EndBootstrapI();
	}

	std::shared_ptr<EventManager> GetEventManager() const
	{
		return m_eventManager;
	}

	const SyncMsgMgr& GetSyncMsgMgr() const
	{
		return m_syncMsgMgr;
	}

	std::shared_ptr<SyncState> RefreshSyncMsg()
	{
		Base::EndBootstrapII();
		return m_syncMsgMgr.NewSyncState(
			Base::GetTimestamper(),
			Base::GetRandomGenerator()
		);
	}

	void RefreshBootstrapPlan(
		const BlockNumber& latestBlkNum,
		const BlockNumber* startBlkNum = nullptr
	)
	{
		if (startBlkNum != nullptr)
		{
			m_startBlockNum = *startBlkNum;
		}
		bool logPlan = false;

		const auto chkptSize =
			Base::GetMonitorConfig().get_checkpointSize().GetVal();

		switch (Base::GetPhase())
		{
		case Phases::BootstrapI:
			// we need to determine when to end bootstrap I phase
			m_bootstrapIEndBlkNum = CalcBootstrapIEndBlkNum(
				latestBlkNum,
				m_startBlockNum,
				chkptSize
			);
			m_planedSyncBlkNum = latestBlkNum;
			logPlan = true;
			break;

		case Phases::BootstrapII:
			if (m_planedSyncBlkNum != latestBlkNum)
			{
				logPlan = true;
			}
			m_planedSyncBlkNum = latestBlkNum;
			break;

		default:
			break;
		}

		if (logPlan)
		{
			Base::GetLogger().Info(
				std::string("Refresh Bootstrap Plan:\n") +
				"\tStart  Block#    " + std::to_string(m_startBlockNum)       + ";\n" +
				"\tLatest Block#    " + std::to_string(latestBlkNum)          + ";\n" +
				"\tChkpt Size       " + std::to_string(chkptSize)             + ";\n" +
				"\tBootI Phase End# " + std::to_string(m_bootstrapIEndBlkNum) + ";\n" +
				"\tPlan Sync Block# " + std::to_string(m_planedSyncBlkNum) + ";\n"
			);
		}
	}

	const BlockNumber& GetStartBlockNum() const
	{
		return m_startBlockNum;
	}

	const BlockNumber& GetBootstrapIEndBlkNum() const
	{
		return m_bootstrapIEndBlkNum;
	}

	const BlockNumber& GetPlanedSyncBlkNum() const
	{
		return m_planedSyncBlkNum;
	}

protected:

	BlockNumber UpdateOnBootstrapI(const std::vector<uint8_t>& hdrBinary)
	{
		// We're loading blocks before the latest checkpoint

		std::unique_ptr<HeaderMgr> header =
			Internal::Obj::Internal::make_unique<HeaderMgr>(
				hdrBinary, 0);
		BlockNumber blkNum = header->GetNumber();

		// 1 check if this is the genesis (very first) block
		if (m_checkpoint.IsEmpty())
		{
			// a. it is the genesis block
			// 1.a. update the monitor security state
			Base::GetMonitorSecState().get_genesisHash() = header->GetHashObj();

			using namespace Internal::Obj::Codec;
			const auto hashStr = Hex::Encode<std::string>(header->GetHash());
			Base::GetLogger().Info(
				"Genesis block #" + std::to_string(blkNum) + "; Hash: " + hashStr
			);
		}
		else
		{
			// b. it is not the genesis block
			// 1.b validate the block
			if (!m_validator->CommonValidate(
					m_checkpoint.GetLastHeader(),
					false,
					*header,
					false
				)
			)
			{
				throw Exception(
					"The given block failed common validation");
			}
		}

		// Callback for validated headers
		m_onHeaderValidated(*header);

		// Add the header to the checkpoint
		m_checkpoint.AddHeader(std::move(header));
		// !!! NOTE: header is invalid after this point !!!

		return blkNum;
	}

	BlockNumber UpdateOnRuntime(const std::vector<uint8_t>& hdrBinary)
	{
		std::unique_ptr<HeaderMgr> header =
			Internal::Obj::Internal::make_unique<HeaderMgr>(
				hdrBinary,
				Base::GetTimestamper().NowInSec()
			);
		BlockNumber blkNum = header->GetNumber();

		// Check offline nodes map first
		if (m_offlineNodes.size() > 0)
		{
			auto offNoIt = m_offlineNodes.find(
				header->GetRawHeader().get_ParentHash()
			);
			if (offNoIt != m_offlineNodes.end())
			{
				// we found the parent node
				HeaderNode* parentNode = offNoIt->second;
				UpdateOnRuntimeAddChild(
					parentNode,
					false,
					std::move(header)
				);
				// !!! NOTE: header is invalid after this point !!!
			}
		}

		// if we didn't find the parent node in the offline nodes map,
		// check the active nodes map then
		if (header != nullptr)
		{
			auto actNoIt = m_activeNodes.find(
				header->GetRawHeader().get_ParentHash()
			);
			if (actNoIt != m_activeNodes.end())
			{
				// we found the parent node
				HeaderNode* parentNode = actNoIt->second;
				UpdateOnRuntimeAddChild(
					parentNode,
					true,
					std::move(header)
				);
				// !!! NOTE: header is invalid after this point !!!
			}
		}

		if (header != nullptr)
		{
			using namespace Internal::Obj::Codec;
			std::string blkHashHex = Hex::Encode<std::string>(header->GetHash());
			Base::GetLogger().Error(
				"Cannot find the parent of block #" + std::to_string(blkNum) +
				"; hash: " + blkHashHex
			);
		}

		return blkNum;
	}

	void RuntimeMaintenance()
	{
		// 1. check for new checkpoint candidates
		HeaderNode* lastChptNode = m_checkpoint.GetLastNodePtr();
		auto confirmedChild = lastChptNode->ReleaseChildHasNDesc(
			static_cast<size_t>(
				Base::GetMonitorConfig().get_checkpointSize().GetVal()
			)
		);
		if (confirmedChild != nullptr)
		{
			// we found a new checkpoint candidate

			// both last node and confirmed child are not active anymore
			if (m_offlineNodes.size() > 0)
			{
				m_offlineNodes.erase(lastChptNode->GetHeader().GetHashObj());
				m_offlineNodes.erase(confirmedChild->GetHeader().GetHashObj());
			}
			m_activeNodes.erase(lastChptNode->GetHeader().GetHashObj());
			m_activeNodes.erase(confirmedChild->GetHeader().GetHashObj());

			// add to checkpoint
			m_checkpoint.AddNode(std::move(confirmedChild));
		}

		// 2. check for expired active nodes
		auto now = Base::GetTimestamper().NowInSec();
		for (auto nodePtr : m_activeNodes)
		{
			if (
				!m_diffChecker->CheckEstDifficulty(
					nodePtr.second->GetHeader(),
					now
				)
			)
			{
				// the node is expired
				m_activeNodes.erase(nodePtr.first);
			}
		}
	}

	void PhaseChangeCheck(const BlockNumber& currBlkNum)
	{
		switch (Base::GetPhase())
		{
		case Phases::BootstrapI:
			if (currBlkNum == m_bootstrapIEndBlkNum)
			{
				EndBootstrapI();
			}
			break;
		case Phases::BootstrapII:
			if (currBlkNum == m_planedSyncBlkNum)
			{
				RefreshSyncMsg();
			}
			break;
		default:
			break;
		}
	}

private:

	void UpdateOnRuntimeAddChild(
		HeaderNode* parentNode,
		bool isParentNodeLive,
		std::unique_ptr<HeaderMgr> header
	)
	{
		auto syncState = m_syncMsgMgr.GetLastSyncState();

		// common validation
		bool isNewNodeLive = syncState->IsSynced();
		bool validateRes = m_validator->CommonValidate(
			parentNode->GetHeader(),
			isParentNodeLive,
			*header,
			isNewNodeLive
		);

		// check difficulty
		bool diffRes = false;
		if (validateRes)
		{
			diffRes = m_diffChecker->CheckDifficulty(
				parentNode->GetHeader(),
				*header
			);
		}

		// if both check passed, add it to the parent node
		if (validateRes && diffRes)
		{
			// Callback for validated headers
			m_onHeaderValidated(*header);

			auto hashObj = header->GetHashObj();

			// add the header to the parent node
			HeaderNode* node =
				parentNode->AddChild(std::move(header), std::move(syncState));
			// !!! NOTE: header is invalid after this point !!!
			// !!! NOTE: syncState is invalid after this point !!!

			// add this node also to the active nodes
			if (isNewNodeLive)
			{
				m_activeNodes.insert(std::make_pair(hashObj, node));
			}
			else
			{
				m_offlineNodes.insert(std::make_pair(hashObj, node));
			}
		}
		else
		{
			using namespace Internal::Obj::Codec;
			BlockNumber blkNum = header->GetNumber();
			std::string blkHashHex = Hex::Encode<std::string>(header->GetHash());
			Base::GetLogger().Error(
				"Validation failed on block #" + std::to_string(blkNum) +
				"; hash: " + blkHashHex
			);
		}
	}

	void OnCheckpointComplete()
	{
		// 1. update the monitor security state about latest checkpoint
		const auto& lastHeader = m_checkpoint.GetLastHeader();
		Base::GetMonitorSecState().get_checkpointHash() =
			lastHeader.GetHashObj();
		Base::GetMonitorSecState().get_checkpointNum() =
			lastHeader.GetRawHeader().get_Number();

		// 2. Increment the checkpoint iterations
		Base::GetMonitorSecState().get_checkpointIter()++;

		// 3. update the difficulty checker
		m_diffChecker->OnChkptUpd(m_checkpoint);

		// on confirmed header callback
		size_t i = 0;
		BlockNumber startBlock = 0;
		BlockNumber endBlock = 0;
		m_checkpoint.IterateCurrWindow(
			[this, &i, &startBlock, &endBlock](const HeaderMgr& header)
			{
				if (i == 0){
					startBlock = header.GetNumber();
				}
				endBlock = header.GetNumber();
				m_onHeaderConfirmed(header);
				++i;
			}
		);
		Base::GetLogger().Debug(
			std::string("Confirmed blocks from: ") +
				"block #" + std::to_string(startBlock) +
				" to block #" + std::to_string(endBlock) +
				" total: " + std::to_string(i) + " blocks"
		);
	}

	/**
	 * @brief `startBlk` and `chkptSize` should be constant, so the plan for
	 *        when to end bootstrap I phase should solely depend on `latestBlk`
	 */
	static uint64_t CalcBootstrapIEndBlkNum(
		uint64_t latestBlk,
		uint64_t startBlk,
		uint64_t chkptSize
	)
	{
		uint64_t numOfBlocks = latestBlk - startBlk + 1;
		uint64_t numOfInterval = numOfBlocks / chkptSize;
		numOfInterval = (numOfInterval > 2) ? numOfInterval - 2 : 0;
		uint64_t endBlkNum = startBlk + (numOfInterval * chkptSize);

		return endBlkNum - 1;
	}

private:

	OnHeaderConfCallback m_onHeaderValidated;
	OnHeaderConfCallback m_onHeaderConfirmed;

	CheckpointMgr m_checkpoint;
	std::unique_ptr<ValidatorBase> m_validator;
	std::unique_ptr<DiffCheckerBase> m_diffChecker;

	std::shared_ptr<EventManager> m_eventManager;
	SyncMsgMgr m_syncMsgMgr;

	// runtime & forks
	// TODO sync manager
	NodeLookUpMap m_offlineNodes;
	NodeLookUpMap m_activeNodes;

	BlockNumber m_startBlockNum;
	BlockNumber m_bootstrapIEndBlkNum;
	BlockNumber m_planedSyncBlkNum;


}; // class EclipseMonitor


} // namespace Eth
} // namespace EclipseMonitor
