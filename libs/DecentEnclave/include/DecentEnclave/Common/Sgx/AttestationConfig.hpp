// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#if defined(DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED) || \
	defined(DECENT_ENCLAVE_PLATFORM_SGX_UNTRUSTED)


#include <cstdint>


namespace DecentEnclave
{
namespace Common
{
namespace Sgx
{


struct AttestationConfig
{
	static constexpr uint16_t sk_quoteTypeUnlinkable = 0;
	static constexpr uint16_t sk_quoteTypeLinkable = 1;

	static constexpr uint16_t sk_kdfIdDefault = 1;
}; // struct AttestationConfig


} // namespace Sgx
} // namespace Common
} // namespace DecentEnclave

#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED || _UNTRUSTED
