// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#ifndef DECENT_ENCLAVE_SGX_EDGE_SOURCES_SYS_IO_T_H
#define DECENT_ENCLAVE_SGX_EDGE_SOURCES_SYS_IO_T_H

#include "sgx_edger8r.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

sgx_status_t ocall_decent_enclave_print_str(const char* str);

sgx_status_t ocall_decent_untrusted_buffer_delete(
	uint8_t data_type,
	void* ptr
);

sgx_status_t ocall_decent_untrusted_timestamp(uint64_t* retval);


// ====================
// Untrusted File
// ====================

sgx_status_t ocall_decent_untrusted_file_open(
	sgx_status_t* retval,
	void** ptr,
	const char* path,
	const char* mode
);

sgx_status_t ocall_decent_untrusted_file_close(void* ptr);

sgx_status_t ocall_decent_untrusted_file_seek(
	sgx_status_t* retval,
	void* ptr,
	ptrdiff_t offset,
	uint8_t whence
);

sgx_status_t ocall_decent_untrusted_file_tell(
	sgx_status_t* retval,
	const void* ptr,
	size_t* out_val
);

sgx_status_t ocall_decent_untrusted_file_flush(
	sgx_status_t* retval,
	void* ptr
);

sgx_status_t ocall_decent_untrusted_file_read(
	sgx_status_t* retval,
	void* ptr,
	size_t size,
	uint8_t** out_buf,
	size_t* out_buf_size
);

sgx_status_t ocall_decent_untrusted_file_write(
	sgx_status_t* retval,
	void* ptr,
	const uint8_t* in_buf,
	size_t in_buf_size,
	size_t* out_size
);


// ====================
// Networking
// ====================


sgx_status_t ocall_decent_endpoint_connect(
	sgx_status_t* retval,
	void** ptr,
	const char* name
);

sgx_status_t ocall_decent_ssocket_disconnect(
	void* ptr
);

sgx_status_t ocall_decent_ssocket_send_raw(
	sgx_status_t* retval,
	void* ptr,
	const uint8_t* in_buf,
	size_t in_buf_size,
	size_t* out_size
);

sgx_status_t ocall_decent_ssocket_recv_raw(
	sgx_status_t* retval,
	void* ptr,
	size_t size,
	uint8_t** out_buf,
	size_t* out_buf_size
);

sgx_status_t ocall_decent_ssocket_async_recv_raw(
	sgx_status_t* retval,
	void* ptr,
	size_t size,
	sgx_enclave_id_t enclave_id,
	uint64_t handler_reg_id
);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !DECENT_ENCLAVE_SGX_EDGE_SOURCES_SYS_IO_T_H
