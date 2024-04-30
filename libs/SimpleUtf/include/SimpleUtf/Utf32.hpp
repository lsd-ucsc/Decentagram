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

template<typename InputIt,
	Internal::EnableIfT<
		Internal::CanTHold<
			typename std::iterator_traits<InputIt>::value_type, 4>::value
	, int> = 0>
inline std::pair<char32_t, InputIt> Utf32ToCodePtOnce(InputIt begin, InputIt end)
{
	if (begin == end)
	{
		throw UtfConversionException("Unexpected Ending" " - "
			"String ends unexpected while reading the next UTF-32 bytes.");
	}
	auto uval = Internal::BitCast2Unsigned(*begin);
	++begin;
	Internal::EnsureByteSize<4>(uval);
	char32_t uval4B = static_cast<char32_t>(uval);

	if(!Internal::IsValidCodePt(uval4B))
	{
		throw UtfConversionException("Invalid Encoding" " - "
			"Invalid UTF-32 bytes.");
	}

	return std::make_pair(
		uval4B,
		begin
	);
}

template<typename OutputIt>
inline void CodePtToUtf32Once(char32_t val, OutputIt oit)
{
	if (!Internal::IsValidCodePt(val))
	{
		throw UtfConversionException("Invalid UTF Code Point" " - "
			+ std::to_string(val) + " is not a valid UTF code point.");
	}

	char32_t resUtf[1] = { static_cast<char32_t>(val) };

	std::copy(std::begin(resUtf), std::end(resUtf), oit);
}

inline size_t CodePtToUtf32OnceGetSize(char32_t val)
{
	if (!Internal::IsValidCodePt(val))
	{
		throw UtfConversionException("Invalid UTF Code Point" " - "
			+ std::to_string(val) + " is not a valid UTF code point.");
	}

	return 1;
}

} // namespace SimpleUtf
