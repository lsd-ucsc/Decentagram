// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cctype>

#include <algorithm>
#include <string>


namespace DecentEnclave
{
namespace Common
{


/**
 * @brief strip/trim spaces from the start of the given string.
 *        NOTE: The operation is done in place.
 *
 * @param s The reference to the string to be stripped
 * @return The reference to the stripped string
 */
inline std::string& StripL(std::string &s)
{
	auto itNotSpace = std::find_if(
		s.begin(),
		s.end(),
		[](char ch)
		{
			return !std::isspace(ch);
		}
	);
	s.erase(s.begin(), itNotSpace);

	return s;
}


/**
 * @brief strip/trim spaces from the end of the given string.
 *        NOTE: The operation is done in place.
 *
 * @param s The reference to the string to be stripped
 * @return The reference to the stripped string
 */
inline std::string& StripR(std::string &s)
{
	auto itNotSpace = std::find_if(
		s.rbegin(),
		s.rend(),
		[](char ch)
		{
			return !std::isspace(ch);
		}
	).base();
	s.erase(itNotSpace, s.end());

	return s;
}


/**
 * @brief strip/trim spaces from both ends of the given string.
 *
 * @param s The reference to the string to be stripped
 * @return The reference to the stripped string
 */
inline std::string& Strip(std::string &s)
{
	return StripL(StripR(s));
}


} // namespace Common
} // namespace DecentEnclave
