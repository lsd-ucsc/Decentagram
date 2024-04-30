// Copyright (c) 2022 SimpleJson
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <string>

#include "NullParser.hpp"
#include "BoolParser.hpp"
#include "StringParser.hpp"
#include "RealNumParser.hpp"
#include "ListParser.hpp"
#include "DictParser.hpp"
#include "StaticDictParser.hpp"

#include "GenericObjectParser.hpp"

#include "NullWriter.hpp"
#include "RealNumWriter.hpp"
#include "StringWriter.hpp"
#include "ListWriter.hpp"
#include "DictWriter.hpp"
#include "ObjectWriter.hpp"

#ifndef SIMPLEJSON_CUSTOMIZED_NAMESPACE
namespace SimpleJson
#else
namespace SIMPLEJSON_CUSTOMIZED_NAMESPACE
#endif
{

using IMContainerType = std::string;
using ToStringType    = std::string;

using NullParser = NullParserImpl<IMContainerType, Internal::Obj::Null>;
using BoolParser = BoolParserImpl<IMContainerType, Internal::Obj::Bool>;
using StringParser = StringParserImpl<IMContainerType, Internal::Obj::String>;

using DictKeyParser = StringParserImpl<
	IMContainerType, Internal::Obj::String, Internal::Obj::HashableObject>;

using GenericNumberParser = GenericNumberParserImpl<
	IMContainerType,
	Internal::Obj::Int64,
	Internal::Obj::Double,
	Internal::Obj::Object>;

using IntegerParser = IntegerParserImpl<IMContainerType, Internal::Obj::Int64>;
using RealNumParser = RealNumParserImpl<IMContainerType, Internal::Obj::Double>;

template<typename _ItemParser>
using ListParserT = ListParserImpl<
	IMContainerType,
	_ItemParser,
	Internal::Obj::ListT<typename _ItemParser::RetType> >;

template<typename _ValParser>
using DictParserT = DictParserImpl<
	IMContainerType,
	DictKeyParser,
	_ValParser,
	Internal::Obj::DictT<
		typename DictKeyParser::RetType, typename _ValParser::RetType> >;

using GenericObjectParser = GenericObjectParserImpl<
	IMContainerType,
	Internal::Obj::Null,
	Internal::Obj::Bool,
	Internal::Obj::Int64,
	Internal::Obj::Double,
	Internal::Obj::String,
	Internal::Obj::HashableObject,
	Internal::Obj::ListT,
	Internal::Obj::DictT,
	Internal::Obj::Object>;

template<
	typename _ParserTp,
	bool _AllowMissingItem,
	bool _AllowExtraItem>
using StaticDictParserT = StaticDictParserImpl<
	IMContainerType,
	StringParser,
	_ParserTp,
	GenericObjectParser,
	_AllowMissingItem,
	_AllowExtraItem,
	Internal::Obj::StaticDict<
		typename Internal::DParserTuple2TupleCore<_ParserTp>::type> >;

// ===================== Writers =====================

using JsonWriterNull = JsonWriterNullImpl<ToStringType>;

using JsonWriterRealNum = JsonWriterRealNumImpl<ToStringType>;

using JsonWriterString = JsonWriterStringImpl<char, ToStringType, IMContainerType>;

template<typename _ValWriter>
using JsonWriterListT =
	JsonWriterListImpl<_ValWriter, ToStringType, IMContainerType>;

template<typename _KeyWriter, typename _ValWriter>
using JsonWriterDictT =
	JsonWriterDictImpl<_KeyWriter, _ValWriter, ToStringType, IMContainerType>;

template<typename _KeyWriter, typename _ValWriter>
using JsonWriterOrdDictT =
	JsonWriterOrdDictImpl<_KeyWriter, _ValWriter, ToStringType>;

template<typename _KeyWriter, typename _ValWriter>
using JsonWriterStaticDictT =
	JsonWriterStaticDictImpl<_KeyWriter, _ValWriter, ToStringType, IMContainerType>;

using JsonWriterKey =
	JsonWriterKeyImpl<
		JsonWriterNull,
		JsonWriterRealNum,
		JsonWriterString,
		ToStringType,
		IMContainerType>;

using JsonWriterObject =
	JsonWriterObjectImpl<
		JsonWriterNull,
		JsonWriterRealNum,
		JsonWriterString,
		JsonWriterKey,
		ToStringType,
		IMContainerType>;

} // namespace SimpleJson
