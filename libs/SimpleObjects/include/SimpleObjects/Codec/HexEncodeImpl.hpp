// Copyright (c) 2023 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstddef>

#include <algorithm>
#include <type_traits>

#include "HexEncodeUtils.hpp"


#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{
namespace Codec
{

namespace Internal
{


template<
	typename _Alphabet,
	size_t _InputValSize
>
struct BytesToHexImpl;

template<
	typename _Alphabet
>
struct BytesToHexImpl<_Alphabet, 1>
{
	using Alphabet = _Alphabet;
	using AlphabetValType = typename Alphabet::value_type;

	static constexpr size_t sk_inputValSize = 1;

	template<
		bool     _KeepLeadingZero,
		typename _OutIt,
		typename _InValType,
		typename _PrefixType
	>
	static _OutIt EncodeSingle(
		_OutIt destIt,
		const _InValType& val,
		const _PrefixType& prefix
	)
	{
		static_assert(
			std::is_integral<_InValType>::value &&
				sizeof(_InValType) == sk_inputValSize,
			"Only support encoding of 1-byte integral type"
		);


		// Add prefix
		destIt = std::copy(std::begin(prefix), std::end(prefix), destIt);

		// the first nibble (half of byte)
		auto nibble1 = (val >> 4) & 0x0F;
		auto nibble2 = val       & 0x0F;

		destIt = HexEncodeSingleImpl<_KeepLeadingZero, Alphabet>::
			Encode(destIt, nibble1, nibble2);

		return destIt;
	}


	template<
		bool     _KeepLeadingZero,
		typename _OutIt,
		typename _InIt,
		typename _PrefixType
	>
	static _OutIt Encode(
		_OutIt destIt,
		_InIt begin,
		_InIt end,
		const _PrefixType& prefix
	)
	{
		// skip leading zeros if needed
		begin = HexEncodeSkipZerosImpl<_KeepLeadingZero>::Encode(begin, end);

		// first byte
		if (begin != end)
		{
			auto val = *begin++;
			using _InValType =
				typename std::remove_reference<decltype(val)>::type;

			destIt =
				EncodeSingle<_KeepLeadingZero, _OutIt, _InValType, _PrefixType>(
					destIt,
					val,
					prefix
				);
		}

		// the rest of bytes
		while (begin != end)
		{
			auto val = *begin++;
			using _InValType =
				typename std::remove_reference<decltype(val)>::type;

			destIt = EncodeSingle<true, _OutIt, _InValType, _PrefixType>(
				destIt,
				val,
				_PrefixType()
			);
		}

		return destIt;
	}

}; // struct BytesToHexImpl<_Alphabet, 1>


template<typename _Alphabet>
struct IntegerToHexImpl
{
	using Alphabet = _Alphabet;
	using AlphabetValType = typename Alphabet::value_type;

	template<
		HexZero  _HexZeroOpt,
		bool     _IgnoreSign,
		typename _OutIt,
		typename _IntegerType,
		typename _PrefixType
	>
	static _OutIt Encode(
		_OutIt destIt,
		const _IntegerType& val,
		const _PrefixType& prefix
	)
	{
		static constexpr bool sk_isSigned =
			std::is_signed<_IntegerType>::value;

		destIt =
			IntegerToHexEncodeSignChImpl<_Alphabet, _IgnoreSign, sk_isSigned>::
				Encode(destIt, val);

		// Add prefix
		destIt = std::copy(std::begin(prefix), std::end(prefix), destIt);

		return IntegerToHexEncodeSignedImpl<
				_Alphabet,
				_HexZeroOpt,
				sk_isSigned
			>::Encode(destIt, val);
	}
}; // struct IntegerToHexImpl


} // namespace Internal


} // namespace Codec
} // namespace SimpleObjects
