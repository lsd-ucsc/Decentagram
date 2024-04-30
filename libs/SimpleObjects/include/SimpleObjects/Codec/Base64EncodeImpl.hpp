// Copyright (c) 2023 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include "Base64EncodeUtils.hpp"


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


template<typename _Alphabet, bool _AddPadding>
struct Base64EncodeImpl :
	Base64Encode3Impl<_Alphabet, _AddPadding>
{
	using Base = Base64Encode3Impl<_Alphabet, _AddPadding>;

	using Alphabet = typename Base::Alphabet;

	static constexpr bool sk_addPadding = Base::sk_addPadding;


	template<typename _OutIt, typename _InIt>
	static _OutIt Encode(
		_OutIt destIt,
		_InIt begin,
		_InIt end
	)
	{
		while (begin != end)
		{
			auto byte1 = *begin++;
			if (begin == end)
			{
				return Base::Encode3(destIt, byte1);
			}
			auto byte2 = *begin++;
			if (begin == end)
			{
				return Base::Encode3(destIt, byte1, byte2);
			}
			auto byte3 = *begin++;
			destIt = Base::Encode3(destIt, byte1, byte2, byte3);
		}
		return destIt;
	}
};



} // namespace Internal


} // namespace Codec
} // namespace SimpleObjects
