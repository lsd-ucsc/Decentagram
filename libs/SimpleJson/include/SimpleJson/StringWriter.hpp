// Copyright (c) 2022 SimpleJson
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "WriterConfig.hpp"
#include "Internal/SimpleObjects.hpp"

#ifndef SIMPLEJSON_CUSTOMIZED_NAMESPACE
namespace SimpleJson
#else
namespace SIMPLEJSON_CUSTOMIZED_NAMESPACE
#endif
{

template<typename _CharType, typename _ToStringType, typename _ContainerType>
struct JsonWriterStringImpl
{

	using AsciiTraitType = Internal::Utf::AsciiTraits<_CharType>;

	template<typename OutputIt>
	static void WriteUXXXX(char16_t val, OutputIt dest)
	{
		constexpr char alphabet[] = "0123456789ABCDEF";

		uint8_t nibble = 0;

		nibble = static_cast<uint8_t>((val >> 12) & 0x0F);
		*dest++ = *(std::begin(alphabet) + nibble);

		nibble = static_cast<uint8_t>((val >>  8) & 0x0F);
		*dest++ = *(std::begin(alphabet) + nibble);

		nibble = static_cast<uint8_t>((val >>  4) & 0x0F);
		*dest++ = *(std::begin(alphabet) + nibble);

		nibble = static_cast<uint8_t>((val >>  0) & 0x0F);
		*dest++ = *(std::begin(alphabet) + nibble);
	}

	template<typename _OutputIt>
	inline static void Write(_OutputIt dest,
		const Internal::Obj::StringBaseObject<_CharType, _ToStringType>& obj,
		const WriterConfig&,
		const WriterStates&)
	{
		*dest++ = '\"';
		auto it = obj.begin();
		while(it != obj.end())
		{
			auto ch = (*it);
			if(AsciiTraitType::IsAsciiFast(ch))
			{
				// ASCII code
				switch(ch)
				{
				// escape
				case '"':
					*dest++ = '\\';
					*dest++ = '\"';
					break;
				case '\\':
					*dest++ = '\\';
					*dest++ = '\\';
					break;
				case '/':
					*dest++ = '\\';
					*dest++ = '/';
					break;
				case '\b':
					*dest++ = '\\';
					*dest++ = 'b';
					break;
				case '\f':
					*dest++ = '\\';
					*dest++ = 'f';
					break;
				case '\n':
					*dest++ = '\\';
					*dest++ = 'n';
					break;
				case '\r':
					*dest++ = '\\';
					*dest++ = 'r';
					break;
				case '\t':
					*dest++ = '\\';
					*dest++ = 't';
					break;

				// default ASCII character
				default:
					*dest++ = ch;
				}
				++it;
			}
			else
			{
				// Assuming it's UTF-8
				size_t contCount = 0;
				std::tie(contCount, std::ignore) =
					Internal::Utf::Internal::Utf8ReadLeading(ch);

				_ContainerType tmp = { ch };
				for (size_t i = 0; i < contCount; ++i)
				{
					++it;
					tmp.push_back(*it);
				}

				char16_t pair[2] = { 0 };
				Internal::Utf::CodePtToUtf16Once(
					Internal::Utf::Utf8ToCodePtOnce(
						tmp.begin(), tmp.end()).first,
					std::begin(pair));

				*dest++ = '\\';
				*dest++ = 'u';

				WriteUXXXX(pair[0], dest);

				if (Internal::Utf::Internal::IsUtf16SurrogateFirst(pair[0]))
				{
					*dest++ = '\\';
					*dest++ = 'u';

					WriteUXXXX(pair[1], dest);
				}

				++it;
			}
		}

		*dest++ = '\"';

	}

}; // struct JsonWriterRealNumImpl

} // namespace SimpleJson
