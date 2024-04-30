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
inline void PreCheckCatNull(size_t pos, const _ListObjType& l)
{
	using namespace Internal::SimRlp::Internal::Obj;
	Internal::CheckRlpListTypeSizeEq("CAT Null", pos, l,
		std::pair<std::string, ObjCategory>("Bytes", ObjCategory::Bytes)
	);

	const auto& specs = l[0].AsBytes();
	if (specs.size() != 1)
	{
		throw ParseError(
			"CAT Null's specs bytes should be exactly 1 byte",
			pos);
	}
	if (GetCatIdFromByte(specs[0]) != CatId::Null)
	{
		throw ParseError(
			"The given RLP list is not in CAT Null",
			pos);
	}
}


template<
	typename _ListObjType,
	typename _NullObjType>
struct TransformCatNullImpl
{
	using RetType = _NullObjType;


	RetType operator()(size_t pos, _ListObjType&& l)
	{
		PreCheckCatNull(pos, l);

		return RetType();
	}
}; // struct TransformCatNullImpl


using TransformCatNull = TransformCatNullImpl<
	Internal::SimRlp::ListObjType,
	Internal::SimRlp::Internal::Obj::Null>;


using CatNullParser = PrimitiveParserBase<TransformCatNull>;


// ====================
// Writers
// ====================


template<
	typename _OutCtnType>
struct CatNullWriterImpl
{
	using Self = CatNullWriterImpl<_OutCtnType>;

	using Concatenator = Internal::SimRlp::OutContainerConcat<_OutCtnType>;


	inline static _OutCtnType Write()
	{
		Concatenator ccntr;
		_OutCtnType outBytes;

		// 1.specs
		outBytes.push_back(SerializeCatId(CatId::Null));

		// 2.raw data
		// N/A

		// 3.build RLP list
		return Internal::SimRlp::
			SerializeBytes<Internal::SimRlp::RlpEncTypeCat::List>(
				outBytes,
				ccntr
			);
	}

}; // struct CatNullWriterImpl


using CatNullWriter =
	CatNullWriterImpl<
		Internal::SimRlp::OutputContainerType>;


} // namespace AdvancedRlp
