// Copyright (c) 2023 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#include <DecentEnclave/Common/DecentTlsConfig.hpp>
#include <DecentEnclave/Common/Logging.hpp>
#include <DecentEnclave/Common/Platform/Print.hpp>
#include <DecentEnclave/Common/Sgx/MbedTlsInit.hpp>

#include <DecentEnclave/Trusted/PlatformId.hpp>
#include <DecentEnclave/Trusted/SKeyring.hpp>
#include <DecentEnclave/Trusted/Sgx/EnclaveIdentity.hpp>
#include <DecentEnclave/Trusted/AppCertRequester.hpp>

#include <EclipseMonitor/Eth/DataTypes.hpp>

#include "Certs.hpp"
#include "Keys.hpp"
#include "End2EndLatTest.hpp"

using namespace DecentEnclave;
using namespace DecentEnclave::Common;
using namespace DecentEnclave::Common::Sgx;
using namespace DecentEnclave::Trusted;
using namespace DecentEnclave::Trusted::Sgx;

using namespace mbedTLScpp;


namespace End2EndLatency
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


void Init(
	const EclipseMonitor::Eth::ContractAddr& pubsubAddr,
	const EclipseMonitor::Eth::ContractAddr& publisherAddr,
	const EclipseMonitor::Eth::ContractAddr& subscriberAddr
)
{
	GlobalInitialization();

	PrintMyInfo();

	RequestAppCert<DecentCert_Secp256r1>("Secp256r1");
	RequestAppCert<DecentCert_Secp256k1>("Secp256k1");

	RunTest(pubsubAddr, publisherAddr, subscriberAddr);
}

} // namespace End2EndLatency


extern "C" sgx_status_t ecall_decent_end2end_lat_init(
	const uint8_t* pubsub_addr,
	const uint8_t* pub_addr,
	const uint8_t* sub_addr
)
{
	try
	{
		EclipseMonitor::Eth::ContractAddr pubsubAddr;
		std::copy(pubsub_addr, pubsub_addr + pubsubAddr.size(), pubsubAddr.begin());
		EclipseMonitor::Eth::ContractAddr publisherAddr;
		std::copy(pub_addr, pub_addr + publisherAddr.size(), publisherAddr.begin());
		EclipseMonitor::Eth::ContractAddr subscriberAddr;
		std::copy(sub_addr, sub_addr + subscriberAddr.size(), subscriberAddr.begin());

		End2EndLatency::Init(
			pubsubAddr,
			publisherAddr,
			subscriberAddr
		);
		return SGX_SUCCESS;
	}
	catch(const std::exception& e)
	{
		Platform::Print::StrErr(e.what());
		return SGX_ERROR_UNEXPECTED;
	}
}
