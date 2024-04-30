// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "CatBytes.hpp"
#include "CatNull.hpp"
#include "CatBoolean.hpp"
#include "CatInteger.hpp"
#include "CatString.hpp"
#include "CatArray.hpp"
#include "CatDict.hpp"
#include "CatStaticDict.hpp"
#include "GenericTransform.hpp"
#include "GenericWriter.hpp"

#ifndef ADVANCEDRLP_CUSTOMIZED_NAMESPACE
namespace AdvancedRlp
#else
namespace ADVANCEDRLP_CUSTOMIZED_NAMESPACE
#endif
{

// ====================
// Parsers
// ====================


using TransformGeneric = TransformGenericImpl<
	Internal::SimRlp::ListObjType,
	TransformCatBytes,
	TransformCatNull,
	TransformCatFalse,
	TransformCatTrue,
	TransformCatInteger,
	TransformCatNull, // Float - no in use
	TransformCatString,
	TransformCatArray,
	TransformCatDict,
	Internal::SimRlp::Internal::Obj::Object>;


template<typename _ContainerType>
using GenericParserT = CompositeParserBase<
	_ContainerType,
	TransformGeneric,
	Internal::SimRlp::SelfParserPlaceholder>;


using GenericParser = GenericParserT<Internal::SimRlp::InputContainerType>;


using CatArrayParser = CompositeParserBase<
	Internal::SimRlp::InputContainerType,
	TransformCatArray,
	GenericParser>;


using CatDictParser = CompositeParserBase<
	Internal::SimRlp::InputContainerType,
	TransformCatDict,
	GenericParser>;


template<typename _ContainerType>
inline typename GenericParserT<_ContainerType>::RetType Parse(
	const _ContainerType& container
)
{
	return GenericParserT<_ContainerType>().Parse(container);
}


// ====================
// Writers
// ====================


using GenericRealNumWriter =
	GenericRealNumWriterImpl<
		Internal::SimRlp::Internal::Obj::RealNumBaseObj,
		CatBooleanWriter,
		CatIntegerWriter,
		CatIntegerWriter,
		Internal::SimRlp::OutputContainerType>;


using GenericWriter =
	GenericWriterImpl<
		Internal::SimRlp::Internal::Obj::BaseObj,
		CatBytesWriter,
		CatNullWriter,
		GenericRealNumWriter,
		CatStringWriter,
		CatArrayWriterT,
		CatDictWriterT,
		CatStaticDictWriterT,
		Internal::SimRlp::OutputContainerType>;


}
