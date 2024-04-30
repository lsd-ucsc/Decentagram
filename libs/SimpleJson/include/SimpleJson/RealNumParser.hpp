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

namespace Internal
{
//====================
// Parsers for numbers
//====================

// Definition of numbers in JSON can be found at
// https://datatracker.ietf.org/doc/html/rfc7159#section-6
// https://tools.ietf.org/id/draft-ietf-json-rfc4627bis-09.html#rfc.section.6

// number         := [ minus ] int [ frac ] [ exp ]

// 	decimal-point := %x2E                        ; .
// 	digit1-9      := %x31-39                     ; 1-9
// 	e             := %x65 / %x45                 ; e E
// 	exp           := e [ minus / plus ] 1*DIGIT  ;
// 	frac          := decimal-point 1*DIGIT       ;
// 	int           := zero / ( digit1-9 *DIGIT )  ;
// 	minus         := %x2D                        ; -
// 	plus          := %x2B                        ; +
// 	zero          := %x30                        ; 0

/**
 * @brief Peek the first character, and check if it's a negative sign,
 *        or something else
 *        - If it's negative sign, advance the ISM, return false
 *        - If it's something else and a sign is optional,
 *            return true (i.e., positive), since we usually assume number
 *            without a sign is a positive value
 *        - If a sign it not optional, an exception will be thrown
 *
 */
template<bool _Optional, typename _InputChType>
inline bool ParseNSign(InputStateMachineIf<_InputChType>& ism)
{
	switch(ism.GetChar())
	{
	case '-':
		ism.GetCharAndAdv();
		return false;
	default:
		return (_Optional ? true : throw ParseError(
			"Expecting a negative sign", ism.GetLineCount(), ism.GetColCount()));
	}
}

/**
 * @brief Peek the first character, and check if it's a negative sign, positive
 *        sign, or something else
 *        - If it's negative sign, advance the ISM, return false
 *        - If it's positive sign, advance the ISM, return true
 *        - If it's something else and a sign is optional,
 *            return true (i.e., positive), since we usually assume number
 *            without a sign is a positive value
 *        - If a sign it not optional, an exception will be thrown
 *
 */
template<bool _Optional, typename _InputChType>
inline bool ParseNorPSign(InputStateMachineIf<_InputChType>& ism)
{
	switch(ism.GetChar())
	{
	case '-':
		ism.GetCharAndAdv();
		return false;
	case '+':
		ism.GetCharAndAdv();
		return true;
	default:
		return (_Optional ? true : throw ParseError(
			"Expecting a numeric sign", ism.GetLineCount(), ism.GetColCount()));
	}
}

/**
 * @brief Parse a series of digit characters ['0'-'9'], and put them into the
 *        output iterator
 *
 */
template<typename _InputChType, typename OutputIt>
inline void ParseNumDigits(
	InputStateMachineIf<_InputChType>& ism, OutputIt dest)
{
	size_t len = 0;

	while(true)
	{
		if (ism.IsEnd())
		{
			if (len == 0)
			{
				throw ParseError("Unexpected Ends",
					ism.GetLineCount(), ism.GetColCount());
			}
			else
			{
				return;
			}
		}

		switch (ism.GetChar())
		{
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
			*dest++ = ism.GetCharAndAdv();
			++len;
			break;

		default:
		{
			if (len == 0)
			{
				throw ParseError("Expecting a numeric value",
					ism.GetLineCount(), ism.GetColCount());
			}
			else
			{
				return;
			}
		}
		}
	}
}

/**
 * @brief Parse a integer value, and put them into the
 *        output iterator
 *
 */
template<typename _InputChType, typename OutputIt>
inline void ParseInt(
	InputStateMachineIf<_InputChType>& ism, OutputIt dest)
{
	switch (ism.GetChar())
	{
	case '0': // the number is a single '0'
		*dest++ = ism.GetCharAndAdv();
		return;

	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9': // the number doesn't start with 0
		return ParseNumDigits(ism, dest);

	default:
		throw ParseError("Expecting a numeric value",
					ism.GetLineCount(), ism.GetColCount());
	}
}

/**
 * @brief Parse the fraction part, which is 'decimal-point 1*DIGIT'
 *
 */
template<bool _Optional, typename _InputChType, typename OutputIt>
inline void ParseFrac(InputStateMachineIf<_InputChType>& ism, OutputIt dest)
{
	if (ism.IsEnd() && _Optional)
	{
		return;
	}

	if (ism.GetChar() == '.')
	{
		ism.GetCharAndAdv();
		*dest++ = '.';
		return ParseNumDigits(ism, dest);
	}
	else if (!_Optional)
	{
		throw ParseError("Expecting a decimal point",
			ism.GetLineCount(), ism.GetColCount());
	}
}

/**
 * @brief Parse the exponential part, which is 'e [ minus / plus ] 1*DIGIT'
 *
 */
template<bool _Optional, typename _InputChType, typename OutputIt>
inline void ParseExp(InputStateMachineIf<_InputChType>& ism, OutputIt dest)
{
	if (ism.IsEnd() && _Optional)
	{
		return;
	}

	auto ch = ism.GetChar();
	if (ch == 'e' || ch == 'E')
	{
		ism.GetCharAndAdv();
		*dest++ = 'e';
		if (!ParseNorPSign<true>(ism))
		{
			*dest++ = '-';
		}
		return ParseNumDigits(ism, dest);
	}
	else if (!_Optional)
	{
		throw ParseError("Expecting an exponential sign",
			ism.GetLineCount(), ism.GetColCount());
	}
}

/**
 * @brief Parse number, which is '[ minus ] int [ frac ] [ exp ]'
 *
 * @return a pair of "[minus] int" and "[ frac ] [ exp ]"
 *
 */
template<
	typename _ContainerType,
	typename _InputChType = typename _ContainerType::value_type>
inline std::pair<_ContainerType, _ContainerType> ParseNum(
	InputStateMachineIf<_InputChType>& ism)
{
	_ContainerType iRes;
	_ContainerType fRes;

	if (!ParseNSign<true>(ism))
	{
		iRes.push_back('-');
	}

	ParseInt(ism, std::back_inserter(iRes));

	ParseFrac<true>(ism, std::back_inserter(fRes));

	ParseExp<true>(ism, std::back_inserter(fRes));

	return std::make_pair(iRes, fRes);
}

} // namespace Internal


/**
 * @brief Parser for general numeric objects such as integer number and
 *        real number
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
	typename _IntType,
	typename _RealType,
	typename _RetType>
class GenericNumberParserImpl : public ParserBase<_ContainerType, _RetType>
{
public: // static members:

	using Base = ParserBase<_ContainerType, _RetType>;
	using Self = GenericNumberParserImpl<
		_ContainerType, _IntType, _RealType, _RetType>;

	using ContainerType = _ContainerType;
	using InputChType   = typename ContainerType::value_type;
	using IntType       = _IntType;
	using RealType      = _RealType;
	using RetType       = _RetType;
	using IteratorType  = Internal::Obj::FrIterator<InputChType, true>;
	using ISMType       = ForwardIteratorStateMachine<IteratorType>;

public:

	GenericNumberParserImpl() = default;

	// LCOV_EXCL_START
	virtual ~GenericNumberParserImpl() = default;
	// LCOV_EXCL_STOP

	using Base::Parse;

	virtual RetType Parse(InputStateMachineIf<InputChType>& ism) const override
	{
		ContainerType iRes;
		ContainerType fRes;

		ism.SkipWhiteSpace();

		std::tie(iRes, fRes) = Internal::ParseNum<ContainerType>(ism);

		if (fRes.size() > 0)
		{
			// It's a real number
			ContainerType fullRes = iRes + fRes;
			double val = atof(fullRes.c_str());
			return RealType(val);
		}
		else
		{
			// It's a interger number
			ContainerType fullRes = iRes + fRes;
			long long val = atoll(fullRes.c_str());
			return IntType(val);
		}
	}

}; // class GenericNumberParserImpl


/**
 * @brief Parser for integer numer
 *
 * @tparam _ContainerType Type of containers which *may* be needed during
 *                        intermediate steps.
 *                        Meanwhile, the input character type is inferred by
 *                        `_ContainerType::value_type`
 * @tparam _ObjType       The type used to construct the integer type
 * @tparam _RetType       The type that will be returned by the parser
 */
template<
	typename _ContainerType,
	typename _ObjType,
	typename _RetType = _ObjType>
class IntegerParserImpl : public ParserBase<_ContainerType, _RetType>
{
public: // static members:

	using Base = ParserBase<_ContainerType, _RetType>;
	using Self = IntegerParserImpl<_ContainerType, _ObjType, _RetType>;

	using ContainerType = _ContainerType;
	using InputChType   = typename ContainerType::value_type;
	using ObjType       = _ObjType;
	using RetType       = _RetType;
	using IteratorType  = Internal::Obj::FrIterator<InputChType, true>;
	using ISMType       = ForwardIteratorStateMachine<IteratorType>;

public:

	IntegerParserImpl() = default;

	// LCOV_EXCL_START
	virtual ~IntegerParserImpl() = default;
	// LCOV_EXCL_STOP

	using Base::Parse;

	virtual RetType Parse(InputStateMachineIf<InputChType>& ism) const override
	{
		ContainerType iRes;
		ContainerType fRes;

		ism.SkipWhiteSpace();

		std::tie(iRes, fRes) = Internal::ParseNum<ContainerType>(ism);

		if (fRes.size() > 0)
		{
			throw ParseError(
				"Expecteding a integer while a real number is parsed",
				ism.GetLineCount(), ism.GetColCount());
		}
		else
		{
			// It's a interger number
			ContainerType fullRes = iRes + fRes;
			long long val = atoll(fullRes.c_str());
			return ObjType(val);
		}
	}

}; // class IntegerParserImpl


/**
 * @brief Parser for real numbers
 *
 * @tparam _ContainerType Type of containers which *may* be needed during
 *                        intermediate steps.
 *                        Meanwhile, the input character type is inferred by
 *                        `_ContainerType::value_type`
 * @tparam _ObjType       The type used to construct the real type
 * @tparam _RetType       The type that will be returned by the parser
 */
template<
	typename _ContainerType,
	typename _ObjType,
	typename _RetType = _ObjType>
class RealNumParserImpl : public ParserBase<_ContainerType, _RetType>
{
public: // static members:

	using Base = ParserBase<_ContainerType, _RetType>;
	using Self = RealNumParserImpl<_ContainerType, _ObjType, _RetType>;

	using ContainerType = _ContainerType;
	using InputChType   = typename ContainerType::value_type;
	using ObjType       = _ObjType;
	using RetType       = _RetType;
	using IteratorType  = Internal::Obj::FrIterator<InputChType, true>;
	using ISMType       = ForwardIteratorStateMachine<IteratorType>;

public:

	RealNumParserImpl() = default;

	// LCOV_EXCL_START
	virtual ~RealNumParserImpl() = default;
	// LCOV_EXCL_STOP

	using Base::Parse;

	virtual RetType Parse(InputStateMachineIf<InputChType>& ism) const override
	{
		ContainerType iRes;
		ContainerType fRes;

		ism.SkipWhiteSpace();

		std::tie(iRes, fRes) = Internal::ParseNum<ContainerType>(ism);

		if (fRes.size() > 0)
		{
			// It's a real number
			ContainerType fullRes = iRes + fRes;
			double val = atof(fullRes.c_str());
			return ObjType(val);
		}
		else
		{
			throw ParseError(
				"Expecteding a real number while an integer is parsed",
				ism.GetLineCount(), ism.GetColCount());
		}
	}

}; // class GenericNumberParserImpl

} // namespace SimpleJson
