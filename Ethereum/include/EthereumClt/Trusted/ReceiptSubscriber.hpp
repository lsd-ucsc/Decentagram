// Copyright (c) 2023 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <mutex>

#include <DecentEnclave/Common/Logging.hpp>
#include <DecentEnclave/Trusted/DecentLambdaSvr.hpp>
#include <DecentEnclave/Trusted/HeartbeatEmitterMgr.hpp>

#include <AdvancedRlp/AdvancedRlp.hpp>

#include "BlockchainMgr.hpp"
#include "DataType.hpp"


namespace EthereumClt
{
namespace Trusted
{


using ReceiptQueue = SimpleObjects::ListT<SimpleObjects::Object>;


struct ThreadedReceiptQueue
{
	ReceiptQueue  m_receiptQueue;
	std::mutex    m_mutex;
}; // struct ThreadedReceiptQueue


inline void SubscribedReceiptHandler(
	ThreadedReceiptQueue& recQueue,
	DecentEnclave::Common::Logger& logger,
	const EclipseMonitor::Eth::HeaderMgr& headerMgr,
	const EclipseMonitor::Eth::ReceiptLogEntry& log
)
{
	// 1. Copy log data
	SimpleObjects::List topics;
	topics.reserve(log.m_topics.size());
	for (const auto& topic : log.m_topics)
	{
		topics.push_back(SimpleObjects::Bytes(
			std::vector<uint8_t>(topic.begin(), topic.end())
		));
	}
	SimpleObjects::List list;
	list.push_back(SimpleObjects::Bytes(headerMgr.GetRawHeader().get_Number()));
	list.push_back(std::move(topics));
	list.push_back(SimpleObjects::Bytes(log.m_logData));

	// 2. Save event to event queue
	{
		std::lock_guard<std::mutex> lock(recQueue.m_mutex);
		recQueue.m_receiptQueue.push_back(std::move(list));
	}

	// 3. Debug message
	logger.Debug(
		"Emit an event at block #" + std::to_string(headerMgr.GetNumber())
	);
}


inline EclipseMonitor::Eth::EventDescription
BuildSubscribedEventDescr(
	const EclipseMonitor::Eth::ContractAddr& contAddr,
	const std::vector<EclipseMonitor::Eth::EventTopic>& notifyEvTopics,
	std::shared_ptr<ThreadedReceiptQueue> receiptQueue
)
{
	std::shared_ptr<DecentEnclave::Common::Logger> logger =
		std::make_shared<DecentEnclave::Common::Logger>(
			DecentEnclave::Common::LoggerFactory::GetLogger(
				std::string("Receipt from ") +
					SimpleObjects::Codec::Hex::
						Encode<std::string>(contAddr)
			)
		);

	EclipseMonitor::Eth::EventDescription eventDesc(
		contAddr,
		notifyEvTopics,
		[receiptQueue, logger](
			const EclipseMonitor::Eth::HeaderMgr& headerMgr,
			const EclipseMonitor::Eth::ReceiptLogEntry& log,
			EclipseMonitor::Eth::EventCallbackId
		) -> void
		{
			SubscribedReceiptHandler(*receiptQueue, *logger, headerMgr, log);
		}
	);

	return eventDesc;
}


template<typename _NetConfig>
inline void SubscribedReceiptEmitter(
	LambdaMsgSocket& socket,
	ThreadedReceiptQueue& recQueue,
	BlockchainMgr<_NetConfig>& bcMgr,
	EclipseMonitor::Eth::EventCallbackId listenId
)
{
	static const SimpleObjects::String sk_labelSecState("SecState");
	static const SimpleObjects::String sk_labelLatestBlkNum("LatestBlkNum");
	static const SimpleObjects::String sk_labelReceipts("Receipts");

	try
	{
		ReceiptQueue outQueue;
		{
			std::lock_guard<std::mutex> lock(recQueue.m_mutex);
			outQueue = std::move(recQueue.m_receiptQueue);
			recQueue.m_receiptQueue = ReceiptQueue();
		}

		auto secState = SimpleObjects::Bytes(AdvancedRlp::GenericWriter::Write(
			bcMgr.GetMonitorSecState()
		));
		auto latestBlkNum = bcMgr.GetLastValidatedBlkNum();

		SimpleObjects::Dict respDict;
		respDict[sk_labelSecState] = std::move(secState);
		respDict[sk_labelLatestBlkNum] = std::move(latestBlkNum);
		respDict[sk_labelReceipts] = std::move(outQueue);

		std::vector<uint8_t> respMsg =
			AdvancedRlp::GenericWriter::Write(respDict);

		socket.SizedSendBytes(respMsg);
	}
	catch(const std::exception&)
	{
		bcMgr.GetEventManager().Cancel(listenId);
		throw;
	}
}


template<typename _NetConfig>
inline void ReceiptSubReq(
	std::shared_ptr<BlockchainMgr<_NetConfig> > bcMgrPtr,
	LambdaMsgSocketPtr& socket,
	const LambdaMsgIdExt& msgIdExt,
	const LambdaMsgContent& msgContentAdvRlp
)
{
	using namespace DecentEnclave::Common;
	using namespace DecentEnclave::Trusted;

	(void)msgIdExt;

	static Logger s_logger =
		LoggerFactory::GetLogger("EthereumClt::Trusted::ReceiptSubReq");
	static const SimpleObjects::String sk_labelContract("contract");
	static const SimpleObjects::String sk_labelTopics("topics");

	// 1. get the address of the contract
	auto msgContent = AdvancedRlp::Parse(msgContentAdvRlp);
	const auto& msgContentDict = msgContent.AsDict();
	const auto& conAddrObjBase = msgContentDict[sk_labelContract].AsBytes();
	EclipseMonitor::Eth::ContractAddr conAddr;
	if (conAddrObjBase.size() != conAddr.size())
	{
		s_logger.Error("The length of the given contract address is invalid");
		return;
	}
	std::copy(conAddrObjBase.begin(), conAddrObjBase.end(), conAddr.begin());

	// 2. get the topics
	const auto& topicsObjBase = msgContentDict[sk_labelTopics].AsList();
	std::vector<EclipseMonitor::Eth::EventTopic> topics;
	topics.reserve(topicsObjBase.size());
	for (const auto& topicObjBase : topicsObjBase)
	{
		const auto& topicObj = topicObjBase.AsBytes();
		if (topicObj.size() != std::tuple_size<EclipseMonitor::Eth::EventTopic>::value)
		{
			s_logger.Error("The length of the given topic is invalid");
			return;
		}
		EclipseMonitor::Eth::EventTopic topic;
		std::copy(topicObj.begin(), topicObj.end(), topic.begin());
		topics.push_back(topic);
	}

	// 3. subscribe to receipt
	s_logger.Debug("Subscribing to receipts from contract @" +
		SimpleObjects::Codec::Hex::Encode<std::string>(conAddr)
	);
	std::shared_ptr<ThreadedReceiptQueue> receiptQueue =
		std::make_shared<ThreadedReceiptQueue>();
	auto listenId = bcMgrPtr->GetEventManager().Listen(
		BuildSubscribedEventDescr(
			conAddr,
			topics,
			receiptQueue
		)
	);

	// 4. set up heartbeat emitter
	std::shared_ptr<LambdaMsgSocket> ownedSocket = std::move(socket);

	HeartbeatEmitterMgr::GetInstance().AddEmitter(
		[ownedSocket, receiptQueue, bcMgrPtr, listenId]()
		{
			SubscribedReceiptEmitter(*ownedSocket, *receiptQueue, *bcMgrPtr, listenId);
		}
	);

	s_logger.Debug("Received a subscribe request");
}


} // namespace Trusted
} // namespace EthereumClt
