// Copyright (c) 2022 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <memory>
#include <vector>

#include "../Exceptions.hpp"
#include "../Internal/SimpleObj.hpp"
#include "../SyncMsgMgrBase.hpp"

#include "HeaderMgr.hpp"

namespace EclipseMonitor
{
namespace Eth
{


class HeaderNode;

class HeaderNode
{
public: // static members:
	struct ChildInfo
	{
		/**
		 * @brief Number of descendants that *this* child has
		 *
		 */
		size_t m_numOfDesc;
		std::unique_ptr<HeaderNode> m_child;

		ChildInfo(
			size_t numOfDesc,
			std::unique_ptr<HeaderNode> child) :
			m_numOfDesc(numOfDesc),
			m_child(std::move(child))
		{}
	};

public:
	HeaderNode(
		std::unique_ptr<HeaderMgr> header,
		std::shared_ptr<SyncState> syncState
	) :
		m_children(),
		m_parent(nullptr),
		m_header(std::move(header)),
		m_syncState(syncState)
	{}

	// LCOV_EXCL_START
	~HeaderNode() = default;
	// LCOV_EXCL_STOP

	HeaderNode* AddChild(
		std::unique_ptr<HeaderMgr> childHeader,
		std::shared_ptr<SyncState> syncState
	)
	{
		auto child = Internal::Obj::Internal::make_unique<HeaderNode>(
			std::move(childHeader), syncState
		);
		HeaderNode* childPtr = child.get();

		// if parent is a live node (passed the sync phase),
		// then so do its children
		//child->m_isLive = m_isLive;

		// link child's parent
		child->m_parent = this;
		// append child to children list AND set number of descendants to 0
		m_children.emplace_back(0, std::move(child));
		// !!! NOTE: child is invalid after this point !!!

		// Inform all ancestors that a new descendants has been added
		if (m_parent != nullptr)
		{
			m_parent->AddDescCount(this);
		}

		return childPtr;
	}

	const HeaderMgr& GetHeader() const
	{
		if (m_header == nullptr)
		{
			throw Exception("Header is already released");
		}
		return *m_header;
	}

	std::unique_ptr<HeaderMgr> ReleaseHeader()
	{
		return std::move(m_header);
	}

	std::unique_ptr<HeaderNode> ReleaseChildHasNDesc(
		size_t numOfDesc)
	{
		// find the child in the children list
		auto it = std::find_if(m_children.begin(), m_children.end(),
			[&](const ChildInfo& childInfo) -> bool
			{
				return childInfo.m_numOfDesc >= numOfDesc;
			});

		if (it == m_children.end())
		{
			return nullptr;
		}

		// remove the child from the children list
		std::unique_ptr<HeaderNode> child = std::move(it->m_child);
		m_children.erase(it);
		child->m_parent = nullptr;

		return child;
	}

	size_t GetNumOfChildren() const
	{
		return m_children.size();
	}

	const HeaderNode* GetParent() const
	{
		return m_parent;
	}

protected:

	void AddDescCount(HeaderNode* childPtr)
	{
		// ensure that the child is a child of this node
		if (childPtr->m_parent != this)
		{
			throw Exception(
				"The given node pointer is not a child of this node");
		}

		// find the child in the children list
		auto it = std::find_if(m_children.begin(), m_children.end(),
			[&](const ChildInfo& childInfo) -> bool
			{
				return childInfo.m_child.get() == childPtr;
			});

		if (it == m_children.end())
		{
			// This should never happen, unless we have implementation error
			throw Exception(
				"The given node pointer is not a child of this node");
		}

		// increment the number of descendants
		++(it->m_numOfDesc);

		// Inform all ancestors that a new descendants has been added
		if (m_parent != nullptr)
		{
			m_parent->AddDescCount(this);
		}
	}


private:
	std::vector<ChildInfo> m_children;
	HeaderNode* m_parent;
	std::unique_ptr<HeaderMgr> m_header;
	std::shared_ptr<SyncState> m_syncState;

}; // class HeaderNode


} // namespace Eth
} // namespace EclipseMonitor
