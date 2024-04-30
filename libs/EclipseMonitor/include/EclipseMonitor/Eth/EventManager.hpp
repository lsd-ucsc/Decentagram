// Copyright (c) 2023 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "../Internal/SimpleObj.hpp"
#include "../Logging.hpp"

#include "DataTypes.hpp"
#include "EventDescription.hpp"
#include "Receipt.hpp"
#include "ReceiptsMgr.hpp"


namespace EclipseMonitor
{
namespace Eth
{


class EventManager
{
public: // static members:

	using EventDescrpMap =
		std::unordered_map<
			EventCallbackId,
			std::unique_ptr<EventDescription>
		>;
	using EventDescKIt =
		typename EventDescrpMap::const_iterator;

	using LogEntriesKRefType =
		typename ReceiptsMgr::LogEntriesKRefType;
	using CallbackPlan =
		std::pair<
			std::pair<
				EventCallbackId,
				typename EventDescription::NotifyCallbackType
			>,
			std::vector<LogEntriesKRefType>
		>;

public:

	EventManager() :
		m_eventDescMapMutex(),
		m_eventDescMap(),
		m_logger(LoggerFactory::GetLogger("EventManager"))
	{}

	~EventManager() = default;

	EventCallbackId Listen(EventDescription&& subDesc)
	{
		std::lock_guard<std::mutex> lock(m_eventDescMapMutex);

		std::unique_ptr<EventDescription> subDescPtr =
			Internal::Obj::Internal::make_unique<EventDescription>(
				std::move(subDesc)
			);
		EventCallbackId id =
			reinterpret_cast<EventCallbackId>(subDescPtr.get());
		m_eventDescMap.emplace(id, std::move(subDescPtr));

		return id;
	}

	void Cancel(EventCallbackId id)
	{
		std::lock_guard<std::mutex> lock(m_eventDescMapMutex);

		auto it = m_eventDescMap.find(id);
		if (it != m_eventDescMap.end())
		{
			m_eventDescMap.erase(it);
		}
	}

	size_t GetNumOfListeners() const
	{
		std::lock_guard<std::mutex> lock(m_eventDescMapMutex);

		return m_eventDescMap.size();
	}

	template<typename _ReceiptsMgrGetter>
	void CheckEvents(
		const HeaderMgr& headerMgr,
		_ReceiptsMgrGetter receiptsMgrGetter
	) const
	{
		std::vector<CallbackPlan> callbackPlans;

		// !!NOTE!!: Must make sure the receiptsMgr is still alive when we make
		// callbacks, since the callback plan has references to the
		// data owned by the receiptsMgr.
		std::unique_ptr<ReceiptsMgr> receiptsMgr;

		{
			std::lock_guard<std::mutex> lock(m_eventDescMapMutex);

			// find if any subscription is found via the bloom filter.
			auto bloomedEvents =
				BloomEventDesc_Locked(headerMgr.GetBloomFilter());

			// nothing found in bloom filter;
			// By the nature of bloom filter, there is no false negative.
			// Thus, stop here
			if (bloomedEvents.empty())
			{
				return;
			}


			m_logger.Debug(
				"Found " + std::to_string(bloomedEvents.size()) +
				" positives in bloom filter at block #" +
				std::to_string(headerMgr.GetNumber())
			);


			// otherwise, check the receipt root, and check the receipt logs
			// we must verify the receipt root first, because we also want to
			// ensure if the event is not found in the receipt, it is really
			// not there.
			receiptsMgr =
				Internal::Obj::Internal::make_unique<ReceiptsMgr>(
					receiptsMgrGetter(headerMgr.GetNumber())
				);
			if (
				receiptsMgr->GetRootHashBytes() !=
				headerMgr.GetRawHeader().get_ReceiptsRoot()
			)
			{
				throw Exception("Receipts root mismatch");
			}


			// search through the receipt managers
			callbackPlans = GenCallbackPlan_Locked(
				*receiptsMgr,
				bloomedEvents,
				m_logger
			);
		}

		// Now we've finished searching through the receipt managers
		// and the subscription map is unlocked.

		ConductCallbackPlan(headerMgr, callbackPlans);
	}

private: // helper functions

	std::vector<EventDescKIt> BloomEventDesc_Locked(
		const BloomFilter& bloom
	) const
	{
		std::vector<EventDescKIt> bloomedEvents;

		for (
			auto it = m_eventDescMap.cbegin();
			it != m_eventDescMap.cend();
			++it
		)
		{
			if (
				bloom.AreHashesInBloom(
					it->second->m_hashes.cbegin(),
					it->second->m_hashes.cend()
				)
			)
			{
				bloomedEvents.emplace_back(it);
			}
		}

		return bloomedEvents;
	}

	static std::vector<CallbackPlan> GenCallbackPlan_Locked(
		const ReceiptsMgr& receiptsMgr,
		const std::vector<EventDescKIt>& bloomedEvents_locked,
		const Logger& logger
	)
	{
		std::vector<CallbackPlan> plans;

		for (const auto& bloomedEvent : bloomedEvents_locked)
		{
			auto logKRefs = receiptsMgr.SearchEvents(
				bloomedEvent->second->m_contractAddr,
				bloomedEvent->second->m_topics.cbegin(),
				bloomedEvent->second->m_topics.cend()
			);
			if (!logKRefs.empty())
			{
				logger.Debug(
					"Found " + std::to_string(logKRefs.size()) +
					" events in current receipt"
				);
				plans.emplace_back(
					std::make_pair(
						bloomedEvent->first,
						bloomedEvent->second->m_notifyCallback
					),
					std::move(logKRefs)
				);
			}
		}

		return plans;
	}

	static void ConductCallbackPlan(
		const HeaderMgr& hdrMgr,
		const std::vector<CallbackPlan>& plans
	)
	{
		for (const auto& plan : plans)
		{
			const auto& id = plan.first.first;
			const auto& callback = plan.first.second;
			for (const auto& logKRef : plan.second)
			{
				callback(hdrMgr, logKRef, id);
			}
		}
	}

private:

	mutable std::mutex  m_eventDescMapMutex;
	EventDescrpMap      m_eventDescMap;
	Logger              m_logger;
}; // class EventManager


} // namespace Eth
} // namespace EclipseMonitor
