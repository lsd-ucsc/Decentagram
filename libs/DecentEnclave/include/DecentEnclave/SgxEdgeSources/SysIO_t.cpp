// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#include <cstddef>
#include <cstdint>

#include <functional>
#include <vector>

#include <sgx_error.h>

#include "../Common/Platform/Print.hpp"
#include "../Trusted/Sgx/ComponentConnection.hpp"


extern "C" sgx_status_t ecall_decent_ssocket_async_recv_raw_callback(
	uint64_t handler_reg_id,
	const uint8_t* in_data,
	size_t in_data_size,
	uint8_t has_error_occurred
)
{
	using namespace DecentEnclave::Trusted::Sgx;

	try
	{
		auto& handler = GetSSocketAsyncCallbackHandler();
		handler.DispatchCallback(
			handler_reg_id,
			true, // dispose this registration entry after callback
			std::vector<uint8_t>(in_data, in_data + in_data_size),
			has_error_occurred != 0
		);
		return SGX_SUCCESS;
	}
	catch(const std::exception& e)
	{
		DecentEnclave::Common::Platform::Print::StrDebug(
			"ecall_decent_ssocket_async_recv_raw_callback failed with error " +
			std::string(e.what())
		);
		return SGX_ERROR_UNEXPECTED;
	}

}
