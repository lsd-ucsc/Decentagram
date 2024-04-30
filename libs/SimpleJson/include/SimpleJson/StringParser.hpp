// Copyright (c) 2022 SimpleJson
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "ParserBase.hpp"
#include "Internal/SimpleUtf.hpp"

#ifndef SIMPLEJSON_CUSTOMIZED_NAMESPACE
namespace SimpleJson
#else
namespace SIMPLEJSON_CUSTOMIZED_NAMESPACE
#endif
{

/**
 * @brief Parser for String type object
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
class StringParserImpl : public ParserBase<_ContainerType, _RetType>
{
public: // static members:

	using Base = ParserBase<_ContainerType, _RetType>;
	using Self = StringParserImpl<_ContainerType, _ObjType, _RetType>;

	using ContainerType = _ContainerType;
	using InputChType   = typename ContainerType::value_type;
	using ObjType       = _ObjType;
	using RetType       = _RetType;
	using IteratorType  = Internal::Obj::FrIterator<InputChType, true>;
	using ISMType       = ForwardIteratorStateMachine<IteratorType>;

	using AsciiTraitType = Internal::Utf::AsciiTraits<InputChType>;

public:

	StringParserImpl() = default;

	// LCOV_EXCL_START
	virtual ~StringParserImpl() = default;
	// LCOV_EXCL_STOP

	using Base::Parse;

	virtual RetType Parse(InputStateMachineIf<InputChType>& ism) const override
	{
		return Parse2Obj(ism);
	}

protected:

	// definition: https://datatracker.ietf.org/doc/html/rfc7159#section-7
	ObjType Parse2Obj(InputStateMachineIf<InputChType>& ism) const
	{
		auto res = ObjType();
		auto ch = ism.SkipSpaceAndGetCharAndAdv();
		if (ch == '\"')
		{
			while(true)
			{
				ch = ism.GetCharAndAdv();
				// Case 1 - ending quote
				if (ch == '\"') // Ending
				{
					return res;
				}
				// Case 2 - Escape something
				else if (ch == '\\')
				{
					ParseEscapeSomething(ism, res);
				}
				// Case 3 - normal ASCII character
				else if (AsciiTraitType::IsAsciiFast(ch))
				{
					res.push_back(ch);
				}
				// Case 4 - none of above cases match; let's assume it's UTF-8
				else
				{
					try
					{
						size_t contCount = 0;
						std::tie(contCount, std::ignore) =
							Internal::Utf::Internal::Utf8ReadLeading(ch);

						ContainerType tmp = { ch };
						for (size_t i = 0; i < contCount; ++i)
						{
							tmp.push_back(ism.GetCharAndAdv());
						}

						// UTF-8 => UTF-8 to validate encoding
						Internal::Utf::UtfConvertOnce(
							Internal::Utf::Utf8ToCodePtOnce<typename ContainerType::const_iterator>,
							Internal::Utf::CodePtToUtf8Once<std::back_insert_iterator<ObjType> >,
							tmp.cbegin(), tmp.cend(),
							std::back_inserter(res));
					}
					catch(const Internal::Utf::UtfConversionException& e)
					{
						throw ParseError(
							std::string("Invalid Unicode - ") + e.what(),
							ism.GetLineCount(), ism.GetColCount());
					}
				}
			}
		}

		throw ParseError("Unexpected character",
			ism.GetLineCount(), ism.GetColCount());
	}

private:

	char16_t ParseUXXXX(InputStateMachineIf<InputChType>& ism) const
	{
		char16_t res = 0;
		for (size_t i = 0; i < 4; ++i)
		{
			res <<= 4;

			auto ch = ism.GetCharAndAdv();
			switch (ch)
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
				res |= static_cast<uint8_t>(ch - '0');
				break;

			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
				res |= static_cast<uint8_t>((ch - 'A') + 0xa);
				break;

			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
				res |= static_cast<uint8_t>((ch - 'a') + 0xa);
				break;

			default:
				throw ParseError("Invalid \\uXXXX escape",
					ism.GetLineCount(), ism.GetColCount());
			}
		}

		return res;
	}

	char16_t ParseEscapeUXXXX(InputStateMachineIf<InputChType>& ism) const
	{
		if (ism.GetCharAndAdv() == '\\' &&
			ism.GetCharAndAdv() == 'u')
		{
			return ParseUXXXX(ism);
		}
		throw ParseError("Unexpected character",
			ism.GetLineCount(), ism.GetColCount());
	}

	void ParseEscapeSomething(
		InputStateMachineIf<InputChType>& ism, ObjType& res) const
	{
		auto ch = ism.GetCharAndAdv();
		switch (ch)
		{
		// normal escape cases
		case '\"':
			res.push_back('\"');
			break;
		case '\\':
			res.push_back('\\');
			break;
		case '/':
			res.push_back('/');
			break;
		case 'b':
			res.push_back('\b');
			break;
		case 'f':
			res.push_back('\f');
			break;
		case 'n':
			res.push_back('\n');
			break;
		case 'r':
			res.push_back('\r');
			break;
		case 't':
			res.push_back('\t');
			break;
		// \uXXXX escape case
		case 'u':
			{
				char16_t pair[2] = { 0 };
				bool isPair = false;

				pair[0] = ParseUXXXX(ism);

				try
				{
					// Case 1 - It's a surrogate pair
					if (Internal::Utf::Internal::IsUtf16SurrogateFirst(pair[0]))
					{
						pair[1] = ParseEscapeUXXXX(ism);
						isPair = true;
					}
					// Case 2 - Not a pair
					else
					{ /* Nothing to do */ }

					Internal::Utf::Utf16ToUtf8(
						std::begin(pair),
						std::begin(pair) + (isPair ? 2 : 1),
						std::back_inserter(res));
				}
				catch(const Internal::Utf::UtfConversionException& e)
				{
					throw ParseError(
						std::string("Invalid Unicode - ") + e.what(),
						ism.GetLineCount(), ism.GetColCount());
				}
			}
			break;
		// invalid escape
		default:
			throw ParseError("Unexpected character",
				ism.GetLineCount(), ism.GetColCount());
		}
	}

}; // class StringParserImpl

} // namespace SimpleJson
