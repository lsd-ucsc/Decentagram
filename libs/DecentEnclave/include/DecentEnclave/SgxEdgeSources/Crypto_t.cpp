// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#include <cstddef>

#include <sgx_trts.h>


extern "C" int mbedtls_hardware_poll(
	void*,
	unsigned char *output,
	size_t len,
	size_t *olen
)
{
	sgx_status_t enclaveRet = sgx_read_rand(output, len);
	if (enclaveRet != SGX_SUCCESS)
	{
		*olen = 0;
		return -1;
	}

	*olen = len;
	return 0;
}

