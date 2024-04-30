// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "ParserUtils.hpp"

#ifndef ADVANCEDRLP_CUSTOMIZED_NAMESPACE
namespace AdvancedRlp
#else
namespace ADVANCEDRLP_CUSTOMIZED_NAMESPACE
#endif
{


// ====================
// Parsers
// ====================


template<typename _ListObjType>
inline bool PreCheckCatBoolean(size_t pos, const _ListObjType& l)
{
	using namespace Internal::SimRlp::Internal::Obj;
	Internal::CheckRlpListTypeSizeEq(
		"CAT True/False", pos, l,
		std::pair<std::string, ObjCategory>("Bytes", ObjCategory::Bytes)
	);

	const auto& specs = l[0].AsBytes();
	if (specs.size() != 1)
	{
		throw ParseError(
			"CAT True/False's specs bytes should be exactly 1 byte",
			pos);
	}

	auto catId = GetCatIdFromByte(specs[0]);
	if ((catId != CatId::True) && (catId != CatId::False))
	{
		throw ParseError(
			"The given RLP list is not in CAT True/False",
			pos);
	}

	return (catId == CatId::True);
}


template<
	typename _ListObjType,
	typename _BoolObjType>
struct TransformCatBooleanImpl
{
	using RetType = _BoolObjType;


	RetType operator()(size_t pos, _ListObjType&& l)
	{
		auto res = PreCheckCatBoolean(pos, l);

		return RetType(res);
	}
}; // struct TransformCatBooleanImpl


using TransformCatBoolean = TransformCatBooleanImpl<
	Internal::SimRlp::ListObjType,
	Internal::SimRlp::Internal::Obj::Bool>;

using TransformCatFalse = TransformCatBoolean;
using TransformCatTrue  = TransformCatBoolean;


using CatBooleanParser = PrimitiveParserBase<TransformCatBoolean>;

using CatFalseParser = CatBooleanParser;
using CatTrueParser = CatBooleanParser;


// ====================
// Writers
// ====================


template<
	typename _InObjType,
	typename _OutCtnType>
struct CatBooleanWriterImpl
{
	using Self = CatBooleanWriterImpl<_InObjType, _OutCtnType>;

	using Concatenator = Internal::SimRlp::OutContainerConcat<_OutCtnType>;


	inline static _OutCtnType Write(const _InObjType& val)
	{
		Concatenator ccntr;
		_OutCtnType outBytes;

		// 1.specs
		auto catId = val.IsTrue() ?
			SerializeCatId(CatId::True) :
			SerializeCatId(CatId::False);
		outBytes.push_back(catId);

		// 2.raw data
		// N/A

		// 3.build RLP list
		return Internal::SimRlp::
			SerializeBytes<Internal::SimRlp::RlpEncTypeCat::List>(
				outBytes,
				ccntr
			);
	}

}; // struct CatBooleanWriterImpl


using CatBooleanWriter =
	CatBooleanWriterImpl<
		Internal::SimRlp::Internal::Obj::RealNumBaseObj,
		Internal::SimRlp::OutputContainerType>;

} // namespace AdvancedRlp
