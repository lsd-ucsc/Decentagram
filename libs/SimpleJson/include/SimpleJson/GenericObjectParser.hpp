// Copyright (c) 2022 SimpleJson
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "NullParser.hpp"
#include "BoolParser.hpp"
#include "StringParser.hpp"
#include "RealNumParser.hpp"
#include "ListParser.hpp"
#include "DictParser.hpp"

#ifndef SIMPLEJSON_CUSTOMIZED_NAMESPACE
namespace SimpleJson
#else
namespace SIMPLEJSON_CUSTOMIZED_NAMESPACE
#endif
{


/**
 * @brief Parser for generic object
 *
 * @tparam _ContainerType Type of containers which *may* be needed during
 *                        intermediate steps.
 *                        Meanwhile, the input character type is inferred by
 *                        `_ContainerType::value_type`
 * @tparam _IntType       The type used to construct the integer type
 * @tparam _RealType      The type used to construct the real type
 * @tparam _RetType       The type that will be returned by the parser
 */
template<
	typename _ContainerType,
	typename _NullType,
	typename _BoolType,
	typename _IntType,
	typename _RealType,
	typename _StrType,
	typename _HashObjType,
	template<typename> class _ListType,
	template<typename,typename> class _DictType,
	typename _RetType>
class GenericObjectParserImpl : public ParserBase<_ContainerType, _RetType>
{
public: // static members:

	using Base = ParserBase<_ContainerType, _RetType>;
	using Self = GenericObjectParserImpl<
		_ContainerType,
		_NullType,
		_BoolType,
		_IntType,
		_RealType,
		_StrType,
		_HashObjType,
		_ListType,
		_DictType,
		_RetType>;

	using ContainerType = _ContainerType;
	using InputChType   = typename ContainerType::value_type;

	using RetType       = _RetType;
	using IteratorType  = Internal::Obj::FrIterator<InputChType, true>;
	using ISMType       = ForwardIteratorStateMachine<IteratorType>;

	using NullParser = NullParserImpl<_ContainerType, _NullType>;
	using BoolParser = BoolParserImpl<_ContainerType, _BoolType>;

	using GenericNumberParser =
		GenericNumberParserImpl<_ContainerType, _IntType, _RealType, RetType>;

	using StringParser =
		StringParserImpl<_ContainerType, _StrType>;

	using DictKeyParser =
		StringParserImpl<_ContainerType, _StrType, _HashObjType>;

	using ListParser =
		ListParserImpl<_ContainerType, Self, _ListType<RetType> >;

	using DictParser = DictParserImpl<
		_ContainerType,
		DictKeyParser,
		Self,
		_DictType<_HashObjType, RetType> >;

	using NullParserPtr = std::unique_ptr<NullParser>;
	using BoolParserPtr = std::unique_ptr<BoolParser>;
	using GenericNumberParserPtr = std::unique_ptr<GenericNumberParser>;
	using StringParserPtr = std::unique_ptr<StringParser>;
	using ListParserPtr = std::unique_ptr<ListParser>;
	using DictParserPtr = std::unique_ptr<DictParser>;

public:

	GenericObjectParserImpl() = default;

	// LCOV_EXCL_START
	virtual ~GenericObjectParserImpl() = default;
	// LCOV_EXCL_STOP

	using Base::Parse;

	virtual const NullParser* GetNullParser() const
	{
		if (!m_nullParser)
		{
			m_nullParser = Internal::make_unique<NullParser>();
		}
		return m_nullParser.get();
	}

	virtual const BoolParser* GetBoolParser() const
	{
		if (!m_boolParser)
		{
			m_boolParser = Internal::make_unique<BoolParser>();
		}
		return m_boolParser.get();
	}

	virtual const GenericNumberParser* GetNumberParser() const
	{
		if (!m_numberParser)
		{
			m_numberParser = Internal::make_unique<GenericNumberParser>();
		}
		return m_numberParser.get();
	}

	virtual const StringParser* GetStringParser() const
	{
		if (!m_stringParser)
		{
			m_stringParser = Internal::make_unique<StringParser>();
		}
		return m_stringParser.get();
	}

	virtual const ListParser* GetListParser() const
	{
		if (!m_listParser)
		{
			m_listParser = Internal::make_unique<ListParser>();
		}
		return m_listParser.get();
	}

	virtual const DictParser* GetDictParser() const
	{
		if (!m_dictParser)
		{
			m_dictParser = Internal::make_unique<DictParser>();
		}
		return m_dictParser.get();
	}

	virtual void SetNullParser(NullParserPtr p)
	{
		m_nullParser = std::move(p);
	}

	virtual void SetBoolParser(BoolParserPtr p)
	{
		m_boolParser = std::move(p);
	}

	virtual void SetNumberParser(GenericNumberParserPtr p)
	{
		m_numberParser = std::move(p);
	}

	virtual void SetStringParser(StringParserPtr p)
	{
		m_stringParser = std::move(p);
	}

	virtual void SetListParser(ListParserPtr p)
	{
		m_listParser = std::move(p);
	}

	virtual void SetDictParser(DictParserPtr p)
	{
		m_dictParser = std::move(p);
	}

	virtual RetType Parse(InputStateMachineIf<InputChType>& ism) const override
	{
		auto ch = ism.SkipSpaceAndGetChar();

		switch(ch)
		{
			case 'n':   // null   -- Null
				return GetNullParser()->Parse(ism);
			case 't':   // true   -- Bool
			case 'f':   // false  -- Bool
				return GetBoolParser()->Parse(ism);
			case '-':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				return GetNumberParser()->Parse(ism);
			case '\"':  // "..."  -- String
				return GetStringParser()->Parse(ism);
			case '[':   // []     -- List
				return GetListParser()->Parse(ism);
			case '{':   // {}     -- Dict
				return GetDictParser()->Parse(ism);
			default:
				break;
		}

		throw ParseError("Unexpected character",
			ism.GetLineCount(), ism.GetColCount());
	}

private:

	mutable NullParserPtr m_nullParser;
	mutable BoolParserPtr m_boolParser;
	mutable GenericNumberParserPtr m_numberParser;
	mutable StringParserPtr m_stringParser;
	mutable ListParserPtr m_listParser;
	mutable DictParserPtr m_dictParser;

}; // class GenericObjectParserImpl

} // namespace SimpleJson
