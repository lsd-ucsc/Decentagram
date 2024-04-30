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

template<typename _ValType,
	EnableIfT<
	IsIntegral<_ValType>::value &&
	!IsSigned<_ValType>::value
	, int> = 0>
inline bool IsUtf16Surrogate(const _ValType& val)
{
	// 1101 1xxx xxxx xxxx
	return (val & 0x8000U) && //  1000 0000 0000 0000
		(val & 0x4000U) &&    //  0100 0000 0000 0000
		!(val & 0x2000U) &&   // !0010 0000 0000 0000
		(val & 0x1000U) &&    //  0001 0000 0000 0000
		(val & 0x0800U);      //  0000 1000 0000 0000
}

template<typename _ValType,
	EnableIfT<
	IsIntegral<_ValType>::value &&
	!IsSigned<_ValType>::value
	, int> = 0>
inline bool IsUtf16SurrogateFirst(const _ValType& val)
{
	// 1101 10xx xxxx xxxx
	return IsUtf16Surrogate(val) && //  1101 1xxx xxxx xxxx
		!(val & 0x0400U);           // !0000 0100 0000 0000
}

template<typename _ValType,
	EnableIfT<
	IsIntegral<_ValType>::value &&
	!IsSigned<_ValType>::value
	, int> = 0>
inline bool IsUtf16SurrogateSecond(const _ValType& val)
{
	// 1101 11xx xxxx xxxx
	return IsUtf16Surrogate(val) && // 1101 1xxx xxxx xxxx
		(val & 0x0400U);            // 0000 0100 0000 0000
}

} // namespace Internal

template<typename InputIt,
	Internal::EnableIfT<
		Internal::CanTHold<
			typename std::iterator_traits<InputIt>::value_type, 2>::value
	, int> = 0>
inline std::pair<char32_t, InputIt> Utf16ToCodePtOnce(InputIt begin, InputIt end)
{
	if (begin == end)
	{
		throw UtfConversionException("Unexpected Ending" " - "
			"String ends unexpected while reading the next UTF-16 bytes.");
	}

	auto uval1 = Internal::BitCast2Unsigned(*begin);
	++begin;
	Internal::EnsureByteSize<2>(uval1);

	if (Internal::IsUtf16SurrogateFirst(uval1))
	// Surrogate Pairs
	{
		if (begin == end)
		{
			throw UtfConversionException("Unexpected Ending" " - "
				"String ends unexpected while reading the next UTF-16 bytes.");
		}
		auto uval2 = Internal::BitCast2Unsigned(*begin);
		++begin;
		Internal::EnsureByteSize<2>(uval2);

		if (Internal::IsUtf16SurrogateSecond(uval2))
		{
			char32_t res = 0x10000U;
			res += static_cast<char32_t>((uval1 & 0x03FFU) << 10);
			res += static_cast<char32_t>((uval2 & 0x03FFU));

			if (!Internal::IsValidCodePt(res))
			{
				throw UtfConversionException("Invalid Code Point" " - "
					"The code point read from the given UTF-16 encoding is invalid.");
			}

			return std::make_pair(
				res,
				begin
			);
		}
	}
	else if (!Internal::IsUtf16SurrogateSecond(uval1))
	// !Surrogate First && !Surrogate Second
	{
		char32_t res = static_cast<char32_t>(uval1);;

		if (!Internal::IsValidCodePt(res))
		{
			throw UtfConversionException("Invalid Code Point" " - "
				"The code point read from the given UTF-16 encoding is invalid.");
		}

		return std::make_pair(
			res,
			begin
		);
	}

	throw UtfConversionException("Invalid Encoding" " - "
		"Invalid UTF-16 leading bytes.");
}

template<typename OutputIt>
inline void CodePtToUtf16Once(char32_t val, OutputIt oit)
{
	if (!Internal::IsValidCodePt(val))
	{
		throw UtfConversionException("Invalid UTF Code Point" " - "
			+ std::to_string(val) + " is not a valid UTF code point.");
	}

	char16_t resUtf[2] = { 0 };

	if ((/* 0x0000U <= val && */ val <= 0xD7FFU) ||
		(0xE000U <= val && val <= 0xFFFFU))
	// Single 16 bits encoding
	{
		resUtf[0] = static_cast<char16_t>(val);
		std::copy(std::begin(resUtf), std::begin(resUtf) + 1, oit);
	}
	else
	// Surrogate Pairs
	{
		char32_t code = (val - 0x10000U);

		resUtf[0] = static_cast<char16_t>(0xD800U | (code >> 10));
		resUtf[1] = static_cast<char16_t>(0xDC00U | (code & 0x3FFU));

		std::copy(std::begin(resUtf), std::end(resUtf), oit);
	}
}

inline size_t CodePtToUtf16OnceGetSize(char32_t val)
{
	if (!Internal::IsValidCodePt(val))
	{
		throw UtfConversionException("Invalid UTF Code Point" " - "
			+ std::to_string(val) + " is not a valid UTF code point.");
	}

	if ((/* 0x0000U <= val && */ val <= 0xD7FFU) ||
		(0xE000U <= val && val <= 0xFFFFU))
	// Single 16 bits encoding
	{
		return 1;
	}
	else
	// Surrogate Pairs
	{
		return 2;
	}
}

} // namespace SimpleUtf
