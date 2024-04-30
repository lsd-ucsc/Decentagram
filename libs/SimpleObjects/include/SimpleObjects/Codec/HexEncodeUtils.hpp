// Copyright (c) 2023 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <array>

#include "HexUtils.hpp"


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


template<typename _ValType>
struct  HexUpperCaseAlphabet
{
	using value_type = _ValType;

	static constexpr std::array<_ValType, 16> Alphabet()
	{
		return {
			'0', '1', '2', '3', '4', '5', '6', '7',
			'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
		};
	}
}; // struct HexUpperCaseAlphabet

template<typename _ValType>
struct  HexLowerCaseAlphabet
{
	using value_type = _ValType;

	static constexpr std::array<_ValType, 16> Alphabet()
	{
		return {
			'0', '1', '2', '3', '4', '5', '6', '7',
			'8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
		};
	}
}; // struct HexLowerCaseAlphabet


//==============================================================================
// BytesToHex
//==============================================================================


template<bool _KeepLeadingZero, typename _Alphabet>
struct HexEncodeSingleImpl;

template<typename _Alphabet>
struct HexEncodeSingleImpl<true, _Alphabet>
{
	using Alphabet = _Alphabet;

	template<typename _OutIt, typename _NibbleType>
	static _OutIt Encode(
		_OutIt destIt,
		_NibbleType nibble1,
		_NibbleType nibble2
	)
	{
		static constexpr auto sk_lut = Alphabet::Alphabet();

		// we want to keep both nibbles, no matter they are zeros or not
		*destIt++ = sk_lut[nibble1];
		*destIt++ = sk_lut[nibble2];

		return destIt;
	}
}; // struct HexEncodeSingleImpl<true>

template<typename _Alphabet>
struct HexEncodeSingleImpl<false, _Alphabet>
{
	using Alphabet = _Alphabet;

	template<typename _OutIt, typename _NibbleType>
	static _OutIt Encode(
		_OutIt destIt,
		_NibbleType nibble1,
		_NibbleType nibble2
	)
	{
		static constexpr auto sk_lut = Alphabet::Alphabet();

		if (nibble1 != 0)
		{
			// both nibbles are meaningful
			*destIt++ = sk_lut[nibble1];
			*destIt++ = sk_lut[nibble2];
		}
		else if (nibble2 != 0)
		{
			// only the second nibble is meaningful
			*destIt++ = sk_lut[nibble2];
		}
		// else both nibbles are zero, do nothing

		return destIt;
	}
}; // struct HexEncodeSingleImpl<false>


template<bool _KeepLeadingZero>
struct HexEncodeSkipZerosImpl;

template<>
struct HexEncodeSkipZerosImpl<true>
{
	template<typename _InIt>
	static _InIt Encode(_InIt begin, _InIt /* end */)
	{
		// Dont't skip
		return begin;
	}
}; // struct HexEncodeSkipZerosImpl<true>

template<>
struct HexEncodeSkipZerosImpl<false>
{
	template<typename _InIt>
	static _InIt Encode(_InIt begin, _InIt end)
	{
		while (begin != end && *begin == 0)
		{
			++begin;
		}
		return begin;
	}
}; // struct HexEncodeSkipZerosImpl<false>


//==============================================================================
// IntegerToHex
//==============================================================================


template<
	typename _Alphabet,
	HexZero  _HexZeroOpt
>
struct IntegerToHexEncodeValImpl;

template<typename _Alphabet>
struct IntegerToHexEncodeValImpl<_Alphabet, HexZero::Keep>
{
	using Alphabet = _Alphabet;

	template<
		typename _OutIt,
		typename _IntegerType
	>
	static _OutIt Encode(
		_OutIt destIt,
		const _IntegerType& val,
		size_t fromBitsPos
	)
	{
		static constexpr auto sk_lut = Alphabet::Alphabet();
		static constexpr size_t sk_nibbleBitsSize = 4;

		for (
			size_t i = fromBitsPos;
			i > 0; // stop at the LSB
			i -= sk_nibbleBitsSize // move to the next nibble
		)
		{
			auto ch = sk_lut[(val >> (i - sk_nibbleBitsSize)) & 0x0F];
			*destIt++ = ch;
		}

		return destIt;
	}
}; // struct IntegerToHexEncodeValImpl<_Alphabet, HexZero::Keep>

template<typename _Alphabet>
struct IntegerToHexEncodeValImpl<_Alphabet, HexZero::SkipAll>
{
	using Alphabet = _Alphabet;

	template<
		typename _OutIt,
		typename _IntegerType
	>
	static _OutIt Encode(
		_OutIt destIt,
		const _IntegerType& val,
		size_t fromBitsPos
	)
	{
		static constexpr size_t sk_nibbleBitsSize = 4;

		for (
			size_t i = fromBitsPos;
			i > 0; // stop at the LSB
			i -= sk_nibbleBitsSize // move to the next nibble
		)
		{
			auto nibble = (val >> (i - sk_nibbleBitsSize)) & 0x0F;

			if (nibble != 0)
			{
				// found first non-zero nibble
				// take the keep-leading-zero route
				return IntegerToHexEncodeValImpl<_Alphabet, HexZero::Keep>::
					Encode(destIt, val, i);
			}
		}

		// all nibbles are zero
		return destIt;
	}
}; // struct IntegerToHexEncodeValImpl<_Alphabet, HexZero::SkipAll>

template<typename _Alphabet>
struct IntegerToHexEncodeValImpl<_Alphabet, HexZero::AtLeastOne>
{
	using Alphabet = _Alphabet;
	using AlphabetValType = typename Alphabet::value_type;

	template<
		typename _OutIt,
		typename _IntegerType
	>
	static _OutIt Encode(
		_OutIt destIt,
		const _IntegerType& val,
		size_t fromBitsPos
	)
	{
		static constexpr size_t sk_nibbleBitsSize = 4;

		for (
			size_t i = fromBitsPos;
			i > 0; // stop at the LSB
			i -= sk_nibbleBitsSize // move to the next nibble
		)
		{
			auto nibble = (val >> (i - sk_nibbleBitsSize)) & 0x0F;

			if (nibble != 0)
			{
				// found first non-zero nibble
				// take the keep-leading-zero route
				return IntegerToHexEncodeValImpl<_Alphabet, HexZero::Keep>::
					Encode(destIt, val, i);
			}
		}

		// all nibbles are zero
		*destIt++ = AlphabetValType('0');
		return destIt;
	}
}; // struct IntegerToHexEncodeValImpl<_Alphabet, HexZero::AtLeastOne>


template<
	typename _Alphabet,
	HexZero  _HexZeroOpt,
	bool     _IsSigned
>
struct IntegerToHexEncodeSignedImpl;

template<typename _Alphabet, HexZero _HexZeroOpt>
struct IntegerToHexEncodeSignedImpl<_Alphabet, _HexZeroOpt, false>
{
	template<
		typename _OutIt,
		typename _IntegerType
	>
	static _OutIt Encode(
		_OutIt destIt,
		const _IntegerType& val
	)
	{
		static constexpr size_t sk_intSize = sizeof(_IntegerType);
		static constexpr size_t sk_intBitsSize = sk_intSize * 8;

		return IntegerToHexEncodeValImpl<_Alphabet, _HexZeroOpt>::
			Encode(destIt, val, sk_intBitsSize);
	}
}; // struct IntegerToHexEncodeSignedImpl<_Alphabet, _HexZeroOpt, false>

template<typename _Alphabet, HexZero _HexZeroOpt>
struct IntegerToHexEncodeSignedImpl<_Alphabet, _HexZeroOpt, true>
{
	template<
		typename _OutIt,
		typename _IntegerType
	>
	static _OutIt Encode(
		_OutIt destIt,
		const _IntegerType& val
	)
	{
		using _UnsignedEncoder =
			IntegerToHexEncodeSignedImpl<_Alphabet, _HexZeroOpt, false>;

		if (val < 0)
		{
			return _UnsignedEncoder::Encode(destIt, -val);
		}
		else
		{
			return _UnsignedEncoder::Encode(destIt, val);
		}
	}
}; // struct IntegerToHexEncodeSignedImpl<_Alphabet, _HexZeroOpt, true>


template<typename _Alphabet>
struct IntegerToHexEncodeIgnoreSignImpl
{
	using Alphabet = _Alphabet;
	using AlphabetValType = typename Alphabet::value_type;

	template<typename _OutIt, typename _IntegerType>
	static _OutIt Encode(_OutIt dest, const _IntegerType& /* val */)
	{
		return dest;
	}
}; // struct IntegerToHexEncodeIgnoreSignImpl


template<typename _Alphabet>
struct IntegerToHexEncodeKeepSignImpl
{
	using Alphabet = _Alphabet;
	using AlphabetValType = typename Alphabet::value_type;

	template<typename _OutIt, typename _IntegerType>
	static _OutIt Encode(_OutIt dest, const _IntegerType& val)
	{
		if (val < 0)
		{
			*dest++ = AlphabetValType('-');
		}
		return dest;
	}
}; // struct IntegerToHexEncodeKeepSignImpl


template<typename _Alphabet, bool _IgnoreSign, bool _IsSigned>
struct IntegerToHexEncodeSignChImpl;

template<typename _Alphabet>
struct IntegerToHexEncodeSignChImpl<_Alphabet, true, true> :
	public IntegerToHexEncodeIgnoreSignImpl<_Alphabet>
{}; // struct IntegerToHexEncodeSignChImpl<_Alphabet, true, true>

template<typename _Alphabet>
struct IntegerToHexEncodeSignChImpl<_Alphabet, true, false> :
	public IntegerToHexEncodeIgnoreSignImpl<_Alphabet>
{}; // struct IntegerToHexEncodeSignChImpl<_Alphabet, true, false>

template<typename _Alphabet>
struct IntegerToHexEncodeSignChImpl<_Alphabet, false, false> :
	public IntegerToHexEncodeIgnoreSignImpl<_Alphabet>
{}; // struct IntegerToHexEncodeSignChImpl<_Alphabet, false, false>

template<typename _Alphabet>
struct IntegerToHexEncodeSignChImpl<_Alphabet, false, true> :
	public IntegerToHexEncodeKeepSignImpl<_Alphabet>
{}; // struct IntegerToHexEncodeSignChImpl<_Alphabet, false, true>

} // namespace Internal


} // namespace Codec
} // namespace SimpleObjects
