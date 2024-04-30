// Copyright (c) 2023 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include "Base64DecodeImpl.hpp"
#include "Base64EncodeImpl.hpp"


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
	typename _ValueGetter,
	bool _checkTrailing,
	bool _Padding
>
struct Base64
{

	using Alphabet = _Alphabet;
	using ValueGetter = _ValueGetter;

	static constexpr bool sk_checkTrailing = _checkTrailing;
	static constexpr bool sk_Padding = _Padding;

	using Sizes = Base64Sizes;
	using Encoder = Base64EncodeImpl<Alphabet, sk_Padding>;
	using Decoder = Base64DecodeImpl<
		ValueGetter,
		sk_checkTrailing,
		sk_Padding
	>;


	//==========
	// Encoding bytes functions
	//==========


	template<
		typename _OutContainer,
		typename _InContainer,
		typename std::enable_if<
			sizeof(typename _InContainer::value_type) == 1,
			int
		>::type = 0
	>
	static _OutContainer Encode(const _InContainer& src)
	{
		_OutContainer dest;
		dest.reserve(
			Sizes::EstEncodedSize(src.size())
		);

		Encoder::Encode(
			std::back_inserter(dest),
			std::begin(src),
			std::end(src)
		);

		return dest;
	}


	template<
		typename _OutContainer,
		typename _InValueType,
		size_t   _InSize,
		typename std::enable_if<sizeof(_InValueType) == 1, int>::type = 0
	>
	static _OutContainer Encode(const _InValueType (&src)[_InSize])
	{
		_OutContainer dest;
		dest.reserve(Sizes::EstEncodedSize(_InSize));

		Encoder::Encode(
			std::back_inserter(dest),
			std::begin(src),
			std::end(src)
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
	static _OutContainer Encode(_InIt begin, _InIt end)
	{
		_OutContainer dest;

		Encoder::Encode(
			std::back_inserter(dest),
			begin,
			end
		);

		return dest;
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
		_InIt end
	)
	{
		return Encoder::Encode(
			destIt,
			begin,
			end
		);
	}


	//==========
	// Decoding Bytes functions
	//==========


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
		_OutContainer dest;
		dest.reserve(Sizes::EstDecodedSize(src.size()));

		Decoder::Decode(
			std::back_inserter(dest),
			std::begin(src),
			std::end(src)
		);

		return dest;
	}

	template<
		typename _OutContainer,
		typename _InValueType,
		size_t   _InSize,
		typename std::enable_if<sizeof(_InValueType) == 1, int>::type = 0
	>
	static _OutContainer Decode(const _InValueType (&src)[_InSize])
	{
		_OutContainer dest;
		dest.reserve(Sizes::EstDecodedSize(_InSize));

		Decoder::Decode(
			std::back_inserter(dest),
			std::begin(src),
			std::end(src)
		);

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
		_OutContainer dest;

		Decoder::Decode(
			std::back_inserter(dest),
			begin,
			end
		);

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
		return Decoder::Decode(
			dest,
			begin,
			end
		);
	}

}; // struct Base64


} // namespace Internal


template<bool _isRestricted>
using Base64C = Internal::Base64<
	Internal::Base64Alphabet<char>,
	Internal::Base64ValueGetter<
		uint8_t,
		char,
		std::conditional<_isRestricted, std::true_type, std::false_type>::type::value
	>,
	std::conditional<_isRestricted, std::true_type, std::false_type>::type::value,
	std::conditional<_isRestricted, std::true_type, std::false_type>::type::value
>;

using Base64 = Base64C<true>;


} // namespace Codec
} // namespace SimpleObjects
