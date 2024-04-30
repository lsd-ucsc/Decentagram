// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#include <limits>

#include "Common.hpp"
#include "Exceptions.hpp"

#ifndef SIMPLERLP_CUSTOMIZED_NAMESPACE
namespace SimpleRlp
#else
namespace SIMPLERLP_CUSTOMIZED_NAMESPACE
#endif
{

//====================
// Encoding
//====================


/**
 * @brief Encoding a primitive integer value into a RLP byte sequence.
 *
 * @tparam _IntType      The type of the input integer value
 * @tparam _EndianType   The endian type that the input integer value is in
 *                       (usually depending on the platform)
 * @tparam _IsDestSigned Is the type of each individual byte in the
 *                       *destination container* a signed value?
 *                       Currently we only support unsigned types
 *                       (e.g., array of uint8_t); thus, signed types
 *                       (e.g., array of char or int8_t) does not work
 */
template<typename _IntType, Endian _EndianType, bool _IsDestSigned>
struct EncodePrimitiveIntValue;

template<typename _IntType>
struct EncodePrimitiveIntValue<_IntType, Endian::little, false>
{
	static size_t EncodedWidth(const _IntType& inVal)
	{
		static constexpr size_t sk_inTypeSize = sizeof(_IntType);
		static constexpr size_t sk_bitsPerByte = 8;

		size_t zeroBytes = 0;
		for (size_t i = sk_inTypeSize; i > 0; --i)
		{
			if (((inVal >> ((i - 1) * sk_bitsPerByte)) & 0xFFU) == 0)
			{
				++zeroBytes;
			}
			else
			{
				break;
			}
		}

		return sk_inTypeSize - zeroBytes;
	}

	template<typename _DestIt>
	static _DestIt Encode(const _IntType& inVal, _DestIt destIt)
	{
		static constexpr size_t sk_bitsPerByte = 8;

		// Write small endian data as big endian bytes
		size_t widthNeeded = EncodedWidth(inVal);
		for (size_t i = widthNeeded; i > 0; --i)
		{
			*(destIt++) = static_cast<uint8_t>(
				(inVal >> ((i - 1) * sk_bitsPerByte)) & 0xFFU
			);
		}

		return destIt;
	}

	template<typename _CtnType>
	static void Encode(_CtnType& res, const _IntType& inVal)
	{
		Encode(inVal, std::back_inserter(res));
	}
}; // EncodePrimitiveIntValue<_IntType, Endian::little, false>

namespace Internal
{


template<Endian _EndianType, bool _IsDestSigned>
struct EncodeSizeValue
{
	static size_t EncodedWidth(size_t inSize)
	{
		return EncodePrimitiveIntValue<size_t, _EndianType, _IsDestSigned>::
			EncodedWidth(inSize);
	}

	template<typename _DestIt>
	static _DestIt Encode(size_t inSize, _DestIt destIt)
	{
		return EncodePrimitiveIntValue<size_t, _EndianType, _IsDestSigned>::
			Encode(inSize, destIt);
	}

	template<typename _CtnType>
	static void Encode(_CtnType& res, size_t inSize)
	{
		EncodePrimitiveIntValue<size_t, _EndianType, _IsDestSigned>::
			Encode(res, inSize);
	}
}; // struct EncodeSizeValue


template<RlpEncTypeCat _RlpCat, bool _IsValSigned>
struct EncodeRlpBytesImpl;

template<>
struct EncodeRlpBytesImpl<RlpEncTypeCat::Bytes, false>
{
	static constexpr bool sk_isValSigned = false;

	template<typename _CtnType>
	static _CtnType GenLeadingBytes(size_t byteSize)
	{
		if (byteSize <= 55)
		{
			// RlpEncodeType::BytesShort
			_CtnType res;
			res.push_back(static_cast<uint8_t>(0x80U + byteSize));
			return res;
		}
		else
		{
			// RlpEncodeType::BytesLong
			_CtnType res;
			res.push_back(0);
			EncodeSizeValue<Endian::native, sk_isValSigned>::Encode(
				res, byteSize
			);
			res[0] = static_cast<uint8_t>(0xB7U + (res.size() - 1));
			return res;
		}
	}

	static size_t CalcLeadingBytesSize(size_t byteSize)
	{
		if (byteSize <= 55)
		{
			// RlpEncodeType::BytesShort
			// 1 byte
			return 1;
		}
		else
		{
			// RlpEncodeType::BytesLong
			auto lenValSize =
				EncodeSizeValue<Endian::native, sk_isValSigned>::
					EncodedWidth(byteSize);
			// 1 byte + lenValSize bytes
			return 1 + lenValSize;
		}
	}

	template<typename _CtnType, typename _OutConcatOp>
	static _CtnType SerializeBytes(
		const _CtnType& inBytes, _OutConcatOp concatOp
	)
	{
		// Special case - if the input is just 1 byte
		if ((inBytes.size() == 1) && (inBytes[0] <= 0x7FU))
		{
			return inBytes;
		}

		auto leadBytes = GenLeadingBytes<_CtnType>(inBytes.size());

		concatOp(leadBytes, inBytes);

		return leadBytes;
	}

	template<typename _ValType>
	static size_t CalcSerializedSize(
		size_t inBytesSize,
		const _ValType* firstBytePtr
	)
	{
		// Special case - if the input is just 1 byte
		if ((inBytesSize == 1) && (*firstBytePtr <= 0x7FU))
		{
			return 1;
		}

		auto leadBytesSize = CalcLeadingBytesSize(inBytesSize);

		return leadBytesSize + inBytesSize;
	}
}; // struct EncodeRlpBytesImpl<RlpEncTypeCat::Bytes, false>

template<>
struct EncodeRlpBytesImpl<RlpEncTypeCat::List, false>
{
	static constexpr bool sk_isValSigned = false;

	template<typename _CtnType>
	static _CtnType GenLeadingBytes(size_t byteSize)
	{
		if (byteSize <= 55)
		{
			// RlpEncodeType::ListShort
			_CtnType res;
			res.push_back(static_cast<uint8_t>(0xC0U + byteSize));
			return res;
		}
		else
		{
			// RlpEncodeType::ListLong
			_CtnType res;
			res.push_back(0);
			EncodeSizeValue<Endian::native, sk_isValSigned>::Encode(
				res, byteSize
			);
			res[0] = static_cast<uint8_t>(0xF7U + (res.size() - 1));
			return res;
		}
	}

	static size_t CalcLeadingBytesSize(size_t byteSize)
	{
		if (byteSize <= 55)
		{
			// RlpEncodeType::ListShort
			// 1 byte
			return 1;
		}
		else
		{
			// RlpEncodeType::ListLong
			auto lenValSize =
				EncodeSizeValue<Endian::native, sk_isValSigned>::
					EncodedWidth(byteSize);
			// 1 byte + lenValSize bytes
			return 1 + lenValSize;
		}
	}

	template<typename _CtnType, typename _OutConcatOp>
	static _CtnType SerializeBytes(
		const _CtnType& inBytes, _OutConcatOp concatOp
	)
	{
		auto leadBytes = GenLeadingBytes<_CtnType>(inBytes.size());

		concatOp(leadBytes, inBytes);

		return leadBytes;
	}

	static size_t CalcSerializedSize(size_t inBytesSize)
	{
		auto leadBytesSize = CalcLeadingBytesSize(inBytesSize);

		return leadBytesSize + inBytesSize;
	}
}; // struct EncodeRlpBytesImpl<RlpEncTypeCat::List, false>

} // namespace Internal


/**
 * @brief Helper function to serialize a serial of bytes or a concatenated
 *        list of bytes
 *        NOTE: this function assume the individual values in the given bytes
 *        container are in the range of 1 byte [0, 255].
 *
 * @tparam _CtnType The type of the container used for input and output
 * @tparam _OutConcatOp The type of the `concatOp` param
 *
 * @param rlpCat    The category of the given bytes;
 *                  - `RlpEncTypeCat::Bytes` means that the given bytes is
 *                    just a serial of bytes
 *                  - `RlpEncTypeCat::List` means that the given bytes is
 *                    a concatenated list of bytes
 * @param inBytes   The input bytes, which could either be a serial of bytes,
 *                  or a concatenated list of bytes
 * @param concatOp  A function used to concatenate two output container
 *                  instances
 *
 * @return serialized bytes encoded in RLP
 */
template<RlpEncTypeCat _RlpCat, typename _CtnType, typename _OutConcatOp>
inline _CtnType SerializeBytes(const _CtnType& inBytes, _OutConcatOp concatOp)
{
	using _DestValType = typename _CtnType::value_type;
	return Internal::EncodeRlpBytesImpl<
		_RlpCat,
		std::numeric_limits<_DestValType>::is_signed
	>::SerializeBytes(inBytes, concatOp);
}

// template<typename _CtnType, typename _OutConcatOp>
// inline _CtnType SerializeBytes(
// 	RlpEncTypeCat rlpCat, const _CtnType& inBytes, _OutConcatOp concatOp
// )
// {
// 	switch (rlpCat)
// 	{
// 	case RlpEncTypeCat::Bytes:
// 		return SerializeBytes<RlpEncTypeCat::Bytes>(inBytes, concatOp);
// 	case RlpEncTypeCat::List:
// 		return SerializeBytes<RlpEncTypeCat::List>(inBytes, concatOp);
// 	default:
// 		throw SerializeError("Invalid RLP encoding type");
// 	}
// }

template<RlpEncTypeCat _RlpCat>
struct SerializedSize;

template<>
struct SerializedSize<RlpEncTypeCat::Bytes>
{
	template<typename _ValType>
	static size_t Calc(size_t inBytesSize, const _ValType* firstBytePtr)
	{
		return Internal::EncodeRlpBytesImpl<
			RlpEncTypeCat::Bytes,
			std::numeric_limits<_ValType>::is_signed
		>::CalcSerializedSize(inBytesSize, firstBytePtr);
	}
}; // struct SerializedSize<RlpEncTypeCat::Bytes>

template<>
struct SerializedSize<RlpEncTypeCat::List>
{
	template<typename _ValType>
	static size_t Calc(size_t inBytesSize)
	{
		return Internal::EncodeRlpBytesImpl<
			RlpEncTypeCat::List,
			std::numeric_limits<_ValType>::is_signed
		>::CalcSerializedSize(inBytesSize);
	}
}; // struct SerializedSize<RlpEncTypeCat::List>

} // namespace SimpleRlp
