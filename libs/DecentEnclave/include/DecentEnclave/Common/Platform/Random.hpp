// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
#include "../../Trusted/Sgx/Random.hpp"
#else
#include <mbedTLScpp/CtrDrbg.hpp>
#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED

namespace DecentEnclave
{
namespace Common
{
namespace Platform
{

#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
using RandGenerator = Trusted::Sgx::RandGenerator;
#else
using RandGenerator = mbedTLScpp::CtrDrbg<>;
#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED

} // namespace Platform
} // namespace Common
} // namespace DecentEnclave
