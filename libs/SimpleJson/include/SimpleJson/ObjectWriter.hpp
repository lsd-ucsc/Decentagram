// Copyright (c) 2022 SimpleJson
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "Internal/SimpleObjects.hpp"

#include "Exceptions.hpp"
#include "WriterConfig.hpp"

#ifndef SIMPLEJSON_CUSTOMIZED_NAMESPACE
namespace SimpleJson
#else
namespace SIMPLEJSON_CUSTOMIZED_NAMESPACE
#endif
{

template<
	typename _NullWriter,
	typename _NumWriter,
	typename _StrWriter,
	typename _ToStringType,
	typename _ContainerType>
struct JsonWriterKeyImpl
{

	using NullWriter = _NullWriter;
	using NumWriter = _NumWriter;
	using StrWriter = _StrWriter;

	template<typename _OutputIt>
	inline static void Write(_OutputIt destIt,
		const Internal::Obj::HashableBaseObject<_ToStringType>& obj,
		const WriterConfig& config,
		const WriterStates& state)
	{
		switch(obj.GetCategory())
		{
		case Internal::Obj::ObjCategory::Null:
			*destIt++ = '\"';
			NullWriter::Write(destIt, config, state);
			*destIt++ = '\"';
			break;
		case Internal::Obj::ObjCategory::Bool:
		case Internal::Obj::ObjCategory::Integer:
		case Internal::Obj::ObjCategory::Real:
			*destIt++ = '\"';
			NumWriter::Write(destIt, obj.AsRealNum(), config, state);
			*destIt++ = '\"';
			break;
		case Internal::Obj::ObjCategory::String:
			StrWriter::Write(destIt, obj.AsString(), config, state);
			break;
		default:
			throw SerializeTypeError(obj.GetCategoryName());
		}
	}

}; // struct JsonWriterKeyImpl

template<
	typename _NullWriter,
	typename _NumWriter,
	typename _StrWriter,
	typename _KeyWriter,
	typename _ToStringType,
	typename _ContainerType>
struct JsonWriterObjectImpl
{

	using Self =
		JsonWriterObjectImpl<
			_NullWriter,
			_NumWriter,
			_StrWriter,
			_KeyWriter,
			_ToStringType,
			_ContainerType>;

	using NullWriter = _NullWriter;
	using NumWriter  = _NumWriter;
	using StrWriter  = _StrWriter;
	using ListWriter =
		JsonWriterListImpl<Self, _ToStringType, _ContainerType>;
	using KeyWriter  = _KeyWriter;
	using DictWriter =
		JsonWriterDictImpl<KeyWriter, Self, _ToStringType, _ContainerType>;
	using OrdDictWriter =
		JsonWriterOrdDictImpl<KeyWriter, Self, _ToStringType>;
	using StaticDictWriter =
		JsonWriterStaticDictImpl<KeyWriter, Self, _ToStringType, _ContainerType>;

	template<typename _OutputIt>
	inline static void Write(_OutputIt destIt,
		const Internal::Obj::BaseObject<_ToStringType>& obj,
		const WriterConfig& config,
		const WriterStates& state)
	{
		switch(obj.GetCategory())
		{
		case Internal::Obj::ObjCategory::Null:
			NullWriter::Write(destIt, config, state);
			break;
		case Internal::Obj::ObjCategory::Bool:
		case Internal::Obj::ObjCategory::Integer:
		case Internal::Obj::ObjCategory::Real:
			NumWriter::Write(destIt, obj.AsRealNum(), config, state);
			break;
		case Internal::Obj::ObjCategory::String:
			StrWriter::Write(destIt, obj.AsString(), config, state);
			break;
		case Internal::Obj::ObjCategory::List:
			ListWriter::Write(destIt, obj.AsList(), config, state);
			break;
		case Internal::Obj::ObjCategory::Dict:
			if (config.m_orderDict)
			{
				OrdDictWriter::Write(destIt, obj.AsDict(), config, state);
			}
			else
			{
				DictWriter::Write(destIt, obj.AsDict(), config, state);
			}
			break;
		case Internal::Obj::ObjCategory::StaticDict:
			StaticDictWriter::Write(destIt, obj.AsStaticDict(), config, state);
			break;
		default:
			throw SerializeTypeError(obj.GetCategoryName());
		}
	}

}; // struct JsonWriterObjectImpl

} // namespace SimpleJson
