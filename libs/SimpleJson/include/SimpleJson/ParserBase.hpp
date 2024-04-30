// Copyright (c) 2022 SimpleJson
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "InputStateMachine.hpp"
#include "Internal/SimpleObjects.hpp"

#ifndef SIMPLEJSON_CUSTOMIZED_NAMESPACE
namespace SimpleJson
#else
namespace SIMPLEJSON_CUSTOMIZED_NAMESPACE
#endif
{

/**
 * @brief Basic implementation of Parsers
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
	typename _RetType>
class ParserBase
{
public: // static members:

	using ContainerType = _ContainerType;
	using InputChType   = typename ContainerType::value_type;
	using RetType       = _RetType;
	using IteratorType  = Internal::Obj::FrIterator<InputChType, true>;
	using ISMType       = ForwardIteratorStateMachine<IteratorType>;

public:

	ParserBase() = default;

	// LCOV_EXCL_START
	virtual ~ParserBase() = default;
	// LCOV_EXCL_STOP

	virtual RetType Parse(InputStateMachineIf<InputChType>& ism) const = 0;

	virtual RetType Parse(const ContainerType& ctn) const
	{
		ISMType ism(
			Internal::Obj::ToFrIt<true>(ctn.cbegin()),
			Internal::Obj::ToFrIt<true>(ctn.cend()));

		return Parse(ism);
	}

	virtual RetType ParseTillEnd(const ContainerType& ctn) const
	{
		ISMType ism(
			Internal::Obj::ToFrIt<true>(ctn.cbegin()),
			Internal::Obj::ToFrIt<true>(ctn.cend()));

		auto res = Parse(ism);

		ism.SkipWhiteSpace();

		if (!ism.IsEnd())
		{
			throw ParseError("Extra Data",
				ism.GetLineCount(), ism.GetColCount());
		}

		return res;
	}

}; // class ParserBase

} // namespace SimpleJson
