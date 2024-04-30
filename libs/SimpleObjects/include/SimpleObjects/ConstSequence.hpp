// Copyright (c) 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.



#pragma once

#include <cstdint>

#include <array>
#include <string>
#include <type_traits>

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{

namespace Internal
{

template<size_t Len, typename T, T headArg, T... args>
struct StripDataTailImpl;

template<size_t Len, char headArg, char... args>
struct StripStrTailImpl;

} // namespace Internal

template<typename _DataType, _DataType... data>
struct DataSeq
{
	using ValType = _DataType;

	template <ValType pre>
	using Prepend = DataSeq<ValType, pre, data...>;

	template<size_t Len>
	using StripTail = typename Internal::StripDataTailImpl<Len, ValType, data...>::type;

	template<template<typename, _DataType...> class _OtherType>
	using ToOther = _OtherType<ValType, data...>;

	static constexpr size_t sk_size = sizeof...(data);

	using CArrayType = ValType[sk_size];
	using CArrayRefType = typename
		std::add_lvalue_reference<CArrayType>::type;
	using CArrayKRefType = typename
		std::add_lvalue_reference<typename
			std::add_const<CArrayType>::type>::type;

	using ArrayType = std::array<ValType, sk_size>;
	using ArrayRefType = typename
		std::add_lvalue_reference<ArrayType>::type;

	static constexpr ArrayType sk_data()
	{
		return { data... };
	}

	static CArrayKRefType sk_cData()
	{
		static const CArrayType skData = { data... };
		return skData;
	}

}; // struct DataSeq

template<char... chars>
struct StrSeq : public DataSeq<char, chars...>
{
	using Base = DataSeq<char, chars...>;

	template<char pre>
	using StrPrepend = StrSeq<pre, chars...>;

	template<size_t Len>
	using StripStrTail = typename Internal::StripStrTailImpl<Len, chars...>::type;

	using CStrType = char[Base::sk_size + 1];
	using CStrRefType = typename
		std::add_lvalue_reference<CStrType>::type;
	using CStrKRefType = typename
		std::add_lvalue_reference<typename
			std::add_const<CStrType>::type>::type;

	static CStrKRefType sk_cstr()
	{
		static constexpr char skStr[] = { chars..., '\0' };

		return skStr;
	}

	static const std::string& sk_str()
	{
		static constexpr auto skData = Base::sk_data();
		static const std::string skStr(
			skData.begin(),
			skData.end());

		return skStr;
	}
}; // struct StrSeq

namespace Internal
{

template<size_t arrayLen, typename _DataType, _DataType... data>
struct ToSequence;

template<size_t arrayLen, typename _DataType, _DataType... data>
struct ToSequence
{
	using type = typename DataSeq<_DataType, data...>::template StripTail<arrayLen>;
}; // struct ToSequence

template<size_t arrayLen, char... data>
struct ToSequence<arrayLen, char, data...>
{
	using type = typename StrSeq<data...>::template StripStrTail<arrayLen - 1>;
}; // struct ToSequence

template<typename T, T headArg, T... args>
struct StripDataTailImpl<1, T, headArg, args...>
{
	using type = DataSeq<T, headArg>;
}; // struct StripDataTailImpl

template<size_t Len, typename T, T headArg, T... args>
struct StripDataTailImpl
{
	using _type = typename StripDataTailImpl<Len - 1, T, args...>::type;

	using type = typename _type::template Prepend<headArg>;
}; // struct StripDataTailImpl

template<char headArg, char... args>
struct StripStrTailImpl<1, headArg, args...>
{
	using type = StrSeq<headArg>;
}; // struct StripStrTailImpl

template<size_t Len, char headArg, char... args>
struct StripStrTailImpl
{
	using _type = typename StripStrTailImpl<Len - 1, args...>::type;

	using type = typename _type::template StrPrepend<headArg>;
}; // struct StripStrTailImpl

template<size_t i, typename T, size_t n>
constexpr T ArrayGetI(const T(&a)[n])
{
	return i < n ? a[i] : T();
}

template<typename T, size_t n>
constexpr T ArrayLen(const T(&)[n])
{
	return n;
}

} // namespace Internal
} // namespace SimpleObjects

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#	define SIMPLEOBJECTS_EXPANDARRAY_2(  S, I) \
		::SimpleObjects::Internal::ArrayGetI<(I)>(S),\
		::SimpleObjects::Internal::ArrayGetI<(I) + 1>(S)
#	define SIMPLEOBJECTS_TO_SEQUENCE(L,T,D) \
		::SimpleObjects::Internal::ToSequence< L, T, D >::type
#	define SIMPLEOBJECTS_ARRAY_LEN(A) \
		::SimpleObjects::Internal::ArrayLen(A)
#else
#	define SIMPLEOBJECTS_EXPANDARRAY_2(  S, I) \
		::SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE::Internal::ArrayGetI<(I)>(S),\
		::SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE::Internal::ArrayGetI<(I) + 1>(S)
#	define SIMPLEOBJECTS_TO_SEQUENCE(L,T,D) \
		::SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE::ToSequence< L, T, D >::type
#	define SIMPLEOBJECTS_ARRAY_LEN(A) \
		::SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE::Internal::ArrayLen(A)
#endif

#define SIMPLEOBJECTS_EXPANDARRAY_4(  S, I) \
		SIMPLEOBJECTS_EXPANDARRAY_2(  S, (I)),\
		SIMPLEOBJECTS_EXPANDARRAY_2(  S, (I) + 2)
#define SIMPLEOBJECTS_EXPANDARRAY_8(  S, I) \
		SIMPLEOBJECTS_EXPANDARRAY_4(  S, (I)),\
		SIMPLEOBJECTS_EXPANDARRAY_4(  S, (I) + 4)
#define SIMPLEOBJECTS_EXPANDARRAY_16( S, I) \
		SIMPLEOBJECTS_EXPANDARRAY_8(  S, (I)),\
		SIMPLEOBJECTS_EXPANDARRAY_8(  S, (I) + 8)
#define SIMPLEOBJECTS_EXPANDARRAY_32( S, I) \
		SIMPLEOBJECTS_EXPANDARRAY_16( S, (I)),\
		SIMPLEOBJECTS_EXPANDARRAY_16( S, (I) + 16)
#define SIMPLEOBJECTS_EXPANDARRAY_64( S, I) \
		SIMPLEOBJECTS_EXPANDARRAY_32( S, (I)),\
		SIMPLEOBJECTS_EXPANDARRAY_32( S, (I) + 32)
#define SIMPLEOBJECTS_EXPANDARRAY_128(S, I) \
		SIMPLEOBJECTS_EXPANDARRAY_64( S, (I)),\
		SIMPLEOBJECTS_EXPANDARRAY_64( S, (I) + 64)
#define SIMPLEOBJECTS_EXPANDARRAY_256(S, I) \
		SIMPLEOBJECTS_EXPANDARRAY_128(S, (I)),\
		SIMPLEOBJECTS_EXPANDARRAY_128(S, (I) + 128)

#define SIMPLEOBJECTS_EXPANDARRAY(    S) \
		SIMPLEOBJECTS_EXPANDARRAY_256(S,  0 )

#define SIMPLEOBJECTS_CONST_ARRAY(S) \
	SIMPLEOBJECTS_TO_SEQUENCE(\
		SIMPLEOBJECTS_ARRAY_LEN(S),\
		std::decay<decltype(S[0])>::type,\
		SIMPLEOBJECTS_EXPANDARRAY(S)\
	)
#define SIMPLEOBJECTS_CONST_STRING(S) \
	SIMPLEOBJECTS_TO_SEQUENCE(\
		SIMPLEOBJECTS_ARRAY_LEN(S), \
		std::decay<decltype(S[0])>::type, \
		SIMPLEOBJECTS_EXPANDARRAY(S)\
	)
#define SIMOBJ_KARRAY(S) SIMPLEOBJECTS_CONST_ARRAY(S)
#define SIMOBJ_KSTR(S)   SIMPLEOBJECTS_CONST_STRING(S)
