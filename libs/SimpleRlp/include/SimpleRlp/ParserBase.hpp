// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <SimpleObjects/Iterator.hpp>

#include "Internal/SimpleObjects.hpp"

#include "InputStateMachine.hpp"
#include "RlpDecoding.hpp"

#ifndef SIMPLERLP_CUSTOMIZED_NAMESPACE
namespace SimpleRlp
#else
namespace SIMPLERLP_CUSTOMIZED_NAMESPACE
#endif
{

template<typename _T>
struct TransformPassthrough
{
	using RetType = _T;

	_T operator()(size_t pos, _T&& v)
	{
		(void)pos;

		return std::forward<_T>(v);
	}
}; // struct TransformPassthrough

template<typename _ValType, typename _BytesType>
struct TransformByteToBytes
{
	using RetType = _BytesType;

	_BytesType operator()(_ValType val)
	{
		return _BytesType({ val });
	}
}; // struct TransformByteToBytes

/**
 * @brief Placeholder to indicate the List parser of a List parser is itself
 *
 */
struct SelfParserPlaceholder {};

/**
 * @brief Placeholder for Auto/Default types
 *
 */
struct AutoPlaceholder {};

/**
 * @brief Basic implementation of Parsers
 *
 * @tparam _ContainerType Type of containers used as input
 * @tparam _ByteValType   The type used to store a byte
 * @tparam _RetType       The type that will be returned by the parser
 */
template<
	typename _ContainerType,
	typename _ByteValType,
	typename _RetType>
class ParserBase
{
public: // static members:

	using ContainerType   = _ContainerType;
	using InputByteType   = _ByteValType;
	using RetType         = _RetType;
	using IteratorType    = Internal::Obj::FrIterator<InputByteType, true>;
	using ISMType         = ForwardIteratorStateMachine<IteratorType>;

public:

	ParserBase() = default;

	// LCOV_EXCL_START
	virtual ~ParserBase() = default;
	// LCOV_EXCL_STOP

	virtual RetType Parse(
		InputStateMachineIf<InputByteType>& ism,
		RlpEncodeType rlpType,
		InputByteType rlpVal,
		size_t& byteLeft) const = 0;

	virtual RetType Parse(
		InputStateMachineIf<InputByteType>& ism,
		size_t& byteLeft) const
	{
		CheckByteLeft(byteLeft, 1, ism.GetBytesCount());
		InputByteType rlpByte = ism.GetByteAndAdv();

		RlpEncodeType rlpType;
		InputByteType rlpVal;
		std::tie(rlpType, rlpVal) =
			DecodeRlpLeadingByte(rlpByte, ism.GetBytesCount());

		return Parse(ism, rlpType, rlpVal, byteLeft);
	}

	virtual RetType Parse(
		IteratorType begin,
		IteratorType end,
		size_t size,
		bool checkExtra = true
	) const
	{
		ISMType ism(begin, end);

		auto res = Parse(ism, size);

		if (checkExtra && (size != 0))
		{
			throw ParseError("Extra data found at the end of input data",
				ism.GetBytesCount());
		}

		return res;
	}

	virtual RetType Parse(const ContainerType& ctn, bool checkExtra = true) const
	{
		return Parse(
			Internal::Obj::ToFrIt<true>(ctn.cbegin()),
			Internal::Obj::ToFrIt<true>(ctn.cend()),
			ctn.size(),
			checkExtra
		);
	}

protected:

	static void CheckByteLeft(size_t& byteLeft, size_t byteNeeded, size_t pos)
	{
		if (byteNeeded > byteLeft)
		{
			throw ParseError("Expecting more input data than what is left",
				pos);
		}
		byteLeft -= byteNeeded;
	}

	static size_t ProcRlpListHeader(
		InputStateMachineIf<InputByteType>& ism,
		RlpEncodeType rlpType,
		InputByteType rlpVal,
		size_t& byteLeft)
	{
		size_t size = 0;
		switch (rlpType)
		{
		case RlpEncodeType::ListShort:
			size = static_cast<size_t>(rlpVal);
			break;

		case RlpEncodeType::ListLong:
			{
				size_t sizeSize = static_cast<size_t>(rlpVal);
				CheckByteLeft(byteLeft, sizeSize, ism.GetBytesCount());
				size =
					Internal::ParseSizeValue<Endian::native>::Parse(
						sizeSize, ism.GetBytesCount(),
						[&]() {
							return ism.GetByteAndAdv();
						}
					);
			}
			break;

		case RlpEncodeType::Byte:
		case RlpEncodeType::BytesShort:
		case RlpEncodeType::BytesLong:
		default:
			throw ParseError("Expecting a list data",
				ism.GetBytesCount());
		}

		return size;
	}

	static size_t ProcRlpBytesHeader(
		InputStateMachineIf<InputByteType>& ism,
		RlpEncodeType rlpType,
		InputByteType rlpVal,
		size_t& byteLeft)
	{
		size_t size = 0;
		switch (rlpType)
		{
		// protected internal function; caller should take care of this:
		// case RlpEncodeType::Byte:
		// 	break;

		case RlpEncodeType::BytesShort:
			size = static_cast<size_t>(rlpVal);
			break;

		case RlpEncodeType::BytesLong:
			{
				size_t sizeSize = static_cast<size_t>(rlpVal);
				CheckByteLeft(byteLeft, sizeSize, ism.GetBytesCount());
				size =
					Internal::ParseSizeValue<Endian::native>::Parse(
						sizeSize, ism.GetBytesCount(),
						[&]() {
							return ism.GetByteAndAdv();
						}
					);
			}
			break;

		case RlpEncodeType::ListShort:
		case RlpEncodeType::ListLong:
		default:
			throw ParseError("Expecting a byte string data",
				ism.GetBytesCount());
		}

		return size;
	}

}; // class ParserBase

} // namespace SimpleRlp
