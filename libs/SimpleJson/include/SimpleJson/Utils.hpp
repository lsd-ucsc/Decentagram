// Copyright (c) 2022 SimpleJson
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <iterator>

#ifndef SIMPLEJSON_CUSTOMIZED_NAMESPACE
namespace SimpleJson
#else
namespace SIMPLEJSON_CUSTOMIZED_NAMESPACE
#endif
{
namespace Internal
{

template<typename _CharType>
inline bool IsSpaceCh(const _CharType& ch)
{
	return (ch == ' ') ||
		(ch == '\r') ||
		(ch == '\n') ||
		(ch == '\t') ||
		(ch == '\v') ||
		(ch == '\f');
}

template<typename _ItType>
inline bool IsSpaceIt(_ItType it)
{
	return IsSpaceCh(*it);
}

template<typename _ItType>
inline _ItType SkipLeadingSpace(_ItType begin, _ItType end)
{
	while(begin != end && IsSpaceIt(begin))
	{
		++begin;
	}

	return begin;
}

template<typename _OutIt, typename _InIt>
inline void RepeatOutput(_OutIt out, _InIt begin, _InIt end, size_t repTime)
{
	for (size_t i = 0; i < repTime; ++i)
	{
		std::copy(begin, end, out);
	}
}

template<typename _OutIt, typename _Container>
inline void RepeatOutput(_OutIt out, const _Container& ctn, size_t repTime)
{
	RepeatOutput(out, ctn.begin(), ctn.end(), repTime);
}

} // namespace Internal
} // namespace SimpleJson
