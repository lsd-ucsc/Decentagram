// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <string>
#include <vector>

#include "../Common/AesGcmStreamSocket.hpp"
#include "../Common/CertStore.hpp"
#include "../Common/DecentCerts.hpp"
#include "../Common/Internal/SimpleObj.hpp"
#include "../Common/Keyring.hpp"
#include "../Common/KeyringKey.hpp"
#include "../Common/Platform/Print.hpp"
#include "../Common/Platform/Random.hpp"
#include "AuthListMgr.hpp"
#include "ComponentConnection.hpp"
#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
#include "Sgx/LaInitiator.hpp"
#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED

#include <mbedTLScpp/X509Req.hpp>


namespace DecentEnclave
{
namespace Trusted
{


class AppCertRequester
{
public: // static members:

	using KeyringType    = Common::Keyring;
	using KeyType        = Common::KeyringKey::PKeyType;
	using SecSocketWrap  = Common::AesGcmStreamSocket<128>;
	using RandType       = Common::Platform::RandGenerator;
	using HandshakerType = Common::AesGcmSocketHandshaker<128>;

	static std::vector<uint8_t> GenerateCSR(
		const KeyType& key
	)
	{
		RandType rand;

		mbedTLScpp::X509ReqWriter writer(
			mbedTLScpp::HashType::SHA256,
			key,
			"CN=DecentApp,O=DecentEnclave,OU=DecentApp"
		);

		return writer.GetDer(rand);
	}

public:

	AppCertRequester(
		const std::string& svrName,
		const std::string& keyName
	) :
		AppCertRequester(svrName, keyName, keyName)
	{}

	AppCertRequester(
		const std::string& svrName,
		const std::string& keyName,
		const std::string& certName
	) :
		m_svrName(svrName),
		m_keyName(keyName),
		m_certName(certName),
		m_csr(GenerateCSR(KeyringType::GetInstance()[keyName].GetPkey())),
		m_appCertReq(BuildAppCertReq())
	{}

	~AppCertRequester() = default;

	std::string Request()
	{
		static const std::string sk_reqBody = "{\"method\":\"req_app_cert\"}";

		auto socket = ComponentConnection::Connect(m_svrName);
		socket->SizedSendBytes(sk_reqBody);

		auto secSocket =
			SecSocketWrap::FromHandshake(
				BuildHandshake(),
				std::move(socket),
				Common::Internal::Obj::Internal::make_unique<RandType>()
			);

		secSocket->SizedSendBytes(m_appCertReq);

		auto pemChain = secSocket->SizedRecvBytes<std::string>();

		Common::Platform::Print::StrInfo(
			"Certificate chain received:\n" + pemChain
		);

		return pemChain;
	}

private:

	std::vector<uint8_t> BuildAppCertReq()
	{
		Common::AppCertRequest certReq;
		certReq.get_KeyName() = m_keyName;
		certReq.get_CSR() = Common::Internal::Obj::Bytes(m_csr);
		certReq.get_AuthList() = Common::Internal::Obj::Bytes(
			AuthListMgr::GetInstance().GetAuthListAdvRlp()
		);

		return AdvancedRlp::GenericWriter::Write(certReq);
	}

#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
	std::unique_ptr<HandshakerType> BuildHandshake()
	{
		using LaInitiator = Sgx::LaInitiator;

		return Common::Internal::Obj::Internal::make_unique<LaInitiator>();
	}
#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED

	std::string m_svrName;
	std::string m_keyName;
	std::string m_certName;

	std::vector<uint8_t> m_csr;
	std::vector<uint8_t> m_appCertReq;

}; // class AppCertRequester


} // namespace Trusted
} // namespace DecentEnclave
