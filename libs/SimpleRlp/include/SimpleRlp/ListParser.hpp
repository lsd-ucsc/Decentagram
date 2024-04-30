// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <type_traits>
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
	typename _ObjType,
	typename _TransformFunc,
	typename _InnerBytesParserType,
	typename _InnerListParserType,
	typename _RetType = typename _TransformFunc::RetType>
class ListParserImpl :
	public ParserBase<_ContainerType, _ByteValType, _RetType>
{
public: // static members:

	using Self =
		ListParserImpl<
			_ContainerType,
			_ByteValType,
			_ObjType,
			_TransformFunc,
			_InnerBytesParserType,
			_InnerListParserType,
			_RetType>;
	using Base = ParserBase<_ContainerType, _ByteValType, _RetType>;

	using InnerBytesParser = _InnerBytesParserType;
	using InnerListParser  = typename std::conditional<
		std::is_same<_InnerListParserType, SelfParserPlaceholder>::value,
		Self,
		_InnerListParserType>::type;

	using InputByteType   = _ByteValType;
	using ObjType         = _ObjType;
	using RetType         = _RetType;

	using ObjTypeValueType = typename ObjType::value_type;

public:

	ListParserImpl() :
		m_bytesParser(),
		m_listParser()
	{}

	// LCOV_EXCL_START
	virtual ~ListParserImpl() = default;
	// LCOV_EXCL_STOP

	virtual const InnerBytesParser& GetBytesParser() const
	{
		if (m_bytesParser.get() == nullptr)
		{
			m_bytesParser = Internal::make_unique<InnerBytesParser>();
		}
		return *m_bytesParser;
	}

	virtual const InnerListParser& GetListParser() const
	{
		if (m_listParser.get() == nullptr)
		{
			m_listParser = Internal::make_unique<InnerListParser>();
		}
		return *m_listParser;
	}

	using Base::Parse;

	virtual RetType Parse(
		InputStateMachineIf<InputByteType>& ism,
		RlpEncodeType rlpType,
		InputByteType rlpVal,
		size_t& byteLeft) const override
	{
		ObjType obj;

		size_t size = Base::ProcRlpListHeader(ism, rlpType, rlpVal, byteLeft);

		Base::CheckByteLeft(byteLeft, size, ism.GetBytesCount());
		while (size > 0)
		{
			InputByteType nextByte = ism.GetByteAndAdv();
			--size;

			RlpEncodeType nextType;
			InputByteType nextVal;
			std::tie(nextType, nextVal) =
				DecodeRlpLeadingByte(nextByte, ism.GetBytesCount());

			switch (nextType)
			{
			case RlpEncodeType::Byte:
			case RlpEncodeType::BytesShort:
			case RlpEncodeType::BytesLong:
				obj.push_back(
					GetBytesParser().Parse(
						ism, nextType, nextVal, size));
				break;

			case RlpEncodeType::ListShort:
			case RlpEncodeType::ListLong:
				obj.push_back(
					GetListParser().Parse(
						ism, nextType, nextVal, size));
				break;

			// LCOV_EXCL_START
			default:
				throw ParseError("Unrecognized RLP leading byte",
					ism.GetBytesCount());
			// LCOV_EXCL_STOP
			}
		}

		return _TransformFunc()(ism.GetBytesCount(), std::move(obj));
	}

private:

	mutable std::unique_ptr<InnerBytesParser> m_bytesParser;
	mutable std::unique_ptr<InnerListParser>  m_listParser;

}; // class ListParserImpl

} // namespace SimpleRlp
