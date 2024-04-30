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
 * @brief Parser for Dict type object
 *
 * @tparam _ContainerType Type of containers which *may* be needed during
 *                        intermediate steps.
 *                        Meanwhile, the input character type is inferred by
 *                        `_ContainerType::value_type`
 * @tparam _KeyParser     The parser used to parse individual key in the dict
 * @tparam _ValParser     The parser used to parse individual value in the dict
 * @tparam _ObjType       The type of the dict object
 * @tparam _RetType       The type that will be returned by the parser
 */
template<
	typename _ContainerType,
	typename _KeyParser,
	typename _ValParser,
	typename _ObjType,
	typename _RetType = _ObjType>
class DictParserImpl : public ParserBase<_ContainerType, _RetType>
{
public: // static members:

	using Base = ParserBase<_ContainerType, _RetType>;
	using Self = DictParserImpl<
		_ContainerType, _KeyParser, _ValParser, _ObjType, _RetType>;

	using ContainerType = _ContainerType;
	using InputChType   = typename ContainerType::value_type;
	using KeyParser     = _KeyParser;
	using ValParser     = _ValParser;
	using KeyParserPtr  = std::unique_ptr<KeyParser>;
	using ValParserPtr  = std::unique_ptr<ValParser>;
	using ObjType       = _ObjType;
	using RetType       = _RetType;
	using IteratorType  = Internal::Obj::FrIterator<InputChType, true>;
	using ISMType       = ForwardIteratorStateMachine<IteratorType>;

public:

	/**
	 * @brief Construct a Dict Parser with the default ItemParser given
	 *
	 */
	DictParserImpl() :
		m_keyParser(Internal::make_unique<KeyParser>()),
		m_valParser(Internal::make_unique<ValParser>())
	{}

	/**
	 * @brief Construct a Dict Parser with a derived ItemParser instance
	 *        constructed by the caller
	 *
	 * @param keyParser
	 * @param valParser
	 */
	DictParserImpl(KeyParserPtr&& keyParser, KeyParserPtr&& valParser) :
		m_keyParser(std::forward<KeyParserPtr>(keyParser)),
		m_valParser(std::forward<ValParserPtr>(valParser))
	{}

	DictParserImpl(const DictParserImpl& other) = delete;

	/**
	 * @brief Move Construct
	 *
	 * @param other
	 */
	DictParserImpl(DictParserImpl&& other) :
		m_keyParser(std::forward<KeyParserPtr>(other.m_keyParser)),
		m_valParser(std::forward<ValParserPtr>(other.m_valParser))
	{}

	// LCOV_EXCL_START
	virtual ~DictParserImpl() = default;
	// LCOV_EXCL_STOP

	using Base::Parse;

	virtual const ValParser* GetValParser(
		const typename KeyParser::RetType&) const
	{
		return m_valParser.get();
	}

	virtual RetType Parse(InputStateMachineIf<InputChType>& ism) const override
	{
		return Parse2Obj(ism);
	}

protected:

	ObjType Parse2Obj(InputStateMachineIf<InputChType>& ism) const
	{
		ObjType d;

		auto ch = ism.SkipSpaceAndGetCharAndAdv();

		if (ch == '{')
		{
			// check if there is at least one element
			ch = ism.SkipSpaceAndGetChar();
			if (ch == '}')
			{
				ism.GetCharAndAdv(); // consume '}'
				return d;
			}
			else
			{
				auto k = m_keyParser->Parse(ism);
				ism.ExpDelimiter(':');
				auto v = GetValParser(k)->Parse(ism);
				d.InsertOrAssign(std::move(k), std::move(v));
			}

			// Check if there is following items
			ch = ism.SkipSpaceAndGetChar();
			while(ch == ',')
			{
				ism.GetCharAndAdv(); // consume ','

				auto k = m_keyParser->Parse(ism);
				ism.ExpDelimiter(':');
				auto v = GetValParser(k)->Parse(ism);
				d.InsertOrAssign(std::move(k), std::move(v));

				ch = ism.SkipSpaceAndGetChar();
			}

			// approach to the end of list
			if (ch == '}')
			{
				ism.GetCharAndAdv(); // consume '}'
				return d;
			}
		}

		throw ParseError("Unexpected character",
			ism.GetLineCount(), ism.GetColCount());
	}

private:

	KeyParserPtr m_keyParser;
	ValParserPtr m_valParser;

}; // class DictParserImpl

} // namespace SimpleJson
