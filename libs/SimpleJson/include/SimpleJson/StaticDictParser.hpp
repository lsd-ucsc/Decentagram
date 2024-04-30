// Copyright (c) 2022 SimpleJson
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <SimpleObjects/Utils.hpp>
#include <SimpleObjects/StaticDict.hpp>

#include "ParserBase.hpp"

#ifndef SIMPLEJSON_CUSTOMIZED_NAMESPACE
namespace SimpleJson
#else
namespace SIMPLEJSON_CUSTOMIZED_NAMESPACE
#endif
{

namespace Internal
{


// template<template<typename> class _Transform, typename _T>
// struct TupleTransform;

// template<template<typename> class _Transform, typename ..._Items>
// struct TupleTransform<_Transform, std::tuple<_Items...> >
// {
// 	using type = std::tuple<
// 		typename _Transform<_Items>::type ...
// 	>;
// }; // struct TupleTransform

// static_assert(std::is_same<
// 	typename TupleTransform<
// 		std::make_unsigned,
// 		std::tuple<int, int, int> >::type,
// 	std::tuple<unsigned int, unsigned int, unsigned int>
// 	>::value,
// 	"Programming Error - TupleTransform");

// template<
// 	template<typename, typename> class _Transform,
// 	typename _T>
// struct DPairTransform;

// template<
// 	template<typename, typename> class _Transform,
// 	typename _First,
// 	typename _Second>
// struct DPairTransform<_Transform, std::pair<_First, _Second> >
// {
// 	using type = typename _Transform<_First, _Second>::type;
// }; // struct DPairTransform

template<typename _Key, typename _ValParser>
struct DPairParser2Val
{
	using type = std::pair<_Key, typename _ValParser::RetType>;
}; // struct DPairParser2Val

template<typename _ParserPair>
using DParserPair2PairCore =
	Internal::Obj::Internal::DPairTransform<DPairParser2Val, _ParserPair>;

template<typename _ParserTp>
using DParserTuple2TupleCore =
	Internal::Obj::Internal::TupleTransform<DParserPair2PairCore, _ParserTp>;


} // namespace Internal

/**
 * @brief Parse a dict object and stores in a static dict object
 *
 * @tparam _ContainerType Type of containers which *may* be needed during
 *                        intermediate steps.
 *                        Meanwhile, the input character type is inferred by
 *                        `_ContainerType::value_type`
 * @tparam _ParserTp      The tuple type for the static map between dict key
 *                        and corresponding parser; similar to TupleCore in
 *                        StaticDict in SimpleObjects
 * @tparam _AllowMissingItem Whether or not an item is allowed to be not present
 *                           in the input string
 * @tparam _AllowExtraItem   Whether or not to allow an item is presented in the
 *                           input string but not specified in the parser tuple;
 *                           NOTE: Even if the extra item is allow, its value
 *                           will be ignored, and will not be stored in the
 *                           dict object
 * @tparam _RetType          The type of the returned parsed object
 */
template<
	typename _ContainerType,
	typename _KeyParser,
	typename _ParserTp,
	typename _FallbackValParse,
	bool _AllowMissingItem,
	bool _AllowExtraItem,
	typename _RetType>
class StaticDictParserImpl : public ParserBase<_ContainerType, _RetType>
{
public: // static members:

	using Base = ParserBase<_ContainerType, _RetType>;
	using Self = StaticDictParserImpl<
		_ContainerType,
		_KeyParser,
		_ParserTp,
		_FallbackValParse,
		_AllowMissingItem,
		_AllowExtraItem,
		_RetType>;

	using ContainerType = _ContainerType;
	using InputChType   = typename ContainerType::value_type;
	using RetType       = _RetType;
	using IteratorType  = Internal::Obj::FrIterator<InputChType, true>;
	using ISMType       = ForwardIteratorStateMachine<IteratorType>;

	using KeyParser   = _KeyParser;
	using ParserTuple = _ParserTp;
	using FallbackValParse = _FallbackValParse;
	using TupleCore   = typename RetType::TupleCore;

	using _ParsedValChecklist =
		std::array<size_t, std::tuple_size<ParserTuple>::value>;

	static constexpr bool sk_allowMissingItem = _AllowMissingItem;
	static constexpr bool sk_allowExtraItem = _AllowExtraItem;

	struct ParseValueCallBack
	{

		ParseValueCallBack(
			const typename KeyParser::RetType* keyPtr,
			InputStateMachineIf<InputChType>* ismPtr,
			_ParsedValChecklist* clPtr):
			m_targetKeyPtr(keyPtr),
			m_ismPtr(ismPtr),
			m_clPtr(clPtr),
			m_parsed(0)
		{}

		~ParseValueCallBack() = default;

		template<typename _KeyType, typename _ValType, typename _ValParserType>
		void operator()(
			size_t i,
			std::pair<_KeyType, _ValType>& core,
			const std::pair<_KeyType, _ValParserType>& parser)
		{
			if (core.first.key == (*m_targetKeyPtr))
			{
				// A match is found
				core.second = parser.second.Parse(*m_ismPtr);
				++m_parsed;
				++((*m_clPtr)[i]);
			}
		}

		const typename KeyParser::RetType* m_targetKeyPtr;
		InputStateMachineIf<InputChType>* m_ismPtr;
		_ParsedValChecklist* m_clPtr;
		size_t m_parsed;

	}; // struct ParseValueCallBack

public:

	StaticDictParserImpl() = default;

	// LCOV_EXCL_START
	virtual ~StaticDictParserImpl() = default;
	// LCOV_EXCL_STOP

	using Base::Parse;

	virtual RetType Parse(InputStateMachineIf<InputChType>& ism) const override
	{
		TupleCore resTp;
		_ParsedValChecklist checklist = { 0 };

		auto ch = ism.SkipSpaceAndGetCharAndAdv();

		if (ch == '{')
		{
			// check if there is at least one element
			ch = ism.SkipSpaceAndGetChar();
			if (ch == '}')
			{
				ism.GetCharAndAdv(); // consume '}'
				CheckMissingItem(ism, checklist);
				return RetType(std::move(resTp));
			}
			else
			{
				ParseKeyValPair(ism, resTp, checklist);
			}

			// Check if there is following items
			ch = ism.SkipSpaceAndGetChar();
			while(ch == ',')
			{
				ism.GetCharAndAdv(); // consume ','

				ParseKeyValPair(ism, resTp, checklist);

				ch = ism.SkipSpaceAndGetChar();
			}

			// approach to the end of list
			if (ch == '}')
			{
				ism.GetCharAndAdv(); // consume '}'
				CheckMissingItem(ism, checklist);
				return RetType(std::move(resTp));
			}
		}

		throw ParseError("Unexpected character",
			ism.GetLineCount(), ism.GetColCount());
	}

	void ParseKeyValPair(
		InputStateMachineIf<InputChType>& ism,
		TupleCore& resTp,
		_ParsedValChecklist& checklist) const
	{
		auto k = m_keyParser.Parse(ism);
		ism.ExpDelimiter(':');

		ParseValueCallBack cb(&k, &ism, &checklist);

		Internal::Obj::Internal::TupleOperation::BinOp(resTp, m_parserTp, cb);

		if (cb.m_parsed == 0)
		{
			// The given key does not have a matching parser
			if(sk_allowExtraItem)
			{
				// extra item is allowed, use fallback parser
				FallbackValParse().Parse(ism);
			}
			else
			{
				// Extra Item
				// -> It presents in the input string
				//    but doesn't present in the parser tuple and tuple core

				throw ParseError("Extra item found in static dict",
					ism.GetLineCount(), ism.GetColCount());
			}
		}
	}

	void CheckMissingItem(
		const InputStateMachineIf<InputChType>& ism,
		const _ParsedValChecklist& checklist) const
	{
		// Missing item
		// -> It presents in the parser tuple and tuple core,
		//    but doesn't present in the input string
		if (!sk_allowMissingItem)
		{
			// missing item is not allowed
			for (const auto& count : checklist)
			{
				if (count == 0)
				{
					// this item didn't have a match input
					throw ParseError("A key-value pair is missing in the given"
						" static dict",
						ism.GetLineCount(), ism.GetColCount());
				}
			}
		}
	}

private:

	KeyParser m_keyParser;
	ParserTuple m_parserTp;

}; // class StaticDictParserImpl

} // namespace SimpleJson
