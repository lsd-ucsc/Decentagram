// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#ifdef DECENT_ENCLAVE_PLATFORM_SGX_UNTRUSTED

#include "../EnclaveBase.hpp"

#include <string>

#include <sgx_urts.h>
#include <sgx_edger8r.h>

#include <SimpleSysIO/SysCall/Files.hpp>

#include "../../Common/Internal/SimpleSysIO.hpp"
#include "../../Common/Sgx/DevModeDefs.hpp"
#include "../../Common/Sgx/Exceptions.hpp"


extern "C" sgx_status_t ecall_enclave_common_init(
	sgx_enclave_id_t eid,
	sgx_status_t* retval,
	sgx_enclave_id_t enclave_id
);


namespace DecentEnclave
{
namespace Untrusted
{
namespace Sgx
{


class SgxEnclave : virtual public Untrusted::EnclaveBase
{
public:
	SgxEnclave(
		const std::string& enclaveImgPath = DECENT_ENCLAVE_PLATFORM_SGX_IMAGE,
		const std::string& launchTokenPath = DECENT_ENCLAVE_PLATFORM_SGX_TOKEN
	) :
		m_encId(0)
	{
		namespace _SysCall = Common::Internal::SysIO::SysCall;

		sgx_launch_token_t token = { 0 };
		static constexpr size_t tokenLen = sizeof(sgx_launch_token_t);

		try
		{
			auto file = _SysCall::RBinaryFile::Open(launchTokenPath);
			auto tokenBuf = file->ReadBytes<std::vector<uint8_t> >();
			if (tokenLen == tokenBuf.size())
			{
				std::copy(tokenBuf.begin(), tokenBuf.end(), std::begin(token));
			}
		}
		catch(const std::exception&)
		{
			// Failed to open token file, maybe it doesn't exist, which is fine
		}

		int updated = 0;
		sgx_status_t ret = sgx_create_enclave(
			enclaveImgPath.c_str(),
			DECENTENCLAVE_SGX_DEBUG_FLAG,
			&token,
			&updated,
			&m_encId,
			nullptr
		);
		DECENTENCLAVE_CHECK_SGX_RUNTIME_ERROR(
			ret,
			sgx_create_enclave
		);

		if (updated == 1)
		{
			std::vector<uint8_t> tokenBuf(std::begin(token), std::end(token));

			auto file = _SysCall::WBinaryFile::Create(launchTokenPath);
			file->WriteBytes(tokenBuf);
		}

		DECENTENCLAVE_SGX_ECALL_CHECK_ERROR_E_R(
			ecall_enclave_common_init,
			m_encId,
			m_encId
		);
	}

	SgxEnclave(const SgxEnclave& other) = delete;
	SgxEnclave(SgxEnclave&& other) = delete;

	// LCOV_EXCL_START
	virtual ~SgxEnclave()
	{
		sgx_destroy_enclave(m_encId);
	}
	// LCOV_EXCL_STOP

	SgxEnclave& operator=(const SgxEnclave& other) = delete;
	SgxEnclave& operator=(SgxEnclave&& other) = delete;


	virtual const char* GetPlatformName() const override
	{
		return "SGX";
	}


protected:

	sgx_enclave_id_t m_encId;
}; // class SgxEnclave


} // namespace Sgx
} // namespace Untrusted
} // namespace DecentEnclave

#endif // DECENT_ENCLAVE_PLATFORM_SGX_UNTRUSTED
