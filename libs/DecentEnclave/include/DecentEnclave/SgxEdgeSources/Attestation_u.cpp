// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#include <cstring>

#include <thread>

#include <sgx_ukey_exchange.h>
#include <sgx_uae_epid.h>
#include <SimpleRlp/SimpleRlp.hpp>

#include "../Common/Internal/SimpleRlp.hpp"
#include "../Common/Platform/Print.hpp"
#include "../Common/Sgx/EpidRaMessages.hpp"
#include "../Common/Sgx/IasRequester.hpp"
#include "../Untrusted/Sgx/UntrustedBuffer.hpp"


extern "C"
sgx_status_t
decent_ra_get_ga(
	sgx_enclave_id_t eid,
	sgx_status_t* retval,
	sgx_ra_context_t context,
	sgx_ec256_public_t* g_a
);

extern "C"
sgx_status_t
decent_ra_proc_msg2_trusted(
	sgx_enclave_id_t eid,
	sgx_status_t* retval,
	sgx_ra_context_t context,
	const sgx_ra_msg2_t* p_msg2,
	const sgx_target_info_t* p_qe_target,
	sgx_report_t* p_report,
	sgx_quote_nonce_t* p_nonce
);

extern "C"
sgx_status_t
decent_ra_get_msg3_trusted(
	sgx_enclave_id_t eid,
	sgx_status_t* retval,
	sgx_ra_context_t context,
	uint32_t quote_size,
	sgx_report_t* qe_report,
	sgx_ra_msg3_t* p_msg3,
	uint32_t msg3_size
);

extern "C" sgx_status_t ocall_decent_attest_get_extended_epid_grp_id(
	uint32_t* extGrpId
)
{
	if (extGrpId == nullptr)
	{
		return SGX_ERROR_INVALID_PARAMETER;
	}

	sgx_status_t sdkRes = sgx_get_extended_epid_group_id(extGrpId);

	return sdkRes;
}

extern "C" sgx_status_t ocall_decent_attest_get_msg1(
	uint64_t enclave_id,
	uint32_t ra_ctx,
	sgx_ra_msg1_t* msg1
)
{
	if (msg1 == nullptr)
	{
		return SGX_ERROR_INVALID_PARAMETER;
	}

	sgx_ra_msg1_t tmpMsg1;
	sgx_status_t enclaveRet = SGX_ERROR_UNEXPECTED;
	try{
		std::thread tmpThread(
			[&enclaveRet, ra_ctx, enclave_id, &tmpMsg1]() {
				enclaveRet = sgx_ra_get_msg1(
					ra_ctx,
					enclave_id,
					decent_ra_get_ga,
					&tmpMsg1
				);
			}
		);
		tmpThread.join();
	}
	catch (const std::exception& e)
	{
		DecentEnclave::Common::Platform::Print::StrErr(
			std::string("Exception caught in ocall_decent_attest_get_msg1 - ") +
			e.what()
		);
	}

	*msg1 = tmpMsg1;

	return enclaveRet;
}


extern "C" sgx_status_t ocall_decent_attest_get_msg3(
	uint64_t enclave_id,
	uint32_t ra_ctx,
	const sgx_ra_msg2_t* msg2,
	size_t msg2_size,
	uint8_t** out_msg3,
	size_t* out_msg3_size
)
{
	if (
		msg2 == nullptr ||
		out_msg3 == nullptr ||
		*out_msg3 != nullptr
	)
	{
		return SGX_ERROR_INVALID_PARAMETER;
	}

	try
	{
		sgx_ra_msg3_t* tmpMsg3 = nullptr;
		uint32_t tmpMsg3Size = 0;
		sgx_status_t enclaveRet = SGX_SUCCESS;
		std::thread tmpThread(
			[
				&enclaveRet,
				enclave_id,
				ra_ctx,
				msg2,
				msg2_size,
				&tmpMsg3,
				&tmpMsg3Size
			]() {
				enclaveRet = sgx_ra_proc_msg2(
					ra_ctx,
					enclave_id,
					decent_ra_proc_msg2_trusted,
					decent_ra_get_msg3_trusted,
					msg2,
					static_cast<uint32_t>(msg2_size),
					&tmpMsg3,
					&tmpMsg3Size
				);
			}
		);
		tmpThread.join();

		if (enclaveRet != SGX_SUCCESS)
		{
			return enclaveRet;
		}

		//Copy msg3 to our buffer pointer to avoid the mix use of malloc and delete[];
		*out_msg3 = new uint8_t[tmpMsg3Size];
		*out_msg3_size = tmpMsg3Size;
		std::memcpy(*out_msg3, tmpMsg3, tmpMsg3Size);
		std::free(tmpMsg3);

		return SGX_SUCCESS;
	}
	catch(const std::exception& e)
	{
		DecentEnclave::Common::Platform::Print::StrErr(
			std::string("Exception caught in ocall_decent_attest_get_msg3 - ") +
			e.what()
		);
		return SGX_ERROR_UNEXPECTED;
	}
}


extern "C" sgx_status_t ocall_decent_attest_ias_req_get_sigrl(
	const void* ias_requester_ptr,
	const sgx_epid_group_id_t* gid,
	char** out_sigrl,
	size_t* out_sigrl_size
)
{
	using _IasRequester = DecentEnclave::Common::Sgx::IasRequester;
	using _UBuffer = DecentEnclave::Untrusted::Sgx::UntrustedBuffer<char>;

	if (
		ias_requester_ptr == nullptr ||
		gid == nullptr ||
		out_sigrl == nullptr ||
		*out_sigrl != nullptr ||
		out_sigrl_size == nullptr
	)
	{
		return SGX_ERROR_INVALID_PARAMETER;
	}

	const _IasRequester& iasRequester =
		*static_cast<const _IasRequester*>(ias_requester_ptr);

	std::string sigrlB64 = iasRequester.GetSigrl(*gid);

	_UBuffer uBuffer = _UBuffer::Allocate(sigrlB64.size());
	std::copy(sigrlB64.begin(), sigrlB64.end(), uBuffer.m_data);

	*out_sigrl      = uBuffer.m_data;
	*out_sigrl_size = uBuffer.m_size;

	return SGX_SUCCESS;
}


extern "C" sgx_status_t ocall_decent_attest_ias_req_get_report(
	const void* ias_requester_ptr,
	const char* req_body,
	uint8_t** out_report,
	size_t* out_report_size
)
{
	using _IasRequester = DecentEnclave::Common::Sgx::IasRequester;
	using _IasReportSet = DecentEnclave::Common::Sgx::IasReportSet;
	using _UBuffer = DecentEnclave::Untrusted::Sgx::UntrustedBuffer<uint8_t>;

	if (
		ias_requester_ptr == nullptr ||
		req_body == nullptr ||
		out_report == nullptr ||
		*out_report != nullptr ||
		out_report_size == nullptr
	)
	{
		return SGX_ERROR_INVALID_PARAMETER;
	}

	const _IasRequester& iasRequester =
		*static_cast<const _IasRequester*>(ias_requester_ptr);

	_IasReportSet iasReportSet = iasRequester.GetReport(req_body);

	auto iasReportSetARlp = DecentEnclave::Common::Internal::
		Rlp::WriterGeneric::Write(iasReportSet);
	_UBuffer uBuffer = _UBuffer::Allocate(iasReportSetARlp.size());
	std::copy(iasReportSetARlp.begin(), iasReportSetARlp.end(), uBuffer.m_data);

	*out_report      = uBuffer.m_data;
	*out_report_size = uBuffer.m_size;

	return SGX_SUCCESS;
}

