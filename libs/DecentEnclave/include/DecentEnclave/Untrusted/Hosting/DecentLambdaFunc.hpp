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


class DecentLambdaFunc
{
public: // static members:

	using SocketType = Common::Internal::SysIO::StreamSocketBase;

public:
	DecentLambdaFunc() = default;


	// LCOV_EXCL_START
	virtual ~DecentLambdaFunc() = default;
	// LCOV_EXCL_STOP


	virtual void HandleCall(std::unique_ptr<SocketType> sock) = 0;


	// TODO: deterministic message interface
	// virtual void HandleMsg(
	// const std::vector<uint8_t>& eventId,
	// const std::vector<uint8_t>& content,
	// const std::vector<uint8_t>& signature,
	// std::unique_ptr<Connection> connection
	// ) = 0;


}; // class DecentLambdaFunc


} // namespace Hosting
} // namespace Untrusted
} // namespace DecentEnclave
