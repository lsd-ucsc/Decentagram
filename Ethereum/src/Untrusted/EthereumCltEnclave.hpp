// Copyright (c) 2022 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <DecentEnclave/Common/Sgx/Exceptions.hpp>
#include <DecentEnclave/Untrusted/Sgx/DecentSgxEnclave.hpp>
#include <EclipseMonitor/Eth/DataTypes.hpp>
#include <EclipseMonitor/MonitorReport.hpp>

#include <EthereumClt/Untrusted/BlockReceiver.hpp>
#include <EthereumClt/Untrusted/HostBlockService.hpp>


extern "C" sgx_status_t ecall_ethereum_clt_init(
	sgx_enclave_id_t eid,
	sgx_status_t*    retval,
	const uint8_t*   in_conf,
	size_t           in_conf_size,
	uint64_t         start_blk_num,
	const uint8_t*   in_sync_addr,
	const char*      in_sync_esign,
	const uint8_t*   in_pubsub_addr,
	void*            host_blk_svc
);
extern "C" sgx_status_t ecall_ethereum_clt_recv_block(
	sgx_enclave_id_t eid,
	sgx_status_t*    retval,
	const uint8_t*   blk_data,
	size_t           blk_size
);


namespace EthereumClt
{

class EthereumCltEnclave :
	public DecentEnclave::Untrusted::Sgx::DecentSgxEnclave,
	public BlockReceiver
{
public: // static members:

	using Base = DecentEnclave::Untrusted::Sgx::DecentSgxEnclave;


public:


	EthereumCltEnclave(
		const EclipseMonitor::MonitorConfig& mConf,
		EclipseMonitor::Eth::BlockNumber startBlkNum,
		const EclipseMonitor::Eth::ContractAddr& syncContractAddr,
		const std::string& syncEventSign,
		const EclipseMonitor::Eth::ContractAddr& pubsubContractAddr,
		std::shared_ptr<HostBlockService> hostBlockService,
		const std::vector<uint8_t>& authList,
		const std::string& enclaveImgPath = DECENT_ENCLAVE_PLATFORM_SGX_IMAGE,
		const std::string& launchTokenPath = DECENT_ENCLAVE_PLATFORM_SGX_TOKEN
	) :
		Base(authList, enclaveImgPath, launchTokenPath),
		m_hostBlockService(hostBlockService)
	{
		auto mConfAdvRlp = AdvancedRlp::GenericWriter::Write(mConf);

		DECENTENCLAVE_SGX_ECALL_CHECK_ERROR_E_R(
			ecall_ethereum_clt_init,
			m_encId,
			mConfAdvRlp.data(),
			mConfAdvRlp.size(),
			startBlkNum,
			syncContractAddr.data(),
			syncEventSign.c_str(),
			pubsubContractAddr.data(),
			m_hostBlockService.get()
		);
	}


	virtual void RecvBlock(const std::vector<uint8_t>& blockRlp) override
	{
		DECENTENCLAVE_SGX_ECALL_CHECK_ERROR_E_R(
			ecall_ethereum_clt_recv_block,
			m_encId,
			blockRlp.data(),
			blockRlp.size()
		);
	}


private:
	std::shared_ptr<HostBlockService> m_hostBlockService;
}; // class EthereumCltEnclave

} // namespace EthereumClt
