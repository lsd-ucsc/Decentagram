// Copyright (c) 2022 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <algorithm>
#include <memory>

#include "DAA.hpp"
#include "HeaderMgr.hpp"

namespace EclipseMonitor
{
namespace Eth
{

class ValidatorBase
{
public:
	ValidatorBase() = default;

	// LCOV_EXCL_START
	virtual ~ValidatorBase() = default;
	// LCOV_EXCL_STOP

	virtual bool CommonValidate(
		const HeaderMgr& parent,
		bool isParentLive,
		const HeaderMgr& current,
		bool isCurrLive
	) const = 0;
};

template<typename _NetConfig>
class Validator : public ValidatorBase
{
public: // Static member:
	using Base = ValidatorBase;
	using Self = Validator;

	using NetConfig = _NetConfig;

public:
	Validator(std::unique_ptr<DAABase> diffCalculator) :
		m_diffCalculator(std::move(diffCalculator))
	{}

	// LCOV_EXCL_START
	virtual ~Validator() = default;
	// LCOV_EXCL_STOP

	virtual bool CommonValidate(
		const HeaderMgr& parent,
		bool isParentLive,
		const HeaderMgr& current,
		bool isCurrLive
	) const override
	{
		// 1. check block number
		if (current.GetNumber() != parent.GetNumber() + 1)
		{
			return false;
		}

		// 2. check local time (0 means history block)
		// TODO
		(void)isParentLive;
		(void)isCurrLive;

		// 2. check parent_hash == parent.hash
		const auto& parentHash = current.GetRawHeader().get_ParentHash();
		if ((parentHash.size() != parent.GetHash().size()) ||
			(!std::equal(parent.GetHash().begin(), parent.GetHash().end(),
				parentHash.data())))
		{
			return false;
		}

		// 3. check difficulty value
		if (NetConfig::IsBlockOfParis(current.GetNumber()))
		{
			// blocks after Paris, difficulty is always 0
			if (current.GetDiff() != 0)
			{
				return false;
			}
		}
		else
		{
			// blocks before Paris, need to check difficulty
			auto expDiff = (*m_diffCalculator)(parent, current);
			if (current.GetDiff() != expDiff)
			{
				return false;
			}
		}

		// 4. Check hash puzzle
		// TODO

		return true;
	}

private:

	std::unique_ptr<DAABase> m_diffCalculator;
}; // class Validator


} // namespace Eth
} // namespace EclipseMonitor
