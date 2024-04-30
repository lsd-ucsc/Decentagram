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

template<
	typename _ListObjType,
	typename _BytesTransform,
	typename _NullTransform,
	typename _FalseTransform,
	typename _TrueTransform,
	typename _IntegerTransform,
	typename _FloatTransform, // not implemented yet
	typename _StringTransform,
	typename _ArrayTransform,
	typename _DictTransform,
	typename _RetType>
struct TransformGenericImpl
{

	using Self = TransformGenericImpl<
			_ListObjType,
			_BytesTransform,
			_NullTransform,
			_FalseTransform,
			_TrueTransform,
			_IntegerTransform,
			_FloatTransform,
			_StringTransform,
			_ArrayTransform,
			_DictTransform,
			_RetType>;

	using BytesTransform     = _BytesTransform;
	using NullTransform      = _NullTransform;
	using FalseTransform     = _FalseTransform;
	using TrueTransform      = _TrueTransform;
	using IntegerTransform   = _IntegerTransform;
	using FloatTransform     = _FloatTransform; // not implemented yet
	using StringTransform    = _StringTransform;
	using ArrayTransform     = _ArrayTransform;
	using DictTransform      = _DictTransform;
	using RetType            = _RetType;

	RetType operator()(size_t pos, _ListObjType&& l)
	{
		using namespace Internal::SimRlp::Internal::Obj;
		Internal::CheckRlpListTypeSizeLe("AdvancedRlp", pos, l,
			std::pair<std::string, ObjCategory>("Bytes", ObjCategory::Bytes)
		);

		const auto& specs = l[0].AsBytes();
		const CatId cid = GetCatIdFromByte(specs[0]);
		switch (cid)
		{
		case CatId::Bytes:
			return RetType(BytesTransform()(pos, std::forward<_ListObjType>(l)));

		case CatId::Null:
			return RetType(NullTransform()(pos, std::forward<_ListObjType>(l)));

		case CatId::False:
			return RetType(FalseTransform()(pos, std::forward<_ListObjType>(l)));

		case CatId::True:
			return RetType(TrueTransform()(pos, std::forward<_ListObjType>(l)));

		case CatId::Integer:
			return RetType(IntegerTransform()(pos, std::forward<_ListObjType>(l)));

		// case CatId::Float:
		// 	return RetType(FloatTransform()(pos, std::forward<_ListObjType>(l)));

		case CatId::String:
			return RetType(StringTransform()(pos, std::forward<_ListObjType>(l)));

		case CatId::Array:
			return RetType(ArrayTransform()(pos, std::forward<_ListObjType>(l)));

		case CatId::Dict:
			return RetType(DictTransform()(pos, std::forward<_ListObjType>(l)));

		// This error should be throw by `GetCatIdFromByte` already
		// LCOV_EXCL_START
		default:
		{
			std::string errMsg = "Unsupported CAT ID value - ";
			Internal::SimRlp::Internal::Obj::Internal::
			ByteToHEX<true, std::string::value_type>(
				std::back_inserter(errMsg), specs[0]);
			throw ParseError(errMsg, pos);
		}
		// LCOV_EXCL_STOP
		}

	}

}; // struct TransformGenericImpl


} // namespace AdvancedRlp
