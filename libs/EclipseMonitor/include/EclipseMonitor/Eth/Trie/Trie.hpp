// Copyright 2022 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "../../Internal/SimpleObj.hpp"

#include "BranchNode.hpp"
#include "ExtensionNode.hpp"
#include "LeafNode.hpp"
#include "Nibbles.hpp"
#include "TrieNode.hpp"

namespace EclipseMonitor
{
namespace Eth
{
namespace Trie
{

/**
 * @brief This Trie implementation is based on a simplified version
 *        of Ethereum's Patricia Merkle Trie.
 *        source: https://github.com/zhangchiqing/merkle-patricia-trie
 *
 */
class PatriciaTrie
{
public:

	PatriciaTrie() :
		m_root()
	{}

	// LCOV_EXCL_START
	~PatriciaTrie() = default;
	// LCOV_EXCL_STOP

	void Reset()
	{
		m_root.reset();
	}

	/**
	 * @brief calculates the root hash of the trie.
	 *        For transactions and receipts, this function would produce
	 *        the "transactionsRoot" and "receiptsRoot", respectively.
	 *
	 * @return Hash of the root node, or empty hash if the trie is empty.
	 */
	Internal::Obj::Bytes Hash()
	{
		if (m_root == nullptr)
		{
			return EmptyNode::EmptyNodeHash();
		}
		return m_root->GetNodeBase().Hash();
	}

	void Put(
		const std::vector<uint8_t>& keyRlp,
		const Internal::Obj::BytesBaseObj& value
	)
	{
		std::vector<Nibble> nibbles = NibbleHelper::FromBytes(keyRlp);
		PutKey(m_root, nibbles, value);
	}

private:

	static void PutKeyEmptyNode(
		std::unique_ptr<Node>& node,
		const std::vector<Nibble>& nibbles,
		const Internal::Obj::BytesBaseObj& value
	)
	{
		std::unique_ptr<NodeBase> leafBase =
			LeafNode::NewLeafNodeFromNibbles(nibbles, value);
		std::unique_ptr<Node> leaf =
			Internal::Obj::Internal::make_unique<Node>(std::move(leafBase));
		node = std::move(leaf);
	}

	/**
	 * @brief leaf node, convert to Extension node, add new branch with new leaf
	 *
	 * @param node
	 * @param nibbles
	 * @param value
	 */
	static void PutKeyLeafNode(
		std::unique_ptr<Node>& node,
		const std::vector<Nibble>& nibbles,
		const Internal::Obj::BytesBaseObj& value
	)
	{
		const LeafNode* leaf =
			static_cast<const LeafNode*>(&(node->GetNodeBase()));

		const std::vector<Nibble>& leafPath = leaf->GetPath();

		uint8_t matched = NibbleHelper::PrefixMatchedLen(nibbles, leafPath);

		if (matched == nibbles.size() && matched == leafPath.size())
		{
			// replace leaf with new value
			auto newLeafBase =
				LeafNode::NewLeafNodeFromNibbles(leafPath, value);
			std::unique_ptr<Node> newLeaf =
				Internal::Obj::Internal::make_unique<Node>(
					std::move(newLeafBase)
				);
			node.reset();
			node = std::move(newLeaf);

			return;
		}

		auto branchBase =
			Internal::Obj::Internal::make_unique<BranchNode>();

		// set the branch value
		if (matched == leafPath.size())
		{
			branchBase->SetValue(leaf->GetValue());
		}

		if (matched == nibbles.size())
		{
			branchBase->SetValue(value);
		}

		// assign LeafNode to branch
		if (matched < leafPath.size())
		{
			Nibble branchNibble(leafPath[matched]);
			std::vector<Nibble> leafNibbles(
				leafPath.begin() + matched + 1,
				leafPath.end()
			);
			auto newLeafBase =
				LeafNode::NewLeafNodeFromNibbles(
					leafNibbles,
					leaf->GetValue()
				);
			std::unique_ptr<Node> newLeaf =
				Internal::Obj::Internal::make_unique<Node>(
					std::move(newLeafBase)
				);
			branchBase->SetBranch(branchNibble, std::move(newLeaf));
		}


		if (matched < nibbles.size())
		{
			Nibble branchNibble(nibbles[matched]);
			std::vector<Nibble> leafNibbles(
				nibbles.begin() + matched + 1,
				nibbles.end()
			);
			auto newLeafBase =
				LeafNode::NewLeafNodeFromNibbles(leafNibbles, value);
			std::unique_ptr<Node> newLeaf =
				Internal::Obj::Internal::make_unique<Node>(
					std::move(newLeafBase)
				);
			branchBase->SetBranch(branchNibble, std::move(newLeaf));
		}

		std::unique_ptr<Node> branch =
			Internal::Obj::Internal::make_unique<Node>(
				std::move(branchBase)
			);

		// if some Nibbles match, make branch part of an ExtensionNode
		if (matched > 0)
		{
			std::vector<Nibble> sharedNibbles(
				leafPath.begin(),
				leafPath.begin() + matched
			);
			std::unique_ptr<ExtensionNode> extensionBase =
				Internal::Obj::Internal::make_unique<ExtensionNode>(
					std::move(sharedNibbles),
					std::move(branch)
				);
			std::unique_ptr<Node> extension =
				Internal::Obj::Internal::make_unique<Node>(
					std::move(extensionBase)
				);

			node.reset();
			node = std::move(extension);
		}
		else
		{
			node.reset();
			node = std::move(branch);
		}
	}

	/**
	 * @brief branch node, update value if nibbles are empty,
	 *        otherwise update nibble branch
	 *
	 * @param node
	 * @param nibbles
	 * @param value
	 */
	static void PutKeyBranchNode(
		std::unique_ptr<Node>& node,
		const std::vector<Nibble>& nibbles,
		const Internal::Obj::BytesBaseObj& value
	)
	{
		BranchNode* branch =
			static_cast<BranchNode*>(node->GetNodeBasePtr().get());

		if (nibbles.size() == 0)
		{
			branch->SetValue(value);
			return;
		}

		Nibble branchNibble(nibbles[0]);
		std::vector<Nibble> remaining(
			nibbles.begin() + 1,
			nibbles.end()
		);
		std::unique_ptr<Node>& branchNode = branch->GetBranch(branchNibble);
		PutKey(branchNode, remaining, value);
	}

	/**
	 * @brief Extension node
	 *
	 * @param node
	 * @param nibbles
	 * @param value
	 */
	static void PutKeyExtensionNode(
		std::unique_ptr<Node>& node,
		const std::vector<Nibble>& nibbles,
		const Internal::Obj::BytesBaseObj& value
	)
	{
		ExtensionNode* extension =
			static_cast<ExtensionNode*>(node->GetNodeBasePtr().get());

		std::vector<Nibble> extensionPath = extension->GetPath();
		uint8_t matched =
			NibbleHelper::PrefixMatchedLen(nibbles, extensionPath);

		if (matched < extensionPath.size())
		{
			std::vector<Nibble> sharedNibbles(
				extensionPath.begin(),
				extensionPath.begin() + matched
			);
			Nibble branchNibble(extensionPath[matched]);
			std::vector<Nibble> remaining(
				extensionPath.begin() + matched + 1,
				extensionPath.end()
			);

			Nibble nodeBranchNibble(nibbles[matched]);
			std::vector<Nibble> nodeLeafNibbles(
				nibbles.begin() + matched + 1,
				nibbles.end()
			);

			auto branchBase =
				Internal::Obj::Internal::make_unique<BranchNode>();
			std::unique_ptr<Node> nextNode =
				std::move(extension->GetNext());

			if (remaining.size() == 0)
			{
				branchBase->SetBranch(branchNibble, std::move(nextNode));
			}
			else
			{
				auto newExtensionBase =
					Internal::Obj::Internal::make_unique<ExtensionNode>(
						std::move(remaining),
						std::move(nextNode)
					);
				std::unique_ptr<Node> newExtension =
					Internal::Obj::Internal::make_unique<Node>(
						std::move(newExtensionBase)
					);
				branchBase->SetBranch(
					branchNibble,
					std::move(newExtension)
				);
			}

			auto remainingLeafBase =
				LeafNode::NewLeafNodeFromNibbles(nodeLeafNibbles, value);
			std::unique_ptr<Node> remainingLeaf =
				Internal::Obj::Internal::make_unique<Node>(
					std::move(remainingLeafBase)
				);
			branchBase->SetBranch(nodeBranchNibble, std::move(remainingLeaf));

			std::unique_ptr<Node> branch =
				Internal::Obj::Internal::make_unique<Node>(
					std::move(branchBase)
				);
			node.reset();

			if (sharedNibbles.size() == 0)
			{
				node = std::move(branch);
			}
			else
			{
				auto newExtensionBase =
					Internal::Obj::Internal::make_unique<ExtensionNode>(
						std::move(sharedNibbles),
						std::move(branch)
					);
				std::unique_ptr<Node> newExtension =
					Internal::Obj::Internal::make_unique<Node>(
						std::move(newExtensionBase)
					);
				node = std::move(newExtension);
			}
			return;
		}
		std::vector<Nibble> remaining(
			nibbles.begin() + matched,
			nibbles.end()
		);
		PutKey(extension->GetNext(), remaining, value);
	}

	static void PutKey(
		std::unique_ptr<Node>& node,
		const std::vector<Nibble>& nibbles,
		const Internal::Obj::BytesBaseObj& value
	)
	{
		// empty node, create a new leaf
		if (node == nullptr)
		{
			PutKeyEmptyNode(node, nibbles, value);
		}
		else
		{
			NodeType nodeType = node->GetNodeType();
			switch(nodeType)
			{
			case NodeType::Leaf:
				PutKeyLeafNode(node, nibbles, value);
				break;
			case NodeType::Branch:
				PutKeyBranchNode(node, nibbles, value);
				break;
			case NodeType::Extension:
				PutKeyExtensionNode(node, nibbles, value);
				break;
			default:
				throw Exception("Invalid node type");
			}
		}
	}

private:

	std::unique_ptr<Node> m_root;

}; // class PatriciaTrie

} // namespace Trie
} // namespace Eth
} // namespace EclipseMonitor
