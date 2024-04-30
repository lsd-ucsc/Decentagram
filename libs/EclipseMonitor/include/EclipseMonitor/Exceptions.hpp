// Copyright (c) 2022 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "Internal/SimpleObj.hpp"

namespace EclipseMonitor
{

/**
 * @brief Parent class of all EclipseMonitor exceptions
 *
 */
class Exception : public Internal::Obj::Exception
{
public:

	using Internal::Obj::Exception::Exception;

	// LCOV_EXCL_START
	virtual ~Exception() = default;
	// LCOV_EXCL_STOP

}; // class Exception

} // namespace EclipseMonitor
