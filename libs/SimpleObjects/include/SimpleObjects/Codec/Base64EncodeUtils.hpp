// Copyright (c) 2023 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstddef>

#include <array>

#include "Base64Utils.hpp"


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
struct Base64Alphabet
{
	using value_type = _ValType;

	static constexpr _ValType Padding()
	{
		return '=';
	}

	static constexpr std::array<_ValType, 64> Alphabet()
	{
		return {
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
			'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
			'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
			'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
			'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
			'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
			'w', 'x', 'y', 'z', '0', '1', '2', '3',
			'4', '5', '6', '7', '8', '9', '+', '/',
		};
	}
}; // struct HexUpperCaseAlphabet


template<typename _Alphabet, bool _AddPadding>
struct Base64EncodePadder;

template<typename _Alphabet>
struct Base64EncodePadder<_Alphabet, true>
{
	template<typename _OutIt>
	static _OutIt Pad(_OutIt destIt)
	{
		*destIt++ = _Alphabet::Padding();
		return destIt;
	}
}; // struct Base64EncodePadder<_Alphabet, true>

template<typename _Alphabet>
struct Base64EncodePadder<_Alphabet, false>
{
	template<typename _OutIt>
	static _OutIt Pad(_OutIt destIt)
	{
		return destIt;
	}
}; // struct Base64EncodePadder<_Alphabet, false>


template<typename _Alphabet, bool _AddPadding>
struct Base64Encode3Impl
{
	using Alphabet = _Alphabet;
	static constexpr bool sk_addPadding = _AddPadding;
	using Padder = Base64EncodePadder<Alphabet, sk_addPadding>;

	static constexpr size_t sk_inputValSize = 1;

	template<typename _OutIt, typename _ByteType>
	static _OutIt Encode3(
		_OutIt destIt,
		const _ByteType& byte1
	)
	{
		static_assert(
			std::is_integral<_ByteType>::value &&
				sizeof(_ByteType) == sk_inputValSize,
			"Only support encoding of 1-byte integral type"
		);

		static constexpr auto sk_lut = Alphabet::Alphabet();

		// 1st char is 0x11111100 of byte1
		*destIt++ = sk_lut[(byte1 >> 2) & 0x3FU];

		// in case of byte2 is not present
		// 2nd char is 0x00000011 of byte1
		*destIt++ = sk_lut[(byte1 << 4) & 0x30U];
		destIt = Padder::Pad(destIt);
		destIt = Padder::Pad(destIt);
		return destIt;
	}

	template<typename _OutIt, typename _ByteType>
	static _OutIt Encode3(
		_OutIt destIt,
		const _ByteType& byte1,
		const _ByteType& byte2
	)
	{
		static_assert(
			std::is_integral<_ByteType>::value &&
				sizeof(_ByteType) == sk_inputValSize,
			"Only support encoding of 1-byte integral type"
		);

		static constexpr auto sk_lut = Alphabet::Alphabet();

		// 1st char is 0x11111100 of byte1
		*destIt++ = sk_lut[(byte1 >> 2) & 0x3FU];

		// encode 2nd char
		// 2nd char is 0x00000011 of byte1 and 0x11110000 of byte2
		*destIt++ = sk_lut[((byte1 << 4) & 0x30U) | (((byte2) >> 4) & 0x0FU)];

		// in case of byte3 is not present
		// 3rd char is 0x00001111 of byte2
		*destIt++ = sk_lut[((byte2) << 2) & 0x3CU];
		destIt = Padder::Pad(destIt);
		return destIt;
	}

	template<typename _OutIt, typename _ByteType>
	static _OutIt Encode3(
		_OutIt destIt,
		const _ByteType& byte1,
		const _ByteType& byte2,
		const _ByteType& byte3
	)
	{
		static_assert(
			std::is_integral<_ByteType>::value &&
				sizeof(_ByteType) == sk_inputValSize,
			"Only support encoding of 1-byte integral type"
		);

		static constexpr auto sk_lut = Alphabet::Alphabet();

		// 1st char is 0x11111100 of byte1
		*destIt++ = sk_lut[(byte1 >> 2) & 0x3FU];

		// encode 2nd char
		// 2nd char is 0x00000011 of byte1 and 0x11110000 of byte2
		*destIt++ = sk_lut[((byte1 << 4) & 0x30U) | (((byte2) >> 4) & 0x0FU)];

		// encode 3rd char
		// 3rd char is 0x00001111 of byte2 and 0x11000000 of byte3
		*destIt++ = sk_lut[(((byte2) << 2) & 0x3CU) | (((byte3) >> 6) & 0x03U)];

		// encode 4th char
		// 4th char is 0x00111111 of byte3
		*destIt++ = sk_lut[(byte3) & 0x3FU];

		return destIt;
	}
}; // struct Base64Encode3Impl<_Alphabet, true>


} // namespace Internal


} // namespace Codec
} // namespace SimpleObjects
