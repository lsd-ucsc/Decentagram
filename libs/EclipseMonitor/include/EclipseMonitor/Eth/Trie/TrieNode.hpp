// Copyright 2022 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "../../Internal/SimpleObj.hpp"
#include "../../Internal/SimpleRlp.hpp"
#include "../Keccak256.hpp"

namespace EclipseMonitor
{
namespace Eth
{
namespace Trie
{

/**
 * @brief Enum for different node types
 *
 */
enum class NodeType
{
	Leaf,
	Branch,
	Extension,
	Null
}; // enum class NodeType


class NodeBase
{
public: // static members:

	using RawRetType = Internal::Obj::List;
	using HashRetType = Internal::Obj::Bytes;
	using SerializedRetType = std::vector<uint8_t>;

public:

	NodeBase() = default;

	// LCOV_EXCL_START
	virtual ~NodeBase() = default;
	// LCOV_EXCL_STOP

	virtual NodeType GetNodeType() const = 0;

	virtual RawRetType Raw() const = 0;

	SerializedRetType Serialize() const
	{
		return GenSerialized(Raw());
	}

	HashRetType Hash() const
	{
		return CalcHash(Serialize());
	}

protected:

	static SerializedRetType GenSerialized(const RawRetType& raw)
	{
		return Internal::Rlp::WriteRlp(raw);
	}

	static size_t CalcSerializedSize(const RawRetType& raw)
	{
		return Internal::Rlp::CalcRlpSize(raw);
	}

	static HashRetType CalcHash(const SerializedRetType& serialized)
	{
		std::array<uint8_t, 32> hashed = Keccak256(serialized);
		return Internal::Obj::Bytes(hashed.begin(), hashed.end());
	}

}; // class NodeBase


class Node
{
public:
	Node() :
		m_node(nullptr)
	{}

	Node(std::unique_ptr<NodeBase> nodeBase) :
		m_node(std::move(nodeBase))
	{}

	// LCOV_EXCL_START
	~Node() = default;
	// LCOV_EXCL_STOP

	void SetNode(std::unique_ptr<NodeBase> nodeBase)
	{
		m_node.reset();
		m_node = std::move(nodeBase);
	}

	NodeType GetNodeType() const
	{
		if (m_node)
		{
			return m_node->GetNodeType();
		}
		else
		{
			return NodeType::Null;
		}
	}

	std::unique_ptr<NodeBase>& GetNodeBasePtr()
	{
		return m_node;
	}

	const NodeBase& GetNodeBase() const
	{
		return *m_node;
	}

private:
	std::unique_ptr<NodeBase> m_node;

}; // class Node


struct EmptyNode
{
	static bool IsEmptyNode(const NodeBase* node)
	{
		return node == nullptr;
	}

	static const Internal::Obj::Bytes& EmptyNodeRaw()
	{
		static Internal::Obj::Bytes inst;
		return inst;
	}

	static Internal::Obj::Bytes EmptyNodeHash()
	{
		// https://github.com/ethereum/go-ethereum/blob/master/trie/trie.go#L32
		Internal::Obj::Bytes emptyTrieHash = {
			0x56, 0xe8, 0x1f, 0x17, 0x1b, 0xcc, 0x55, 0xa6,
			0xff, 0x83, 0x45, 0xe6, 0x92, 0xc0, 0xf8, 0x6e,
			0x5b, 0x48, 0xe0, 0x1b, 0x99, 0x6c, 0xad, 0xc0,
			0x01, 0x62, 0x2f, 0xb5, 0xe3, 0x63, 0xb4, 0x21
		};
		return emptyTrieHash;
	}

}; // struct EmptyNode

} // namespace Trie
} // namespace Eth
} // namespace EclipseMonitor
