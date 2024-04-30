// Copyright (c) 2023 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <DecentEnclave/Common/Sgx/Exceptions.hpp>
#include <DecentEnclave/Trusted/Sgx/UntrustedBuffer.hpp>
#include <SimpleObjects/SimpleObjects.hpp>
#include <SimpleRlp/SimpleRlp.hpp>


extern "C" sgx_status_t ocall_ethereum_clt_get_receipts(
	sgx_status_t* retval,
	const void*   host_blk_svc,
	uint64_t      blk_num,
	uint8_t**     out_buf,
	size_t*       out_buf_size
);

extern "C" sgx_status_t ocall_ethereum_clt_get_latest_blknum(
	sgx_status_t* retval,
	const void*   host_blk_svc,
	uint64_t*     out_blk_num
);


namespace EthereumClt
{
namespace Trusted
{


class HostBlockService
{
public:
	HostBlockService(void* hostBlkSvc) :
		m_ptr(hostBlkSvc)
	{}

	~HostBlockService() = default;

	SimpleObjects::Object GetReceiptsRlpByNum(uint64_t blockNum) const
	{
		DecentEnclave::Trusted::Sgx::UntrustedBuffer<uint8_t> ub;
		DECENTENCLAVE_SGX_OCALL_CHECK_ERROR_E_R(
			ocall_ethereum_clt_get_receipts,
			m_ptr,
			blockNum,
			&(ub.m_data),
			&(ub.m_size)
		);

		auto rlp = ub.CopyToContainer<std::vector<uint8_t> >();

		return SimpleRlp::GeneralParser().Parse(rlp);
	}

	uint64_t GetLatestBlockNum() const
	{
		uint64_t ret;
		DECENTENCLAVE_SGX_OCALL_CHECK_ERROR_E_R(
			ocall_ethereum_clt_get_latest_blknum,
			m_ptr,
			&ret
		);

		return ret;
	}

private:

	void* m_ptr;
}; // class HostBlockService


} // namespace Trusted
} // namespace EthereumClt
