// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED


#include <mbedTLScpp/RandInterfaces.hpp>
#include <sgx_trts.h>

#include "../../Common/Sgx/Exceptions.hpp"


namespace DecentEnclave
{
namespace Trusted
{
namespace Sgx
{


class RandGenerator :
	public mbedTLScpp::RbgInterface
{
public:
	RandGenerator() = default;

	virtual ~RandGenerator() = default;

	virtual void Rand(void* buf, size_t size) override
	{
		sgx_status_t sgxRet = sgx_read_rand(
			reinterpret_cast<unsigned char*>(buf),
			size
		);
		DECENTENCLAVE_CHECK_SGX_RUNTIME_ERROR(
			sgxRet,
			"sgx_read_rand"
		);
	}

}; // class RandGenerator


} // namespace Sgx
} // namespace Trusted
} // namespace DecentEnclave

#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
