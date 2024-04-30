// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#ifndef DECENT_ENCLAVE_SGX_EDGE_SOURCES_SYS_IO_U_H
#define DECENT_ENCLAVE_SGX_EDGE_SOURCES_SYS_IO_U_H

#include "sgx_edger8r.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


sgx_status_t ecall_decent_ssocket_async_recv_raw_callback(
	sgx_enclave_id_t eid,
	sgx_status_t* retval,
	uint64_t handler_reg_id,
	const uint8_t* in_data,
	size_t in_data_size,
	uint8_t has_error_occurred
);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !DECENT_ENCLAVE_SGX_EDGE_SOURCES_SYS_IO_U_H
