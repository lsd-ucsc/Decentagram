// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <memory>

#include "Internal/make_unique.hpp"

#include "ParserBase.hpp"

#ifndef SIMPLERLP_CUSTOMIZED_NAMESPACE
namespace SimpleRlp
#else
namespace SIMPLERLP_CUSTOMIZED_NAMESPACE
#endif
{

template<
	typename _ContainerType,
	typename _ByteValType,
	typename _BytesParser,
	typename _ListParser,
	typename _RetType>
class GeneralParserImpl :
	public ParserBase<_ContainerType, _ByteValType, _RetType>
{
public: // static members:

	using Self =
		GeneralParserImpl<
			_ContainerType,
			_ByteValType,
			_BytesParser,
			_ListParser,
			_RetType>;
	using Base = ParserBase<_ContainerType, _ByteValType, _RetType>;

	using InputByteType   = _ByteValType;
	using BytesParser     = _BytesParser;
	using ListParser      = _ListParser;
	using RetType         = _RetType;

public:

	GeneralParserImpl() :
		m_bytesParser(Internal::make_unique<BytesParser>()),
		m_listParser(Internal::make_unique<ListParser>())
	{}

	// LCOV_EXCL_START
	~GeneralParserImpl() = default;
	// LCOV_EXCL_STOP

	using Base::Parse;

	virtual RetType Parse(
		InputStateMachineIf<InputByteType>& ism,
		RlpEncodeType rlpType,
		InputByteType rlpVal,
		size_t& byteLeft) const override
	{
		switch (rlpType)
		{
		case RlpEncodeType::Byte:
		case RlpEncodeType::BytesShort:
		case RlpEncodeType::BytesLong:
			return m_bytesParser->Parse(ism, rlpType, rlpVal, byteLeft);

		case RlpEncodeType::ListShort:
		case RlpEncodeType::ListLong:
			return m_listParser->Parse(ism, rlpType, rlpVal, byteLeft);

		default:
			throw ParseError("Unknown RLP encoding type",
				ism.GetBytesCount());
		}
	}


private:

	std::unique_ptr<BytesParser> m_bytesParser;
	std::unique_ptr<ListParser>  m_listParser;

}; // class GeneralParserImpl

} // namespace SimpleRlp
