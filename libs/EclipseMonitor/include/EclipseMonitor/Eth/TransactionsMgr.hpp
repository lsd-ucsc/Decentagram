// Copyright (c) 2023 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <vector>

#include "../Internal/SimpleObj.hpp"
#include "../Internal/SimpleRlp.hpp"
#include "Transaction.hpp"
#include "Trie/Trie.hpp"


namespace EclipseMonitor
{
namespace Eth
{


class TransactionsMgr
{
public: // static members


	using TransactionListType = std::vector<Transaction>;


public:

	TransactionsMgr(const Internal::Obj::ListBaseObj& transactions) :
		m_transactions(),
		m_rootHashBytes()
	{
		using _IntWriter = Internal::Rlp::EncodePrimitiveIntValue<
			uint64_t,
			Internal::Rlp::Endian::native,
			false
		>;
		using _KeyRlpWriter =
			Internal::Rlp::WriterBytesImpl<std::vector<uint8_t> >;


		m_transactions.reserve(transactions.size());

		Trie::PatriciaTrie trie;
		size_t i = 0;
		Internal::Obj::Bytes keyBigEndian;
		keyBigEndian.reserve(8); // size_t usually is at most 8 bytes

		for (const auto& transaction : transactions)
		{
			const auto& transactionBytes = transaction.AsBytes();

			// 1. trie
			keyBigEndian.resize(0);
			_IntWriter::Encode(i, std::back_inserter(keyBigEndian));
			std::vector<uint8_t> keyRlp = _KeyRlpWriter::Write(keyBigEndian);
			trie.Put(keyRlp, transactionBytes);

			// 2. transactions
			m_transactions.emplace_back(
				Transaction::FromBytes(transactionBytes)
			);

			++i;
		}

		m_rootHashBytes = trie.Hash();
	}


	const Internal::Obj::Bytes& GetRootHashBytes() const
	{
		return m_rootHashBytes;
	}


private:

	TransactionListType m_transactions;
	Internal::Obj::Bytes m_rootHashBytes;

}; // class TransactionsMgr


} // namespace Eth
} // namespace EclipseMonitor
