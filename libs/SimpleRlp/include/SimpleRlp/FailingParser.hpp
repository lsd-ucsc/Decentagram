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
	RlpEncTypeCat _HintRlpCat,
	typename _RetType>
class FailingParser :
	public ParserBase<_ContainerType, _ByteValType, _RetType>
{
public: // static members:

	using Self =
		FailingParser<
			_ContainerType,
			_ByteValType,
			_HintRlpCat,
			_RetType>;
	using Base = ParserBase<_ContainerType, _ByteValType, _RetType>;

	using ContainerType   = _ContainerType;
	using InputByteType   = _ByteValType;
	using RetType         = _RetType;

public:

	FailingParser() = default;

	// LCOV_EXCL_START
	virtual ~FailingParser() = default;
	// LCOV_EXCL_STOP

	virtual RetType Parse(
		InputStateMachineIf<InputByteType>& ism,
		RlpEncodeType,
		InputByteType,
		size_t&) const override
	{
		throw ParseError(FailingErrorMsg(), ism.GetBytesCount());
	}

	virtual RetType Parse(
		InputStateMachineIf<InputByteType>& ism,
		size_t&) const override
	{
		throw ParseError(FailingErrorMsg(), ism.GetBytesCount());
	}

	virtual RetType Parse(
		const ContainerType&,
		bool = true) const override
	{
		throw ParseError(FailingErrorMsg(), 0);
	}

protected:

	static std::string FailingErrorMsg()
	{
		return (
			std::string("RLP ") +
			Internal::RlpEncTypeCatName<_HintRlpCat>::value() +
			" is not allowed in current context");
	}

}; // class FailingParser

} // namespace SimpleRlp
