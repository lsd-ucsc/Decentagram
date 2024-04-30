// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
#include "Sgx/ComponentConnection.hpp"
#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED


namespace DecentEnclave
{
namespace Trusted
{


#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED

using StreamSocket = Sgx::StreamSocket;
using ComponentConnection = Sgx::ComponentConnection;

#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED


} // namespace Trusted
} // namespace DecentEnclave
