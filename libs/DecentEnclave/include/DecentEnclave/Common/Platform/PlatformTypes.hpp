// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>


namespace DecentEnclave
{
namespace Common
{
namespace Platform
{


enum class PlatformTypes : uint8_t
{
	Untrusted = 0,
	SGX       = 1,

#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
	Native    = SGX,
#else
	Native    = Untrusted,
#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED

}; // enum class PlatformTypes


} // namespace Platform
} // namespace Common
} // namespace DecentEnclave
