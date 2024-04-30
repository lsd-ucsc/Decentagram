// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#include <cstddef>
#include <cstdint>

#include <sgx_error.h>

#include "../Common/Internal/SimpleSysIO.hpp"
#include "../Common/Platform/Print.hpp"
#include "../Common/Sgx/Exceptions.hpp"
#include "../Untrusted/Config/EndpointsMgr.hpp"
#include "sys_io_u.h"


extern "C" sgx_status_t ocall_decent_endpoint_connect(
	void** ptr,
	const char* name
)
{
	using namespace DecentEnclave::Untrusted;
	try
	{
		auto inst = Config::EndpointsMgr::GetInstance().GetStreamSocket(name);
		*ptr = inst.release();
		return SGX_SUCCESS;
	}
	catch(const std::exception& e)
	{
		DecentEnclave::Common::Platform::Print::StrDebug(
			"ocall_decent_endpoint_connect failed with error " +
			std::string(e.what())
		);
		return SGX_ERROR_UNEXPECTED;
	}
}

extern "C" void ocall_decent_ssocket_disconnect(
	void* ptr
)
{
	using namespace DecentEnclave::Untrusted;
	using _SSocketType = Config::EndpointsMgr::StreamSocketType;
	std::unique_ptr<_SSocketType> realPtr(static_cast<_SSocketType*>(ptr));
}

extern "C" sgx_status_t ocall_decent_ssocket_send_raw(
	void* ptr,
	const uint8_t* in_buf,
	size_t in_buf_size,
	size_t* out_size
)
{
	using namespace DecentEnclave::Untrusted;
	using namespace DecentEnclave::Common::Internal::SysIO;
	using _SSocketType = Config::EndpointsMgr::StreamSocketType;
	_SSocketType* realPtr = static_cast<_SSocketType*>(ptr);

	try
	{
		*out_size = StreamSocketRaw::Send(*realPtr, in_buf, in_buf_size);
		return SGX_SUCCESS;
	}
	catch (const std::exception& e)
	{
		DecentEnclave::Common::Platform::Print::StrDebug(
			"ocall_decent_ssocket_send_raw failed with error " +
			std::string(e.what())
		);
		return SGX_ERROR_UNEXPECTED;
	}
}

extern "C" sgx_status_t ocall_decent_ssocket_recv_raw(
	void* ptr,
	size_t size,
	uint8_t** out_buf,
	size_t* out_buf_size
)
{
	using namespace DecentEnclave::Untrusted;
	using namespace DecentEnclave::Common::Internal::SysIO;
	using _SSocketType = Config::EndpointsMgr::StreamSocketType;
	_SSocketType* realPtr = static_cast<_SSocketType*>(ptr);

	try
	{
		*out_buf = new uint8_t[size];
		*out_buf_size = StreamSocketRaw::Recv(*realPtr, *out_buf, size);
		return SGX_SUCCESS;
	}
	catch (const std::exception& e)
	{
		DecentEnclave::Common::Platform::Print::StrDebug(
			"ocall_decent_ssocket_recv_raw failed with error " +
			std::string(e.what())
		);
		return SGX_ERROR_UNEXPECTED;
	}
}


static
inline
typename DecentEnclave::Common::Internal::
	SysIO::StreamSocketBase::AsyncRecvCallback
MakeAsyncRecvCallback(
	sgx_enclave_id_t enclave_id,
	uint64_t handler_reg_id
)
{
	return [
				enclave_id,
				handler_reg_id
			](std::vector<uint8_t> recvData, bool hasErrorOccurred) -> void
		{
			DECENTENCLAVE_SGX_ECALL_CHECK_ERROR_E_R(
				ecall_decent_ssocket_async_recv_raw_callback,
				enclave_id,
				handler_reg_id,
				recvData.data(),
				recvData.size(),
				hasErrorOccurred ? 1 : 0
			);
		};
}


extern "C" sgx_status_t ocall_decent_ssocket_async_recv_raw(
	void* ptr,
	size_t size,
	sgx_enclave_id_t enclave_id,
	uint64_t handler_reg_id
)
{
	using namespace DecentEnclave::Untrusted;
	using namespace DecentEnclave::Common::Internal::SysIO;
	using _SSocketType = Config::EndpointsMgr::StreamSocketType;
	_SSocketType* realPtr = static_cast<_SSocketType*>(ptr);

	try
	{
		StreamSocketRaw::AsyncRecv(
			*realPtr,
			size,
			MakeAsyncRecvCallback(enclave_id, handler_reg_id)
		);
		return SGX_SUCCESS;
	}
	catch (const std::exception& e)
	{
		DecentEnclave::Common::Platform::Print::StrDebug(
			"ocall_decent_ssocket_async_recv_raw failed with error " +
			std::string(e.what())
		);
		return SGX_ERROR_UNEXPECTED;
	}
}
