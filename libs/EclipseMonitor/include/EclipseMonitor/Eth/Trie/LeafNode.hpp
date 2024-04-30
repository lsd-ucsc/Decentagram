// Copyright 2022 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <SimpleObjects/Internal/make_unique.hpp>

#include "Nibbles.hpp"
#include "TrieNode.hpp"

namespace EclipseMonitor
{
namespace Eth
{
namespace Trie
{

class LeafNode : public NodeBase
{
public: // static members:

	static std::unique_ptr<LeafNode> NewLeafNodeFromNibbles(
		const std::vector<Nibble>& nibbles,
		const Internal::Obj::BytesBaseObj& value
	)
	{
		return Internal::Obj::Internal::make_unique<LeafNode>(nibbles, value);
	}

	static std::unique_ptr<LeafNode> NewLeafNodeFromBytes(
		const std::vector <uint8_t>& key,
		const Internal::Obj::BytesBaseObj& value
	)
	{
		std::vector<Nibble> nibbles = NibbleHelper::FromBytes(key);
		return NewLeafNodeFromNibbles(nibbles, value);
	}

public:

	LeafNode(
		const std::vector<Nibble>& otherPath,
		Internal::Obj::Bytes otherValue
	) :
		m_path(otherPath),
		m_value(std::move(otherValue))
	{}

	LeafNode(
		const std::vector<Nibble>& otherPath,
		const Internal::Obj::BytesBaseObj& otherValue
	) :
		LeafNode(
			otherPath,
			Internal::Obj::Bytes(
				otherValue.data(),
				otherValue.data() + otherValue.size()
			)
		)
	{}

	// LCOV_EXCL_START
	virtual ~LeafNode() = default;
	// LCOV_EXCL_STOP

	virtual NodeType GetNodeType() const override
	{
		return NodeType::Leaf;
	}

	virtual Internal::Obj::List Raw() const override
	{
		Internal::Obj::Bytes pathBytes(
			NibbleHelper::ToBytes(
				NibbleHelper::ToPrefixed(m_path, true)
			)
		);

		Internal::Obj::List raw;
		raw.reserve(2);
		raw.push_back(pathBytes);
		raw.push_back(m_value);
		return raw;
	}

	const std::vector<Nibble>& GetPath() const
	{
		return m_path;
	}

	const Internal::Obj::Bytes& GetValue() const
	{
		return m_value;
	}

private:

	std::vector<Nibble> m_path;
	Internal::Obj::Bytes m_value;

}; // class LeafNode

} // namespace Trie
} // namespace Eth
} // namespace EclipseMonitor
