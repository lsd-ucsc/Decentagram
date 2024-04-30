// Copyright (c) 2022 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <functional>
#include <vector>
#include <memory>

#include "../Exceptions.hpp"
#include "../MonitorReport.hpp"

#include "HeaderNode.hpp"
#include "DataTypes.hpp"
#include "HeaderMgr.hpp"

namespace EclipseMonitor
{
namespace Eth
{


class CheckpointMgr
{
public: // Static members

	using OnCompleteCallback = std::function<void()>;

public:
	CheckpointMgr(
		const MonitorConfig& mConf,
		OnCompleteCallback onComplete) :
		m_chkptSize(static_cast<size_t>(mConf.get_checkpointSize().GetVal())),
		m_onComplete(onComplete),
		m_currWindow(),
		m_candidate(),
		m_lastNode()
	{}

	// LCOV_EXCL_START
	~CheckpointMgr() = default;
	// LCOV_EXCL_STOP

	size_t GetNumOfCandidates() const
	{
		return m_candidate.size() +
			(((m_lastNode != nullptr) && m_isLastNodeCandidate) ? 1 : 0);
	}

	/**
	 * @brief Adds a new header to this checkpoint. It expects the headers to be
	 *        added in the order from older one to newer one.
	 *        NOTE: this function assume the caller gives the *correct* node in
	 *              *correct* order, so it will not check if the given header
	 *              is the child of the last header it received
	 *
	 * @param node
	 */
	void AddNode(std::unique_ptr<HeaderNode> node)
	{
		if (node == nullptr)
		{
			throw Exception("The given node is null");
		}

		// Check if the candidate window will be completed
		// after adding this node
		if (GetNumOfCandidates() + 1 >= m_chkptSize)
		{
			// The candidate window will be completed after adding this node
			// 1. if there is last node, move it to candidate
			if (m_lastNode != nullptr)
			{
				m_candidate.emplace_back(m_lastNode->ReleaseHeader());
			}
			// 2. clean current window
			m_currWindow.clear();
			// 3. move candidate to current window
			m_currWindow.swap(m_candidate);
			// 4. finally add the new node to the last node
			m_lastNode = std::move(node);
			// 5. and mark it as non-candidate
			m_isLastNodeCandidate = false;
			// 6. call the callback
			m_onComplete();
		}
		else
		{
			// The candidate window will not be completed after adding this node
			// 1. if there is last node, move it to current window or candidate
			if (m_lastNode != nullptr)
			{
				if (m_isLastNodeCandidate)
				{
					m_candidate.emplace_back(m_lastNode->ReleaseHeader());
				}
				else
				{
					m_currWindow.emplace_back(m_lastNode->ReleaseHeader());
				}
			}
			// 2. finally add the new node to the last node
			m_lastNode = std::move(node);
			// 3. and mark it as candidate
			m_isLastNodeCandidate = true;
		}
	}

	/**
	 * @brief Adds a new header to this checkpoint (during Bootstrap Phase).
	 *        It expects the headers to be added in the order from older one to
	 *        newer one.
	 *        NOTE: this function assume the caller gives the *correct* node in
	 *              *correct* order, so it will not check if the given header
	 *              is the child of the last header it received
	 *
	 * @param node
	 */
	void AddHeader(std::unique_ptr<HeaderMgr> header)
	{
		if (header == nullptr)
		{
			throw Exception("The given header is null");
		}
		if ((m_lastNode != nullptr))
		{
			throw Exception("Checkpoint manager can only accept nodes"
				" during runtime phase");
		}

		// 1. add the new header to the candidate window
		m_candidate.emplace_back(std::move(header));

		// 2. Check if the candidate window is completed
		// after adding this header
		if (GetNumOfCandidates() >= m_chkptSize)
		{
			// The candidate window is completed
			// 2.1. clean current window
			m_currWindow.clear();
			// 2.2. move candidate to current window
			m_currWindow.swap(m_candidate);
			// 2.6. call the callback
			m_onComplete();
		}
	}

	/**
	 * @brief Get the difficulty median value for blocks within the current
	 *        window
	 *        NOTE: this function does not return the statistical median value,
	 *              especially when the number of elements is even; instead, it
	 *              returns the value at the position of (size // 2) as if the
	 *              difficulty values are sorted in ascending order.
	 *
	 * @return the median difficulty value of this checkpoint
	 */
	Difficulty GetDiffMedian() const
	{
		std::vector<Difficulty> diffs;
		IterateCurrWindow(
			[&diffs](const HeaderMgr& header) {
				diffs.push_back(header.GetDiff());
			}
		);

		// reference: https://en.cppreference.com/w/cpp/algorithm/nth_element
		// When the size of the checkpoint is a even number, to get the
		// statistical median value, we need to take the average of
		// the two middle elements.
		auto mit = diffs.begin() + (diffs.size() / 2);
		std::nth_element(diffs.begin(), mit, diffs.end());

		return *mit;
	}

	void EndBootstrapPhase(std::shared_ptr<SyncState> syncState)
	{
		if (m_lastNode != nullptr)
		{
			throw Exception("Checkpoint manager is already in runtime phase");
		}
		if (m_candidate.size() != 0)
		{
			throw Exception("There are still headers in candidate window");
		}

		m_lastNode = Internal::Obj::Internal::make_unique<HeaderNode>(
			std::move(m_currWindow.back()),
			std::move(syncState)
		);
		m_isLastNodeCandidate = false;
		m_currWindow.pop_back();
	}

	HeaderNode* GetLastNodePtr() const
	{
		if (m_lastNode == nullptr)
		{
			throw Exception("No header has been added to this checkpoint");
		}
		return m_lastNode.get();
	}

	const HeaderNode& GetLastNode() const
	{
		return *GetLastNodePtr();
	}

	const HeaderMgr& GetLastHeader() const
	{
		if (m_lastNode != nullptr)
		{
			return m_lastNode->GetHeader();
		}
		else if (!m_candidate.empty())
		{
			return *m_candidate.back();
		}
		else if (!m_currWindow.empty())
		{
			return *m_currWindow.back();
		}
		else
		{
			throw Exception("No header has been added to this checkpoint");
		}
	}

	bool IsEmpty() const
	{
		return (m_lastNode == nullptr) &&
			m_candidate.empty() &&
			m_currWindow.empty();
	}

	/**
	 * @brief Get the block number range of the current checkpoint
	 *
	 * @return a std::pair<BlockNumber, BlockNumber>, where the first element is
	 *         the start block number and the second element is the end block
	 *         number
	 */
	std::pair<BlockNumber, BlockNumber> GetCheckpointBlkNumRange() const
	{
		if (m_currWindow.empty())
		{
			throw Exception("There is no header in the checkpoint");
		}

		auto begin = m_currWindow.front()->GetNumber();
		return std::make_pair(
			begin,
			begin + m_chkptSize - 1
		);
	}

	template<typename _CallBackFuncType>
	void IterateCurrWindow(_CallBackFuncType callback) const
	{
		for (const auto& header : m_currWindow)
		{
			callback(*header);
		}
		if ((m_lastNode != nullptr) && !m_isLastNodeCandidate)
		{
			callback(m_lastNode->GetHeader());
		}
	}

private:
	size_t m_chkptSize;
	OnCompleteCallback m_onComplete;
	std::vector<std::unique_ptr<HeaderMgr> > m_currWindow;
	std::vector<std::unique_ptr<HeaderMgr> > m_candidate;
	std::unique_ptr<HeaderNode> m_lastNode;
	bool m_isLastNodeCandidate;

}; // class CheckpointMgr


} // namespace Eth
} // namespace EclipseMonitor
