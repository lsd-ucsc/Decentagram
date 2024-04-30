// Copyright (c) 2023 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstddef>

#include <iterator>
#include <stdexcept>
#include <utility>

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


template<
	typename _ValType,
	typename _CharType
>
struct HexValueLut
{
	using value_type = _ValType;

	using CharType = _CharType;

	static constexpr value_type GetValue(CharType ch)
	{
		return (ch >= '0' && ch <= '9') ? (ch - '0') :
			(ch >= 'A' && ch <= 'F') ? (ch - 'A' + 10) :
			(ch >= 'a' && ch <= 'f') ? (ch - 'a' + 10) :
			throw std::invalid_argument("Invalid hex character");
	}
}; // struct HexValueLut


template<typename _It>
inline void BitWiseShiftBytesRight(_It begin, _It end, size_t shift)
{
	using _InputValType = typename std::iterator_traits<_It>::value_type;

	if (shift == 0 || shift > 8)
	{
		throw std::invalid_argument("Invalid shift value");
	}
	const size_t shiftComplement = 8 - shift;

	auto shiftedBits = _InputValType();

	while (begin != end)
	{
		auto currVal = *begin;
		*begin++ = (currVal >> shift) | shiftedBits;
		shiftedBits = currVal << shiftComplement;
	}
}


inline constexpr bool HexDecodeCanPadInPlace(std::input_iterator_tag)
{
	return false;
}

inline constexpr bool HexDecodeCanPadInPlace(std::random_access_iterator_tag)
{
	return true;
}

template<typename _InIt>
inline constexpr bool HexDecodeCanPadInPlace()
{
	return HexDecodeCanPadInPlace(
		typename std::iterator_traits<_InIt>::iterator_category()
	);
}


template<bool _KeepLeadingZeroBytes>
struct HexDecodeCheckKeepLeadingZeroBytesImpl;

template<>
struct HexDecodeCheckKeepLeadingZeroBytesImpl<false>
{
	template<
		typename _Decoder,
		typename _DestValType,
		typename _OutIt,
		typename _InIt
	>
	static std::pair<_OutIt, _InIt> SkipZeros(
		_OutIt  destIt,
		_InIt   begin,
		_InIt   end,
		size_t& outDecodedSize
	)
	{
		// skip leading zeros
		while (begin != end)
		{
			auto nibbleCh1  = *begin++;
			outDecodedSize += 1;

			using _InputCharType =
				typename std::remove_reference<decltype(nibbleCh1)>::type;

			auto nibbleCh2 = _InputCharType('0');
			if (begin != end)
			{
				nibbleCh2 = *begin++;
				outDecodedSize += 1;
			}

			if (nibbleCh1 != '0' || nibbleCh2 != '0')
			{
				// we found the first non-zero byte
				*destIt++ = _Decoder::template
					DecodeSingle<_DestValType, _InputCharType>(
						nibbleCh1,
						nibbleCh2
					);
				break;
			}
			// else both nibbles are zero, skip
		}

		return std::make_pair(destIt, begin);
	}


	template<
		typename _Decoder,
		typename _DestValType,
		typename _InputCharType,
		typename _OutIt,
		typename _NibbleType
	>
	static _OutIt DecodeZeros(
		_OutIt        destIt,
		_NibbleType   nibbleCh2
	)
	{
		if(
			(nibbleCh2 != '0') // the first byte is not zero
		)
		{
			*destIt++ = _Decoder::template
				DecodeSingle<_DestValType, _InputCharType>(
					_InputCharType('0'),
					nibbleCh2
				);
		}
		return destIt;
	}
}; // struct HexDecodeCheckKeepLeadingZeroBytesImpl<false>

template<>
struct HexDecodeCheckKeepLeadingZeroBytesImpl<true>
{
	template<
		typename _Decoder,
		typename _DestValType,
		typename _OutIt,
		typename _InIt
	>
	static std::pair<_OutIt, _InIt> SkipZeros(
		_OutIt  destIt,
		_InIt   begin,
		_InIt   /* end */,
		size_t& /* outDecodedSize */
	)
	{
		return std::make_pair(destIt, begin);
	}


	template<
		typename _Decoder,
		typename _DestValType,
		typename _InputCharType,
		typename _OutIt,
		typename _NibbleType
	>
	static _OutIt DecodeZeros(
		_OutIt        destIt,
		_NibbleType   nibbleCh2
	)
	{
		// we want to keep leading zero bytes
		*destIt++ = _Decoder::template
			DecodeSingle<_DestValType, _InputCharType>(
				_InputCharType('0'),
				nibbleCh2
			);
		return destIt;
	}
}; // struct HexDecodeCheckKeepLeadingZeroBytesImpl<true>


template<HexPad   _PadOpt>
struct HexDecodeRandItPadImpl;

template<>
struct HexDecodeRandItPadImpl<HexPad::Front>
{
	template<
		typename _Decoder,
		bool     _KeepLeadingZeroBytes,
		typename _DestValType,
		typename _OutIt,
		typename _InIt
	>
	static std::pair<_OutIt, _InIt> Decode(
		_OutIt  destIt,
		_InIt   begin,
		size_t& outDecodedSize
	)
	{
		// we need to pad it with a zero
		auto nibbleCh2 = *begin++;
		outDecodedSize += 2;

		using _InputCharType =
			typename std::remove_reference<decltype(nibbleCh2)>::type;

		destIt = HexDecodeCheckKeepLeadingZeroBytesImpl<_KeepLeadingZeroBytes>::
			template DecodeZeros<_Decoder, _DestValType, _InputCharType>(
				destIt,
				nibbleCh2
			);

		return std::make_pair(destIt, begin);
	}
}; // struct HexDecodeRandItPadImpl<HexPad::Front>

template<>
struct HexDecodeRandItPadImpl<HexPad::Disabled>
{
	template<
		typename _Decoder,
		bool     _KeepLeadingZeroBytes,
		typename _DestValType,
		typename _OutIt,
		typename _InIt
	>
	static std::pair<_OutIt, _InIt> Decode(
		_OutIt  /* destIt */,
		_InIt   /* begin */,
		size_t& /* outDecodedSize */
	)
	{
		throw std::invalid_argument(
			"Odd number of characters in input string"
		);
	}
}; // struct HexDecodeRandItPadImpl<HexPad::Disabled>


template<HexPad   _PadOpt>
struct HexDecodeCheckPadImpl;

template<>
struct HexDecodeCheckPadImpl<HexPad::Disabled>
{
	static void ThrowIfOdd(size_t size)
	{
		if (size % 2 != 0)
		{
			throw std::invalid_argument("Odd number of hex digits");
		}
	}

	template<typename _It>
	static void ThrowOrShift(size_t size, _It /* begin */, _It /* end */)
	{
		ThrowIfOdd(size);
	}
}; // struct HexDecodeCheckPadImpl<HexPad::Disabled>

template<>
struct HexDecodeCheckPadImpl<HexPad::Front>
{
	static void ThrowIfOdd(size_t /* size */)
	{}

	template<typename _It>
	static void ThrowOrShift(size_t size, _It begin, _It end)
	{
		if (size % 2 != 0)
		{
			// need to shift bytes
			BitWiseShiftBytesRight(begin, end, 4);
		}
	}
}; // struct HexDecodeCheckPadImpl<HexPad::Front>


template<bool _canPadInPlace>
struct HexDecodeCheckCanPadImpl;

template<>
struct HexDecodeCheckCanPadImpl<false>
{
	static void ThrowIfOdd(size_t size)
	{
		if (size % 2 != 0)
		{
			throw std::invalid_argument("Odd number of hex digits");
		}
	}

	template<typename _It>
	static void ShiftIfOdd(size_t size, _It begin, _It end)
	{
		if (size % 2 != 0)
		{
			// need to shift bytes
			BitWiseShiftBytesRight(begin, end, 4);
		}
	}

	template<HexPad _PadOpt, typename _It>
	static void ThrowOrShift(size_t size, _It begin, _It end)
	{
		// the input iterator is not a random access iterator
		// we may need to shift bytes afterwards
		HexDecodeCheckPadImpl<_PadOpt>::ThrowOrShift(size, begin, end);
	}
}; // struct HexDecodeCheckCanPadImpl<false>

template<>
struct HexDecodeCheckCanPadImpl<true>
{
	static void ThrowIfOdd(size_t /* size */)
	{}

	template<typename _It>
	static void ShiftIfOdd(size_t /* size */, _It /* begin */, _It /* end */)
	{}

	template<HexPad _PadOpt, typename _It>
	static void ThrowOrShift(size_t /* size */, _It /* begin */, _It /* end */)
	{}
}; // struct HexDecodeCheckCanPadImpl<true>


} // namespace Internal


} // namespace Codec
} // namespace SimpleObjects
