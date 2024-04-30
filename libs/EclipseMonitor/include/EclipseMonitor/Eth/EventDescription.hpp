// Copyright (c) 2023 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <array>
#include <functional>
#include <vector>

#include "DataTypes.hpp"
#include "HeaderMgr.hpp"
#include "Keccak256.hpp"
#include "Receipt.hpp"


namespace EclipseMonitor
{
namespace Eth
{



struct EventDescription
{
	using HashType = std::array<uint8_t, 32>;

	using NotifyCallbackType =
		std::function<void(
			const HeaderMgr&,
			const ReceiptLogEntry&,
			EventCallbackId
		)>;

	EventDescription(
		ContractAddr       contractAddr,
		std::vector<EventTopic> topics,
		NotifyCallbackType     notifyCallback
	) :
		m_contractAddr(std::move(contractAddr)),
		m_topics(std::move(topics)),
		m_hashes(),
		m_notifyCallback(std::move(notifyCallback))
	{
		m_hashes.reserve(1 + m_topics.size());
		m_hashes.emplace_back(Keccak256(m_contractAddr));
		for (const auto& topic : m_topics)
		{
			m_hashes.emplace_back(Keccak256(topic));
		}
	}

	EventDescription(EventDescription&& other) :
		m_contractAddr(std::move(other.m_contractAddr)),
		m_topics(std::move(other.m_topics)),
		m_hashes(std::move(other.m_hashes)),
		m_notifyCallback(std::move(other.m_notifyCallback))
	{}

	~EventDescription() = default;

	ContractAddr            m_contractAddr;
	std::vector<EventTopic> m_topics;
	std::vector<HashType>   m_hashes;
	NotifyCallbackType      m_notifyCallback;
}; // struct SubDescription



} // namespace Eth
} // namespace EclipseMonitor
