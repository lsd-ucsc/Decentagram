// Copyright (c) 2022 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#include <DecentEnclave/Common/DecentTlsConfig.hpp>
#include <DecentEnclave/Common/Platform/Print.hpp>
#include <DecentEnclave/Common/Sgx/MbedTlsInit.hpp>
#include <DecentEnclave/Common/TlsSocket.hpp>

#include <DecentEnclave/Trusted/PlatformId.hpp>
#include <DecentEnclave/Trusted/SKeyring.hpp>
#include <DecentEnclave/Trusted/Sgx/EnclaveIdentity.hpp>
#include <DecentEnclave/Trusted/AppCertRequester.hpp>
#include <DecentEnclave/Trusted/DecentLambdaClt.hpp>
#include <DecentEnclave/Trusted/HeartbeatRecvMgr.hpp>

#include "Certs.hpp"
#include "Keys.hpp"
#include "EthHeartbeatHandler.hpp"

using namespace DecentEnclave;
using namespace DecentEnclave::Common;
using namespace DecentEnclave::Common::Sgx;
using namespace DecentEnclave::Trusted;
using namespace DecentEnclave::Trusted::Sgx;

using namespace mbedTLScpp;

namespace Revoker
{

void GlobalInitialization()
{
	// Initialize mbedTLS
	MbedTlsInit::Init();

	Trusted::SKeyring::GetMutableInstance(
	).RegisterKey(
		"TestSealKey", 128
	);

	// Register keys
	DecentKey_Secp256r1::Register();
	DecentKey_Secp256k1::Register();

	// Register certificates
	DecentCert_Secp256r1::Register();
	DecentCert_Secp256k1::Register();
}

void PrintMyInfo()
{
	Platform::Print::StrInfo(
		"My platform ID is              : " + Trusted::PlatformId::GetIdHex()
	);

	const auto& selfHash = EnclaveIdentity::GetSelfHashHex();
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


void SubscribeToEthereumClt(
	const EclipseMonitor::Eth::ContractAddr& publisherAddr
)
{
	auto msg = BuildSubscribeMsg(publisherAddr);

	std::shared_ptr<Common::TlsSocket> tlsSocket = MakeLambdaCall(
		"EthereumClt",
		DecentTlsConfig::MakeTlsConfig(
			false,
			"Secp256r1",
			"Secp256r1"
		),
		msg // lvalue reference needed
	);

	std::shared_ptr<HeartbeatTimeConstraint<uint64_t> > hbConstraint =
		std::make_shared<HeartbeatTimeConstraint<uint64_t> >(
			1000
		);

	HeartbeatRecvMgr::GetInstance().AddRecv(
		hbConstraint,
		tlsSocket,
		[](std::vector<uint8_t> heartbeatMsg) -> void
		{
			HandleEthHeartbeatMsg(std::move(heartbeatMsg));
		},
		true
	);
}


void Init(
	const EclipseMonitor::Eth::ContractAddr& publisherAddr
)
{
	GlobalInitialization();

	PrintMyInfo();

	RequestAppCert<DecentCert_Secp256r1>("Secp256r1");
	RequestAppCert<DecentCert_Secp256k1>("Secp256k1");

	SubscribeToEthereumClt(publisherAddr);
}

} // namespace Revoker


extern "C" sgx_status_t ecall_revoker_init(
	const uint8_t* pub_addr
)
{
	try
	{
		EclipseMonitor::Eth::ContractAddr pubAddr;
		std::copy(pub_addr, pub_addr + pubAddr.size(), pubAddr.begin());

		Revoker::Init(pubAddr);

		return SGX_SUCCESS;
	}
	catch(const std::exception& e)
	{
		Platform::Print::StrErr(e.what());
		return SGX_ERROR_UNEXPECTED;
	}
}
