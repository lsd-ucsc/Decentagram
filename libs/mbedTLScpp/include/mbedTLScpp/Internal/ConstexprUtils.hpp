// Copyright (c) 2022 mbedTLScpp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstddef>

#include <array>


#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
namespace Internal
{

template<
	size_t _StrLen,
	bool _IncludeNull = true,
	typename std::enable_if<
		(_StrLen != 0),
		int
	>::type = 0
>
inline std::array<
	char,
	_IncludeNull ? _StrLen : (_StrLen - 1)
> BuildAndRetStrArray(
	const char (&str)[_StrLen]
)
{
	static constexpr size_t _ResLen = _IncludeNull ? _StrLen : (_StrLen - 1);
	std::array<char, _ResLen> ret;
	std::copy(std::begin(str), std::end(str), ret.begin());
	return ret;
}

} // namespace Internal
} // namespace mbedTLScpp
