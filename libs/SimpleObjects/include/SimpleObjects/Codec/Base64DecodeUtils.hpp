// Copyright (c) 2023 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstddef>

#include <array>
#include <stdexcept>
#include <utility>

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


enum class Base64DecodeChType
{
	End,
	Pad,
	Valid,
}; // enum class Base64DecodeChType


template<typename _ValType, typename _CharType>
struct Base64Lut
{
	using value_type = _ValType;
	using CharType = _CharType;

	static constexpr value_type sk_iVal = 64;
	static constexpr value_type sk_pVal = sk_iVal;

	static constexpr CharType sk_startCh = '+';
	static constexpr CharType sk_endCh = 'z';
	static constexpr CharType sk_padCh = '=';

	static constexpr size_t sk_numCh = sk_endCh - sk_startCh + 1;
	static_assert(sk_numCh == 80, "Base64Lut: sk_numCh != 80");

	using LutType = std::array<value_type, sk_numCh>;

	static constexpr LutType GetLut()
	{
		return {
			// 1 valid
			//  +
				62,

			// 3 invalid
			//  ,        -        .
				sk_iVal, sk_iVal, sk_iVal,

			// 1 valid
			//  /
				63,

			// 10 valid
			//  0   1   2   3   4   5   6   7   8   9
				52, 53, 54, 55, 56, 57, 58, 59, 60, 61,

			// 6 invalid
			//  :        ;        <        =        >        ?
				sk_iVal, sk_iVal, sk_iVal, sk_pVal, sk_iVal, sk_iVal,

			// 1 invalid
			//  @
				sk_iVal,

			// 13 valid
			//  A  B  C  D  E  F  G  H  I  J   K   L   M
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,

			// 13 valid
			//  N   O   P   Q   R   S   T   U   V   W   X   Y   Z
				13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 ,24, 25,

			// 5 invalid
			//  [        \        ]        ^        _
				sk_iVal, sk_iVal, sk_iVal, sk_iVal, sk_iVal,

			// 1 invalid
			//  `
				sk_iVal,

			// 13 valid
			//  a   b   c   d   e   f   g   h   i   j   k   l   m
				26, 27, 28, 29, 30, 31, 32, 33, 34, 35 ,36, 37, 38,

			// 13 valid
			//  n   o   p   q   r   s   t   u   v   w   x   y   z
				39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
		};
	}

}; // struct Base64Lut


template<typename _ValType, typename _CharType, bool _throwOnInvalid>
struct Base64LutWithChecker;

template<typename _ValType, typename _CharType>
struct Base64LutWithChecker<_ValType, _CharType, true> :
	Base64Lut<_ValType, _CharType>
{
	using Base = Base64Lut<_ValType, _CharType>;

	using value_type = typename Base::value_type;
	using CharType = typename Base::CharType;

	static constexpr bool sk_throwOnInvalid = true;

	static constexpr CharType CheckChar(CharType ch)
	{
		return (Base::sk_startCh <= ch && ch <= Base::sk_endCh) ?
			ch :
			throw std::invalid_argument("Invalid base64 character");
	}

	static constexpr value_type CheckLutValue(value_type val)
	{
		return (val == Base::sk_iVal) ?
			throw std::invalid_argument("Invalid base64 character") :
			val;
	}
}; // struct Base64LutWithChecker<_ValType, true>

template<typename _ValType, typename _CharType>
struct Base64LutWithChecker<_ValType, _CharType, false> :
	Base64Lut<_ValType, _CharType>
{
	using Base = Base64Lut<_ValType, _CharType>;

	using value_type = typename Base::value_type;
	using CharType = typename Base::CharType;

	static constexpr bool sk_throwOnInvalid = false;

	static constexpr CharType CheckChar(CharType ch)
	{
		return (Base::sk_startCh <= ch && ch <= Base::sk_endCh) ?
			ch :
			Base::sk_iVal;
	}

	static constexpr value_type CheckLutValue(value_type val)
	{
		return val;
	}
}; // struct Base64LutWithChecker<_ValType, false>


template<
	typename _ValType,
	typename _CharType,
	bool _throwOnInvalid
>
struct Base64ValueLut :
	Base64LutWithChecker<_ValType, _CharType, _throwOnInvalid>
{
	using Base = Base64LutWithChecker<_ValType, _CharType, _throwOnInvalid>;

	using value_type = typename Base::value_type;
	using CharType = typename Base::CharType;
	using LutType = typename Base::LutType;

	static constexpr bool sk_throwOnInvalid = _throwOnInvalid;
	static_assert(
		sk_throwOnInvalid == Base::sk_throwOnInvalid,
		"sk_throwOnInvalid != Base::sk_throwOnInvalid"
	);

	static value_type GetValue(CharType ch)
	{
		static constexpr LutType sk_lut = Base::GetLut();
		return Base::CheckLutValue(
			sk_lut[Base::CheckChar(ch) - Base::sk_startCh]
		);
	}

}; // struct Base64ValueLut


template<
	typename _ValType,
	typename _CharType,
	bool _throwOnInvalid
>
struct Base64ValueGetter;

template<typename _ValType, typename _CharType>
struct Base64ValueGetter<_ValType, _CharType, true> :
	Base64ValueLut<_ValType, _CharType, true>
{
	using Base = Base64ValueLut<_ValType, _CharType, true>;

	using value_type = typename Base::value_type;

	static constexpr bool sk_throwOnInvalid = true;
	static_assert(
		sk_throwOnInvalid == Base::sk_throwOnInvalid,
		"sk_throwOnInvalid != Base::sk_throwOnInvalid"
	);

	template<typename _InIt>
	static
	std::tuple<Base64DecodeChType, value_type, _InIt>
	GetNextValue(_InIt begin, _InIt end)
	{
		while (begin != end)
		{
			auto ch = *begin++;
			if (ch == Base::sk_padCh)
			{
				return std::make_tuple(
					Base64DecodeChType::Pad,
					value_type(),
					begin
				);
			}
			auto val = Base::GetValue(ch);
			return std::make_tuple(
				Base64DecodeChType::Valid,
				val,
				begin
			);
		}

		return std::make_tuple(Base64DecodeChType::End, value_type(), begin);
	}
}; // struct Base64ValueGetter<_ValType, _CharType, true>

template<typename _ValType, typename _CharType>
struct Base64ValueGetter<_ValType, _CharType, false> :
	Base64ValueLut<_ValType, _CharType, false>
{
	using Base = Base64ValueLut<_ValType, _CharType, false>;

	using value_type = typename Base::value_type;

	static constexpr bool sk_throwOnInvalid = false;
	static_assert(
		sk_throwOnInvalid == Base::sk_throwOnInvalid,
		"sk_throwOnInvalid != Base::sk_throwOnInvalid"
	);

	template<typename _InIt>
	static
	std::tuple<Base64DecodeChType, value_type, _InIt>
	GetNextValue(_InIt begin, _InIt end)
	{
		while (begin != end)
		{
			auto ch = *begin++;
			if (ch == Base::sk_padCh)
			{
				return std::make_tuple(
					Base64DecodeChType::Pad,
					value_type(),
					begin
				);
			}
			auto val = Base::GetValue(ch);
			if (val != Base::sk_iVal)
			{
				return std::make_tuple(
					Base64DecodeChType::Valid,
					val,
					begin
				);
			}
		}

		return std::make_tuple(Base64DecodeChType::End, value_type(), begin);
	}
}; // struct Base64ValueGetter<_ValType, _CharType, false>


template<typename _ValType, bool _checkTrailing>
struct Base64TrailingChecker;

template<typename _ValType>
struct Base64TrailingChecker<_ValType, true>
{
	using value_type = _ValType;

	static void CheckLast2(value_type val)
	{
		if ((val & 0x03U) != 0)
		{
			throw std::invalid_argument("Invalid base64 trailing");
		}
	}

	static void CheckLast4(value_type val)
	{
		if ((val & 0x0FU) != 0)
		{
			throw std::invalid_argument("Invalid base64 trailing");
		}
	}

}; // struct Base64TrailingChecker<_ValType, true>

template<typename _ValType>
struct Base64TrailingChecker<_ValType, false>
{
	using value_type = _ValType;

	static void CheckLast2(value_type /* val */)
	{
		return;
	}

	static void CheckLast4(value_type /* val */)
	{
		return;
	}

}; // struct Base64TrailingChecker<_ValType, false>


template<typename _DestValType, bool _checkTrailing>
struct Base64AssembleImpl
{

	using DestValType = _DestValType;
	static constexpr bool sk_checkTrailing = _checkTrailing;

	template<typename _OutIt>
	static _OutIt Assemble(
		_OutIt destIt,
		const _DestValType& val1,
		const _DestValType& val2
	)
	{
		using _TrailingChecker =
			Base64TrailingChecker<_DestValType, sk_checkTrailing>;

		// 2 char can only decode to 1 byte

		// at this point, we assume:
		// val1 & val2 are valid base64 characters => only LSB 6 bits are used

		// ensure last ch is well-padded with 0
		_TrailingChecker::CheckLast4(val2);

		// the format is 11111122
		*destIt++ =
			// val1 provides 6 LSB
			(val1 << 2) |
			// val2 provides 2 RSB
			(val2 >> 4);

		return destIt;
	}

	template<typename _OutIt>
	static _OutIt Assemble(
		_OutIt destIt,
		const _DestValType& val1,
		const _DestValType& val2,
		const _DestValType& val3
	)
	{
		using _TrailingChecker =
			Base64TrailingChecker<_DestValType, sk_checkTrailing>;

		// 3 char can only decode to 2 byte

		// at this point, we assume:
		// val1 & val2 & val3 are valid base64 characters

		// ensure last ch is well-padded with 0
		_TrailingChecker::CheckLast2(val3);

		// Byte 1: the format is 11111122
		*destIt++ =
			// val1 provides 6 LSB, all 6 bits are meaningful
			(val1 << 2) |
			// val2 provides 2 RSB, only first 2 bits are meaningful
			(val2 >> 4);

		// Byte 2: the format is 22223333
		*destIt++ =
			// val2 provides 4 LSB, only last 4 bits are meaningful
			((val2 & _DestValType(0x0FU)) << 4) |
			// val3 provides 4 RSB, only first 4 bits are meaningful
			(val3 >> 2);

		return destIt;
	}

	template<typename _OutIt>
	static _OutIt Assemble(
		_OutIt destIt,
		const _DestValType& val1,
		const _DestValType& val2,
		const _DestValType& val3,
		const _DestValType& val4
	)
	{
		// 4 char can only decode to 3 byte

		// at this point, we assume:
		// val1 & val2 & val3 & val4 are valid base64 characters

		// Byte 1: the format is 11111122
		*destIt++ =
			// val1 provides 6 LSB, all 6 bits are meaningful
			(val1 << 2) |
			// val2 provides 2 RSB, only first 2 bits are meaningful
			(val2 >> 4);

		// Byte 2: the format is 22223333
		*destIt++ =
			// val2 provides 4 LSB, only last 4 bits are meaningful
			((val2 & _DestValType(0x0FU)) << 4) |
			// val3 provides 4 RSB, only first 4 bits are meaningful
			(val3 >> 2);

		// Byte 3: the format is 33444444
		*destIt++ =
			// val3 provides 2 LSB, only last 2 bits are meaningful
			((val3 & _DestValType(0x03U)) << 6) |
			// val4 provides 6 RSB, all 6 bits are meaningful
			val4;

		return destIt;
	}

}; // struct Base64AssembleImpl


template<bool _reqPadding>
struct Base64ThrowOnReqPadding;

template<>
struct Base64ThrowOnReqPadding<true>
{
	static void ReqPadding()
	{
		throw std::invalid_argument("Invalid base64 padding");
	}
}; // struct Base64ThrowOnReqPadding<true>

template<>
struct Base64ThrowOnReqPadding<false>
{
	static void ReqPadding()
	{
		return;
	}
}; // struct Base64ThrowOnReqPadding<false>


} // namespace Internal


} // namespace Codec
} // namespace SimpleObjects
