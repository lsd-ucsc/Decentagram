// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "Internal/SimpleUtf.hpp"

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
inline void PreCheckCatString(size_t pos, const _ListObjType& l)
{
	using namespace Internal::SimRlp::Internal::Obj;
	Internal::CheckRlpListTypeSizeEq("CAT String", pos, l,
		std::pair<std::string, ObjCategory>("Bytes", ObjCategory::Bytes),
		std::pair<std::string, ObjCategory>("Bytes", ObjCategory::Bytes)
	);

	const auto& specs = l[0].AsBytes();
	if (specs.size() != 2)
	{
		throw ParseError(
			"CAT String's specs bytes should be exactly 2 byte",
			pos);
	}
	if (GetCatIdFromByte(specs[0]) != CatId::String)
	{
		throw ParseError(
			"The given RLP list is not in CAT String",
			pos);
	}
}


namespace Internal
{
	template<typename _StrObjType>
	struct StrReadRawDataUtf8
	{

		_StrObjType operator()(
			size_t,
			const uint8_t* begin, const uint8_t* end)
		{
			_StrObjType res;
			auto dest = std::back_inserter(res);

			using InputIt = decltype(begin);
			using OutputIt = decltype(dest);

			Internal::Utf::UtfConvert(
				Internal::Utf::Utf8ToCodePtOnce<InputIt>,
				Internal::Utf::CodePtToUtf8Once<OutputIt>,
				begin, end, dest);

			return res;
		// LCOV_EXCL_START
		}
		// LCOV_EXCL_STOP

	}; // struct StrReadRawDataUtf8
} // namespace Internal


template<
	typename _ListObjType,
	typename _StrObjType>
struct TransformCatStringImpl
{
	using RetType = _StrObjType;


	RetType operator()(size_t pos, _ListObjType&& l)
	{
		PreCheckCatString(pos, l);

		// 1. char width
		const auto& specs = l[0].AsBytes();
		auto chWidth = specs[1];

		// 2. raw data
		const auto& rawData = l[1].AsBytes();
		const uint8_t* rawDataBegin = rawData.data();
		const uint8_t* rawDataEnd = rawData.data() + rawData.size();

		switch (chWidth)
		{
		case 0x00U:
			return Internal::StrReadRawDataUtf8<RetType>()(
				pos,
				rawDataBegin, rawDataEnd);

		default:
		{
			std::string chWidthHex;
			Internal::SimRlp::Internal::Obj::Internal::
				ByteToHEX<true, std::string::value_type>(
					std::back_inserter(chWidthHex), chWidth);

			throw ParseError(
				"The given char width value is unknown - " + chWidthHex, pos);
		}
		}
	}
}; // struct TransformCatStringImpl


using TransformCatString = TransformCatStringImpl<
		Internal::SimRlp::ListObjType,
		Internal::SimRlp::Internal::Obj::String>;


using CatStringParser = PrimitiveParserBase<TransformCatString>;


// ====================
// Writers
// ====================


template<
	typename _InObjType,
	typename _OutCtnType>
struct CatStringWriterImpl
{
	using Self = CatStringWriterImpl<_InObjType, _OutCtnType>;

	using Concatenator = Internal::SimRlp::OutContainerConcat<_OutCtnType>;


	inline static _OutCtnType Write(const _InObjType& val)
	{
		using _CharType = typename _InObjType::value_type;
		size_t chWidth = sizeof(_CharType);

		Concatenator ccntr;
		_OutCtnType specs;
		_OutCtnType rawData;

		// 1.specs
		specs.push_back(SerializeCatId(CatId::String));
		specs.push_back(0x00U);
		auto& chWidthByte = specs[1];

		// 2.raw data
		switch (chWidth)
		{
		case 1:
		{
			// 1-byte - assume to be UTF-8
			chWidthByte = 0x00U;

			auto begin = val.cbegin();
			auto end = val.cend();
			auto dest = std::back_inserter(rawData);

			using InputIt = decltype(begin);
			using OutputIt = decltype(dest);

			Internal::Utf::UtfConvert(
				Internal::Utf::Utf8ToCodePtOnce<InputIt>,
				Internal::Utf::CodePtToUtf8Once<OutputIt>,
				begin, end, dest);
			break;
		}

		// LCOV_EXCL_START
		default:
			throw SerializeTypeError(
				std::string(val.GetCategoryName()) + " that is not UTF-8",
				"CatStringWriter");
		// LCOV_EXCL_STOP
		}

		// 3.build RLP list
		specs = Internal::SimRlp::
			SerializeBytes<Internal::SimRlp::RlpEncTypeCat::Bytes>(
				specs,
				ccntr
			);
		rawData = Internal::SimRlp::
			SerializeBytes<Internal::SimRlp::RlpEncTypeCat::Bytes>(
				rawData,
				ccntr
			);
		ccntr(specs, rawData);

		return Internal::SimRlp::
			SerializeBytes<Internal::SimRlp::RlpEncTypeCat::List>(
				specs,
				ccntr
			);
	}

}; // struct CatStringWriterImpl


using CatStringWriter =
	CatStringWriterImpl<
		Internal::SimRlp::Internal::Obj::StringBaseObj,
		Internal::SimRlp::OutputContainerType>;


} // namespace AdvancedRlp
