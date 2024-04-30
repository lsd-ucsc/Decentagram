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

class BranchNode : public NodeBase
{
public: // static members:

	static constexpr uint8_t sk_numNodes = 16;

public:

	BranchNode() :
		m_nodeHasValue(false),
		m_branches(sk_numNodes),
		m_value()
	{}

	// LCOV_EXCL_START
	virtual ~BranchNode() = default;
	// LCOV_EXCL_STOP

	bool HasValue()
	{
		return m_nodeHasValue;
	}

	void SetBranch(
		const Nibble& nibble,
		std::unique_ptr<Node> other
	)
	{
		m_branches[nibble] = std::move(other);
	}

	std::unique_ptr<Node>& GetBranch(Nibble nibble)
	{
		return m_branches[nibble];
	}

	void RemoveBranch(const Nibble& nibble)
	{
		m_branches[nibble].reset();
	}

	void SetValue(const Internal::Obj::BytesBaseObj& otherValue)
	{
		m_nodeHasValue = true;
		m_value = Internal::Obj::Bytes(
				otherValue.data(),
				otherValue.data() + otherValue.size()
			);
	}

	void RemoveValue()
	{
		m_nodeHasValue = false;
		m_value.resize(0);

	}

	virtual NodeType GetNodeType() const override
	{
		return NodeType::Branch;
	}

	virtual Internal::Obj::List Raw() const override
	{
		Internal::Obj::List hashes;
		hashes.resize(sk_numNodes + 1);

		for (uint8_t i = 0; i < sk_numNodes; i++)
		{
			if (!m_branches[i])
			{
				hashes[i] = EmptyNode::EmptyNodeRaw();
			}
			else
			{
				const auto& nodeBase = m_branches[i]->GetNodeBase();
				auto nodeBaseRaw = nodeBase.Raw();
				size_t serializedSize =
					NodeBase::CalcSerializedSize(nodeBaseRaw);

				if (serializedSize >= 32)
				{
					hashes[i] = NodeBase::CalcHash(
						NodeBase::GenSerialized(nodeBaseRaw)
					);
				}
				else
				{
					hashes[i] = nodeBaseRaw;
				}
			}
		}

		hashes[sk_numNodes] = m_value;
		return hashes;
	}

private:

	bool m_nodeHasValue;
	std::vector<std::unique_ptr<Node> > m_branches;
	Internal::Obj::Bytes m_value;

}; // class BranchNode

} // namespace Trie
} // namespace Eth
} // namespace EclipseMonitor
