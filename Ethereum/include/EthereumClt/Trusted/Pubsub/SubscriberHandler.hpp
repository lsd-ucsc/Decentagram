// Copyright (c) 2023 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <AdvancedRlp/AdvancedRlp.hpp>

#include <DecentEnclave/Common/Logging.hpp>
#include <DecentEnclave/Trusted/DecentLambdaSvr.hpp>
#include <DecentEnclave/Trusted/HeartbeatEmitterMgr.hpp>

#include <SimpleObjects/Codec/Hex.hpp>

#include "../BlockchainMgr.hpp"
#include "../DataType.hpp"
#include "SubscriberService.hpp"


namespace EthereumClt
{
namespace Trusted
{

namespace Pubsub
{


inline void NotifyEventHandler(
	ThreadedEventQueue& evQueue,
	DecentEnclave::Common::Logger& logger,
	const EclipseMonitor::Eth::HeaderMgr& headerMgr,
	const EclipseMonitor::Eth::ReceiptLogEntry& log
)
{
	// 1. Extract event message from log data
	std::vector<uint8_t> evMsg;

	using _MsgParser =
		EclipseMonitor::Eth::AbiParser<
			SimpleObjects::ObjCategory::Bytes,
			std::true_type
		>;
	auto abiBegin = log.m_logData.begin();
	auto abiEnd = log.m_logData.end();

	std::tie(evMsg, abiBegin) =
		_MsgParser().ToPrimitive(abiBegin, abiEnd, abiBegin);

	// 2. Save event to event queue
	{
		std::lock_guard<std::mutex> lock(evQueue.m_mutex);
		evQueue.m_eventQueue.push_back(
			EventData({
				EventData::value_type(headerMgr.GetRawHeader().get_Number()),
				EventData::value_type(evMsg),
			})
		);
	}

	// 3. Debug message
	logger.Debug(
		"Emit an event at block #" + std::to_string(headerMgr.GetNumber())
	);
}


inline EclipseMonitor::Eth::EventDescription
BuildNotifyEventDescr(
	const EclipseMonitor::Eth::ContractAddr& evMgrContAddr,
	EclipseMonitor::Eth::EventTopic notifyEvTopic,
	std::shared_ptr<ThreadedEventQueue> eventQueue
)
{
	std::shared_ptr<DecentEnclave::Common::Logger> logger =
		std::make_shared<DecentEnclave::Common::Logger>(
			DecentEnclave::Common::LoggerFactory::GetLogger(
				std::string("Event Manager @") +
					SimpleObjects::Codec::Hex::
						Encode<std::string>(evMgrContAddr)
			)
		);

	EclipseMonitor::Eth::EventDescription eventDesc(
		evMgrContAddr,
		std::vector<EclipseMonitor::Eth::EventTopic>({
			notifyEvTopic,
		}),
		[eventQueue, logger](
			const EclipseMonitor::Eth::HeaderMgr& headerMgr,
			const EclipseMonitor::Eth::ReceiptLogEntry& log,
			EclipseMonitor::Eth::EventCallbackId
		) -> void
		{
			NotifyEventHandler(*eventQueue, *logger, headerMgr, log);
		}
	);

	return eventDesc;
}


inline std::vector<uint8_t> BuildEmittedMsg(
	SimpleObjects::Bytes&& secState,
	SimpleObjects::Bytes&& latestBlkNum,
	EventDataQueue&& evQueue
)
{
	static const SimpleObjects::String sk_labelSecState("SecState");
	static const SimpleObjects::String sk_labelLatestBlkNum("LatestBlkNum");
	static const SimpleObjects::String sk_labelEvents("Events");

	SimpleObjects::Dict respDict;
	respDict[sk_labelSecState] = std::move(secState);
	respDict[sk_labelLatestBlkNum] = std::move(latestBlkNum);
	respDict[sk_labelEvents] = std::move(evQueue);
	std::vector<uint8_t> respMsg =
		AdvancedRlp::GenericWriter::Write(respDict);

	return respMsg;
}


template<typename _NetConfig>
inline void EmitterHandler(
	LambdaMsgSocket& socket,
	ThreadedEventQueue& evQueue,
	BlockchainMgr<_NetConfig>& bcMgr,
	EclipseMonitor::Eth::EventCallbackId listenId
)
{
	try
	{
		EventDataQueue outEvQueue;
		{
			std::lock_guard<std::mutex> lock(evQueue.m_mutex);
			outEvQueue = std::move(evQueue.m_eventQueue);
			evQueue.m_eventQueue = EventDataQueue();
		}

		std::vector<uint8_t> respMsg = BuildEmittedMsg(
			SimpleObjects::Bytes(AdvancedRlp::GenericWriter::Write(
				bcMgr.GetMonitorSecState()
			)),
			bcMgr.GetLastValidatedBlkNum(),
			std::move(outEvQueue)
		);

		socket.SizedSendBytes(respMsg);
	}
	catch(const std::exception&)
	{
		bcMgr.GetEventManager().Cancel(listenId);
		throw;
	}
}


template<typename _NetConfig>
inline void SubReq(
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
		LoggerFactory::GetLogger("EthereumClt::Trusted::PubSub::SubReq");
	static const SimpleObjects::String sk_labelPublisher("publisher");

	// 1. lookup for the on-chain event manager address
	auto msgContent = AdvancedRlp::Parse(msgContentAdvRlp);
	const auto& msgContentDict = msgContent.AsDict();
	const auto& pubAddrObjBase = msgContentDict[sk_labelPublisher].AsBytes();
	SimpleObjects::Bytes pubAddr(pubAddrObjBase.begin(), pubAddrObjBase.end());

	auto eventMgrAddr =
		bcMgrPtr->GetSubscriberService().GetEventMgrAddr(pubAddr);

	if (eventMgrAddr == EclipseMonitor::Eth::ContractAddr())
	{
		s_logger.Error(
			"Failed to find the event manager for publisher @" +
				SimpleObjects::Codec::Hex::Encode<std::string>(pubAddr)
		);
		return;
	}

	// 2. subscribe to event manager first
	s_logger.Debug("Subscribing to event manager @" +
		SimpleObjects::Codec::Hex::Encode<std::string>(eventMgrAddr)
	);
	std::shared_ptr<ThreadedEventQueue> eventQueue =
		std::make_shared<ThreadedEventQueue>();
	auto listenId = bcMgrPtr->GetEventManager().Listen(
		BuildNotifyEventDescr(
			eventMgrAddr,
			bcMgrPtr->GetSubscriberService().GetNotifyEventTopic(),
			eventQueue
		)
	);

	// 3. respond with the current state
	std::vector<uint8_t> respMsg = BuildEmittedMsg(
		SimpleObjects::Bytes(AdvancedRlp::GenericWriter::Write(
			bcMgrPtr->GetMonitorSecState()
		)),
		bcMgrPtr->GetLastValidatedBlkNum(),
		bcMgrPtr->GetSubscriberService().GetPastEvents(eventMgrAddr)
	);
	socket->SizedSendBytes(respMsg);

	// 4. set up heartbeat emitter
	std::shared_ptr<LambdaMsgSocket> ownedSocket = std::move(socket);

	HeartbeatEmitterMgr::GetInstance().AddEmitter(
		[ownedSocket, eventQueue, bcMgrPtr, listenId]()
		{
			EmitterHandler(*ownedSocket, *eventQueue, *bcMgrPtr, listenId);
		}
	);

	s_logger.Debug("Received a subscribe request");
}

} // namespace Pubsub
} // namespace Trusted
} // namespace EthereumClt
