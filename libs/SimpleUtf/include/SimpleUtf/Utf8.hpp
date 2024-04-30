// Copyright (c) 2022 SimpleUtf
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "UtfCommon.hpp"

#ifndef SIMPLEUTF_CUSTOMIZED_NAMESPACE
namespace SimpleUtf
#else
namespace SIMPLEUtf_CUSTOMIZED_NAMESPACE
#endif
{

namespace Internal
{

inline size_t CalcUtf8NumContNeeded(char32_t val)
{
	if (!IsValidCodePt(val))
	{
		throw UtfConversionException("Invalid UTF Code Point" " - "
			+ std::to_string(val) + " is not a valid UTF code point.");
	}

	switch (BitWidthChar(val))
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			return 0;
		case 8:
		case 9:
		case 10:
		case 11:
			return 1;
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
			return 2;
		default:
			return 3;
	}
}

template<typename ValType>
inline std::pair<size_t, uint8_t> Utf8ReadLeading(const ValType& val)
{
	if(!AsciiTraits<ValType>::IsAByte(val))
	{
		throw UtfConversionException("Invalid Encoding" " - "
			"The given value is bigger than a byte");
	}

	// Case 1: 1 byte - (0) ~ (7) bits
	// 0xxxxxxx
	if (AsciiTraits<ValType>::IsAsciiFast(val))
	{
		size_t numCont = 0;
		uint8_t res = static_cast<uint8_t>(val & 0x7F);
		return std::make_pair(numCont, res);
	}
	// Case 2: 2 bytes - (2 + 6 = 8) ~ (5 + 6 = 11) bits
	// 110xxxxx  10xxxxxx
	else if ( // 10000000 is explicitly checked above
		(val & 0x40) && // 01000000
		!(val & 0x20) && // !00100000
		(val & 0x1E) // 00011110 ensure there're val in these bits
	)
	{
		size_t numCont = 1;
		uint8_t res = static_cast<uint8_t>(val & 0x1F);
		return std::make_pair(numCont, res);
	}
	// Case 3: 3 bytes - (0 + 6 + 6 = 12) ~ (4 + 6 + 6 = 16) bits
	// 1110xxxx  10xxxxxx  10xxxxxx
	else if ( // 10000000 is explicitly checked above
		(val & 0x40) && // 01000000
		(val & 0x20) && // 00100000
		!(val & 0x10) // !00010000
	)
	{
		size_t numCont = 2;
		uint8_t res = static_cast<uint8_t>(val & 0x0F);
		return std::make_pair(numCont, res);
	}
	// Case 4: 4 bytes - (0 + 5 + 6 + 6 = 17) ~ (3 + 6 + 6 + 6 = 21) bits
	// 11110xxx  10xxxxxx  10xxxxxx  10xxxxxx
	else if ( // 10000000 is explicitly checked above
		(val & 0x40) && // 01000000
		(val & 0x20) && // 00100000
		(val & 0x10) && // 00010000
		!(val & 0x08)) // !00001000
	{
		size_t numCont = 3;
		uint8_t res = static_cast<uint8_t>(val & 0x07);
		return std::make_pair(numCont, res);
	}
	else
	{
		throw UtfConversionException("Invalid Encoding" " - "
		"Invalid UTF-8 leading byte.");
	}
}

template<typename ValType>
inline uint8_t Utf8ReadCont(const ValType& val)
{
	if(!AsciiTraits<ValType>::IsAByte(val))
	{
		throw UtfConversionException("Invalid Encoding" " - "
			"The given value is bigger than a byte");
	}

	// 10xxxxxx
	if (!AsciiTraits<ValType>::IsAsciiFast(val) && // Non-ASCII - 10000000
		!(val & 0x40)) // !01000000
	{
		uint8_t res = static_cast<uint8_t>(val & 0x3F);
		return res;
	}
	else
	{
		throw UtfConversionException("Invalid Encoding" " - "
		"Invalid UTF-8 continuation byte.");
	}
}

} // namespace Internal

template<typename InputIt>
inline std::pair<char32_t, InputIt> Utf8ToCodePtOnce(InputIt begin, InputIt end)
{
	if (begin == end)
	{
		throw UtfConversionException("Unexpected Ending" " - "
			"String ends unexpected while reading the next UTF-8 char.");
	}

	char32_t res = 0;

	uint8_t leading = 0;
	size_t numCont = 0;
	std::tie(numCont, leading) = Internal::Utf8ReadLeading(*begin);
	++begin;
	res |= leading;

	for (size_t i = 0; i < numCont; ++i)
	{
		uint8_t b = Internal::Utf8ReadCont(*begin);
		++begin;

		res <<= 6;
		res |= b;

		// security check on first continuation byte
		if (i == 0)
		{
			if ((numCont == 2 && res < 0x20U) ||
				(numCont == 3 && res < 0x10U))
			{
				throw UtfConversionException("Invalid Encoding" " - "
					"Invalid UTF-8 continuation byte.");
			}
		}
	}

	if (!Internal::IsValidCodePt(res))
	{
		throw UtfConversionException("Invalid Code Point" " - "
			"The code point read from the given UTF-8 encoding is invalid.");
	}

	return std::make_pair(res, begin);
}

template<typename OutputIt>
inline void CodePtToUtf8Once(char32_t val, OutputIt oit)
{
	size_t numCont = Internal::CalcUtf8NumContNeeded(val);
	char res[4]{0, 0, 0, 0};

	for (size_t i = numCont; i > 0; --i)
	{
		// Flip sign bit:
		uint8_t tmp = 0x80U;
		tmp |= static_cast<uint8_t>((val & 0x3FU));

		res[i] = Internal::BitCast<char>(tmp);

		val >>= 6;
	}

	switch (numCont)
	{
	case 0:
		res[0] = static_cast<char>((val & 0x7F));
		break;
	case 1:
	{
		uint8_t tmp = 0xC0U;
		tmp |= static_cast<uint8_t>((val & 0x1FU));
		res[0] = Internal::BitCast<char>(tmp);
	}
		break;
	case 2:
	{
		uint8_t tmp = 0xE0U;
		tmp |= static_cast<uint8_t>((val & 0x0FU));
		res[0] = Internal::BitCast<char>(tmp);
	}
		break;
	case 3:
	default:
	{
		uint8_t tmp = 0xF0U;
		tmp |= static_cast<uint8_t>((val & 0x07U));
		res[0] = Internal::BitCast<char>(tmp);
	}
		break;
	}

	std::copy(std::begin(res), std::begin(res) + 1 + numCont, oit);
}

inline size_t CodePtToUtf8OnceGetSize(char32_t val)
{
	return 1 + Internal::CalcUtf8NumContNeeded(val);
}

} // namespace SimpleUtf
