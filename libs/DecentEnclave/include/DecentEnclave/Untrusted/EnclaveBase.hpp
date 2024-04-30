// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


namespace DecentEnclave
{
namespace Untrusted
{


class EnclaveBase
{
public:
	EnclaveBase() = default;

	// LCOV_EXCL_START
	virtual ~EnclaveBase() = default;
	// LCOV_EXCL_STOP

	virtual const char* GetPlatformName() const = 0;

}; // class EnclaveBase


} // namespace Untrusted
} // namespace DecentEnclave
