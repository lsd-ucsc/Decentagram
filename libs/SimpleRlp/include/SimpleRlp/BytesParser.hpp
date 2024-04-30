// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

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
	typename _BytesType,
	typename _ByteTransformFunc,
	typename _BytesTransformFunc,
	typename _RetType = typename _BytesTransformFunc::RetType>
class BytesParserImpl :
	public ParserBase<_ContainerType, _ByteValType, _RetType>
{
public: // static members:

	using Self =
		BytesParserImpl<
			_ContainerType,
			_ByteValType,
			_ByteTransformFunc,
			_BytesType,
			_BytesTransformFunc,
			_RetType>;
	using Base = ParserBase<_ContainerType, _ByteValType, _RetType>;

	using InputByteType   = _ByteValType;
	using BytesType       = _BytesType;
	using RetType         = _RetType;

	using BytesTypeValueType = typename BytesType::value_type;

public:

	BytesParserImpl() = default;

	// LCOV_EXCL_START
	virtual ~BytesParserImpl() = default;
	// LCOV_EXCL_STOP

	using Base::Parse;

	virtual RetType Parse(
		InputStateMachineIf<InputByteType>& ism,
		RlpEncodeType rlpType,
		InputByteType rlpVal,
		size_t& byteLeft) const override
	{
		size_t size = 0;
		switch (rlpType)
		{
		case RlpEncodeType::Byte:
			return _ByteTransformFunc()(rlpVal);
			break;

		default:
			size = Base::ProcRlpBytesHeader(ism, rlpType, rlpVal, byteLeft);
			break;
		}

		Base::CheckByteLeft(byteLeft, size, ism.GetBytesCount());
		BytesType obj;
		obj.reserve(size);
		for(size_t i = 0; i < size; ++i)
		{
			obj.push_back(
				static_cast<BytesTypeValueType>(ism.GetByteAndAdv()));
		}

		return _BytesTransformFunc()(ism.GetBytesCount(), std::move(obj));
	}

}; // class BytesParserImpl

} // namespace SimpleRlp
