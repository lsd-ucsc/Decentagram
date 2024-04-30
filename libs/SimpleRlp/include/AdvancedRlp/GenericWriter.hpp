// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "WriterUtils.hpp"

#ifndef ADVANCEDRLP_CUSTOMIZED_NAMESPACE
namespace AdvancedRlp
#else
namespace ADVANCEDRLP_CUSTOMIZED_NAMESPACE
#endif
{


template<
	typename _RealNumObjType,
	typename _BooleanWriter,
	typename _IntegerWriter,
	typename _FloatWriter, // not implemented yet,
	typename _OutCtnType>
struct GenericRealNumWriterImpl
{
	using Self = GenericRealNumWriterImpl<
		_RealNumObjType,
		_BooleanWriter,
		_IntegerWriter,
		_FloatWriter,
		_OutCtnType>;

	using BooleanWriter = _BooleanWriter;
	using IntegerWriter = _IntegerWriter;
	using FloatWriter   = _FloatWriter;


	inline static _OutCtnType Write(const _RealNumObjType& val)
	{
		switch (val.GetNumType())
		{
		case Internal::SimRlp::Internal::Obj::RealNumType::Bool:
			return BooleanWriter::Write(val);

		case Internal::SimRlp::Internal::Obj::RealNumType::Int8:
		case Internal::SimRlp::Internal::Obj::RealNumType::Int16:
		case Internal::SimRlp::Internal::Obj::RealNumType::Int32:
		case Internal::SimRlp::Internal::Obj::RealNumType::Int64:
		case Internal::SimRlp::Internal::Obj::RealNumType::UInt8:
		case Internal::SimRlp::Internal::Obj::RealNumType::UInt16:
		case Internal::SimRlp::Internal::Obj::RealNumType::UInt32:
		case Internal::SimRlp::Internal::Obj::RealNumType::UInt64:
			return IntegerWriter::Write(val);

		default:
			throw SerializeTypeError(
				val.GetNumTypeName(), "GenericRealNumWriter");
		}
	}

}; // struct GenericRealNumWriterImpl


template<
	typename _ObjType,
	typename _BytesWriter,
	typename _NullWriter,
	typename _RealNumWriter,
	typename _StringWriter,
	template<typename> class _ArrayWriter,
	template<typename> class _DictWriter,
	template<typename> class _StaticDictWriter,
	typename _OutCtnType>
struct GenericWriterImpl
{
	using Self = GenericWriterImpl<
		_ObjType,
		_BytesWriter,
		_NullWriter,
		_RealNumWriter,
		_StringWriter,
		_ArrayWriter,
		_DictWriter,
		_StaticDictWriter,
		_OutCtnType>;

	using BytesWriter   = _BytesWriter;
	using NullWriter    = _NullWriter;
	using RealNumWriter = _RealNumWriter;
	using StringWriter  = _StringWriter;

	using ArrayWriter       = _ArrayWriter<Self>;
	using DictWriter        = _DictWriter<Self>;
	using StaticDictWriter  = _StaticDictWriter<Self>;


	inline static _OutCtnType Write(const _ObjType& val)
	{
		using namespace Internal::SimRlp::Internal;

		switch (val.GetCategory())
		{
		case Obj::ObjCategory::Bytes:
			return BytesWriter::Write(val.AsBytes());

		case Obj::ObjCategory::Null:
			return NullWriter::Write();

		case Obj::ObjCategory::Bool:
		case Obj::ObjCategory::Integer:
		case Obj::ObjCategory::Real:
			return RealNumWriter::Write(val.AsRealNum());

		case Obj::ObjCategory::String:
			return StringWriter::Write(val.AsString());

		case Obj::ObjCategory::List:
			return ArrayWriter::Write(val.AsList());

		case Obj::ObjCategory::Dict:
			return DictWriter::Write(val.AsDict());

		case Obj::ObjCategory::StaticDict:
			return StaticDictWriter::Write(val.AsStaticDict());

		default:
			throw SerializeTypeError(val.GetCategoryName(), "GenericWriter");
		}
	}

}; // struct GenericWriterImpl


} // namespace AdvancedRlp
