// Copyright (c) 2023 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <tuple>

#include "Base64DecodeUtils.hpp"


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
	typename _LutType,
	bool _checkTrailing,
	bool _reqPadding
>
struct Base64DecodeImpl
{
	using LutType = _LutType;
	using value_type = typename LutType::value_type;
	using CharType = typename LutType::CharType;

	using AsmType = Base64AssembleImpl<value_type, _checkTrailing>;

	static constexpr bool sk_reqPadding = _reqPadding;
	static constexpr bool sk_throwOnInvalidCh = LutType::sk_throwOnInvalid;

	template<
		typename _OutIt,
		typename _InIt
	>
	static _OutIt Decode(
		_OutIt destIt,
		_InIt begin,
		_InIt end
	)
	{
		using _ValGetter =
			Base64ValueGetter<value_type, CharType, sk_throwOnInvalidCh>;

		Base64DecodeChType chType = Base64DecodeChType::Valid;
		value_type val1 = 0;
		value_type val2 = 0;
		value_type val3 = 0;
		value_type val4 = 0;

		while (begin != end)
		{
			// get ch1
			std::tie(chType, val1, begin) = _ValGetter::GetNextValue(begin, end);

			// if it's end, the input is ended since last iteration, return
			// if it's padding, it doesn't make sense to have padding at the
			// first char of a quad, error
			if (chType == Base64DecodeChType::End)
			{
				return destIt;
			}
			else if (chType == Base64DecodeChType::Pad)
			{
				throw std::invalid_argument("Invalid base64 padding");
			}

			// get ch2
			std::tie(chType, val2, begin) = _ValGetter::GetNextValue(begin, end);

			// if it's end, we can't decode with 1 char, error
			// if it's padding, we can't decode with 1 char, error
			if (
				(chType == Base64DecodeChType::End) ||
				(chType == Base64DecodeChType::Pad)
			)
			{
				throw std::invalid_argument(
					"The length of the Base64 input cannot be 1 char more than"
					" a multiple of 4"
				);
			}

			// get ch3
			std::tie(chType, val3, begin) = _ValGetter::GetNextValue(begin, end);

			// if it's padding, get another pad char, decode 2 chars, and return
			// if it's end, is padding required?
			//     if so, throw because we need 2 padding chars
			//     if not, decode 2 chars, and return
			if (chType == Base64DecodeChType::Pad)
			{
				std::tie(chType, val4, begin) =
					_ValGetter::GetNextValue(begin, end);
				if (chType != Base64DecodeChType::Pad)
				{
					throw std::invalid_argument("Invalid base64 padding");
				}
				return AsmType::Assemble(destIt, val1, val2);
			}
			else if (chType == Base64DecodeChType::End)
			{
				Base64ThrowOnReqPadding<sk_reqPadding>::ReqPadding();
				return AsmType::Assemble(destIt, val1, val2);
			}

			// get ch4
			std::tie(chType, val4, begin) = _ValGetter::GetNextValue(begin, end);

			// if it's padding, decode 3 chars, and return
			// if it's end, is padding required?
			//     if so, throw because we need 1 padding char
			//     if not, decode 3 chars, and return
			if (chType == Base64DecodeChType::Pad)
			{
				return AsmType::Assemble(destIt, val1, val2, val3);
			}
			else if (chType == Base64DecodeChType::End)
			{
				Base64ThrowOnReqPadding<sk_reqPadding>::ReqPadding();
				return AsmType::Assemble(destIt, val1, val2, val3);
			}

			// decode all 4 chars
			destIt = AsmType::Assemble(destIt, val1, val2, val3, val4);
		}

		return destIt;
	}

}; // struct Base64DecodeImpl


} // namespace Internal


} // namespace Codec
} // namespace SimpleObjects
