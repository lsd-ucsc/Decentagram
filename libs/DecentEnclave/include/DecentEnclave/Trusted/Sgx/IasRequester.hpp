// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


// TODO: #ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED


#include <sgx_edger8r.h>

#include "../../Common/Sgx/IasRequester.hpp"
#include "UntrustedBuffer.hpp"


//==========
// Our EDL functions
//==========


extern "C"
sgx_status_t
ocall_decent_attest_ias_req_get_sigrl(
	sgx_status_t* retval,
	const void* ias_requester_ptr,
	const sgx_epid_group_id_t* gid,
	char** out_sigrl,
	size_t* out_sigrl_size
);

extern "C"
sgx_status_t
ocall_decent_attest_ias_req_get_report(
	sgx_status_t* retval,
	const void* ias_requester_ptr,
	const char* req_body,
	uint8_t** out_report,
	size_t* out_report_size
);


namespace DecentEnclave
{
namespace Trusted
{
namespace Sgx
{


class IasRequester :
	public Common::Sgx::IasRequester
{
public:

	IasRequester(void* iasReqPtr) :
		m_iasReqPtr(iasReqPtr)
	{}

	IasRequester(const IasRequester& rhs) = delete;

	IasRequester(IasRequester&& rhs) :
		m_iasReqPtr(rhs.m_iasReqPtr)
	{
		rhs.m_iasReqPtr = nullptr;
	}

	virtual ~IasRequester()
	{
		// Currently we assume the actual object is owned by the untrusted side
		m_iasReqPtr = nullptr;
	}

	virtual std::string GetSigrl(const sgx_epid_group_id_t& gid) const override
	{
		using _UBuffer = UntrustedBuffer<char>;

		sgx_status_t edgeRet = SGX_ERROR_UNEXPECTED;
		sgx_status_t funcRet = SGX_ERROR_UNEXPECTED;
		_UBuffer uBuf;

		edgeRet = ocall_decent_attest_ias_req_get_sigrl(
			&funcRet,
			m_iasReqPtr,
			&gid,
			&(uBuf.m_data),
			&(uBuf.m_size)
		);
		DECENTENCLAVE_CHECK_SGX_RUNTIME_ERROR(
			edgeRet,
			ocall_decent_attest_ias_req_get_sigrl
		);
		DECENTENCLAVE_CHECK_SGX_RUNTIME_ERROR(
			funcRet,
			ocall_decent_attest_ias_req_get_sigrl
		);

		std::string res = uBuf.CopyToContainer<std::string>();
		return res;
	}

	virtual Common::Sgx::IasReportSet GetReport(
		const std::string& reqBody
	) const override
	{
		using _UBuffer = UntrustedBuffer<uint8_t>;

		sgx_status_t edgeRet = SGX_ERROR_UNEXPECTED;
		sgx_status_t funcRet = SGX_ERROR_UNEXPECTED;
		_UBuffer uBuf;

		edgeRet = ocall_decent_attest_ias_req_get_report(
			&funcRet,
			m_iasReqPtr,
			reqBody.c_str(),
			&(uBuf.m_data),
			&(uBuf.m_size)
		);
		DECENTENCLAVE_CHECK_SGX_RUNTIME_ERROR(
			edgeRet,
			ocall_decent_attest_ias_req_get_sigrl
		);
		DECENTENCLAVE_CHECK_SGX_RUNTIME_ERROR(
			funcRet,
			ocall_decent_attest_ias_req_get_sigrl
		);

		std::vector<uint8_t> iasReportSetARlp =
			uBuf.CopyToContainer<std::vector<uint8_t> >();

		return Common::Sgx::IasReportSetParser().Parse(iasReportSetARlp);
	}

private:

	void* m_iasReqPtr;
}; // class IasRequester


} // namespace Sgx
} // namespace Trusted
} // namespace DecentEnclave

// TODO: #endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
