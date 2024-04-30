// Copyright (c) 2022 SimpleJson
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "ParserBase.hpp"
#include "Internal/make_unique.hpp"

#ifndef SIMPLEJSON_CUSTOMIZED_NAMESPACE
namespace SimpleJson
#else
namespace SIMPLEJSON_CUSTOMIZED_NAMESPACE
#endif
{


/**
 * @brief Parser for List type object
 *
 * @tparam _ContainerType Type of containers which *may* be needed during
 *                        intermediate steps.
 *                        Meanwhile, the input character type is inferred by
 *                        `_ContainerType::value_type`
 * @tparam _ItemParser    The parser used to parse individual item in the list
 * @tparam _ObjType       The type of the list object
 * @tparam _RetType       The type that will be returned by the parser
 */
template<
	typename _ContainerType,
	typename _ItemParser,
	typename _ObjType,
	typename _RetType = _ObjType>
class ListParserImpl : public ParserBase<_ContainerType, _RetType>
{
public: // static members:

	using Base = ParserBase<_ContainerType, _RetType>;
	using Self = ListParserImpl<
		_ContainerType, _ItemParser, _ObjType, _RetType>;

	using ContainerType = _ContainerType;
	using InputChType   = typename ContainerType::value_type;
	using ItemParser    = _ItemParser;
	using ItemParserPtr = std::unique_ptr<_ItemParser>;
	using ObjType       = _ObjType;
	using RetType       = _RetType;
	using IteratorType  = Internal::Obj::FrIterator<InputChType, true>;
	using ISMType       = ForwardIteratorStateMachine<IteratorType>;

public:

	/**
	 * @brief Construct a List Parser with the default ItemParser given
	 *
	 */
	ListParserImpl() :
		m_itemParser(Internal::make_unique<ItemParser>())
	{}

	/**
	 * @brief Construct a List Parser with a derived ItemParser instance
	 *        constructed by the caller
	 *
	 * @param other
	 */
	ListParserImpl(ItemParserPtr&& other) :
		m_itemParser(std::forward<ItemParserPtr>(other))
	{}

	ListParserImpl(const ListParserImpl& other) = delete;

	/**
	 * @brief Move Construct
	 *
	 * @param other
	 */
	ListParserImpl(ListParserImpl&& other) :
		m_itemParser(std::forward<ItemParserPtr>(other.m_itemParser))
	{}

	// LCOV_EXCL_START
	virtual ~ListParserImpl() = default;
	// LCOV_EXCL_STOP

	using Base::Parse;

	virtual RetType Parse(InputStateMachineIf<InputChType>& ism) const override
	{
		return Parse2Obj(ism);
	}

protected:

	ObjType Parse2Obj(InputStateMachineIf<InputChType>& ism) const
	{
		ObjType l;

		auto ch = ism.SkipSpaceAndGetCharAndAdv();

		if (ch == '[')
		{
			// check if there is at least one element
			ch = ism.SkipSpaceAndGetChar();
			if (ch == ']')
			{
				ism.GetCharAndAdv(); // consume ']'
				return l;
			}
			else
			{
				l.push_back(m_itemParser->Parse(ism));
			}

			// Check if there is following items
			ch = ism.SkipSpaceAndGetChar();
			while(ch == ',')
			{
				ism.GetCharAndAdv(); // consume ','
				l.push_back(m_itemParser->Parse(ism));

				ch = ism.SkipSpaceAndGetChar();
			}

			// approach to the end of list
			if (ch == ']')
			{
				ism.GetCharAndAdv(); // consume ']'
				return l;
			}
		}

		throw ParseError("Unexpected character",
			ism.GetLineCount(), ism.GetColCount());
	}

private:

	ItemParserPtr m_itemParser;

}; // class ListParserImpl

} // namespace SimpleJson
