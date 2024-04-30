// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <cstdint>

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{
namespace Internal
{

template<typename _T>
struct IsPrimitiveType;

template<typename _T> struct IsPrimitiveType : std::false_type {};

template<> struct IsPrimitiveType<bool    > : std::true_type {};
// char
template<> struct IsPrimitiveType<char         > : std::true_type {};
template<> struct IsPrimitiveType<signed   char> : std::true_type {};
template<> struct IsPrimitiveType<unsigned char> : std::true_type {};
// short
template<> struct IsPrimitiveType<signed   short> : std::true_type {};
template<> struct IsPrimitiveType<unsigned short> : std::true_type {};
// int
template<> struct IsPrimitiveType<signed   int> : std::true_type {};
template<> struct IsPrimitiveType<unsigned int> : std::true_type {};
// long
template<> struct IsPrimitiveType<signed   long> : std::true_type {};
template<> struct IsPrimitiveType<unsigned long> : std::true_type {};
// long long
template<> struct IsPrimitiveType<signed   long long> : std::true_type {};
template<> struct IsPrimitiveType<unsigned long long> : std::true_type {};
// float
template<> struct IsPrimitiveType<float > : std::true_type {};
// double
template<> struct IsPrimitiveType<double> : std::true_type {};

static_assert(IsPrimitiveType<bool    >::value, "Implementation Error");
static_assert(IsPrimitiveType<int8_t  >::value, "Implementation Error");
static_assert(IsPrimitiveType<int16_t >::value, "Implementation Error");
static_assert(IsPrimitiveType<int32_t >::value, "Implementation Error");
static_assert(IsPrimitiveType<int64_t >::value, "Implementation Error");
static_assert(IsPrimitiveType<uint8_t >::value, "Implementation Error");
static_assert(IsPrimitiveType<uint16_t>::value, "Implementation Error");
static_assert(IsPrimitiveType<uint32_t>::value, "Implementation Error");
static_assert(IsPrimitiveType<uint64_t>::value, "Implementation Error");
static_assert(IsPrimitiveType<float   >::value, "Implementation Error");
static_assert(IsPrimitiveType<double  >::value, "Implementation Error");

struct direct_cmp
{
	template<typename T, typename U>
	static constexpr bool cmp_equal(T t, U u) noexcept
	{ return t == u; }

	template<typename T, typename U>
	static constexpr bool cmp_less(T t, U u) noexcept
	{ return t < u; }

	template<typename T, typename U>
	static constexpr bool cmp_greater(T t, U u) noexcept
	{ return t > u; }

	template<typename T, typename U>
	static constexpr int three_way_cmp(T t, U u) noexcept
	// https://stackoverflow.com/questions/48042955/how-is-the-three-way-comparison-operator-different-from-subtraction
	{ return ((t > u) - (t < u)); }
}; // struct direct_cmp

struct DirectCompare
{
	template<typename _LhsType, typename _RhsType>
	static constexpr bool Equal(_LhsType lhs, _RhsType rhs)
	{ return direct_cmp::cmp_equal(lhs, rhs); }

	template<typename _LhsType, typename _RhsType>
	static constexpr bool Less(_LhsType lhs, _RhsType rhs)
	{ return direct_cmp::cmp_less(lhs, rhs); }

	template<typename _LhsType, typename _RhsType>
	static constexpr bool Greater(_LhsType lhs, _RhsType rhs)
	{ return direct_cmp::cmp_greater(lhs, rhs); }

	template<typename _LhsType, typename _RhsType>
	static constexpr int Compare(_LhsType lhs, _RhsType rhs)
	{ return direct_cmp::three_way_cmp(lhs, rhs); }
}; // struct DirectCompare

template<bool _IsTSigned, bool _IsUSigned>
struct cmp_impl;

template<>
struct cmp_impl<true, true> : public direct_cmp
{}; // struct cmp_impl

template<>
struct cmp_impl<false, false> : public direct_cmp
{}; // struct cmp_impl

template<>
struct cmp_impl<true, false>
{
	template<typename T, typename U>
	static constexpr bool cmp_equal(T t, U u) noexcept
	{
		using UT = typename std::make_unsigned<T>::type;
		return (t < 0 ? false : direct_cmp::cmp_equal(UT(t), u));
	}

	template<typename T, typename U>
	static constexpr bool cmp_less(T t, U u) noexcept
	{
		using UT = typename std::make_unsigned<T>::type;
		return (t < 0 ? true : direct_cmp::cmp_less(UT(t), u));
	}

	template<typename T, typename U>
	static constexpr int three_way_cmp(T t, U u) noexcept
	{
		using UT = typename std::make_unsigned<T>::type;
		// t < 0 ==> t < u ==> t - u < 0
		return (t < 0 ? -1 : direct_cmp::three_way_cmp(UT(t), u));
	}
}; // struct cmp_impl

template<>
struct cmp_impl<false, true>
{
	template<typename T, typename U>
	static constexpr bool cmp_equal(T t, U u) noexcept
	{
		using UU = typename std::make_unsigned<U>::type;
		return (u < 0 ? false : direct_cmp::cmp_equal(t, UU(u)));
	}

	template<typename T, typename U>
	static constexpr bool cmp_less(T t, U u) noexcept
	{
		using UU = typename std::make_unsigned<U>::type;
		return (u < 0 ? false : direct_cmp::cmp_less(t, UU(u)));
	}

	template<typename T, typename U>
	static constexpr int three_way_cmp(T t, U u) noexcept
	{
		using UU = typename std::make_unsigned<U>::type;
		// u < 0 ==> t > u ==> t - u > 0
		return (u < 0 ? 1 : direct_cmp::three_way_cmp(t, UU(u)));
	}
}; // struct cmp_impl

/**
 * @brief A comparison that is safe against lossy integer conversion.
 *        source: https://en.cppreference.com/w/cpp/utility/intcmp
 *        Even though this is provided in C++20, we still need to support
 *        older versions
 *
 * @tparam T
 * @tparam U
 * @param t
 * @param u
 * @return
 */
template<typename T, typename U>
inline constexpr bool cmp_equal(T t, U u) noexcept
{
	static_assert(IsPrimitiveType<T>::value, "Expecting a primitive type");
	static_assert(IsPrimitiveType<U>::value, "Expecting a primitive type");

	return cmp_impl<std::is_signed<T>::value,
		std::is_signed<U>::value>::cmp_equal(t, u);
}

template<typename T, typename U>
inline constexpr bool cmp_less(T t, U u) noexcept
{
	static_assert(IsPrimitiveType<T>::value, "Expecting a primitive type");
	static_assert(IsPrimitiveType<U>::value, "Expecting a primitive type");

	return cmp_impl<std::is_signed<T>::value,
		std::is_signed<U>::value>::cmp_less(t, u);
}

template<typename T, typename U>
inline constexpr bool cmp_greater(T t, U u) noexcept
{
	return cmp_less(u, t);
}

template<typename T, typename U>
inline constexpr int three_way_cmp(T t, U u) noexcept
{
	static_assert(IsPrimitiveType<T>::value, "Expecting a primitive type");
	static_assert(IsPrimitiveType<U>::value, "Expecting a primitive type");

	return cmp_impl<std::is_signed<T>::value,
		std::is_signed<U>::value>::three_way_cmp(t, u);
}

/**
 * @brief Comparison for custom class types. Implementation is not given.
 *        The user should provide their own implementation using template
 *        specialization
 *
 * @tparam _LhsType
 * @tparam _RhsType
 */
template<typename _LhsType, typename _RhsType>
struct CustomCompare;

template<bool _IsBothPrimitive, typename _LhsType, typename _RhsType>
struct CompareImpl;

template<typename _LhsType, typename _RhsType>
struct CompareImpl<true, _LhsType, _RhsType>
{
	static constexpr bool Equal(_LhsType lhs, _RhsType rhs)
	{
		return cmp_equal(lhs, rhs);
	}

	static constexpr bool Less(_LhsType lhs, _RhsType rhs)
	{
		return cmp_less(lhs, rhs);
	}

	static constexpr bool Greater(_LhsType lhs, _RhsType rhs)
	{
		return cmp_greater(lhs, rhs);
	}

	static constexpr int Compare(_LhsType lhs, _RhsType rhs)
	{
		return three_way_cmp(lhs, rhs);
	}
}; // struct CompareImpl

template<typename _LhsType, typename _RhsType>
struct CompareImpl<false, _LhsType, _RhsType>
{
	static constexpr bool Equal(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CustomCompare<_LhsType, _RhsType>::Equal(lhs, rhs);
	}

	static constexpr bool Less(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CustomCompare<_LhsType, _RhsType>::Less(lhs, rhs);
	}

	static constexpr bool Greater(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CustomCompare<_LhsType, _RhsType>::Greater(lhs, rhs);
	}

	static constexpr int Compare(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CustomCompare<_LhsType, _RhsType>::Compare(lhs, rhs);
	}
}; // struct CompareImpl

template<typename _LhsType, typename _RhsType>
struct CompareFilterDouble;

template<typename _LhsType, typename _RhsType>
struct CompareFilterDouble
{ // next to impl

	static constexpr bool sk_isBothPrimitive = (
		IsPrimitiveType<_LhsType>::value &&
		IsPrimitiveType<_RhsType>::value);

	static constexpr bool Equal(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CompareImpl<
			sk_isBothPrimitive, _LhsType, _RhsType>::Equal(lhs, rhs);
	}

	static constexpr bool Less(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CompareImpl<
			sk_isBothPrimitive, _LhsType, _RhsType>::Less(lhs, rhs);
	}

	static constexpr bool Greater(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CompareImpl<
			sk_isBothPrimitive, _LhsType, _RhsType>::Greater(lhs, rhs);
	}

	static constexpr int Compare(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CompareImpl<
			sk_isBothPrimitive, _LhsType, _RhsType>::Compare(lhs, rhs);
	}
}; // struct CompareFilterDouble

template<typename _RhsType>
struct CompareFilterDouble<double, _RhsType> : public DirectCompare
{}; // struct CompareFilterDouble

template<typename _LhsType>
struct CompareFilterDouble<_LhsType, double> : public DirectCompare
{}; // struct CompareFilterDouble

template<typename _LhsType, typename _RhsType>
struct CompareFilterFloat;

template<typename _LhsType, typename _RhsType>
struct CompareFilterFloat
{ // next to filter double
	static constexpr bool Equal(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CompareFilterDouble<_LhsType, _RhsType>::Equal(lhs, rhs);
	}
	static constexpr bool Less(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CompareFilterDouble<_LhsType, _RhsType>::Less(lhs, rhs);
	}
	static constexpr bool Greater(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CompareFilterDouble<_LhsType, _RhsType>::Greater(lhs, rhs);
	}
	static constexpr int Compare(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CompareFilterDouble<_LhsType, _RhsType>::Compare(lhs, rhs);
	}
}; // struct CompareFilterFloat

template<typename _RhsType>
struct CompareFilterFloat<float, _RhsType> : public DirectCompare
{}; // struct CompareFilterFloat

template<typename _LhsType>
struct CompareFilterFloat<_LhsType, float> : public DirectCompare
{}; // struct CompareFilterFloat

template<typename _LhsType, typename _RhsType>
struct CompareFilterBool;

template<typename _LhsType, typename _RhsType>
struct CompareFilterBool
{ // next to filter float
	static constexpr bool Equal(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CompareFilterFloat<_LhsType, _RhsType>::Equal(lhs, rhs);
	}
	static constexpr bool Less(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CompareFilterFloat<_LhsType, _RhsType>::Less(lhs, rhs);
	}
	static constexpr bool Greater(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CompareFilterFloat<_LhsType, _RhsType>::Greater(lhs, rhs);
	}
	static constexpr int Compare(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CompareFilterFloat<_LhsType, _RhsType>::Compare(lhs, rhs);
	}
}; // struct CompareFilterBool

template<typename _RhsType>
struct CompareFilterBool<bool, _RhsType>
{
	static constexpr bool Equal(bool lhs, const _RhsType& rhs)
	{ return direct_cmp::cmp_equal(static_cast<_RhsType>(lhs), rhs); }
	static constexpr bool Less(bool lhs, const _RhsType& rhs)
	{ return direct_cmp::cmp_less(static_cast<_RhsType>(lhs), rhs); }
	static constexpr bool Greater(bool lhs, const _RhsType& rhs)
	{ return direct_cmp::cmp_greater(static_cast<_RhsType>(lhs), rhs); }
	static constexpr int Compare(bool lhs, const _RhsType& rhs)
	{ return direct_cmp::three_way_cmp(static_cast<_RhsType>(lhs), rhs); }
}; // struct CompareFilterBool

template<typename _LhsType>
struct CompareFilterBool<_LhsType, bool>
{
	static constexpr bool Equal(const _LhsType& lhs, bool rhs)
	{ return direct_cmp::cmp_equal(lhs, static_cast<_LhsType>(rhs)); }
	static constexpr bool Less(const _LhsType& lhs, bool rhs)
	{ return direct_cmp::cmp_less(lhs, static_cast<_LhsType>(rhs)); }
	static constexpr bool Greater(const _LhsType& lhs, bool rhs)
	{ return direct_cmp::cmp_greater(lhs, static_cast<_LhsType>(rhs)); }
	static constexpr int Compare(const _LhsType& lhs, bool rhs)
	{ return direct_cmp::three_way_cmp(lhs, static_cast<_LhsType>(rhs)); }
}; // struct CompareFilterBool

template<typename _LhsType, typename _RhsType>
struct CompareFilterSame;

template<typename _LhsType, typename _RhsType>
struct CompareFilterSame
{ // next to filter bool
	static constexpr bool Equal(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CompareFilterBool<_LhsType, _RhsType>::Equal(lhs, rhs);
	}
	static constexpr bool Less(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CompareFilterBool<_LhsType, _RhsType>::Less(lhs, rhs);
	}
	static constexpr bool Greater(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CompareFilterBool<_LhsType, _RhsType>::Greater(lhs, rhs);
	}
	static constexpr int Compare(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CompareFilterBool<_LhsType, _RhsType>::Compare(lhs, rhs);
	}
}; // struct CompareFilterSame

template<typename _Type>
struct CompareFilterSame<_Type, _Type> : public DirectCompare
{}; // struct CompareFilterSame

template<typename _LhsType, typename _RhsType>
struct RealNumCompare;

template<typename _LhsType, typename _RhsType>
struct RealNumCompare
{
	static constexpr bool Equal(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CompareFilterSame<_LhsType, _RhsType>::Equal(lhs, rhs);
	}

	static constexpr bool Less(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CompareFilterSame<_LhsType, _RhsType>::Less(lhs, rhs);
	}

	static constexpr bool Greater(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CompareFilterSame<_LhsType, _RhsType>::Greater(lhs, rhs);
	}

	static constexpr int Compare(const _LhsType& lhs, const _RhsType& rhs)
	{
		return CompareFilterSame<_LhsType, _RhsType>::Compare(lhs, rhs);
	}

	static constexpr bool LessEqual(const _LhsType& lhs, const _RhsType& rhs)
	{
		return !Greater(lhs, rhs);
	}

	static constexpr bool GreaterEqual(const _LhsType& lhs, const _RhsType& rhs)
	{
		return !Less(lhs, rhs);
	}
}; // struct Compare

static_assert(!RealNumCompare<uint32_t, int32_t>::Equal(
	static_cast<uint32_t>(10), static_cast<int32_t>(-10)),
	"Implementation Error");
static_assert(RealNumCompare<uint32_t, int32_t>::Equal(
	static_cast<uint32_t>(10), static_cast<int32_t>(10)),
	"Implementation Error");

// static_assert(!(static_cast<uint32_t>(10) > static_cast<int32_t>(-20)),
// 	"Implementation Error");
static_assert(RealNumCompare<uint32_t, int32_t>::Greater(
	static_cast<uint32_t>(10), static_cast<int32_t>(-20)),
	"Implementation Error");
// static_assert(!(static_cast<int32_t>(-20) < static_cast<uint32_t>(10)),
// 	"Implementation Error");
static_assert(RealNumCompare<int32_t, uint32_t>::Less(
	static_cast<int32_t>(-20), static_cast<uint32_t>(10)),
	"Implementation Error");

// static_assert(!(static_cast<uint32_t>(10) >= static_cast<int32_t>(-20)),
// 	"Implementation Error");
static_assert(RealNumCompare<uint32_t, int32_t>::GreaterEqual(
	static_cast<uint32_t>(10), static_cast<int32_t>(-20)),
	"Implementation Error");
// static_assert(!(static_cast<int32_t>(-20) <= static_cast<uint32_t>(10)),
// 	"Implementation Error");
static_assert(RealNumCompare<int32_t, uint32_t>::LessEqual(
	static_cast<int32_t>(-20), static_cast<uint32_t>(10)),
	"Implementation Error");

static_assert((static_cast<uint32_t>(10) >= static_cast<double>(-20)),
	"Implementation Error");
static_assert(RealNumCompare<uint32_t, double>::GreaterEqual(
	static_cast<uint32_t>(10), static_cast<double>(-20)),
	"Implementation Error");
static_assert((static_cast<double>(-20) <= static_cast<uint32_t>(10)),
	"Implementation Error");
static_assert(RealNumCompare<double, uint32_t>::LessEqual(
	static_cast<double>(-20), static_cast<uint32_t>(10)),
	"Implementation Error");

static_assert(RealNumCompare<uint32_t, int32_t>::Compare(
	static_cast<uint32_t>(10), static_cast<int32_t>(-10)) > 0,
	"Implementation Error");
static_assert(RealNumCompare<int32_t, int32_t>::Compare(
	static_cast<int32_t>(10), static_cast<int32_t>(10)) == 0,
	"Implementation Error");
static_assert(RealNumCompare<int32_t, uint32_t>::Compare(
	static_cast<int32_t>(-10), static_cast<uint32_t>(10)) < 0,
	"Implementation Error");
static_assert(RealNumCompare<uint32_t, double>::Compare(
	static_cast<uint32_t>(10), static_cast<double>(-10.0)) > 0,
	"Implementation Error");
static_assert(RealNumCompare<int32_t, double>::Compare(
	static_cast<int32_t>(10), static_cast<double>(10.0)) == 0,
	"Implementation Error");
static_assert(RealNumCompare<double, uint32_t>::Compare(
	static_cast<double>(-10.0), static_cast<uint32_t>(10)) < 0,
	"Implementation Error");
static_assert(RealNumCompare<uint32_t, bool>::Compare(
	static_cast<uint32_t>(10), static_cast<bool>(true)) > 0,
	"Implementation Error");
static_assert(RealNumCompare<int32_t, bool>::Compare(
	static_cast<int32_t>(1), static_cast<bool>(true)) == 0,
	"Implementation Error");
static_assert(RealNumCompare<bool, uint32_t>::Compare(
	static_cast<bool>(true), static_cast<uint32_t>(10)) < 0,
	"Implementation Error");

} // namespace Internal
} // namespace SimpleObjects
