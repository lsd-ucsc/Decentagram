// Copyright (c) 2022 SimpleJson
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "ParserBase.hpp"

#ifndef SIMPLEJSON_CUSTOMIZED_NAMESPACE
namespace SimpleJson
#else
namespace SIMPLEJSON_CUSTOMIZED_NAMESPACE
#endif
{

/**
 * @brief Parser for Null type object
 *
 * @tparam _ContainerType Type of containers which *may* be needed during
 *                        intermediate steps.
 *                        Meanwhile, the input character type is inferred by
 *                        `_ContainerType::value_type`
 * @tparam _ObjType       The type used to construct the parsed object
 * @tparam _RetType       The type that will be returned by the parser;
 *                        it's default to the `_ObjType`
 */
template<
	typename _ContainerType,
	typename _ObjType,
	typename _RetType = _ObjType>
class NullParserImpl : public ParserBase<_ContainerType, _RetType>
{
public: // static members:

	using Base = ParserBase<_ContainerType, _RetType>;
	using Self = NullParserImpl<_ContainerType, _ObjType, _RetType>;

	using ContainerType = _ContainerType;
	using InputChType   = typename ContainerType::value_type;
	using ObjType       = _ObjType;
	using RetType       = _RetType;
	using IteratorType  = Internal::Obj::FrIterator<InputChType, true>;
	using ISMType       = ForwardIteratorStateMachine<IteratorType>;

public:

	NullParserImpl() = default;

	// LCOV_EXCL_START
	virtual ~NullParserImpl() = default;
	// LCOV_EXCL_STOP

	using Base::Parse;

	virtual RetType Parse(InputStateMachineIf<InputChType>& ism) const override
	{
		if (ism.SkipSpaceAndGetCharAndAdv() == 'n' &&
			ism.GetCharAndAdv()             == 'u' &&
			ism.GetCharAndAdv()             == 'l' &&
			ism.GetCharAndAdv()             == 'l')
		{
			return ObjType();
		}

		throw ParseError("Unexpected character",
			ism.GetLineCount(), ism.GetColCount());
	}

}; // class NullParserImpl

} // namespace SimpleJson
