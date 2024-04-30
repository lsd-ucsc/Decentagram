// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include "../Common/Exceptions.hpp"


namespace DecentEnclave
{
namespace Untrusted
{


class EnclaveException : public Common::Exception
{
public: // static members:
	using Base = Common::Exception;

public:

	using Base::Base;

	// LCOV_EXCL_START
	virtual ~EnclaveException() = default;
	// LCOV_EXCL_STOP

}; // class EnclaveException


} // namespace Untrusted
} // namespace DecentEnclave
