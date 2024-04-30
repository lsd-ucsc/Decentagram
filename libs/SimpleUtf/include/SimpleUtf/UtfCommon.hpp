// Copyright (c) 2022 SimpleUtf
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#if __cplusplus >= 201709L
#include <bit>

#	ifdef __cpp_lib_bit_cast
#		define SIMPLEUTF_HAS_BIT_CAST_CONSTEXPR
#		define SIMPLEUTF_BIT_CAST_CONSTEXPR constexpr
#	else
#		define SIMPLEUTF_BIT_CAST_CONSTEXPR
#	endif

#	ifdef __cpp_lib_bitops
#		define SIMPLEUTF_HAS_BITOPS_CONSTEXPR
#		define SIMPLEUTF_BITOPS_CONSTEXPR constexpr
#	else
#		define SIMPLEUTF_BITOPS_CONSTEXPR
#	endif

#else
#	define SIMPLEUTF_BIT_CAST_CONSTEXPR
#	define SIMPLEUTF_BITOPS_CONSTEXPR
#endif

#include <cstdint>
#include <cstring>

#include <iterator>
#include <limits>
#include <string>
#include <type_traits>

#include "Exceptions.hpp"

#ifndef SIMPLEUTF_CUSTOMIZED_NAMESPACE
namespace SimpleUtf
#else
namespace SIMPLEUtf_CUSTOMIZED_NAMESPACE
#endif
{

// ==================================================
// unified access to type_traits types
// ==================================================

namespace Internal
{

/**
 * @brief an implementation of std::enable_if_t
 *
 */
template<bool B, class T = void>
using EnableIfT = typename std::enable_if<B,T>::type;

/**
 * @brief an unified access to API of std::result_of/std::invoke_result
 *
 */
#if __cplusplus < 201703L
template<class F, class... ArgTypes>
using InvokeResult = std::result_of<F(ArgTypes...)>;
#else
template<class F, class... ArgTypes>
using InvokeResult = std::invoke_result<F, ArgTypes...>;
#endif

template<class F, class... ArgTypes>
using InvokeResultT = typename InvokeResult<F, ArgTypes...>::type;

/**
 * @brief an unified access to API of std::is_integral
 *
 */
template<typename _T>
using IsIntegral = std::is_integral<_T>;

/**
 * @brief an unified access to API of std::is_signed
 *
 */
template<typename _T>
using IsSigned = std::is_signed<_T>;

/**
 * @brief an unified access to API of std::make_unsigned
 *
 */
template<typename _T>
using MakeUnsignedT = typename std::make_unsigned<_T>::type;

/**
 * @brief an unified access to API of std::bit_cast
 *
 */
#ifdef SIMPLEUTF_HAS_BIT_CAST_CONSTEXPR
template <typename _ToType, typename _FromType>
constexpr _ToType BitCast(const _FromType& from) noexcept
{
	return std::bit_cast<_ToType>(from);
}
#else
// source: https://en.cppreference.com/w/cpp/numeric/bit_cast
template <typename _ToType, typename _FromType>
typename std::enable_if<
	sizeof(_ToType) == sizeof(_FromType) &&
	std::is_trivially_copyable<_FromType>::value &&
	std::is_trivially_copyable<_ToType>::value,
	_ToType>::type
BitCast(const _FromType& src) noexcept
{
	static_assert(std::is_trivially_constructible<_ToType>::value,
		"This implementation additionally requires destination type "
		"to be trivially constructible");

	_ToType dst;
	std::memcpy(&dst, &src, sizeof(_ToType));
	return dst;
}
#endif

/**
 * @brief an unified access to API of std::countl_zero
 *
 */
template<typename _T,
	EnableIfT<
	IsIntegral<_T>::value && // Integer
	!IsSigned<_T>::value     // Unsigned
	, int> = 0>
inline
SIMPLEUTF_BITOPS_CONSTEXPR
size_t
CountLZero(const _T& x) noexcept
{
#ifdef SIMPLEUTF_HAS_BITOPS_CONSTEXPR
	return static_cast<size_t>(std::countl_zero(x));
#else
	_T testBit = _T(1) << (std::numeric_limits<_T>::digits - 1);

	size_t res = 0;
	while(testBit > 0 && !(x & testBit))
	{
		testBit >>= 1;
		++res;
	}

	return res;
#endif
}

/**
 * @brief an unified access to API of std::bit_width
 *
 */
template<typename _T,
	EnableIfT<
	IsIntegral<_T>::value && // Integer
	!IsSigned<_T>::value     // Unsigned
	, int> = 0>
inline
SIMPLEUTF_BITOPS_CONSTEXPR
size_t BitWidth(const _T& x) noexcept
{
	// Source: https://en.cppreference.com/w/cpp/numeric/bit_width
	return static_cast<size_t>(std::numeric_limits<_T>::digits)
		- CountLZero(x);
}

template<typename _T>
inline
SIMPLEUTF_BITOPS_CONSTEXPR
size_t BitWidthChar(const _T& x) noexcept;

template<>
inline
SIMPLEUTF_BITOPS_CONSTEXPR
size_t BitWidthChar<char32_t>(const char32_t& x) noexcept
{
	static_assert(!IsSigned<char32_t>::value,
		"char32_t must be an unsigned type");
	return BitWidth(static_cast<uint32_t>(x));
}

} // namespace Internal

// ==================================================
// Helper functions for bit_cast
// ==================================================

namespace Internal
{

/**
 * @brief If given type is unsigned, return it directly, otherwise,
 *        bit_cast it and return the unsigned type
 *
 */
template<typename _ValType, bool _IsSigned>
struct BitCast2UnsignedImpl;

template<typename _ValType>
struct BitCast2UnsignedImpl<_ValType, false> // unsigned
{
	static constexpr _ValType Cast(const _ValType& val)
	{
		return val;
	}
}; // struct BitCast2UnsignedImpl

template<typename _ValType>
struct BitCast2UnsignedImpl<_ValType, true> // signed
{
	static
	SIMPLEUTF_BIT_CAST_CONSTEXPR
	MakeUnsignedT<_ValType>
	Cast(const _ValType& val)
	{
		return BitCast<MakeUnsignedT<_ValType> >(val);
	}
}; // struct BitCast2UnsignedImpl

template<typename _ValType,
	EnableIfT<
		IsIntegral<_ValType>::value
		, int> = 0>
inline
SIMPLEUTF_BIT_CAST_CONSTEXPR
typename std::conditional<
	IsSigned<_ValType>::value,
		MakeUnsignedT<_ValType>, // true  - signed
		_ValType                 // false - unsigned
	>::type
BitCast2Unsigned(const _ValType& val)
{
	return BitCast2UnsignedImpl<
		_ValType,
		IsSigned<_ValType>::value>::Cast(val);
}

} // namespace Internal

// ==================================================
// Helper functions for checking code point values
// ==================================================

namespace Internal
{

inline bool IsReservedCodePt(char32_t val)
{
	return (0xD800U <= val) && (val <= 0xDFFFU);
}

inline bool IsValidCodePt(char32_t val)
{
	return (val <= 0x10FFFFU) && !IsReservedCodePt(val);
}

} // namespace Internal

inline bool IsValidUtfCodePt(char32_t val)
{
	return Internal::IsValidCodePt(val);
}

// ==================================================
// Helper functions for checking byte size
// ==================================================

namespace Internal
{

template<typename _ValType, size_t _Bytes>
struct TrailingOnesImpl;

template<typename _ValType>
struct TrailingOnesImpl<_ValType, 1>
{
	static constexpr _ValType value = 0xFFU;
}; // struct TrailingOnesImpl

template<typename _ValType, size_t _Bytes>
struct TrailingOnesImpl
{
	static constexpr _ValType value =
		(_ValType(0xFFU) << ((_Bytes - 1) * 8))
			| TrailingOnesImpl<_ValType, _Bytes - 1>::value;
}; // struct TrailingOnesImpl

inline constexpr size_t ByteCeiling(size_t b)
{
	return (b / 8) + (((b % 8) == 0) ? 0 : 1);
}

template<typename _ValType, size_t _Bytes,
	EnableIfT<
		(_Bytes >= 1) &&                 // _Bytes must at least be 1
		IsIntegral<_ValType>::value &&   // integer
		!IsSigned<_ValType>::value &&    // unsigned
		(std::numeric_limits<_ValType>::digits >= (_Bytes * 8)) // enough size
		, int> = 0>
inline constexpr _ValType TrailingOnes()
{
	return
		std::numeric_limits<_ValType>::digits == _Bytes ?
			_ValType(~(_ValType(0x00U))) :
			TrailingOnesImpl<_ValType, _Bytes>::value;
}
static_assert(TrailingOnes<uint64_t, 1>() == 0x00FFULL, "Programming Error");
static_assert(TrailingOnes<uint64_t, 2>() == 0x00FFFFULL, "Programming Error");
static_assert(TrailingOnes<uint64_t, 3>() == 0x00FFFFFFULL, "Programming Error");
static_assert(TrailingOnes<uint64_t, 4>() == 0x00FFFFFFFFULL, "Programming Error");
static_assert(TrailingOnes<uint64_t, 5>() == 0x00FFFFFFFFFFULL, "Programming Error");
static_assert(TrailingOnes<uint64_t, 6>() == 0x00FFFFFFFFFFFFULL, "Programming Error");
static_assert(TrailingOnes<uint64_t, 7>() == 0x00FFFFFFFFFFFFFFULL, "Programming Error");
static_assert(TrailingOnes<uint64_t, 8>() == 0xFFFFFFFFFFFFFFFFULL, "Programming Error");

static_assert(TrailingOnes<uint8_t,  1>() == 0xFFU, "Programming Error");
static_assert(TrailingOnes<uint16_t, 1>() == 0x00FFU, "Programming Error");
static_assert(TrailingOnes<uint32_t, 1>() == 0x00FFU, "Programming Error");
static_assert(TrailingOnes<uint64_t, 1>() == 0x00FFULL, "Programming Error");

/**
 * @brief Ensures the given value can fit in a variable that has size
 *        of `_Bytes`
 *
 * @tparam _ValType
 * @tparam _Bytes
 */
template<size_t _Bytes, typename _ValType,
	EnableIfT<
		(_Bytes >= 1) && // _Bytes must at least be 1
		IsIntegral<_ValType>::value // integer
		, int> = 0>
inline _ValType EnsureByteSize(const _ValType& val)
{
	auto uval = BitCast2Unsigned(val);
	using UType = decltype(uval);
	static_assert(!IsSigned<UType>::value, "Programming Error");

	return ((uval &
		static_cast<UType>(
			~TrailingOnes<UType, _Bytes>())) == 0) ?
		uval :
		throw UtfConversionException("Invalid Encoding" " - "
			"The given value is out of range of "
				+ std::to_string(_Bytes) + " bytes");
}

/**
 * @brief shorthand for getting value_type of a iterator
 *
 */
template<typename _ItType>
using ItValType = typename std::iterator_traits<_ItType>::value_type;

/**
 * @brief Can type _ValType hold value that has size of _ByteSize?
 *
 */
template<typename _ValType, size_t _ByteSize>
using CanTHold = typename std::conditional<
	ByteCeiling(std::numeric_limits<_ValType>::digits) >= _ByteSize,
	std::true_type,
	std::false_type
>::type;

} // namespace Internal


// ==================================================
// Helper functions for checking ASCII values
// ==================================================

namespace Internal
{

template<typename _ValType, bool _IsSigned>
struct AsciiTraitsImpl;

template<typename _ValType>
struct AsciiTraitsImpl<_ValType, false>
{
	static_assert(!IsSigned<_ValType>::value, "Programming Error");

	using UType = _ValType;

	static constexpr _ValType sk_ptLowLimit = 0x20U;
	static constexpr _ValType sk_ptHighLimit = 0x7EU;

	static constexpr bool IsAscii(const _ValType& val)
	{
		// 0011 1000 0110 ~
		// 1100 0111 1001 | 0x7FU
		// 1100 0111 1111 ~
		// 0011 1000 0000 == 0
		// false
		return (~(~val | 0x7FU)) == 0;
	}

	static constexpr bool IsAsciiFast(const _ValType& val)
	{
		return IsAscii(val);
	}

	static constexpr bool IsAByte(const _ValType& val)
	{
		return val <= (std::numeric_limits<uint8_t>::max)();
	}

	static constexpr bool IsPrintable(const _ValType& val)
	{
		return IsAscii(val) &&
			((sk_ptLowLimit <= val) && (val <= sk_ptHighLimit));
	}
}; // struct AsciiTraitsImpl

template<typename _ValType>
struct AsciiTraitsImpl<_ValType, true>
{
	static_assert(IsSigned<_ValType>::value, "Programming Error");

	using UType = MakeUnsignedT<_ValType>;

	static constexpr _ValType sk_ptLowLimit =
		AsciiTraitsImpl<UType, false>::sk_ptLowLimit;
	static constexpr _ValType sk_ptHighLimit =
		AsciiTraitsImpl<UType, false>::sk_ptHighLimit;

	static
	SIMPLEUTF_BIT_CAST_CONSTEXPR
	bool IsAscii(const _ValType& val)
	{
		return AsciiTraitsImpl<UType, false>::IsAscii(
			Internal::BitCast2Unsigned(val));
	}

	static constexpr bool IsAsciiFast(const _ValType& val)
	{
#ifdef SIMPLEUTF_HAS_BIT_CAST_CONSTEXPR
		return IsAscii(val);
#else
		return (0 <= val) && (val <= 0x7F);
#endif
	}

	static constexpr bool IsAByte(const _ValType& val)
	{
		return
			std::numeric_limits<_ValType>::digits >= 8 ?
				(0 <= val &&
					(val <= (std::numeric_limits<uint8_t>::max)())) :
				(((std::numeric_limits<signed char>::min)() <= val) &&
					(val <= (std::numeric_limits<signed char>::max)()));
	}

	static
	SIMPLEUTF_BIT_CAST_CONSTEXPR
	bool IsPrintable(const _ValType& val)
	{
		return IsAscii(val) &&
			((sk_ptLowLimit <= val) && (val <= sk_ptHighLimit));
	}
}; // struct AsciiTraitsImpl

} // namespace Internal

template<
	typename _ValType,
	Internal::EnableIfT<
			Internal::IsIntegral<_ValType>::value &&
			(std::numeric_limits<_ValType>::digits >= 7)
			, int> = 0>
struct AsciiTraits :
	Internal::AsciiTraitsImpl<
		_ValType,
		Internal::IsSigned<_ValType>::value>
{}; // struct AsciiTraits

// IsAscii - Unsigned
static_assert(AsciiTraits<uint8_t>::IsAscii(static_cast<uint8_t>('\0')),
	"Programming Error");
static_assert(AsciiTraits<uint8_t>::IsAscii(static_cast<uint8_t>('\n')),
	"Programming Error");
static_assert(AsciiTraits<uint8_t>::IsAscii(static_cast<uint8_t>('a')),
	"Programming Error");
static_assert(AsciiTraits<uint8_t>::IsAscii(static_cast<uint8_t>('\x7f')),
	"Programming Error");
static_assert(!AsciiTraits<uint8_t>::IsAscii(static_cast<uint8_t>('\x80')),
	"Programming Error");
static_assert(!AsciiTraits<uint8_t>::IsAscii(static_cast<uint8_t>(~0)),
	"Programming Error");

static_assert(!AsciiTraits<uint32_t>::IsAscii(0x0080U),
	"Programming Error");
static_assert(!AsciiTraits<uint32_t>::IsAscii(0x0100U),
	"Programming Error");
static_assert(!AsciiTraits<uint32_t>::IsAscii(0x0101U),
	"Programming Error");

// IsAsciiFast - Signed
static_assert(AsciiTraits<char>::IsAsciiFast('\0'), "Programming Error");
static_assert(AsciiTraits<char>::IsAsciiFast('\n'), "Programming Error");
static_assert(AsciiTraits<char>::IsAsciiFast('a'), "Programming Error");
static_assert(AsciiTraits<char>::IsAsciiFast('\x7f'), "Programming Error");
static_assert(!AsciiTraits<char>::IsAsciiFast('\x80'), "Programming Error");
static_assert(!AsciiTraits<char>::IsAsciiFast(static_cast<char>(~0)),
	"Programming Error");

static_assert(AsciiTraits<int32_t>::IsAsciiFast('\0'), "Programming Error");
static_assert(AsciiTraits<int32_t>::IsAsciiFast('\n'), "Programming Error");
static_assert(!AsciiTraits<int32_t>::IsAsciiFast(-1), "Programming Error");
static_assert(!AsciiTraits<int32_t>::IsAsciiFast(0x0100),
	"Programming Error");

// IsAByte - unsigned
static_assert(AsciiTraits<uint32_t>::IsAByte(0x00FFU), "Programming Error");
static_assert(!AsciiTraits<uint32_t>::IsAByte(0xFF00U),
	"Programming Error");

// IsAByte - signed
static_assert(AsciiTraits<char>::IsAByte(static_cast<char>(~0)),
	"Programming Error");
static_assert(AsciiTraits<char>::IsAByte(static_cast<char>(-1)),
	"Programming Error");
static_assert(AsciiTraits<int32_t>::IsAByte(0x0000), "Programming Error");
static_assert(AsciiTraits<int32_t>::IsAByte(0x0080), "Programming Error");
static_assert(AsciiTraits<int32_t>::IsAByte(0x00FF), "Programming Error");
static_assert(!AsciiTraits<int32_t>::IsAByte(0x01FF), "Programming Error");
static_assert(!AsciiTraits<int32_t>::IsAByte(~0x00FF), "Programming Error");
static_assert(!AsciiTraits<int32_t>::IsAByte(-1), "Programming Error");

// IsPrintable - unsigned
static_assert(AsciiTraits<uint8_t>::IsPrintable(static_cast<uint8_t>('a')),
	"Programming Error");
static_assert(AsciiTraits<uint8_t>::IsPrintable(static_cast<uint8_t>(' ')),
	"Programming Error");
static_assert(AsciiTraits<uint8_t>::IsPrintable(static_cast<uint8_t>('~')),
	"Programming Error");
static_assert(!AsciiTraits<uint8_t>::IsPrintable(static_cast<uint8_t>('\0')),
	"Programming Error");
static_assert(!AsciiTraits<uint8_t>::IsPrintable(static_cast<uint8_t>('\n')),
	"Programming Error");
static_assert(!AsciiTraits<uint8_t>::IsPrintable(static_cast<uint8_t>('\x7f')),
	"Programming Error");
static_assert(!AsciiTraits<uint8_t>::IsPrintable(static_cast<uint8_t>('\x80')),
	"Programming Error");

} // namespace SimpleUtf
