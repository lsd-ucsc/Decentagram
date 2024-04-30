// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <tuple>
#include <type_traits>

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{
namespace Internal
{

struct ItTransformDirect
{
	template<typename _RetType,
		typename _ItType>
	static _RetType GetRef(const _ItType& it)
	{
		return *it;
	}

	template<typename _RetType,
		typename _ItType,
		typename std::enable_if<
			!std::is_pointer<_ItType>::value, bool>::type = true>
	static _RetType GetPtr(const _ItType& it)
	{
		return it.operator->();
	}

	template<typename _RetType,
		typename _ItType,
		typename std::enable_if<
			std::is_pointer<_ItType>::value, bool>::type = true>
	static _RetType GetPtr(const _ItType& it)
	{
		return it;
	}
}; // struct ItTransformDirect

template<size_t _Idx>
struct ItTransformTupleGet
{
	template<typename _RetType,
		typename _ItType>
	static _RetType GetRef(const _ItType& it)
	{
		return std::get<_Idx>(*it);
	}

	template<typename _RetType,
		typename _ItType>
	static _RetType GetPtr(const _ItType& it)
	{
		return &std::get<_Idx>(*it);
	}

}; // struct ItTransformTupleGet

} // Internal
} // namespace SimpleObjects
