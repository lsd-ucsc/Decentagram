// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <memory>

#include <SimpleSysIO/StreamSocketBase.hpp>

#include "../../Common/Internal/SimpleSysIO.hpp"


namespace DecentEnclave
{
namespace Untrusted
{
namespace Hosting
{


class HeartbeatEmitter
{
public:
	HeartbeatEmitter() = default;

	// LCOV_EXCL_START
	virtual ~HeartbeatEmitter() = default;
	// LCOV_EXCL_STOP

	virtual void Heartbeat() = 0;

}; // class HeartbeatEmitter


} // namespace Hosting
} // namespace Untrusted
} // namespace DecentEnclave
