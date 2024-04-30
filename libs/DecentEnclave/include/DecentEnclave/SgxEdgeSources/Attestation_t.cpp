// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#include <cstddef>
#include <cstdint>

#include <vector>

#include <sgx_error.h>

#include "../Common/Platform/Print.hpp"
#include "../Trusted/AuthListMgr.hpp"
#include "../Trusted/Sgx/EnclaveIdentity.hpp"


extern "C" sgx_status_t ecall_enclave_common_init(
	sgx_enclave_id_t enclave_id
)
{
	using namespace DecentEnclave::Common;
	using namespace DecentEnclave::Trusted::Sgx;

	try
	{
		SelfEnclaveId::Init(enclave_id);

		return SGX_SUCCESS;
	}
	catch(const std::exception& e)
	{
		Platform::Print::StrErr(e.what());
		return SGX_ERROR_UNEXPECTED;
	}
}


extern "C" sgx_status_t ecall_decent_common_init(
	const uint8_t* auth_list,
	size_t auth_list_size
)
{
	using namespace DecentEnclave::Common;
	using namespace DecentEnclave::Trusted;

	try
	{
		std::vector<uint8_t> authListAdvRlp(auth_list, auth_list + auth_list_size);

		const auto& authListMgr = AuthListMgr::GetInstance(&authListAdvRlp);

		size_t listLen = authListMgr.GetAuthList().get_HashToName().size();
		Platform::Print::Str(
			"AuthList loaded with " + std::to_string(listLen) + " entries\n"
		);

		return SGX_SUCCESS;
	}
	catch(const std::exception& e)
	{
		Platform::Print::StrErr(e.what());
		return SGX_ERROR_UNEXPECTED;
	}

}
