// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
#include "Sgx/Time.hpp"

namespace DecentEnclave
{
namespace Trusted
{

using UntrustedTime = Sgx::UntrustedTime;

} // namespace Trusted
} // namespace DecentEnclave

#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
