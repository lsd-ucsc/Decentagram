// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstddef>
#include <cstdint>

#include <memory>
#include <vector>

#include <mbedTLScpp/TlsConfig.hpp>
#include <SimpleObjects/Internal/make_unique.hpp>

#include "CertStore.hpp"
#include "Exceptions.hpp"
#include "Internal/SimpleObj.hpp"
#include "Keyring.hpp"
#include "Platform/Print.hpp"
#include "Platform/Random.hpp"


namespace DecentEnclave
{
namespace Common
{


class DecentTlsConfig :
	public mbedTLScpp::TlsConfig
{
public: // static members:

	using Base = mbedTLScpp::TlsConfig;

	static std::shared_ptr<DecentTlsConfig>
	MakeTlsConfig(
		bool isServer,
		const std::string& keyName,
		const std::string& certName
	)
	{
		auto key = Keyring::GetInstance()[keyName].GetPkeyPtr();
		auto cert = CertStore::GetInstance()[certName].GetCertBase();

		return std::make_shared<DecentTlsConfig>(
			true, isServer, false, /* no verification for now (TODO) */
			MBEDTLS_SSL_PRESET_DEFAULT,
			nullptr, //cert, // CA cert, use our own cert as CA
			nullptr,
			cert,
			key,
			Internal::Obj::Internal::make_unique<Platform::RandGenerator>(),
			nullptr
		);
	}

public:

	DecentTlsConfig(
		bool isStream, bool isServer, bool vrfyPeer,
		int preset,
		std::shared_ptr<const mbedTLScpp::X509Cert> ca,
		std::shared_ptr<const mbedTLScpp::X509Crl>  crl,
		std::shared_ptr<const mbedTLScpp::X509Cert> cert,
		std::shared_ptr<const mbedTLScpp::PKeyBase<> > prvKey,
		std::unique_ptr<mbedTLScpp::RbgInterface> rand,
		std::shared_ptr<mbedTLScpp::TlsSessTktMgrIntf> ticketMgr
	) :
		Base(
			isStream, isServer, vrfyPeer,
			preset,
			ca,
			crl,
			cert,
			prvKey,
			std::move(rand),
			ticketMgr,
			mbedTLScpp::TlsVersion::Tls1_2
		)
	{}


	virtual int CustomVerifyCert(
		mbedtls_x509_crt& /* cert */,
		int               /* depth */,
		uint32_t&         flag
	) const override
	{
		Platform::Print::StrDebug("CustomVerifyCert() called");
		flag = 0;
		// TODO: Implement this function.
		return MBEDTLS_EXIT_SUCCESS;
	}


private:


}; // class DecentTlsConfig


} // namespace Common
} // namespace DecentEnclave
