// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <limits>

#include "ParserUtils.hpp"
#include "WriterUtils.hpp"

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
inline void PreCheckCatInteger(size_t pos, const _ListObjType& l)
{
	using namespace Internal::SimRlp::Internal::Obj;
	Internal::CheckRlpListTypeSizeEq("CAT Integer", pos, l,
		std::pair<std::string, ObjCategory>("Bytes", ObjCategory::Bytes),
		std::pair<std::string, ObjCategory>("Bytes", ObjCategory::Bytes)
	);

	const auto& specs = l[0].AsBytes();
	if (specs.size() != 3)
	{
		throw ParseError(
			"CAT Integer's specs bytes should be exactly 3 bytes",
			pos);
	}
	if (GetCatIdFromByte(specs[0]) != CatId::Integer)
	{
		throw ParseError(
			"The given RLP list is not in CAT Integer",
			pos);
	}
}


namespace Internal
{

template<
	typename _UIntType,
	typename _IntType,
	typename _ValToObjConverter>
struct SolveSignedness
{
	using RetType = typename _ValToObjConverter::RetType;

	void CheckSignedRange(size_t pos, const _UIntType& val)
	{
		if (Internal::SimRlp::Internal::Obj::Internal::
			RealNumCompare<_UIntType, _IntType>::Greater(
				val,
				(std::numeric_limits<_IntType>::max)()))
		{
			throw ParseError("The integer received exceeds the range of the"
				" targeting signed integer", pos);
		}

	}

	RetType operator()(size_t pos, uint8_t sigByte, const _UIntType& val)
	{
		switch (sigByte)
		{
		case 0x00U:
			// unsigned
			return _ValToObjConverter().Convert(pos, _UIntType(val));

		case 0x10U:
			// signed
			CheckSignedRange(pos, val);
			return _ValToObjConverter().Convert(pos, _IntType(val));

		case 0x11U:
			// signed negative
			CheckSignedRange(pos, val);
			return _ValToObjConverter().Convert(pos, _IntType(-1 * _IntType(val)));

		default:
		{
			std::string errMsg = "Invalid signedness byte (";
			Internal::SimRlp::Internal::Obj::Internal::
			ByteToHEX<true, std::string::value_type>(
				std::back_inserter(errMsg), sigByte);
			errMsg += ") is given";
			throw ParseError(errMsg, pos);
		}
		}
	}

}; // struct SolveSignedness


} // namespace Internal


template<
	typename _ListObjType,
	typename _ValToObjConverter>
struct TransformCatIntegerImpl
{
	using RetType = typename _ValToObjConverter::RetType;


	RetType operator()(size_t pos, _ListObjType&& l)
	{
		static constexpr Internal::SimRlp::Endian srcEndian =
			Internal::SimRlp::Endian::little;

		PreCheckCatInteger(pos, l);

		// 1. specs
		const auto& specs = l[0].AsBytes();
		// 2. raw data
		const auto& rawData = l[1].AsBytes();
		const auto widthByte = specs[1];
		const auto sigByte = specs[2];

		switch (widthByte)
		{
		case 0x00U:
			// 1 byte
			{
				uint8_t val =
					Internal::RawToPrimitive<srcEndian>::ToInt<uint8_t>(
						rawData.data(), rawData.size());
				return Internal::
					SolveSignedness<uint8_t, int8_t, _ValToObjConverter>()(
						pos, sigByte, val);
			}

		case 0x01U:
			// 2 bytes
			{
				uint16_t val =
					Internal::RawToPrimitive<srcEndian>::ToInt<uint16_t>(
						rawData.data(), rawData.size());
				return Internal::
					SolveSignedness<uint16_t, int16_t, _ValToObjConverter>()(
						pos, sigByte, val);
			}

		case 0x02U:
			// 4 bytes
			{
				uint32_t val =
					Internal::RawToPrimitive<srcEndian>::ToInt<uint32_t>(
						rawData.data(), rawData.size());
				return Internal::
					SolveSignedness<uint32_t, int32_t, _ValToObjConverter>()(
						pos, sigByte, val);
			}

		case 0x03U:
			// 8 bytes
			{
				uint64_t val =
					Internal::RawToPrimitive<srcEndian>::ToInt<uint64_t>(
						rawData.data(), rawData.size());
				return Internal::
					SolveSignedness<uint64_t, int64_t, _ValToObjConverter>()(
						pos, sigByte, val);
			}

		default:
		{
			std::string errMsg = "Invalid integer width byte (";
			Internal::SimRlp::Internal::Obj::Internal::
			ByteToHEX<true, std::string::value_type>(
				std::back_inserter(errMsg), widthByte);
			errMsg += ") is given";
			throw ParseError(errMsg, pos);
		}
		}
	}
}; // struct TransformCatIntegerImpl


template<
	typename _ExpInType,
	typename _RetType>
struct SpecificIntConverterImpl
{
	using ExpInType = _ExpInType;
	using RetType   = _RetType;

	template<typename _OtherType>
	RetType Convert(size_t pos, const _OtherType&)
	{
		using namespace Internal::SimRlp::Internal;

		std::string errMsg = "Expecting integer type of ";
		errMsg += Obj::RealNumTraits<ExpInType>::sk_numTypeName();
		errMsg += ", while an integer in ";
		errMsg += Obj::RealNumTraits<_OtherType>::sk_numTypeName();
		errMsg += " is given";
		throw ParseError(errMsg, pos);
	}

	RetType Convert(size_t, const ExpInType& val)
	{
		return RetType(val);
	}
}; // struct SpecificIntConverterImpl


template<
	typename _PrimIntType>
struct SpecificIntConverter;

template<>
struct SpecificIntConverter<int8_t> :
	public SpecificIntConverterImpl<
		int8_t,
		Internal::SimRlp::Internal::Obj::Int8>
{}; // struct SpecificIntConverter

template<>
struct SpecificIntConverter<uint8_t> :
	public SpecificIntConverterImpl<
		uint8_t,
		Internal::SimRlp::Internal::Obj::UInt8>
{}; // struct SpecificIntConverter

template<>
struct SpecificIntConverter<int16_t> :
	public SpecificIntConverterImpl<
		int16_t,
		Internal::SimRlp::Internal::Obj::Int16>
{}; // struct SpecificIntConverter

template<>
struct SpecificIntConverter<uint16_t> :
	public SpecificIntConverterImpl<
		uint16_t,
		Internal::SimRlp::Internal::Obj::UInt16>
{}; // struct SpecificIntConverter

template<>
struct SpecificIntConverter<int32_t> :
	public SpecificIntConverterImpl<
		int32_t,
		Internal::SimRlp::Internal::Obj::Int32>
{}; // struct SpecificIntConverter

template<>
struct SpecificIntConverter<uint32_t> :
	public SpecificIntConverterImpl<
		uint32_t,
		Internal::SimRlp::Internal::Obj::UInt32>
{}; // struct SpecificIntConverter

template<>
struct SpecificIntConverter<int64_t> :
	public SpecificIntConverterImpl<
		int64_t,
		Internal::SimRlp::Internal::Obj::Int64>
{}; // struct SpecificIntConverter

template<>
struct SpecificIntConverter<uint64_t> :
	public SpecificIntConverterImpl<
		uint64_t,
		Internal::SimRlp::Internal::Obj::UInt64>
{}; // struct SpecificIntConverter


struct GenericIntConverter
{
	using Int8Converter     = SpecificIntConverter<int8_t>;
	using UInt8Converter    = SpecificIntConverter<uint8_t>;
	using Int16Converter    = SpecificIntConverter<int16_t>;
	using UInt16Converter   = SpecificIntConverter<uint16_t>;
	using Int32Converter    = SpecificIntConverter<int32_t>;
	using UInt32Converter   = SpecificIntConverter<uint32_t>;
	using Int64Converter    = SpecificIntConverter<int64_t>;
	using UInt64Converter   = SpecificIntConverter<uint64_t>;

	using RetType           = Internal::SimRlp::Internal::Obj::Object;

	template<typename _OtherType>
	RetType Convert(size_t pos, const _OtherType&)
	{
		using namespace Internal::SimRlp::Internal;

		throw ParseError("The given data type is not supported by "
			"GenericIntConverter", pos);
	}

	RetType Convert(size_t pos,
		const typename Int8Converter::ExpInType& val)
	{
		return Int8Converter().Convert(pos, val);
	}

	RetType Convert(size_t pos,
		const typename UInt8Converter::ExpInType& val)
	{
		return UInt8Converter().Convert(pos, val);
	}

	RetType Convert(size_t pos,
		const typename Int16Converter::ExpInType& val)
	{
		return Int16Converter().Convert(pos, val);
	}

	RetType Convert(size_t pos,
		const typename UInt16Converter::ExpInType& val)
	{
		return UInt16Converter().Convert(pos, val);
	}

	RetType Convert(size_t pos,
		const typename Int32Converter::ExpInType& val)
	{
		return Int32Converter().Convert(pos, val);
	}

	RetType Convert(size_t pos,
		const typename UInt32Converter::ExpInType& val)
	{
		return UInt32Converter().Convert(pos, val);
	}

	RetType Convert(size_t pos,
		const typename Int64Converter::ExpInType& val)
	{
		return Int64Converter().Convert(pos, val);
	}

	RetType Convert(size_t pos,
		const typename UInt64Converter::ExpInType& val)
	{
		return UInt64Converter().Convert(pos, val);
	}

}; // struct GenericIntConverter


template<typename _Converter>
using TransformCatIntegerT = TransformCatIntegerImpl<
	Internal::SimRlp::ListObjType,
	_Converter>;


template<typename _Converter>
using CatIntegerParserT =
	PrimitiveParserBase<TransformCatIntegerT<_Converter> >;

using TransformCatInteger = TransformCatIntegerT<GenericIntConverter>;

using CatIntegerParser = CatIntegerParserT<GenericIntConverter>;


// ====================
// Writers
// ====================


namespace Internal
{


template<typename _InputType, typename _UIntType>
struct BuildIntRawDataImpl
{
	using InputType  = _InputType;
	using UIntType   = _UIntType;

	template<typename _ByteCtn>
	void operator()(
		_ByteCtn& ctn,
		uint8_t& signByte,
		InputType val)
	{
		UIntType uval = (val >= 0) ?
			static_cast<UIntType>(val) :
			static_cast<UIntType>(-1 * val);
		signByte = (val >= 0) ? 0x10U : 0x11U;

		ctn.resize(sizeof(UIntType));
		PrimitiveToRaw<SimRlp::Endian::little>::
			FromInt(ctn.data(), ctn.size(), uval);
	}
}; // struct BuildIntRawDataImpl

template<typename _InputType>
struct BuildUIntRawDataImpl
{
	using InputType  = _InputType;

	template<typename _ByteCtn>
	void operator()(
		_ByteCtn& ctn,
		uint8_t& signByte,
		InputType val)
	{
		signByte = 0x00U;

		ctn.resize(sizeof(InputType));
		PrimitiveToRaw<SimRlp::Endian::little>::
			FromInt(ctn.data(), ctn.size(), val);
	}
}; // struct BuildUIntRawDataImpl

template<typename _IntType>
struct BuildIntRawData;

template<>
struct BuildIntRawData<int8_t> :
	public BuildIntRawDataImpl<int8_t, uint8_t>
{}; // struct BuildIntRawData<int8_t>

template<>
struct BuildIntRawData<uint8_t> :
	public BuildUIntRawDataImpl<uint8_t>
{}; // struct BuildIntRawData<uint8_t>

template<>
struct BuildIntRawData<int16_t> :
	public BuildIntRawDataImpl<int16_t, uint16_t>
{}; // struct BuildIntRawData<int16_t>

template<>
struct BuildIntRawData<uint16_t> :
	public BuildUIntRawDataImpl<uint16_t>
{}; // struct BuildIntRawData<uint16_t>

template<>
struct BuildIntRawData<int32_t> :
	public BuildIntRawDataImpl<int32_t, uint32_t>
{}; // struct BuildIntRawData<int32_t>

template<>
struct BuildIntRawData<uint32_t> :
	public BuildUIntRawDataImpl<uint32_t>
{}; // struct BuildIntRawData<uint32_t>

template<>
struct BuildIntRawData<int64_t> :
	public BuildIntRawDataImpl<int64_t, uint64_t>
{}; // struct BuildIntRawData<int64_t>

template<>
struct BuildIntRawData<uint64_t> :
	public BuildUIntRawDataImpl<uint64_t>
{}; // struct BuildIntRawData<uint64_t>

} // namespace Internal


template<
	typename _InObjType,
	typename _OutCtnType>
struct CatIntegerWriterImpl
{
	using Self = CatIntegerWriterImpl<_InObjType, _OutCtnType>;

	using Concatenator = Internal::SimRlp::OutContainerConcat<_OutCtnType>;


	inline static _OutCtnType Write(const _InObjType& val)
	{
		Concatenator ccntr;
		_OutCtnType specs;
		_OutCtnType rawData;

		// 1.specs
		specs.push_back(SerializeCatId(CatId::Integer));
		specs.push_back(0x00U);
		specs.push_back(0x00U);
		uint8_t& widthByte = specs[1];
		uint8_t& signByte = specs[2];

		// 2.raw data
		switch (val.GetNumType())
		{
		case Internal::SimRlp::Internal::Obj::RealNumType::Int8:
			widthByte = 0x00U;
			Internal::BuildIntRawData<int8_t>()(rawData, signByte, val.AsCppInt8());
			break;

		case Internal::SimRlp::Internal::Obj::RealNumType::UInt8:
			widthByte = 0x00U;
			Internal::BuildIntRawData<uint8_t>()(rawData, signByte, val.AsCppUInt8());
			break;

		case Internal::SimRlp::Internal::Obj::RealNumType::Int16:
		{
			widthByte = 0x01U;
			int16_t vali16 = static_cast<int16_t>(val.AsCppInt32());
			Internal::BuildIntRawData<int16_t>()(rawData, signByte, vali16);
			break;
		}

		case Internal::SimRlp::Internal::Obj::RealNumType::UInt16:
		{
			widthByte = 0x01U;
			int16_t valu16 = static_cast<uint16_t>(val.AsCppUInt32());
			Internal::BuildIntRawData<uint16_t>()(rawData, signByte, valu16);
			break;
		}

		case Internal::SimRlp::Internal::Obj::RealNumType::Int32:
			widthByte = 0x02U;
			Internal::BuildIntRawData<int32_t>()(rawData, signByte, val.AsCppInt32());
			break;

		case Internal::SimRlp::Internal::Obj::RealNumType::UInt32:
			widthByte = 0x02U;
			Internal::BuildIntRawData<uint32_t>()(rawData, signByte, val.AsCppUInt32());
			break;

		case Internal::SimRlp::Internal::Obj::RealNumType::Int64:
			widthByte = 0x03U;
			Internal::BuildIntRawData<int64_t>()(rawData, signByte, val.AsCppInt64());
			break;

		case Internal::SimRlp::Internal::Obj::RealNumType::UInt64:
			widthByte = 0x03U;
			Internal::BuildIntRawData<uint64_t>()(rawData, signByte, val.AsCppUInt64());
			break;

		default:
			throw SerializeTypeError(val.GetNumTypeName(), "CatIntegerWriter");
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

}; // struct CatIntegerWriterImpl


using CatIntegerWriter =
	CatIntegerWriterImpl<
		Internal::SimRlp::Internal::Obj::RealNumBaseObj,
		Internal::SimRlp::OutputContainerType>;


} // namespace AdvancedRlp
