// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED


#include <cstring>

#include <SimpleSysIO/IOStreamBase.hpp>

#include "../../Common/Internal/SimpleSysIO.hpp"
#include "../../Common/Sgx/Exceptions.hpp"
#include "../../SgxEdgeSources/sys_io_t.h"
#include "UntrustedBuffer.hpp"


namespace DecentEnclave
{
namespace Trusted
{
namespace Sgx
{


struct UntrustedFileImpl
{
public:

	UntrustedFileImpl(const std::string& path, const std::string& mode):
		m_ptr(nullptr)
	{
		DECENTENCLAVE_SGX_OCALL_CHECK_ERROR_E_R(
			ocall_decent_untrusted_file_open,
			&m_ptr,
			path.c_str(),
			mode.c_str()
		);
	}

	~UntrustedFileImpl()
	{
		ocall_decent_untrusted_file_close(
			m_ptr
		);
	}

	void Seek(
		std::ptrdiff_t offset,
		Common::Internal::SysIO::SeekWhence whence =
			Common::Internal::SysIO::SeekWhence::Begin
	)
	{
		DECENTENCLAVE_SGX_OCALL_CHECK_ERROR_E_R(
			ocall_decent_untrusted_file_seek,
			m_ptr,
			offset,
			static_cast<uint8_t>(whence)
		);
	}

	size_t Tell() const
	{
		size_t ret = 0;
		DECENTENCLAVE_SGX_OCALL_CHECK_ERROR_E_R(
			ocall_decent_untrusted_file_tell,
			m_ptr,
			&ret
		);
		return ret;
	}

	void Flush()
	{
		DECENTENCLAVE_SGX_OCALL_CHECK_ERROR_E_R(
			ocall_decent_untrusted_file_flush,
			m_ptr
		);
	}

	size_t ReadBytesRaw(void* buffer, size_t size)
	{
		UntrustedBuffer<uint8_t> ub;
		DECENTENCLAVE_SGX_OCALL_CHECK_ERROR_E_R(
			ocall_decent_untrusted_file_read,
			m_ptr,
			size,
			&(ub.m_data),
			&(ub.m_size)
		);
		std::memcpy(buffer, ub.m_data, ub.m_size);
		return ub.m_size;
	}

	size_t WriteBytesRaw(const void* buffer, size_t size)
	{
		size_t ret = 0;
		DECENTENCLAVE_SGX_OCALL_CHECK_ERROR_E_R(
			ocall_decent_untrusted_file_write,
			m_ptr,
			static_cast<const uint8_t*>(buffer),
			size,
			&ret
		);
		return ret;
	}


private:

	void* m_ptr;

}; // struct UntrustedFileImpl


} // namespace Sgx
} // namespace Trusted
} // namespace DecentEnclave

#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
