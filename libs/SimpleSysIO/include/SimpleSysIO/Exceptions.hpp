// Copyright (c) 2022 SimpleSysIO
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "Internal/SimpleObjects.hpp"

#ifndef SIMPLESYSIO_CUSTOMIZED_NAMESPACE
namespace SimpleSysIO
#else
namespace SIMPLESYSIO_CUSTOMIZED_NAMESPACE
#endif
{

/**
 * @brief Parent class of all SimpleSysIO exceptions
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

} // namespace SimpleSysIO
