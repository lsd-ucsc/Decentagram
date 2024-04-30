// Copyright (c) 2022 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#include <sgx_edger8r.h>

#include <DecentEnclave/Common/Platform/Print.hpp>
#include <DecentEnclave/Common/Sgx/MbedTlsInit.hpp>

#include <DecentEnclave/Trusted/AppCertRequester.hpp>
#include <DecentEnclave/Trusted/DecentLambdaSvr.hpp>
#include <DecentEnclave/Trusted/PlatformId.hpp>
#include <DecentEnclave/Trusted/SKeyring.hpp>
#include <DecentEnclave/Trusted/Sgx/EnclaveIdentity.hpp>

#include <EthereumClt/Trusted/BlockchainMgr.hpp>
#include <EthereumClt/Trusted/Pubsub/SubscriberHandler.hpp>
#include <EthereumClt/Trusted/ReceiptSubscriber.hpp>

#include <EclipseMonitor/MonitorReport.hpp>

#include "Certs.hpp"
#include "Keys.hpp"


using EthChainConfig = EclipseMonitor::Eth::GoerliConfig;


namespace EthereumClt
{


std::shared_ptr<Trusted::BlockchainMgr<EthChainConfig> >
	g_blockchainMgr;


void GlobalInitialization()
{
	using namespace DecentEnclave::Common;

	// Initialize mbedTLS
	Sgx::MbedTlsInit::Init();

	using namespace DecentEnclave::Trusted;
	// Register keys
	DecentKey_Secp256r1::Register();
	DecentKey_Secp256k1::Register();

	// Register certificates
	DecentCert_Secp256r1::Register();
	DecentCert_Secp256k1::Register();
}


void PrintMyInfo()
{
	using namespace DecentEnclave::Common;
	using namespace DecentEnclave::Trusted;

	Platform::Print::StrInfo(
		"My platform ID is              : " + PlatformId::GetIdHex()
	);

	const auto& selfHash = DecentEnclave::Trusted::Sgx::EnclaveIdentity::GetSelfHashHex();
	Platform::Print::StrInfo(
		"My enclave hash is             : " + selfHash
	);

	std::string secp256r1KeyFp =
		DecentKey_Secp256r1::GetInstance().GetKeySha256Hex();
	std::string secp256k1KeyFp =
		DecentKey_Secp256k1::GetInstance().GetKeySha256Hex();
	std::string keyringHash = Keyring::GetInstance().GenHashHex();
	Platform::Print::StrInfo(
		"My key fingerprint (SECP256R1) : " + secp256r1KeyFp
	);
	Platform::Print::StrInfo(
		"My key fingerprint (SECP256K1) : " + secp256k1KeyFp
	);
	Platform::Print::StrInfo(
		"My keyring hash is             : " + keyringHash
	);
}


template<typename _CertStoreCertType>
inline void RequestAppCert(const std::string& keyName)
{
	std::string pemChain = DecentEnclave::Trusted::AppCertRequester(
		"DecentServer",
		keyName
	).Request();

	auto cert = std::make_shared<mbedTLScpp::X509Cert>(
		mbedTLScpp::X509Cert::FromPEM(pemChain)
	);

	_CertStoreCertType::Update(cert);
}


inline void HandlePubsubSubReq(
	DecentEnclave::Trusted::LambdaHandlerMgr::SocketPtrType& socket,
	const DecentEnclave::Trusted::LambdaHandlerMgr::MsgIdExtType& msgIdExt,
	const DecentEnclave::Trusted::LambdaHandlerMgr::MsgContentType& msgContent
)
{
	Trusted::Pubsub::SubReq(
		g_blockchainMgr,
		socket,
		msgIdExt,
		msgContent
	);
}


inline void HandleReceiptSubReq(
	DecentEnclave::Trusted::LambdaHandlerMgr::SocketPtrType& socket,
	const DecentEnclave::Trusted::LambdaHandlerMgr::MsgIdExtType& msgIdExt,
	const DecentEnclave::Trusted::LambdaHandlerMgr::MsgContentType& msgContent
)
{
	Trusted::ReceiptSubReq(
		g_blockchainMgr,
		socket,
		msgIdExt,
		msgContent
	);
}


void Init(
	const EclipseMonitor::MonitorConfig& mConf,
	EclipseMonitor::Eth::BlockNumber startBlkNum,
	const EclipseMonitor::Eth::ContractAddr& syncContractAddr,
	const std::string& syncEventSign,
	const EclipseMonitor::Eth::ContractAddr& pubsubContractAddr,
	std::unique_ptr<Trusted::HostBlockService> blkSvc
)
{
	using namespace DecentEnclave::Trusted;

	GlobalInitialization();
	PrintMyInfo();

	RequestAppCert<DecentCert_Secp256r1>("Secp256r1");
	RequestAppCert<DecentCert_Secp256k1>("Secp256k1");

	g_blockchainMgr = std::make_shared<Trusted::BlockchainMgr<EthChainConfig> >(
		mConf,
		startBlkNum,
		syncContractAddr,
		syncEventSign,
		SimpleObjects::Internal::
			make_unique<Trusted::Pubsub::SubscriberService>(
				pubsubContractAddr,
				"ServiceDeployed(address)",
				"PublisherRegistered(address,address)",
				"NotifySubscribers(bytes)"
		),
		std::move(blkSvc)
	);

	LambdaServerConfig lambdaSvrConfig(
		"Secp256r1",
		"Secp256r1"
	);
	LambdaServerConfig::GetInstance(&lambdaSvrConfig);

	LambdaHandlerMgr::GetInstance().RegisterHandler(
		"PubSub.Subscribe",
		HandlePubsubSubReq
	);
	LambdaHandlerMgr::GetInstance().RegisterHandler(
		"Receipt.Subscribe",
		HandleReceiptSubReq
	);
}


void RecvBlock(const std::vector<uint8_t>& hdrRlp)
{
	g_blockchainMgr->AppendBlock(hdrRlp);
}


} // namespace EthereumClt


extern "C" sgx_status_t ecall_ethereum_clt_init(
	const uint8_t* in_conf,
	size_t in_conf_size,
	uint64_t start_blk_num,
	const uint8_t* in_sync_addr,
	const char* in_sync_esign,
	const uint8_t* in_pubsub_addr,
	void* host_blk_svc
)
{
	using namespace EthereumClt;

	try
	{
		std::vector<uint8_t> mConfAdvRlp(in_conf, in_conf + in_conf_size);

		EclipseMonitor::MonitorConfig mConf =
			EclipseMonitor::MonitorConfigParser().Parse(mConfAdvRlp);

		EclipseMonitor::Eth::ContractAddr syncContractAddr;
		std::copy(
			in_sync_addr,
			in_sync_addr + syncContractAddr.size(),
			syncContractAddr.begin()
		);

		std::string syncEventSign(in_sync_esign);

		EclipseMonitor::Eth::ContractAddr pubsubContractAddr;
		std::copy(
			in_pubsub_addr,
			in_pubsub_addr + pubsubContractAddr.size(),
			pubsubContractAddr.begin()
		);

		std::unique_ptr<Trusted::HostBlockService> blkSvc =
			SimpleObjects::Internal::
				make_unique<Trusted::HostBlockService>(host_blk_svc);

		EthereumClt::Init(
			mConf,
			start_blk_num,
			syncContractAddr,
			syncEventSign,
			pubsubContractAddr,
			std::move(blkSvc)
		);
		return SGX_SUCCESS;
	}
	catch(const std::exception& e)
	{
		using namespace DecentEnclave::Common;
		Platform::Print::StrErr(e.what());
		return SGX_ERROR_UNEXPECTED;
	}
}


extern "C" sgx_status_t ecall_ethereum_clt_recv_block(
	const uint8_t* hdr_rlp,
	size_t hdr_size
)
{
	try
	{
		std::vector<uint8_t> hdrRlp(hdr_rlp, hdr_rlp + hdr_size);
		EthereumClt::RecvBlock(hdrRlp);

		return SGX_SUCCESS;
	}
	catch(const std::exception& e)
	{
		using namespace DecentEnclave::Common;
		Platform::Print::StrErr(e.what());
		return SGX_ERROR_UNEXPECTED;
	}
}
