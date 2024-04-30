// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED


#include "../../Common/Sgx/Exceptions.hpp"
#include "../../SgxEdgeSources/sys_io_t.h"


namespace DecentEnclave
{
namespace Trusted
{
namespace Sgx
{


struct UntrustedTime
{

	static uint64_t Timestamp()
	{
		uint64_t ret = 0;
		sgx_status_t edgeRet = ocall_decent_untrusted_timestamp(&ret);
		DECENTENCLAVE_CHECK_SGX_RUNTIME_ERROR(
			edgeRet,
			ocall_decent_untrusted_timestamp
		);
		return ret;
	}

}; // struct UntrustedTime


} // namespace Sgx
} // namespace Trusted
} // namespace DecentEnclave

#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
