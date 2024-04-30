// Copyright (c) 2022 SimpleJson
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#pragma once


#include "DefaultTypes.hpp"

#ifndef SIMPLEJSON_CUSTOMIZED_NAMESPACE
namespace SimpleJson
#else
namespace SIMPLEJSON_CUSTOMIZED_NAMESPACE
#endif
{

inline static Internal::Obj::Object LoadStr(const IMContainerType& str)
{
	GenericObjectParser parser;
	return parser.ParseTillEnd(str);
}

template<typename _ObjType>
struct FindObjWriter;

template<>
struct FindObjWriter<Internal::Obj::Object>
{
	using type = JsonWriterObject;
}; // struct FindObjWriter

template<>
struct FindObjWriter<Internal::Obj::BaseObj> :
	public FindObjWriter<Internal::Obj::Object>
{}; // struct FindObjWriter

template<>
struct FindObjWriter<Internal::Obj::HashableObject> :
	public FindObjWriter<Internal::Obj::Object>
{}; // struct FindObjWriter

template<>
struct FindObjWriter<Internal::Obj::HashableBaseObj> :
	public FindObjWriter<Internal::Obj::Object>
{}; // struct FindObjWriter

template<>
struct FindObjWriter<Internal::Obj::Null>
{
	using type = JsonWriterNull;
}; // struct FindObjWriter

template<>
struct FindObjWriter<Internal::Obj::Bool>
{
	using type = JsonWriterRealNum;
}; // struct FindObjWriter

template<>
struct FindObjWriter<Internal::Obj::Int64> :
	public FindObjWriter<Internal::Obj::Bool>
{}; // struct FindObjWriter

template<>
struct FindObjWriter<Internal::Obj::Double> :
	public FindObjWriter<Internal::Obj::Bool>
{}; // struct FindObjWriter

template<>
struct FindObjWriter<Internal::Obj::String>
{
	using type = JsonWriterString;
}; // struct FindObjWriter

template<>
struct FindObjWriter<Internal::Obj::List>
{
	using type = JsonWriterListT<JsonWriterObject>;
}; // struct FindObjWriter

template<>
struct FindObjWriter<Internal::Obj::Dict>
{
	using type = JsonWriterDictT<JsonWriterKey, JsonWriterObject>;
}; // struct FindObjWriter

template<
	typename _ObjType,
	typename _WriterType = typename FindObjWriter<_ObjType>::type>
inline static ToStringType DumpStr(
	const _ObjType& obj,
	WriterConfig config = WriterConfig())
{
	ToStringType res;
	_WriterType::Write(
		std::back_inserter(res),
		obj,
		config,
		WriterStates()
	);
	return res;
}

} // namespace SimpleJson
