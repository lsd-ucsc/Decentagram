// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED


#include <string>
#include <vector>

#include <sgx_utils.h>

#include <SimpleObjects/ToString.hpp>

#include "../../Common/Internal/SimpleObj.hpp"
#include "../../Common/Sgx/Exceptions.hpp"


namespace DecentEnclave
{
namespace Trusted
{
namespace Sgx
{


struct EnclaveIdentity
{

	static sgx_report_t GenSelfReport()
	{
		sgx_report_t res;
		sgx_status_t sgxRet = sgx_create_report(nullptr, nullptr, &res);
		DECENTENCLAVE_CHECK_SGX_RUNTIME_ERROR(sgxRet, sgx_create_report);

		return res;
	}


	static std::vector<uint8_t> GenSelfHash()
	{
		sgx_report_t rep = GenSelfReport();

		return std::vector<uint8_t>(
			rep.body.mr_enclave.m,
			rep.body.mr_enclave.m + sizeof(rep.body.mr_enclave.m)
		);
	}


	static const std::vector<uint8_t>& GetSelfHash()
	{
		static const std::vector<uint8_t> sk_selfHash = GenSelfHash();

		return sk_selfHash;
	}


	static std::string BuildSelfHashHex()
	{
		return Common::Internal::Obj::Codec::HEX::
			template Encode<std::string>(GetSelfHash());
	}


	static const std::string& GetSelfHashHex()
	{

		static const std::string sk_selfHashHex = BuildSelfHashHex();

		return sk_selfHashHex;
	}


}; // struct EnclaveIdentity


struct SelfEnclaveId
{

	static sgx_enclave_id_t Init(sgx_enclave_id_t id)
	{
		static const sgx_enclave_id_t sk_enclaveId = id;

		return sk_enclaveId;
	}

	static sgx_enclave_id_t Get()
	{
		return Init(0);
	}

}; // struct SelfEnclaveId



} // namespace Sgx
} // namespace Trusted
} // namespace DecentEnclave

#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
