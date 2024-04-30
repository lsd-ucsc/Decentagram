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


template<
	typename _ContainerType,
	typename _ByteValType,
	typename _ParserTp,
	typename _BytesParser,
	typename _FallbackValParse,
	bool _AllowMissingItem,
	bool _AllowExtraItem,
	typename _RetType>
class CatStaticDictParserImpl :
	public Internal::SimRlp::StaticDictParserImpl<
		_ContainerType,
		_ByteValType,
		_ParserTp,
		_FallbackValParse,
		_AllowMissingItem,
		_AllowExtraItem,
		_RetType>
{
public: // static members:

	using Self =
		CatStaticDictParserImpl<
			_ContainerType,
			_ByteValType,
			_ParserTp,
			_BytesParser,
			_FallbackValParse,
			_AllowMissingItem,
			_AllowExtraItem,
			_RetType>;
	using Base = Internal::SimRlp::StaticDictParserImpl<
			_ContainerType,
			_ByteValType,
			_ParserTp,
			_FallbackValParse,
			_AllowMissingItem,
			_AllowExtraItem,
			_RetType>;

	using BytesParser     = _BytesParser;
	using InputByteType   = _ByteValType;
	using RetType         = _RetType;
	using ParserTuple     = _ParserTp;

	using TupleCore        = typename RetType::TupleCore;

public:

	using Base::Base;

	// LCOV_EXCL_START
	virtual ~CatStaticDictParserImpl() = default;
	// LCOV_EXCL_STOP

	using Base::Parse;

	virtual RetType Parse(
		Internal::SimRlp::InputStateMachineIf<InputByteType>& ism,
		Internal::SimRlp::RlpEncodeType rlpType,
		InputByteType rlpVal,
		size_t& byteLeft) const override
	{
		size_t size = Base::ProcRlpListHeader(ism, rlpType, rlpVal, byteLeft);

		Base::CheckByteLeft(byteLeft, size, ism.GetBytesCount());

		// parse spec bytes
		auto specs = BytesParser().Parse(ism, size);
		PreCheckSpecs(ism.GetBytesCount(), specs);

		return Base::ProcDictItems(ism, size);
	}

private:

	static void PreCheckSpecs(
		size_t pos,
		const typename BytesParser::RetType& specs)
	{
		if (specs.size() != 1)
		{
			throw ParseError(
				"CAT StaticDict's specs bytes should be exactly 1 byte",
				pos);
		}
		if (GetCatIdFromByte(specs[0]) != CatId::StaticDict)
		{
			throw ParseError(
				"The given RLP list is not in CAT StaticDict",
				pos);
		}
	}

}; // class CatStaticDictParserImpl


template<
	typename _ParserTp,
	bool _AllowMissingItem,
	bool _AllowExtraItem,
	typename _StaticDictType = Internal::SimRlp::AutoPlaceholder>
using CatStaticDictParserT = CatStaticDictParserImpl<
	Internal::SimRlp::InputContainerType,
	Internal::SimRlp::ByteValType,
	_ParserTp,
	Internal::SimRlp::BytesParser,
	Internal::SimRlp::GeneralParser,
	_AllowMissingItem,
	_AllowExtraItem,
	Internal::SimRlp::StaticDictPickRetType<_ParserTp, _StaticDictType>
	>;


// ====================
// Writers
// ====================


template<
	typename _InStaticDictObjType,
	typename _GenericWriter,
	typename _OutCtnType>
struct CatStaticDictWriterImpl
{
	using Self = CatArrayWriterImpl<
		_InStaticDictObjType,
		_GenericWriter,
		_OutCtnType>;

	using GenericWriter = _GenericWriter;

	using Concatenator = Internal::SimRlp::OutContainerConcat<_OutCtnType>;


	inline static _OutCtnType Write(const _InStaticDictObjType& val)
	{
		Concatenator ccntr;
		_OutCtnType outBytes;

		// 1.specs
		auto catId = SerializeCatId(CatId::StaticDict);
		outBytes.push_back(catId);

		// 2.items
		auto itEnd = val.end();
		for (auto it = val.begin(); it != val.end(); ++it)
		{
			ccntr(outBytes, GenericWriter::Write(it->second.get()));
		}

		// 3.build RLP list
		return Internal::SimRlp::
			SerializeBytes<Internal::SimRlp::RlpEncTypeCat::List>(
				outBytes,
				ccntr
			);
	}

}; // struct CatStaticDictWriterImpl


template<typename _GenericWriter>
using CatStaticDictWriterT =
	CatStaticDictWriterImpl<
		Internal::SimRlp::Internal::Obj::StaticDictBaseObj,
		_GenericWriter,
		Internal::SimRlp::OutputContainerType>;


} // namespace AdvancedRlp
