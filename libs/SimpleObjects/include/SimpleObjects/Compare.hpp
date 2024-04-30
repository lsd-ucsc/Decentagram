// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#if __cplusplus > 201703L
#	include <version>
#	ifdef __cpp_lib_three_way_comparison
#		include <compare>
#	endif
#endif
#include <iterator>
#include <type_traits>

#include "RealNumCompare.hpp"

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{
namespace Internal
{

template<typename _InItA, typename _InItB,
	typename std::enable_if<
		IsPrimitiveType<
			typename std::iterator_traits<_InItA>::value_type>::value &&
		IsPrimitiveType<
			typename std::iterator_traits<_InItB>::value_type>::value
		, int>::type = 0>
inline int LexicographicalCompareThreeWay(
	_InItA beginA, _InItA endA,
	_InItB beginB, _InItB endB)
{
	// ref: https://en.cppreference.com/w/cpp/algorithm/lexicographical_compare_three_way

	using _AType = typename std::iterator_traits<_InItA>::value_type;
	using _BType = typename std::iterator_traits<_InItB>::value_type;

	bool isNotEndA = (beginA != endA);
	bool isNotEndB = (beginB != endB);
	while (isNotEndA && isNotEndB)
	{
		int cmpRes = RealNumCompare<_AType, _BType>::Compare(*beginA, *beginB);
		if (cmpRes != 0)
		{
			return cmpRes;
		}

		++beginA;
		++beginB;
		isNotEndA = (beginA != endA);
		isNotEndB = (beginB != endB);
	}

	// return isNotEndA ? 1 :
	// 		(isNotEndB ? -1 :
	// 					(0));
	// isNotEndA   isNotEndB   ret
	//     1          1        unreachable
	//     1          0         1
	//     0          1        -1
	//     0          0         0
	return isNotEndA - isNotEndB;
}

} // namespace Internal

/**
 * @brief The compare order for SimpleObjects
 *
 */
enum class ObjectOrder
{
	// LHS object is considered *less* than the RHS object
	Less,
	// LHS object is considered *equal* to the RHS object
	Equal,
	// LHS object is considered *greater* than the RHS object
	Greater,
	// Two objects on LHS and RHS are same, meanwhile values in this type
	// cannot be determined which is less/greater than which
	EqualUnordered,
	// Two objects on LHS and RHS are different, and it cannot be determined
	// which is less/greater than which
	NotEqualUnordered,
}; // enum class ObjectOrder

namespace Internal
{

template<typename _InItA, typename _InItB>
inline ObjectOrder ObjectRangeCompareThreeWay(
	_InItA beginA, _InItA endA,
	_InItB beginB, _InItB endB)
{
	auto eqRes = ObjectOrder::Equal;
	bool isNotEndA = (beginA != endA);
	bool isNotEndB = (beginB != endB);
	while (isNotEndA && isNotEndB)
	{
		ObjectOrder cmpRes = (*beginA).BaseObjectCompare(*beginB);
		if ((cmpRes != ObjectOrder::Equal) &&
			(cmpRes != ObjectOrder::EqualUnordered))
		{
			return cmpRes;
		}
		if (cmpRes == ObjectOrder::EqualUnordered)
		{
			eqRes = ObjectOrder::EqualUnordered;
		}

		++beginA;
		++beginB;
		isNotEndA = (beginA != endA);
		isNotEndB = (beginB != endB);
	}

	return isNotEndA ? ObjectOrder::Greater :
			(isNotEndB ? ObjectOrder::Less :
						(eqRes));
}

} // namespace Internal

} // namespace SimpleObjects
