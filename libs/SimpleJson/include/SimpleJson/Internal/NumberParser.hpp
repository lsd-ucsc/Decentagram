// Parser definition refers to: https://tools.ietf.org/id/draft-ietf-json-rfc4627bis-09.html
#pragma once

#include <tuple>
#include <string>
#include <limits>
#include <type_traits>

#include "ParserHelpers.hpp"

#ifndef SIMPLEJSON_CUSTOMIZED_NAMESPACE
namespace SimpleJson
#else
namespace SIMPLEJSON_CUSTOMIZED_NAMESPACE
#endif
{
	namespace Parser
	{
		namespace Internal
		{
			template<bool _IsImmd, bool _IsOptional = true, typename InputIt>
			inline bool ParseNumNegativeSign(InputIt& begin, InputIt end, const InputIt oriPos)
			{
				if (_IsOptional && (begin == end))
				{
					return false;
				}

				auto tmpCh = _IsImmd ? ImmdPeekChar(begin, end, oriPos) : PeekChar(begin, end, oriPos);
				switch (tmpCh)
				{
				case '-':
					++begin;
					return true;

				default:
					return _IsOptional ? false : throw ParseError("Expecting a negative sign", oriPos, begin);
				}
			}

			template<bool _IsImmd, bool _IsOptional = true, typename InputIt>
			inline bool ParseNumSign(InputIt& begin, InputIt end, const InputIt oriPos)
			{
				if (_IsOptional && (begin == end))
				{
					return true;
				}

				auto tmpCh = _IsImmd ? ImmdPeekChar(begin, end, oriPos) : PeekChar(begin, end, oriPos);
				switch (tmpCh)
				{
				case '-':
					++begin;
					return false;
				case '+':
					++begin;
					return true;

				default:
					return _IsOptional ? true : throw ParseError("Expecting a numeric sign", oriPos, begin);
				}
			}

			template<bool _IsOptional = true, typename InputIt, typename OutputIt>
			inline void ParseNumDigits(InputIt& begin, InputIt end, const InputIt oriPos, OutputIt dest)
			{
				size_t len = 0;

				while(true)
				{
					if (begin == end)
					{
						if (!_IsOptional && len == 0)
						{
							throw ParseError("Unexpected Ends", oriPos, begin);
						}
						else
						{
							return;
						}
					}

					auto tmpCh = ImmdPeekChar(begin, end, oriPos);

					switch (tmpCh)
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
						std::copy(begin, begin + 1, dest);
						++begin;
						++len;
						break;

					default:
					{
						if (!_IsOptional && len == 0)
						{
							throw ParseError("Expecting a numeric value", oriPos, begin);
						}
						else
						{
							return;
						}
					}
					}
				}
			}

			template<typename InputIt, typename OutputIt>
			inline void ParseNumInt(InputIt& begin, InputIt end, const InputIt oriPos, OutputIt dest)
			{
				switch(ImmdPeekChar(begin, end, oriPos))
				{
				case '0':
					std::copy(begin, begin + 1, dest);
					++begin;
					return;

				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					std::copy(begin, begin + 1, dest);
					++begin;
					return ParseNumDigits<>(begin, end, oriPos, dest);

				default:
					throw ParseError("Expecting a numeric value", oriPos, begin);
				}
			}

			template<bool _IsOptional, typename InputIt, typename OutputIt>
			inline void ParseNumFrac(InputIt& begin, InputIt end, const InputIt oriPos, OutputIt dest)
			{
				if (_IsOptional && (begin == end))
				{
					return;
				}

				if (ImmdPeekChar(begin, end, oriPos) == '.')
				{
					++begin;
					return ParseNumDigits<false>(begin, end, oriPos, dest);
				}
				else if (!_IsOptional)
				{
					throw ParseError("Expecting a decimal point", oriPos, begin);
				}
			}

			template<bool _IsOptional, typename InputIt, typename OutputIt>
			inline void ParseNumExp(InputIt& begin, InputIt end, const InputIt oriPos, bool& isPos, OutputIt dest)
			{
				if (_IsOptional && (begin == end))
				{
					return;
				}

				auto tmpCh = ImmdPeekChar(begin, end, oriPos);
				if (tmpCh == 'e' || tmpCh == 'E')
				{
					++begin;
					isPos = ParseNumSign<true, true>(begin, end, oriPos);
					return ParseNumDigits<false>(begin, end, oriPos, dest);
				}
				else if (!_IsOptional)
				{
					throw ParseError("Expecting a decimal point", oriPos, begin);
				}
			}


			template<bool _IsOptional, typename InputIt, typename OutputIt>
			inline void ParseNumExpSignInplace(InputIt& begin, InputIt end, const InputIt oriPos, OutputIt dest)
			{
				constexpr char negativeSign[1] = { '-' };

				if (_IsOptional && (begin == end))
				{
					return;
				}

				auto tmpCh = ImmdPeekChar(begin, end, oriPos);
				if (tmpCh == 'e' || tmpCh == 'E')
				{
					++begin;
					if (!ParseNumSign<true, true>(begin, end, oriPos))
					{
						std::copy(std::begin(negativeSign), std::end(negativeSign), dest);
					}
					return ParseNumDigits<false>(begin, end, oriPos, dest);
				}
				else if (!_IsOptional)
				{
					throw ParseError("Expecting a decimal point", oriPos, begin);
				}
			}

			template<typename InputIt, typename IntOutputIt, typename FracOutputIt, typename ExpOutputIt>
			inline void ParseNumber(InputIt& begin, InputIt end, const InputIt oriPos,
				bool& isPos, IntOutputIt idest,
				FracOutputIt fdest,
				bool& isExpPos, ExpOutputIt edest)
			{
				isPos = !ParseNumNegativeSign<false>(begin, end, oriPos);
				ParseNumInt(begin, end, oriPos, idest);
				ParseNumFrac<true>(begin, end, oriPos, fdest);
				ParseNumExp<true>(begin, end, oriPos, isExpPos, edest);

				begin = SkipLeadingSpace(begin, end);
			}

			template<typename InputIt, typename IntOutputIt, typename FracOutputIt, typename ExpOutputIt>
			inline void ParseNumberSignInplace(InputIt& begin, InputIt end, const InputIt oriPos,
				IntOutputIt idest,
				FracOutputIt fdest,
				ExpOutputIt edest)
			{
				constexpr char negativeSign[1] = { '-' };

				if (ParseNumNegativeSign<false>(begin, end, oriPos))
				{
					std::copy(std::begin(negativeSign), std::end(negativeSign), idest);
				}
				ParseNumInt(begin, end, oriPos, idest);
				ParseNumFrac<true>(begin, end, oriPos, fdest);
				ParseNumExpSignInplace<true>(begin, end, oriPos, edest);

				begin = SkipLeadingSpace(begin, end);
			}
		}

		namespace Internal
		{
			template<typename _HighType, typename _LowType>
			inline _LowType StdNumberDownCast(const _HighType& highVal)
			{
				if (std::numeric_limits<_LowType>::lowest() <= highVal &&
					highVal <= (std::numeric_limits<_LowType>::max)())
				{
					return static_cast<_LowType>(highVal);
				}
				else
				{
					throw RangeErrorException("Value parsed is out of range of the target type");
				}
			}

			template<typename _ValType>
			_ValType ComponentsToNumber(const std::string& intStr,
				const std::string& fracStr,
				const std::string& expStr);

			inline std::string AssembleRealNumber(const std::string& intStr,
				const std::string& fracStr,
				const std::string& expStr)
			{
				std::string numStr = intStr;
				if (fracStr.size())
				{
					numStr.push_back('.');
					numStr.append(fracStr);
				}

				if (expStr.size())
				{
					numStr.push_back('e');
					numStr.append(expStr);
				}

				return numStr;
			}
		}

		// 1. int
		//     2. char
		//     3. signed char
		//     4. short
		// 5. long
		// 6. long long

		// 7. unsigned long
		//     8. unsigned char
		//     9. unsigned short
		//     10. unsigned int
		// 11. unsigned long long

		// 12. float
		// 13. double
		// 14. long double

		/* 1 */ template<>
		inline int Internal::ComponentsToNumber(const std::string& intStr,
			const std::string& fracStr,
			const std::string& expStr)
		{
			if (fracStr.size() > 0 || expStr.size() > 0)
			{
				throw ParseError("Expecting an integer value");
			}

			try
			{
				return std::stoi(intStr);
			}
			catch(const std::out_of_range&)
			{
				throw RangeErrorException("Value parsed is out of range of the target type");
			}
		}

		/* 2 */ template<>
		inline char Internal::ComponentsToNumber(const std::string& intStr,
			const std::string& fracStr,
			const std::string& expStr)
		{
			using HigherType = int;
			using ResType = char;

			auto higherRes = ComponentsToNumber<HigherType>(intStr,
				fracStr, expStr);

			return StdNumberDownCast<decltype(higherRes), ResType>(
				higherRes
			);
		}

		/* 3 */ template<>
		inline signed char Internal::ComponentsToNumber(const std::string& intStr,
			const std::string& fracStr,
			const std::string& expStr)
		{
			using HigherType = int;
			using ResType = signed char;

			auto higherRes = ComponentsToNumber<HigherType>(intStr,
				fracStr, expStr);

			return StdNumberDownCast<decltype(higherRes), ResType>(
				higherRes
			);
		}

		/* 4 */ template<>
		inline short Internal::ComponentsToNumber(const std::string& intStr,
			const std::string& fracStr,
			const std::string& expStr)
		{
			using HigherType = int;
			using ResType = short;

			auto higherRes = ComponentsToNumber<HigherType>(intStr,
				fracStr, expStr);

			return StdNumberDownCast<decltype(higherRes), ResType>(
				higherRes
			);
		}

		/* 5 */ template<>
		inline long Internal::ComponentsToNumber(const std::string& intStr,
			const std::string& fracStr,
			const std::string& expStr)
		{
			if (fracStr.size() > 0 || expStr.size() > 0)
			{
				throw ParseError("Expecting an integer value");
			}

			try
			{
				return std::stol(intStr);
			}
			catch(const std::out_of_range&)
			{
				throw RangeErrorException("Value parsed is out of range of the target type");
			}
		}

		/* 6 */ template<>
		inline long long Internal::ComponentsToNumber(const std::string& intStr,
			const std::string& fracStr,
			const std::string& expStr)
		{
			if (fracStr.size() > 0 || expStr.size() > 0)
			{
				throw ParseError("Expecting an integer value");
			}

			try
			{
				return std::stoll(intStr);
			}
			catch(const std::out_of_range&)
			{
				throw RangeErrorException("Value parsed is out of range of the target type");
			}
		}

		/**
		 * @brief Convert results from ParseNumberSignInplace to integer value.
		 *        Requirements: Assuming input is an integer value that satisfies JSON standard.
		 *        Inputs like "-001" could return wrong result
		 *
		 */
		/* 7 */ template<>
		inline unsigned long Internal::ComponentsToNumber(const std::string& intStr,
			const std::string& fracStr,
			const std::string& expStr)
		{
			if (fracStr.size() > 0 || expStr.size() > 0)
			{
				throw ParseError("Expecting an integer value");
			}
			if (intStr.size() > 0 && intStr[0] == '-')
			{
				if (intStr.size() > 1 && intStr[1] != '0')
				{
					throw RangeErrorException("Value parsed is out of range of the target type");
				}
			}

			try
			{
				return std::stoul(intStr);
			}
			catch(const std::out_of_range&)
			{
				throw RangeErrorException("Value parsed is out of range of the target type");
			}
		}

		/* 8 */ template<>
		inline unsigned char Internal::ComponentsToNumber(const std::string& intStr,
			const std::string& fracStr,
			const std::string& expStr)
		{
			using HigherType = unsigned long;
			using ResType = unsigned char;

			auto higherRes = ComponentsToNumber<HigherType>(intStr,
				fracStr, expStr);

			return StdNumberDownCast<decltype(higherRes), ResType>(
				higherRes
			);
		}

		/* 9 */ template<>
		inline unsigned short Internal::ComponentsToNumber(const std::string& intStr,
			const std::string& fracStr,
			const std::string& expStr)
		{
			using HigherType = unsigned long;
			using ResType = unsigned short;

			auto higherRes = ComponentsToNumber<HigherType>(intStr,
				fracStr, expStr);

			return StdNumberDownCast<decltype(higherRes), ResType>(
				higherRes
			);
		}

		/* 10 */ template<>
		inline unsigned int Internal::ComponentsToNumber(const std::string& intStr,
			const std::string& fracStr,
			const std::string& expStr)
		{
			using HigherType = unsigned long;
			using ResType = unsigned int;

			auto higherRes = ComponentsToNumber<HigherType>(intStr,
				fracStr, expStr);

			static_assert(std::is_same<decltype(higherRes), HigherType>::value, "Programming error.");

			return StdNumberDownCast<decltype(higherRes), ResType>(
				higherRes
			);
		}

		/* 11 */ template<>
		inline unsigned long long Internal::ComponentsToNumber(const std::string& intStr,
			const std::string& fracStr,
			const std::string& expStr)
		{
			if (fracStr.size() > 0 || expStr.size() > 0)
			{
				throw ParseError("Expecting an integer value");
			}
			if (intStr.size() > 0 && intStr[0] == '-')
			{
				if (intStr.size() > 1 && intStr[1] != '0')
				{
					throw RangeErrorException("Value parsed is out of range of the target type");
				}
			}

			try
			{
				return std::stoull(intStr);
			}
			catch(const std::out_of_range&)
			{
				throw RangeErrorException("Value parsed is out of range of the target type");
			}
		}

		/* 12 */ template<>
		inline float Internal::ComponentsToNumber(const std::string& intStr,
			const std::string& fracStr,
			const std::string& expStr)
		{
			if (fracStr.size() == 0 && expStr.size() == 0)
			{
				throw ParseError("Type error - Expecting a float/real type number");
			}

			try
			{
				return std::stof(
					AssembleRealNumber(intStr, fracStr, expStr)
				);
			}
			catch(const std::out_of_range&)
			{
				throw RangeErrorException("Value parsed is out of range of the target type");
			}
		}

		/* 13 */ template<>
		inline double Internal::ComponentsToNumber(const std::string& intStr,
			const std::string& fracStr,
			const std::string& expStr)
		{
			if (fracStr.size() == 0 && expStr.size() == 0)
			{
				throw ParseError("Type error - Expecting a float/real type number");
			}

			try
			{
				return std::stod(
					AssembleRealNumber(intStr, fracStr, expStr)
				);
			}
			catch(const std::out_of_range&)
			{
				throw RangeErrorException("Value parsed is out of range of the target type");
			}
		}

		/* 14 */ template<>
		inline long double Internal::ComponentsToNumber(const std::string& intStr,
			const std::string& fracStr,
			const std::string& expStr)
		{
			if (fracStr.size() == 0 && expStr.size() == 0)
			{
				throw ParseError("Type error - Expecting a float/real type number");
			}

			try
			{
				return std::stold(
					AssembleRealNumber(intStr, fracStr, expStr)
				);
			}
			catch(const std::out_of_range&)
			{
				throw RangeErrorException("Value parsed is out of range of the target type");
			}
		}
	}
}
