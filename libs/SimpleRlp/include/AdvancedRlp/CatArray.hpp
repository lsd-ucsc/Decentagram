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
inline void PreCheckCatArray(size_t pos, const _ListObjType& l)
{
	using namespace Internal::SimRlp::Internal::Obj;
	Internal::CheckRlpListTypeSizeLe("CAT Array", pos, l,
		std::pair<std::string, ObjCategory>("Bytes", ObjCategory::Bytes)
	);

	const auto& specs = l[0].AsBytes();
	if (specs.size() != 1)
	{
		throw ParseError(
			"CAT Array's specs bytes should be exactly 1 byte",
			pos);
	}
	if (GetCatIdFromByte(specs[0]) != CatId::Array)
	{
		throw ParseError(
			"The given RLP list is not in CAT Array",
			pos);
	}
}


template<
	typename _ListObjType,
	typename _ArrayObjType>
struct TransformCatArrayImpl
{
	using RetType        = _ArrayObjType;
	using ArrayObjType   = _ArrayObjType;


	RetType operator()(size_t pos, _ListObjType&& l)
	{
		PreCheckCatArray(pos, l);

		ArrayObjType res;

		for (size_t i = 1; i < l.size(); ++i)
		{
			res.push_back(std::move(l[i]));
		}

		return res;
	}
}; // struct TransformCatString


using TransformCatArray = TransformCatArrayImpl<
	Internal::SimRlp::ListObjType,
	Internal::SimRlp::Internal::Obj::List>;


// ====================
// Writers
// ====================


template<
	typename _InArrayObjType,
	typename _GenericWriter,
	typename _OutCtnType>
struct CatArrayWriterImpl
{
	using Self = CatArrayWriterImpl<
		_InArrayObjType,
		_GenericWriter,
		_OutCtnType>;

	using GenericWriter = _GenericWriter;

	using Concatenator = Internal::SimRlp::OutContainerConcat<_OutCtnType>;


	inline static _OutCtnType Write(const _InArrayObjType& val)
	{
		Concatenator ccntr;
		_OutCtnType outBytes;

		// 1.specs
		auto catId = SerializeCatId(CatId::Array);
		outBytes.push_back(catId);

		// 2.items
		auto itEnd = val.end();
		for (auto it = val.begin(); it != itEnd; ++it)
		{
			ccntr(outBytes, GenericWriter::Write(*it));
		}

		// 3.build RLP list
		return Internal::SimRlp::
			SerializeBytes<Internal::SimRlp::RlpEncTypeCat::List>(
				outBytes,
				ccntr
			);
	}

}; // struct CatArrayWriterImpl


template<typename _GenericWriter>
using CatArrayWriterT =
	CatArrayWriterImpl<
		Internal::SimRlp::Internal::Obj::ListBaseObj,
		_GenericWriter,
		Internal::SimRlp::OutputContainerType>;


} // namespace AdvancedRlp
