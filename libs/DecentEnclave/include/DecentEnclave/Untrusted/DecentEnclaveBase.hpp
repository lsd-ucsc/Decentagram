// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <vector>

#include "EnclaveBase.hpp"
#include "Hosting/DecentLambdaFunc.hpp"
#include "Hosting/HeartbeatEmitter.hpp"


namespace DecentEnclave
{
namespace Untrusted
{


class DecentEnclaveBase :
	virtual public EnclaveBase,
	virtual public Hosting::DecentLambdaFunc,
	virtual public Hosting::HeartbeatEmitter
{
public: // static members:

	using EncBase = EnclaveBase;
	using LmdFuncBase = Hosting::DecentLambdaFunc;
	using HeartbeatBase = Hosting::HeartbeatEmitter;

public:
	DecentEnclaveBase() = default;

	// LCOV_EXCL_START
	virtual ~DecentEnclaveBase() = default;
	// LCOV_EXCL_STOP

}; // class DecentEnclaveBase


} // namespace Untrusted
} // namespace DecentEnclave
