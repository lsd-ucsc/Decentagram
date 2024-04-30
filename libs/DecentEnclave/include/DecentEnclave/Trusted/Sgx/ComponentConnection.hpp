// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


// #ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED

#include <memory>
#include <string>

#include <SimpleObjects/Internal/make_unique.hpp>
#include <SimpleSysIO/StreamSocketBase.hpp>

#include "../../Common/Internal/SimpleObj.hpp"
#include "../../Common/Internal/SimpleSysIO.hpp"
#include "../../Common/Sgx/Exceptions.hpp"
#include "../../SgxEdgeSources/sys_io_t.h"
#include "../UntrustedAsyncEventHandler.hpp"
#include "EnclaveIdentity.hpp"
#include "UntrustedBuffer.hpp"


namespace DecentEnclave
{
namespace Trusted
{
namespace Sgx
{


using SSocketAsyncCallbackType =
	std::function<void(const std::vector<uint8_t>, bool)>;

using SSocketAsyncCallbackHandler =
	Trusted::UntrustedAsyncEventHandler<SSocketAsyncCallbackType>;


inline SSocketAsyncCallbackHandler& GetSSocketAsyncCallbackHandler()
{
	static SSocketAsyncCallbackHandler s_handler;
	return s_handler;
}


class StreamSocket : public Common::Internal::SysIO::StreamSocketBase
{
public: // static members:

	using Base = Common::Internal::SysIO::StreamSocketBase;

public:

	StreamSocket(Base* ptr) :
		m_ptr(ptr)
	{}

	// LCOV_EXCL_START
	virtual ~StreamSocket()
	{
		ocall_decent_ssocket_disconnect(m_ptr);
	}
	// LCOV_EXCL_STOP

	virtual size_t SendRaw(const void* data, size_t size) override
	{
		size_t retSize = 0;
		DECENTENCLAVE_SGX_OCALL_CHECK_ERROR_E_R(
			ocall_decent_ssocket_send_raw,
			m_ptr,
			static_cast<const uint8_t*>(data),
			size,
			&retSize
		);
		return retSize;
	}

	virtual size_t RecvRaw(void* data, size_t size) override
	{
		UntrustedBuffer<uint8_t> ub;
		DECENTENCLAVE_SGX_OCALL_CHECK_ERROR_E_R(
			ocall_decent_ssocket_recv_raw,
			m_ptr,
			size,
			&(ub.m_data),
			&(ub.m_size)
		);
		std::memcpy(data, ub.m_data, ub.m_size);
		return ub.m_size;
	}

	virtual void AsyncRecvRaw(
		size_t buffSize,
		AsyncRecvCallback callback
	) override
	{
		auto& handler = GetSSocketAsyncCallbackHandler();
		auto regId = handler.RegisterCallback(std::move(callback));
		DECENTENCLAVE_SGX_OCALL_CHECK_ERROR_E_R(
			ocall_decent_ssocket_async_recv_raw,
			m_ptr,
			buffSize,
			SelfEnclaveId::Get(),
			regId
		);
	}

private:

	Base* m_ptr;
}; // class StreamSocket


struct ComponentConnection
{

	static std::unique_ptr<StreamSocket>
	Connect(const std::string& componentName)
	{
		void* ptr = nullptr;
		DECENTENCLAVE_SGX_OCALL_CHECK_ERROR_E_R(
			ocall_decent_endpoint_connect,
			&ptr,
			componentName.c_str()
		);
		return Common::Internal::Obj::Internal::
			make_unique<StreamSocket>(
				static_cast<Common::Internal::SysIO::StreamSocketBase*>(ptr)
			);
	}

}; // struct ComponentConnection


} // namespace Sgx
} // namespace Trusted
} // namespace DecentEnclave

// #endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
