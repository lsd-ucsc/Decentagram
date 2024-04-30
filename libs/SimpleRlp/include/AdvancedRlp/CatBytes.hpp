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
inline void PreCheckCatBytes(size_t pos, const _ListObjType& l)
{
	using namespace Internal::SimRlp::Internal::Obj;
	Internal::CheckRlpListTypeSizeEq("CAT Bytes", pos, l,
		std::pair<std::string, ObjCategory>("Bytes", ObjCategory::Bytes),
		std::pair<std::string, ObjCategory>("Bytes", ObjCategory::Bytes)
	);

	const auto& specs = l[0].AsBytes();
	if (specs.size() != 1)
	{
		throw ParseError(
			"CAT Bytes's specs bytes should be exactly 1 byte",
			pos);
	}
	if (GetCatIdFromByte(specs[0]) != CatId::Bytes)
	{
		throw ParseError(
			"The given RLP list is not in CAT Bytes",
			pos);
	}
}


template<
	typename _ListObjType,
	typename _BytesObjType>
struct TransformCatBytesImpl
{
	using RetType = _BytesObjType;


	RetType operator()(size_t pos, _ListObjType&& l)
	{
		PreCheckCatBytes(pos, l);

		const auto& rawDataObj = l[1].AsBytes();

		return RetType(
			rawDataObj.data(),
			rawDataObj.data() + rawDataObj.size());
	}
}; // struct TransformCatBytesImpl


using TransformCatBytes = TransformCatBytesImpl<
	Internal::SimRlp::ListObjType,
	Internal::SimRlp::BytesObjType>;


using CatBytesParser = PrimitiveParserBase<TransformCatBytes>;


// ====================
// Writers
// ====================


template<
	typename _InObjType,
	typename _OutCtnType,
	typename _RlpBytesWriter>
struct CatBytesWriterImpl
{
	using Self = CatBytesWriterImpl<_InObjType, _OutCtnType, _RlpBytesWriter>;

	using RlpBytesWriter  = _RlpBytesWriter;

	using Concatenator = Internal::SimRlp::OutContainerConcat<_OutCtnType>;


	inline static _OutCtnType Write(const _InObjType& inBytes)
	{
		Concatenator ccntr;
		_OutCtnType outBytes;

		// 1.specs
		outBytes.push_back(SerializeCatId(CatId::Bytes));

		// 2.raw data
		auto rawData = RlpBytesWriter::Write(inBytes);
		ccntr(outBytes, rawData);

		// 3.build RLP list
		return Internal::SimRlp::
			SerializeBytes<Internal::SimRlp::RlpEncTypeCat::List>(
				outBytes,
				ccntr
			);
	}

}; // struct CatBytesWriterImpl


using CatBytesWriter =
	CatBytesWriterImpl<
		Internal::SimRlp::Internal::Obj::BytesBaseObj,
		Internal::SimRlp::OutputContainerType,
		Internal::SimRlp::WriterBytesImpl<
			Internal::SimRlp::OutputContainerType> >;


} // namespace AdvancedRlp
