// Copyright (c) 2023 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstddef>
#include <cstdint>

#include <iterator>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "HexEncodeImpl.hpp"
#include "HexDecodeImpl.hpp"


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
	typename _HexValueLut
>
struct Hex
{
	using Alphabet = _Alphabet;
	using AlphabetValType = typename Alphabet::value_type;
	using PrefixType = std::basic_string<AlphabetValType>;

	using HexValueLut = _HexValueLut;


	//==========
	// Encoding bytes functions
	//==========


	template<
		typename _OutContainer,
		bool     _KeepLeadingZero,
		typename _InContainer,
		typename std::enable_if<
			sizeof(typename _InContainer::value_type) == 1,
			int
		>::type = 0
	>
	static _OutContainer Encode(
		const _InContainer& src,
		const PrefixType& prefix = PrefixType()
	)
	{
		static constexpr size_t sk_inValTypeSize =
			sizeof(typename _InContainer::value_type);

		_OutContainer dest;
		dest.reserve(src.size() * 2 + prefix.size());

		BytesToHexImpl<Alphabet, sk_inValTypeSize>::
			template Encode<_KeepLeadingZero>(
				std::back_inserter(dest),
				std::begin(src),
				std::end(src),
				prefix
			);

		return dest;
	}


	template<
		typename _OutContainer,
		typename _InContainer,
		typename std::enable_if<
			sizeof(typename _InContainer::value_type) == 1,
			int
		>::type = 0
	>
	static _OutContainer Encode(
		const _InContainer& src,
		const PrefixType& prefix = PrefixType()
	)
	{
		// default to keep leading zeros
		return Encode<_OutContainer, true>(src, prefix);
	}


	template<
		typename _OutContainer,
		bool     _KeepLeadingZero,
		typename _InValueType,
		size_t   _InSize,
		typename std::enable_if<sizeof(_InValueType) == 1, int>::type = 0
	>
	static _OutContainer Encode(
		const _InValueType (&src)[_InSize],
		const PrefixType& prefix = PrefixType()
	)
	{
		static constexpr size_t sk_inValTypeSize = sizeof(_InValueType);

		_OutContainer dest;
		dest.reserve(_InSize * 2 + prefix.size());

		BytesToHexImpl<Alphabet, sk_inValTypeSize>::
			template Encode<_KeepLeadingZero>(
				std::back_inserter(dest),
				std::begin(src),
				std::end(src),
				prefix
			);

		return dest;
	}


	template<
		typename _OutContainer,
		typename _InValueType,
		size_t   _InSize,
		typename std::enable_if<sizeof(_InValueType) == 1, int>::type = 0
	>
	static _OutContainer Encode(
		const _InValueType (&src)[_InSize],
		const PrefixType& prefix = PrefixType()
	)
	{
		// default to keep leading zeros
		return Encode<_OutContainer, true>(src, prefix);
	}


	template<
		typename _OutContainer,
		bool     _KeepLeadingZero,
		typename _InIt,
		typename std::enable_if<
			sizeof(typename std::iterator_traits<_InIt>::value_type) == 1,
			int
		>::type = 0
	>
	static _OutContainer Encode(
		_InIt begin,
		_InIt end,
		const PrefixType& prefix = PrefixType()
	)
	{
		static constexpr size_t sk_inValTypeSize =
			sizeof(typename std::iterator_traits<_InIt>::value_type);

		_OutContainer dest;

		BytesToHexImpl<Alphabet, sk_inValTypeSize>::
			template Encode<_KeepLeadingZero>(
				std::back_inserter(dest),
				begin,
				end,
				prefix
			);

		return dest;
	}


	template<
		typename _OutContainer,
		typename _InIt,
		typename std::enable_if<
			sizeof(typename std::iterator_traits<_InIt>::value_type) == 1,
			int
		>::type = 0
	>
	static _OutContainer Encode(
		_InIt begin,
		_InIt end,
		const PrefixType& prefix = PrefixType()
	)
	{
		// default to keep leading zeros
		return Encode<_OutContainer, true>(begin, end, prefix);
	}


	template<
		bool     _KeepLeadingZero,
		typename _OutIt,
		typename _InIt,
		typename std::enable_if<
			sizeof(typename std::iterator_traits<_InIt>::value_type) == 1,
			int
		>::type = 0
	>
	static _OutIt Encode(
		_OutIt destIt,
		_InIt begin,
		_InIt end,
		const PrefixType& prefix = PrefixType()
	)
	{
		static constexpr size_t sk_inValTypeSize =
			sizeof(typename std::iterator_traits<_InIt>::value_type);

		return BytesToHexImpl<Alphabet, sk_inValTypeSize>::
			template Encode<_KeepLeadingZero>(
				destIt,
				begin,
				end,
				prefix
			);
	}


	template<
		typename _OutIt,
		typename _InIt,
		typename std::enable_if<
			sizeof(typename std::iterator_traits<_InIt>::value_type) == 1,
			int
		>::type = 0
	>
	static _OutIt Encode(
		_OutIt destIt,
		_InIt begin,
		_InIt end,
		const PrefixType& prefix = PrefixType()
	)
	{
		// default to keep leading zeros
		return Encode<true>(destIt, begin, end, prefix);
	}


	//==========
	// Encoding Integer functions
	//==========


	template<
		typename _OutContainer,
		HexZero  _ZeroOpt,
		bool     _IgnoreSign,
		typename _IntegerType,
		typename std::enable_if<
			std::is_integral<_IntegerType>::value,
			int
		>::type = 0
	>
	static _OutContainer Encode(
		const _IntegerType& src,
		const PrefixType& prefix = "0x"
	)
	{
		static constexpr size_t sk_intTypeSize =
			sizeof(_IntegerType);

		_OutContainer dest;
		dest.reserve(sk_intTypeSize * 2 + prefix.size());

		IntegerToHexImpl<Alphabet>::
			template Encode<_ZeroOpt, _IgnoreSign>(
				std::back_inserter(dest),
				src,
				prefix
			);

		return dest;
	}


	template<
		typename _OutContainer,
		typename _IntegerType,
		typename std::enable_if<
			std::is_integral<_IntegerType>::value,
			int
		>::type = 0
	>
	static _OutContainer Encode(
		const _IntegerType& src,
		const PrefixType& prefix = "0x"
	)
	{
		return Encode<_OutContainer, HexZero::AtLeastOne, false>(src, prefix);
	}


	template<
		HexZero  _ZeroOpt,
		bool     _IgnoreSign,
		typename _OutIt,
		typename _IntegerType,
		typename std::enable_if<
			std::is_integral<_IntegerType>::value,
			int
		>::type = 0
	>
	static _OutIt Encode(
		_OutIt destIt,
		const _IntegerType& src,
		const PrefixType& prefix = "0x"
	)
	{
		destIt = IntegerToHexImpl<Alphabet>::
			template Encode<_ZeroOpt, _IgnoreSign>(
				destIt,
				src,
				prefix
			);

		return destIt;
	}


	template<
		typename _OutIt,
		typename _IntegerType,
		typename std::enable_if<
			std::is_integral<_IntegerType>::value,
			int
		>::type = 0
	>
	static _OutIt Encode(
		_OutIt destIt,
		const _IntegerType& src,
		const PrefixType& prefix = "0x"
	)
	{
		return Encode<HexZero::AtLeastOne, false>(destIt, src, prefix);
	}


	//==========
	// Decoding Bytes functions
	//==========


	template<
		typename _OutContainer,
		bool     _KeepLeadingZero,
		HexPad   _PadOpt,
		typename _InContainer,
		typename std::enable_if<
			(sizeof(typename _OutContainer::value_type) == 1) &&
			(_PadOpt == HexPad::Disabled || _PadOpt == HexPad::Front),
			int
		>::type = 0
	>
	static _OutContainer Decode(
		const _InContainer& src
	)
	{
		using _OutputValType = typename _OutContainer::value_type;
		static constexpr size_t sk_outputValSize = sizeof(_OutputValType);
		using _InItType = typename _InContainer::const_iterator;
		static constexpr bool sk_canPadInPlace =
			HexDecodeCanPadInPlace<_InItType>();

		HexDecodeCheckPadImpl<_PadOpt>::ThrowIfOdd(src.size());

		_OutContainer dest;
		dest.reserve((src.size() + 1) / 2);

		size_t decodedSize = 0;
		HexToBytesImpl<HexValueLut, sk_outputValSize>::
			template Decode<_KeepLeadingZero, _PadOpt, _OutputValType>(
				std::back_inserter(dest),
				std::begin(src),
				std::end(src),
				decodedSize
			);

		HexDecodeCheckCanPadImpl<sk_canPadInPlace>::
			ShiftIfOdd(decodedSize, dest.begin(), dest.end());

		return dest;
	}

	template<
		typename _OutContainer,
		typename _InContainer,
		typename std::enable_if<
			sizeof(typename _OutContainer::value_type) == 1,
			int
		>::type = 0
	>
	static _OutContainer Decode(
		const _InContainer& src
	)
	{
		// default to keep leading zeros
		return Decode<_OutContainer, true, HexPad::Disabled>(src);
	}


	template<
		typename _OutContainer,
		bool     _KeepLeadingZero,
		HexPad   _PadOpt,
		typename _InValueType,
		size_t   _InSize,
		typename std::enable_if<
			(sizeof(typename _OutContainer::value_type) == 1) &&
			(_PadOpt == HexPad::Disabled || _PadOpt == HexPad::Front),
			int
		>::type = 0
	>
	static _OutContainer Decode(
		const _InValueType(&src)[_InSize]
	)
	{
		using _OutputValType = typename _OutContainer::value_type;
		static constexpr size_t sk_outputValSize = sizeof(_OutputValType);
		static constexpr bool sk_canPadInPlace = true;

		HexDecodeCheckPadImpl<_PadOpt>::ThrowIfOdd(_InSize);

		_OutContainer dest;
		dest.reserve((_InSize + 1) / 2);

		size_t decodedSize = 0;
		HexToBytesImpl<HexValueLut, sk_outputValSize>::
			template Decode<_KeepLeadingZero, _PadOpt, _OutputValType>(
				std::back_inserter(dest),
				std::begin(src),
				std::end(src),
				decodedSize
			);

		HexDecodeCheckCanPadImpl<sk_canPadInPlace>::
			ShiftIfOdd(decodedSize, dest.begin(), dest.end());

		return dest;
	}

	template<
		typename _OutContainer,
		typename _InValueType,
		size_t   _InSize,
		typename std::enable_if<
			sizeof(typename _OutContainer::value_type) == 1,
			int
		>::type = 0
	>
	static _OutContainer Decode(
		const _InValueType(&src)[_InSize]
	)
	{
		// default to keep leading zeros
		return Decode<_OutContainer, true, HexPad::Disabled>(src);
	}


	template<
		typename _OutContainer,
		bool     _KeepLeadingZero,
		HexPad   _PadOpt,
		typename _InIt,
		typename std::enable_if<
			(sizeof(typename _OutContainer::value_type) == 1) &&
			(_PadOpt == HexPad::Disabled || _PadOpt == HexPad::Front),
			int
		>::type = 0
	>
	static _OutContainer Decode(
		_InIt begin, _InIt end
	)
	{
		using _OutputValType = typename _OutContainer::value_type;
		static constexpr size_t sk_outputValSize = sizeof(_OutputValType);
		static constexpr bool sk_canPadInPlace =
			HexDecodeCanPadInPlace<_InIt>();

		_OutContainer dest;

		size_t decodedSize = 0;
		HexToBytesImpl<HexValueLut, sk_outputValSize>::
			template Decode<_KeepLeadingZero, _PadOpt, _OutputValType>(
				std::back_inserter(dest),
				begin,
				end,
				decodedSize
			);

		// the input iterator is not a random access iterator
		// we may need to shift bytes afterwards
		HexDecodeCheckCanPadImpl<sk_canPadInPlace>::template
			ThrowOrShift<_PadOpt>(decodedSize, dest.begin(), dest.end());

		return dest;
	}

	template<
		typename _OutContainer,
		typename _InIt,
		typename std::enable_if<
			sizeof(typename _OutContainer::value_type) == 1,
			int
		>::type = 0
	>
	static _OutContainer Decode(
		_InIt begin, _InIt end
	)
	{
		// default to keep leading zeros
		return Decode<_OutContainer, true, HexPad::Disabled>(begin, end);
	}


	template<
		bool     _KeepLeadingZero,
		HexPad   _PadOpt,
		typename _OutIt,
		typename _InIt,
		typename std::enable_if<
			(_PadOpt == HexPad::Disabled || _PadOpt == HexPad::Front),
			int
		>::type = 0
	>
	static _OutIt Decode(
		_OutIt dest,
		_InIt begin, _InIt end
	)
	{
		using _OutItValType = typename std::iterator_traits<_OutIt>::value_type;
		using _OutputValType = typename std::conditional<
			std::is_same<_OutItValType, void>::value,
			typename HexValueLut::value_type,
			_OutItValType
		>::type;

		static constexpr size_t sk_outputValSize = sizeof(_OutputValType);
		static constexpr bool sk_canPadInPlace =
			HexDecodeCanPadInPlace<_InIt>();

		size_t decodedSize = 0;
		dest = HexToBytesImpl<HexValueLut, sk_outputValSize>::
			template Decode<_KeepLeadingZero, _PadOpt, _OutputValType>(
				dest,
				begin,
				end,
				decodedSize
			);

		HexDecodeCheckCanPadImpl<sk_canPadInPlace>::ThrowIfOdd(decodedSize);

		return dest;
	}

	template<
		typename _OutIt,
		typename _InIt
	>
	static _OutIt Decode(
		_OutIt dest,
		_InIt begin, _InIt end
	)
	{
		// default to keep leading zeros
		return Decode<true, HexPad::Disabled>(dest, begin, end);
	}


	//==========
	// Decoding Integer functions
	//==========


}; // struct Hex



} // namespace Internal


struct Hex :
	public Internal::Hex<
		Internal::HexLowerCaseAlphabet<char>,
		Internal::HexValueLut<uint8_t, char>
	>
{}; // struct Hex


struct HEX :
	public Internal::Hex<
		Internal::HexUpperCaseAlphabet<char>,
		Internal::HexValueLut<uint8_t, char>
	>
{}; // struct HEX


} // namespace Codec
} // namespace SimpleObjects
