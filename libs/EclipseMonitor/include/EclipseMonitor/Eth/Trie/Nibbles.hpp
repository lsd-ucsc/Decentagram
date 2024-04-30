// Copyright 2022 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <cstdint>

#include <vector>
#include <string>

#include "../../Exceptions.hpp"

namespace EclipseMonitor
{
namespace Eth
{
namespace Trie
{

class NibblesConversionException : public Exception
{
public:
	using Exception::Exception;

	// LCOV_EXCL_START
	virtual ~NibblesConversionException() = default;
	// LCOV_EXCL_STOP

}; // class NibblesConversionException

using Nibble = uint8_t;

struct NibbleHelper
{
	static bool IsNibble(const uint8_t& n)
	{
		return (n <= 15);
	}

	static std::vector<Nibble> FromBytes(const std::vector<uint8_t>& bytes)
	{
		std::vector<Nibble> nibbles;
		nibbles.reserve(bytes.size() * 2);

		for (const uint8_t& byte: bytes)
		{
			nibbles.push_back(static_cast<uint8_t>((byte >> 4) & 0x0F));
			nibbles.push_back(static_cast<uint8_t>(byte & 0x0F));
		}

		return nibbles;
	}

	static std::vector<uint8_t> ToBytes(const std::vector<Nibble>& nibbles)
	{
		std::vector<uint8_t> nibbleBytes;
		nibbleBytes.reserve(nibbles.size() / 2);

		for (size_t i = 0; i < nibbles.size(); i += 2)
		{
			uint8_t nibbleByte =
				static_cast<uint8_t>(nibbles[i] << 4) |
				static_cast<uint8_t>(nibbles[i + 1]);
			nibbleBytes.push_back(nibbleByte);
		}

		return nibbleBytes;
	}

	static std::vector<Nibble> ToPrefixed(
		const std::vector<Nibble>& nibbles,
		bool isLeafNode
	)
	{
		std::vector<Nibble> prefixed;
		prefixed.reserve(2 + nibbles.size());

		if (nibbles.size() % 2 == 1)
		{
			prefixed = {1};
		}
		else
		{
			prefixed = {0, 0};
		}

		// then the Nibbles
		prefixed.insert(prefixed.end(), nibbles.begin(), nibbles.end());

		if (isLeafNode)
		{
			prefixed[0] += 2;
		}

		return prefixed;
	}

	static uint8_t PrefixMatchedLen(
		const std::vector<Nibble>& nibbles1,
		const std::vector<Nibble>& nibbles2
	)
	{
		uint8_t matchedLen = 0;

		for (
				size_t i = 0;
				(
					(i < nibbles1.size() && i < nibbles2.size()) &&
					(nibbles1[i] == nibbles2[i])
				);
				++i, ++matchedLen
			)
		{}
		return matchedLen;
	}

}; // struct NibbleHelper

} // namespace Trie
} // namespace Eth
} // namespace EclipseMonitor
