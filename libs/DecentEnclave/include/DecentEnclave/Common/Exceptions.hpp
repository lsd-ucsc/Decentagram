// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <SimpleObjects/Exception.hpp>

#include "Internal/SimpleObj.hpp"


namespace DecentEnclave
{
namespace Common
{

/**
 * @brief Parent class of all DecentEnclave exceptions
 *
 */
class Exception : public Internal::Obj::Exception
{
public: // static members:

	using Base = Internal::Obj::Exception;

public:

	using Base::Base;

	// LCOV_EXCL_START
	virtual ~Exception() = default;
	// LCOV_EXCL_STOP

}; // class Exception


class InvalidArgumentException : public Exception
{
public: // static members:

	using Base = Exception;

public:

	using Base::Base;

	// LCOV_EXCL_START
	virtual ~InvalidArgumentException() = default;
	// LCOV_EXCL_STOP

}; // class InvalidArgumentException


} // namespace Common
} // namespace DecentEnclave
