// Copyright (c) 2022 EclipseMonitor, EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <cstdint>

#include <vector>

#include "../Exceptions.hpp"
#include "../Internal/SimpleObj.hpp"
#include "../Internal/SimpleRlp.hpp"
#include "DataTypes.hpp"

namespace EclipseMonitor
{
namespace Eth
{

/*
 * Receipt parses an rlp-encoded Ethereum receipt, and can determine whether
 * an event has been emitted by a smart contract
 *
 * The structure of an Ethereum receipt can be found in the following link:
 * https://github.com/ethereum/go-ethereum/blob/master/core/types/receipt.go
 *
 * The rlp receipt data is expected to come from the Geth API call
 * "debug_getRawReceipts". This API call, however, does not encode the entire
 * receipt but only four fields, namely:
 * 	- 1. Status
 * 	- 2. Cumulative gas used
 * 	- 3. Bloom
 * 	- 4. Logs
 *
 * The logs of each receipt contains the data, if any, relating to events that
 * were emitted by a contract.
 *
 * The structure of a receipt log can be found here:
 * https://github.com/ethereum/go-ethereum/blob/master/core/types/log.go
 *
 * This log structure, too, is also not fully encoded to rlp. The rlp encoding
 * from the API call only contains the first three fields:
 *
 * 	- 1. Address
 * 	- 2. Topics
 * 	- 3. Data
 *
*/


struct ReceiptLogEntry
{

	ReceiptLogEntry(const Internal::Obj::ListBaseObj& logEntry) :
		m_contractAddr(),
		m_topics(),
		m_logData()
	{
		// Get contract address from entry
		const auto& addrBytes = logEntry[0].AsBytes();
		if (addrBytes.size() != m_contractAddr.size())
		{
			throw Exception(
				"The contract address found in log entry has "
				"invalid length"
			);
		}
		std::copy(
			addrBytes.data(),
			addrBytes.data() + addrBytes.size(),
			m_contractAddr.begin()
		);

		// Get log topics from entry
		const auto& logTopics = logEntry[1].AsList();
		m_topics.resize(logTopics.size());
		size_t currTopicIdx = 0;
		for (const auto& topic : logTopics)
		{
			const auto& topicBytes = topic.AsBytes();
			if (topicBytes.size() != m_topics[currTopicIdx].size())
			{
				throw Exception(
					"The topic found in log entry has invalid length"
				);
			}
			std::copy(
				topicBytes.data(),
				topicBytes.data() + topicBytes.size(),
				m_topics[currTopicIdx].begin()
			);
			++currTopicIdx;
		}

		// Get log data
		const auto& logDataBytes = logEntry[2].AsBytes();
		m_logData.assign(
			logDataBytes.data(),
			logDataBytes.data() + logDataBytes.size()
		);
	}

	~ReceiptLogEntry() = default;


	template<typename _TopicsIt>
	bool IsEventEmitted(
		const ContractAddr& addr,
		_TopicsIt inTpBegin,
		_TopicsIt inTpEnd
	) const
	{
		// Check contract address
		if (m_contractAddr != addr)
		{
			return false;
		}

		// Check topics
		// in case the desired event is emitted,
		// [inTpBegin, inTpEnd) should be a subset of m_topics
		auto tpIt = m_topics.begin();
		for (auto it = inTpBegin; it != inTpEnd; ++it, ++tpIt)
		{
			if (tpIt == m_topics.end())
			{
				// more topics in [inTpBegin, inTpEnd) than in m_topics
				return false;
			}
			else if (*it != *tpIt)
			{
				// topic mismatch
				return false;
			}
			// else, *it == *tpIt, continue
		}

		// everything in [inTpBegin, inTpEnd) is found
		// from m_topics.begin() to tpIt
		// In case inTpBegin == inTpEnd, caller doesn't care about topics
		// we also should return true

		return true;
	}

	ContractAddr m_contractAddr;
	std::vector<EventTopic> m_topics;
	std::vector<uint8_t> m_logData;
}; // struct ReceiptLogEntry


class Receipt
{
public: // static members:

	static Internal::Obj::Object ParseReceipt(
		const Internal::Obj::BytesBaseObj& rlpBytes
	)
	{
		uint8_t firstByte = rlpBytes[0];

		auto itBegin = rlpBytes.begin();
		auto itEnd = rlpBytes.end();
		size_t size = rlpBytes.size();
		if (firstByte == 0x01 || firstByte == 0x02 || firstByte == 0x03)
		{
			++itBegin;
			--size;
		}

		using _FrItType = typename Internal::Rlp::GeneralParser::IteratorType;
		return Internal::Rlp::GeneralParser().Parse(
			_FrItType(itBegin.CopyPtr()),
			_FrItType(itEnd.CopyPtr()),
			size
		);
	}

	static Receipt FromBytes(
		const Internal::Obj::BytesBaseObj& rlpBytes
	)
	{
		return Receipt(ParseReceipt(rlpBytes));
	}

	using LogEntriesType = std::vector<ReceiptLogEntry>;
	using LogEntriesKItType = typename LogEntriesType::const_iterator;
	using LogEntriesKRefType = std::reference_wrapper<const ReceiptLogEntry>;

public:

	explicit Receipt(Internal::Obj::Object receiptObj) :
		m_logEntries()
	{
		const auto& receiptBody = receiptObj.AsList();
		const auto& receiptLogs = receiptBody[3].AsList();
		for (const auto& logEntry : receiptLogs)
		{
			m_logEntries.emplace_back(logEntry.AsList());
		}
	};

	Receipt(Receipt&& other) :
		m_logEntries(std::move(other.m_logEntries))
	{}

	// LCOV_EXCL_START
	~Receipt() = default;
	// LCOV_EXCL_STOP

	template<typename _TopicsIt>
	std::vector<LogEntriesKRefType> SearchEvents(
		const ContractAddr& addr,
		_TopicsIt topicsBegin,
		_TopicsIt topicsEnd
	) const
	{
		std::vector<LogEntriesKRefType> res;

		for (const auto& logEntry: m_logEntries)
		{
			if (logEntry.IsEventEmitted(addr, topicsBegin, topicsEnd))
			{
				res.emplace_back(logEntry);
			}
		}

		return res;
	}

	const LogEntriesType& GetLogEntries() const
	{
		return m_logEntries;
	}

private:
	LogEntriesType m_logEntries;

}; // class Receipt


} // namespace Eth
} // namespace EclipseMonitor
