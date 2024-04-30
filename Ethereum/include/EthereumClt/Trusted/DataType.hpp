// Copyright (c) 2023 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#pragma once


#include <DecentEnclave/Trusted/DecentLambdaSvr.hpp>


namespace EthereumClt
{
namespace Trusted
{


using LambdaMsgSocket =
	typename DecentEnclave::Trusted::LambdaHandlerMgr::SocketType;
using LambdaMsgSocketPtr =
	typename DecentEnclave::Trusted::LambdaHandlerMgr::SocketPtrType;
using LambdaMsgIdExt =
	typename DecentEnclave::Trusted::LambdaHandlerMgr::MsgIdExtType;
using LambdaMsgContent =
	typename DecentEnclave::Trusted::LambdaHandlerMgr::MsgContentType;


} // namespace Trusted
} // namespace EthereumClt
