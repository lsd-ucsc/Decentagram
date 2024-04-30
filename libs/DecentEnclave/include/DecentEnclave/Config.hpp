// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#if defined(DECENT_ENCLAVE_PLATFORM_SGX_UNTRUSTED) || \
	defined(DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED)

#	define DECENT_ENCLAVE_PLATFORM_SGX

#endif // defined(DECENT_ENCLAVE_SGX_UNTRUSTED) || defined(DECENT_ENCLAVE_SGX_TRUSTED)
