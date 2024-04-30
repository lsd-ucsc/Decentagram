// Copyright (c) 2023 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <DecentEnclave/Common/Logging.hpp>

#include <EclipseMonitor/Eth/AbiParser.hpp>
#include <EclipseMonitor/Eth/DataTypes.hpp>
#include <EclipseMonitor/Eth/EventManager.hpp>
#include <EclipseMonitor/Eth/Keccak256.hpp>

#include <SimpleObjects/SimpleObjects.hpp>
#include <SimpleObjects/Codec/Hex.hpp>


namespace EthereumClt
{
namespace Trusted
{
namespace Pubsub
{


/**
 * @brief The data structure that holds the event message and
 *        associated metadata; its structure should be
 *        1. SimpleObjects::Bytes - The block number when the event is emitted
 *        2. SimpleObjects::Bytes - The event message
 *
 */
using EventData  = SimpleObjects::ListT<SimpleObjects::Bytes>;


/**
 * @brief The data structure that holds a sequence of event data;
 *        It could be used to
 *        - store the past events
 *        - the events that are waiting to be pushed to the subscribers
 *
 */
using EventDataQueue = SimpleObjects::ListT<EventData>;


/**
 * @brief The data structure shared between the
 *        - Registered handler in EventManager (Block processing thread)
 *        - Heartbeat message emitter (Heartbeat thread)
 *
 */
struct ThreadedEventQueue
{
	EventDataQueue  m_eventQueue;
	std::mutex      m_mutex;
}; // struct ThreadedEventQueue


class SubscriberService
{
public: // static member:

	/**
	 * @brief Smart contract address of the publisher
	 *
	 */
	using PublisherId    = SimpleObjects::Bytes;

	/**
	 * @brief Smart contract address of the event manager
	 *
	 */
	using EventMgrId     = SimpleObjects::Bytes;

	/**
	 * @brief The map used to loop up the event manager address using
	 *        the publisher address
	 *
	 */
	using EventMgrIdMap  = std::unordered_map<
		PublisherId,
		EclipseMonitor::Eth::ContractAddr
	>;

	/**
	 * @brief The map used to store all past events
	 *
	 */
	using PastEventStore = std::unordered_map<
		EventMgrId,
		std::shared_ptr<ThreadedEventQueue>
	>;

	struct PubsubServiceStore
	{
		PubsubServiceStore(
			const EclipseMonitor::Eth::ContractAddr& pubsubContAddr,
			const std::string& deployEvSign,
			const std::string& regEvSign,
			const std::string& notifyEvSign
		) :
			m_logger(
				DecentEnclave::Common::LoggerFactory::
				GetLogger("PubsubServiceStore")
			),
			m_pubsubContAddr(pubsubContAddr),
			m_deployEvTopic(EclipseMonitor::Eth::Keccak256(deployEvSign)),
			m_regEvTopic(EclipseMonitor::Eth::Keccak256(regEvSign)),
			m_notifyEvTopic(EclipseMonitor::Eth::Keccak256(notifyEvSign)),
			m_isDeployed(false),
			m_evMgrAddrMapMutex(),
			m_evMgrAddrMap()
		{}

		DecentEnclave::Common::Logger m_logger;
		EclipseMonitor::Eth::ContractAddr m_pubsubContAddr;
		EclipseMonitor::Eth::EventTopic m_deployEvTopic;
		EclipseMonitor::Eth::EventTopic m_regEvTopic;
		EclipseMonitor::Eth::EventTopic m_notifyEvTopic;
		std::atomic_bool m_isDeployed;
		std::mutex       m_evMgrAddrMapMutex;
		EventMgrIdMap    m_evMgrAddrMap;
		std::mutex       m_pastEventStoreMutex;
		PastEventStore   m_pastEventStore;
	}; // struct PubsubServiceStore

public:
	SubscriberService(
		const EclipseMonitor::Eth::ContractAddr& pubsubContAddr,
		const std::string& deployEvSign,
		const std::string& regEvSign,
		const std::string& notifyEvSign
	) :
		m_svcStore(std::make_shared<PubsubServiceStore>(
			pubsubContAddr,
			deployEvSign,
			regEvSign,
			notifyEvSign
		))
	{}

	void Start(
		std::shared_ptr<EclipseMonitor::Eth::EventManager> eventMgrPtr
	)
	{
		eventMgrPtr->Listen(BuildDeployEventDescr(
			eventMgrPtr,
			m_svcStore
		));
	}

	EclipseMonitor::Eth::ContractAddr GetEventMgrAddr(
		const PublisherId& publisherAddr
	) const
	{
		std::lock_guard<std::mutex> lock(m_svcStore->m_evMgrAddrMapMutex);
		auto it = m_svcStore->m_evMgrAddrMap.find(publisherAddr);
		if (it == m_svcStore->m_evMgrAddrMap.end())
		{
			return EclipseMonitor::Eth::ContractAddr();
		}
		return it->second;
	}

	EclipseMonitor::Eth::EventTopic GetNotifyEventTopic() const
	{
		return m_svcStore->m_notifyEvTopic;
	}

	EventDataQueue GetPastEvents(
		const EclipseMonitor::Eth::ContractAddr& evMgrAddr
	) const
	{
		EventMgrId evMgrId(
			evMgrAddr.begin(),
			evMgrAddr.end()
		);
		EventDataQueue pastEvents;

		std::lock_guard<std::mutex> lock(m_svcStore->m_pastEventStoreMutex);
		auto it = m_svcStore->m_pastEventStore.find(evMgrId);
		if (it != m_svcStore->m_pastEventStore.end())
		{
			pastEvents = it->second->m_eventQueue;
		}
		return pastEvents;
	}

private: // helper functions:

	//===== Notify Event (past store) =====

	static void NotifyPastEventHandler(
		PubsubServiceStore& svcStore,
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

		// 2. Save event to pass event store
		std::shared_ptr<ThreadedEventQueue> evQueue;
		{
			EventMgrId evMgrId(
				log.m_contractAddr.begin(),
				log.m_contractAddr.end()
			);
			std::lock_guard<std::mutex> lock(svcStore.m_pastEventStoreMutex);
			auto it = svcStore.m_pastEventStore.find(evMgrId);
			if (it != svcStore.m_pastEventStore.end())
			{
				evQueue = it->second;
			}
			else
			{
				svcStore.m_logger.Error(
					"Past event store not found for given event manager address"
				);
				return;
			}
		}
		{
			std::lock_guard<std::mutex> lock(evQueue->m_mutex);
			evQueue->m_eventQueue.push_back(
				EventData({
					EventData::value_type(headerMgr.GetRawHeader().get_Number()),
					EventData::value_type(evMsg),
				})
			);
		}

		// 3. Debug message
		svcStore.m_logger.Debug(
			std::string("Event Manager @") +
			SimpleObjects::Codec::Hex::Encode<std::string>(log.m_contractAddr) +
			" emit an event at block #" +
			std::to_string(headerMgr.GetNumber())
		);
	}

	static EclipseMonitor::Eth::EventDescription
	BuildNotifyPastEventDescr(
		const EclipseMonitor::Eth::ContractAddr& evMgrContAddr,
		std::shared_ptr<PubsubServiceStore> svcStore
	)
	{
		EclipseMonitor::Eth::EventDescription eventDesc(
			evMgrContAddr,
			std::vector<EclipseMonitor::Eth::EventTopic>({
				svcStore->m_notifyEvTopic,
			}),
			[svcStore](
				const EclipseMonitor::Eth::HeaderMgr& headerMgr,
				const EclipseMonitor::Eth::ReceiptLogEntry& log,
				EclipseMonitor::Eth::EventCallbackId
			) -> void
			{
				NotifyPastEventHandler(*svcStore, headerMgr, log);
			}
		);

		return eventDesc;
	}

	// ===== Registration Event =====

	static void RegEventHandler(
		std::weak_ptr<EclipseMonitor::Eth::EventManager> weakEventMgr,
		std::shared_ptr<PubsubServiceStore> svcStore,
		const EclipseMonitor::Eth::HeaderMgr& headerMgr,
		const EclipseMonitor::Eth::ReceiptLogEntry& log
	)
	{
		// 1. Extract contract addresses from the receipt log
		std::vector<uint8_t> pubAddrAbi;
		std::vector<uint8_t> evMgrAddrAbi;

		using _AddrParser =
			EclipseMonitor::Eth::AbiParser<
				SimpleObjects::ObjCategory::Bytes,
				EclipseMonitor::Eth::AbiSize<32>
			>;
		auto abiBegin = log.m_logData.begin();
		auto abiEnd = log.m_logData.end();

		std::tie(pubAddrAbi, abiBegin) =
			_AddrParser().ToPrimitive(abiBegin, abiEnd, abiBegin);
		std::tie(evMgrAddrAbi, abiBegin) =
			_AddrParser().ToPrimitive(abiBegin, abiEnd, abiBegin);

		EclipseMonitor::Eth::ContractAddr pubAddr;
		std::copy(pubAddrAbi.begin() + 12, pubAddrAbi.end(), pubAddr.begin());
		EclipseMonitor::Eth::ContractAddr evMgrAddr;
		std::copy(evMgrAddrAbi.begin() + 12, evMgrAddrAbi.end(), evMgrAddr.begin());

		PublisherId pubAddrBytes(pubAddr.begin(), pubAddr.end());
		EventMgrId  evMgrAddrBytes(evMgrAddr.begin(), evMgrAddr.end());

		// 2. Save the address mapping
		{

			std::lock_guard<std::mutex> lock(svcStore->m_evMgrAddrMapMutex);
			svcStore->m_evMgrAddrMap[pubAddrBytes] = evMgrAddr;
		}
		{
			std::lock_guard<std::mutex> lock(svcStore->m_pastEventStoreMutex);
			svcStore->m_pastEventStore.emplace(
				evMgrAddrBytes,
				std::make_shared<ThreadedEventQueue>()
			);
		}

		// 3. Register event listening for the event manager
		auto eventMgr = weakEventMgr.lock();
		if (eventMgr)
		{
			eventMgr->Listen(BuildNotifyPastEventDescr(
				evMgrAddr,
				svcStore
			));
		}

		// 4. Log when a new publisher has been added
		auto pubAddrStr =
			SimpleObjects::Codec::Hex::Encode<std::string>(pubAddr);
		auto evMgrAddrStr =
			SimpleObjects::Codec::Hex::Encode<std::string>(evMgrAddr);
		svcStore->m_logger.Info(
			"Publisher @" + pubAddrStr + " registered with event manager @" +
			evMgrAddrStr + " at block #" +
			std::to_string(headerMgr.GetNumber())
		);
	}

	static EclipseMonitor::Eth::EventDescription
	BuildRegEventDescr(
		std::weak_ptr<EclipseMonitor::Eth::EventManager> weakEventMgr,
		std::shared_ptr<PubsubServiceStore> svcStore
	)
	{
		EclipseMonitor::Eth::EventDescription eventDesc(
			svcStore->m_pubsubContAddr,
			std::vector<EclipseMonitor::Eth::EventTopic>({
				svcStore->m_regEvTopic,
			}),
			[weakEventMgr, svcStore](
				const EclipseMonitor::Eth::HeaderMgr& headerMgr,
				const EclipseMonitor::Eth::ReceiptLogEntry& log,
				EclipseMonitor::Eth::EventCallbackId
			) -> void
			{
				RegEventHandler(weakEventMgr, svcStore, headerMgr, log);
			}
		);

		return eventDesc;
	}

	//===== Deployment Event =====

	static void DeployEventHandler(
		std::weak_ptr<EclipseMonitor::Eth::EventManager> weakEventMgr,
		std::shared_ptr<PubsubServiceStore> svcStore,
		const EclipseMonitor::Eth::HeaderMgr& headerMgr,
		EclipseMonitor::Eth::EventCallbackId cbID
	)
	{
		// 1. Mark the service as deployed
		svcStore->m_isDeployed.store(true);

		// 2. log when the Pub-Sub service contract has been deployed
		svcStore->m_logger.Info(
			"Pub-Sub service contract is deployed at block #" +
				std::to_string(headerMgr.GetNumber())
		);

		auto eventMgr = weakEventMgr.lock();
		if (eventMgr)
		{
			// 3. stop listening to this event since it is already deployed
			eventMgr->Cancel(cbID);

			// 4. now we can start listening to registration events
			eventMgr->Listen(BuildRegEventDescr(
				weakEventMgr,
				svcStore
			));
		}
	}

	static EclipseMonitor::Eth::EventDescription
	BuildDeployEventDescr(
		std::weak_ptr<EclipseMonitor::Eth::EventManager> weakEventMgr,
		std::shared_ptr<PubsubServiceStore> svcStore
	)
	{
		EclipseMonitor::Eth::EventDescription eventDesc(
			svcStore->m_pubsubContAddr,
			std::vector<EclipseMonitor::Eth::EventTopic>({
				svcStore->m_deployEvTopic,
				AddressToAbiAddress(
					svcStore->m_pubsubContAddr
				),
			}),
			[weakEventMgr, svcStore](
				const EclipseMonitor::Eth::HeaderMgr& headerMgr,
				const EclipseMonitor::Eth::ReceiptLogEntry&,
				EclipseMonitor::Eth::EventCallbackId cbID
			) -> void
			{
				DeployEventHandler(weakEventMgr, svcStore, headerMgr, cbID);
			}
		);

		return eventDesc;
	}

	static std::array<uint8_t, 32> AddressToAbiAddress(
		const EclipseMonitor::Eth::ContractAddr& addr
	)
	{
		static_assert(
			std::tuple_size<EclipseMonitor::Eth::ContractAddr>::value + 12 ==
			std::tuple_size<std::array<uint8_t, 32> >::value,
			"Address size mismatch"
		);

		std::array<uint8_t, 32> ret;
		std::fill(ret.begin(), ret.begin() + 12, 0);
		std::copy(addr.begin(), addr.end(), ret.begin() + 12);
		return ret;
	}

private:
	std::shared_ptr<PubsubServiceStore> m_svcStore;

}; // class SubscriberService


} // namespace Pubsub
} // namespace Trusted
} // namespace EthereumClt
