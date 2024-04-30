// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstddef>


#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{
namespace Internal
{

/**
 * @brief A generic template function to convert a value into a string
 *        representation
 *
 * @tparam _OutputType  The output string type
 * @tparam _InputType   The input value type
 * @param val           The input value
 * @return The output string
 */
template<typename _OutputType, typename _InputType>
_OutputType ToString(const _InputType& val);

template<typename _OutputType>
inline _OutputType ToString(const typename _OutputType::value_type* val)
{
	return val;
}

template<typename _OutputType, typename _ItType>
inline _OutputType ToString(_ItType begin, _ItType end)
{
	return _OutputType(begin, end);
}

template<bool _Prefix, typename _CharType, typename _OutIt, typename _ByteType>
inline void ByteToHEX(_OutIt destIt, const _ByteType& b)
{
	static constexpr char alphabet[] = "0123456789ABCDEF";

	if
#if __cplusplus >= 201703L
		constexpr
#endif
	(_Prefix)
	{
		*destIt++ = static_cast<_CharType>('\\');
		*destIt++ = static_cast<_CharType>('x');
	}

	// the first nibble (half of byte)
	*destIt++ = static_cast<_CharType>(
		alphabet[((b >> 4) & 0x0F)]);

	// the second nibble (half of byte)
	*destIt++ = static_cast<_CharType>(
		alphabet[(b & 0x0F)]);
}


} // Internal
} // namespace SimpleObjects
