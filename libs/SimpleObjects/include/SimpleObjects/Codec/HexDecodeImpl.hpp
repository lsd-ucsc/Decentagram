// Copyright (c) 2023 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstddef>

#include <iterator>
#include <stdexcept>
#include <tuple>
#include <type_traits>

#include "HexDecodeUtils.hpp"


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
	typename _HexValueLut,
	size_t _OutputValSize
>
struct HexToBytesImpl;


template<typename _HexValueLut>
struct HexToBytesImpl<_HexValueLut, 1>
{
	using HexValueLut = _HexValueLut;
	static constexpr size_t sk_outputValSize = 1;

	using Self = HexToBytesImpl<HexValueLut, sk_outputValSize>;


	template<
		typename _OutputType,
		typename _InputCharType
	>
	static _OutputType DecodeSingle(
		const _InputCharType& nibbleCh1,
		const _InputCharType& nibbleCh2
	)
	{
		static_assert(
			std::is_integral<_OutputType>::value &&
				sizeof(_OutputType) == sk_outputValSize,
			"Only support decoding to 1-byte integral type"
		);

		auto nibble1 = HexValueLut::GetValue(nibbleCh1);
		auto nibble2 = HexValueLut::GetValue(nibbleCh2);

		return (nibble1 << 4) | nibble2;
	}


	template<
		bool     _KeepLeadingZeroBytes,
		HexPad   _PadOpt, // ignored
		typename _DestValType,
		typename _OutIt,
		typename _InIt
	>
	static _OutIt Decode(
		_OutIt destIt,
		_InIt begin,
		_InIt end,
		size_t& outDecodedSize,
		std::input_iterator_tag
	)
	{
		// skip leading zeros if needed
		std::tie(destIt, begin) =
			HexDecodeCheckKeepLeadingZeroBytesImpl<_KeepLeadingZeroBytes>::template
				SkipZeros<Self, _DestValType>(destIt, begin, end, outDecodedSize);

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

			*destIt++ =
				DecodeSingle<_DestValType, _InputCharType>(
					nibbleCh1,
					nibbleCh2
				);
		}

		return destIt;
	}


	template<
		bool     _KeepLeadingZeroBytes,
		HexPad   _PadOpt,
		typename _DestValType,
		typename _OutIt,
		typename _InIt,
		typename std::enable_if<
			_PadOpt == HexPad::Disabled || _PadOpt == HexPad::Front,
			int
		>::type = 0
	>
	static _OutIt Decode(
		_OutIt destIt,
		_InIt begin,
		_InIt end,
		size_t& outDecodedSize,
		std::random_access_iterator_tag
	)
	{
		auto inputSize = end - begin;

		if ((begin != end) && (inputSize % 2 != 0))
		{
			// input is not empty, AND
			// there're odd number of bytes,
			// where the first byte is a single nibble

			std::tie(destIt, begin) = HexDecodeRandItPadImpl<_PadOpt>::template
				Decode<Self, _KeepLeadingZeroBytes, _DestValType>(
					destIt, begin, outDecodedSize
				);
		}

		// now the rest of number bytes should be even
		// use the regular function to decode the rest
		return Decode<_KeepLeadingZeroBytes, HexPad::Disabled, _DestValType>(
			destIt,
			begin,
			end,
			outDecodedSize,
			std::input_iterator_tag()
		);
	}


	template<
		bool     _KeepLeadingZeroBytes,
		HexPad   _PadOpt,
		typename _DestValType,
		typename _OutIt,
		typename _InIt
	>
	static _OutIt Decode(
		_OutIt destIt,
		_InIt begin,
		_InIt end,
		size_t& outDecodedSize
	)
	{
		return Decode<_KeepLeadingZeroBytes, _PadOpt, _DestValType>(
			destIt,
			begin,
			end,
			outDecodedSize,
			typename std::iterator_traits<_InIt>::iterator_category()
		);
	}
}; // struct HexToBytesImpl<_HexValueLut, 1>


} // namespace Internal


} // namespace Codec
} // namespace SimpleObjects
