// Copyright 2022 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include "Nibbles.hpp"
#include "TrieNode.hpp"


namespace EclipseMonitor
{
namespace Eth
{
namespace Trie
{

class ExtensionNode : public NodeBase
{
public:

	ExtensionNode(
		std::vector<Nibble>&& otherPath,
		std::unique_ptr<Node> next
	) :
		m_path(std::move(otherPath)),
		m_next(std::move(next))
	{}

	// LCOV_EXCL_START
	virtual ~ExtensionNode() = default;
	// LCOV_EXCL_STOP

	virtual NodeType GetNodeType() const override
	{
		return NodeType::Extension;
	}

	virtual Internal::Obj::List Raw() const override
	{
		Internal::Obj::List hashes;
		hashes.resize(2);

		std::vector<Nibble> prefixedPath =
			NibbleHelper::ToPrefixed(m_path, false);
		std::vector<uint8_t> pathBytes = NibbleHelper::ToBytes(prefixedPath);
		Internal::Obj::Bytes pathBytesObject(std::move(pathBytes));
		hashes[0] = pathBytesObject;

		const auto& nextBase = m_next->GetNodeBase();
		auto nextBaseRaw = nextBase.Raw();
		size_t serializedSize =
			NodeBase::CalcSerializedSize(nextBaseRaw);

		if (serializedSize >= 32)
		{
			hashes[1] = NodeBase::CalcHash(
				NodeBase::GenSerialized(nextBaseRaw)
			);
		}
		else
		{
			hashes[1] = nextBaseRaw;
		}

		return hashes;
	}

	std::vector<Nibble>& GetPath()
	{
		return m_path;
	}

	std::unique_ptr<Node>& GetNext()
	{
		return m_next;
	}

private:

	std::vector<Nibble> m_path;
	std::unique_ptr<Node> m_next;

}; // class ExtensionNode

} // namespace Trie
} // namespace Eth
} // namespace EclipseMonitor
