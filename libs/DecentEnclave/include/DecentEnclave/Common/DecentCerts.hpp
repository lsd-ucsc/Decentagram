// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <string>
#include <vector>

#include <AdvancedRlp/AdvancedRlp.hpp>
#include <mbedTLScpp/BigNumber.hpp>
#include <mbedTLScpp/MsgDigestBase.hpp>
#include <mbedTLScpp/PKey.hpp>
#include <mbedTLScpp/RandInterfaces.hpp>
#include <mbedTLScpp/X509Cert.hpp>
#include <SimpleObjects/Codec/Hex.hpp>
#include <SimpleObjects/SimpleObjects.hpp>

#include "Internal/SimpleObj.hpp"
#include "Internal/SimpleRlp.hpp"
#include "DecentOid.hpp"
#include "Keyring.hpp"


namespace DecentEnclave
{
namespace Common
{


struct DecentCertConfigs
{
	std::string m_ver;
	std::string m_enclaveType;


#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
	static const DecentCertConfigs& GetDefault()
	{
		static const DecentCertConfigs sk_config = {
			"1",
			"SGX_EPID"
		};
		return sk_config;
	}
#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
}; // struct DecentCertConfigs


inline mbedTLScpp::X509Cert IssueSelfRaCert(
	const mbedTLScpp::PKeyBase<>& prvKey,
	const std::string& keyName,
	const std::string& selfHashHex,
	const std::vector<uint8_t>& stdRepData,
	const std::vector<uint8_t>& selfRaReportRlp,
	mbedTLScpp::RbgInterface& rand
)
{
	const DecentCertConfigs& certConfig = DecentCertConfigs::GetDefault();

	mbedTLScpp::X509CertWriter certWriter =
		mbedTLScpp::X509CertWriter::SelfSign(
			mbedTLScpp::HashType::SHA256,
			prvKey,
			"CN=" + selfHashHex + "_" + keyName +
				",O=DecentEnclave,OU=DecentServer"
		);

	mbedTLScpp::BigNum serialNum = mbedTLScpp::BigNum::Rand(32, rand);
	auto decentExtEncKeyHash = Keyring::GetInstance().GenKeyHashList();

	certWriter.SetValidationTime( // There is no trusted inside the enclave
		"20220101000000",
		"20220101000000"
	).SetBasicConstraints(
		true,
		-1
	).SetKeyUsage(
		MBEDTLS_X509_KU_DIGITAL_SIGNATURE |
		MBEDTLS_X509_KU_CRL_SIGN |
		MBEDTLS_X509_KU_KEY_CERT_SIGN |
		MBEDTLS_X509_KU_KEY_AGREEMENT |
		MBEDTLS_X509_KU_NON_REPUDIATION
	).SetNsType(
		MBEDTLS_X509_NS_CERT_TYPE_SSL_CA |
		MBEDTLS_X509_NS_CERT_TYPE_EMAIL_CA |
		MBEDTLS_X509_NS_CERT_TYPE_OBJECT_SIGNING_CA
	).SetSerialNum(
		serialNum
	).SetV3Extension(
		DecentOid::GetVersionOid(),
		false, // Can't make these ext critical, it's not well supported by mbedTLS
		mbedTLScpp::CtnFullR(certConfig.m_ver)
	).SetV3Extension(
		DecentOid::GetEnclaveTypeOid(),
		false,
		mbedTLScpp::CtnFullR(certConfig.m_enclaveType)
	).SetV3Extension(
		DecentOid::GetKeyringHashOid(),
		false,
		mbedTLScpp::CtnFullR(decentExtEncKeyHash)
	).SetV3Extension(
		DecentOid::GetSgxStdReportDataOid(),
		false,
		mbedTLScpp::CtnFullR(stdRepData)
	).SetV3Extension(
		DecentOid::GetSgxSelfRaReportOid(),
		false,
		mbedTLScpp::CtnFullR(selfRaReportRlp)
	);

	auto der = certWriter.GetDer(rand);

	return mbedTLScpp::X509Cert::FromDER(mbedTLScpp::CtnFullR(der));
}


template<typename _PkTraitType>
inline mbedTLScpp::X509Cert IssueAppCert(
	const mbedTLScpp::X509Cert& caCert,
	const mbedTLScpp::PKeyBase<>& prvKey,
	const mbedTLScpp::PKeyBase<_PkTraitType>& appPubKey,
	const std::string& keyName,
	const std::vector<uint8_t>& enclaveHash,
	const std::vector<uint8_t>& authList,
	mbedTLScpp::RbgInterface& rand
)
{
	const DecentCertConfigs& certConfig = DecentCertConfigs::GetDefault();

	std::string enclaveHashHex = Internal::Obj::Codec::HEX::
		template Encode<std::string>(enclaveHash);

	mbedTLScpp::X509CertWriter certWriter =
		mbedTLScpp::X509CertWriter::CaSign(
			mbedTLScpp::HashType::SHA256,
			caCert,
			prvKey,
			appPubKey,
			"CN=" + enclaveHashHex + "_" + keyName +
				",O=DecentEnclave,OU=DecentApp"
		);

	mbedTLScpp::BigNum serialNum = mbedTLScpp::BigNum::Rand(32, rand);

	certWriter.SetValidationTime( // There is no trusted inside the enclave
		"20220101000000",
		"20220101000000"
	).SetBasicConstraints(
		true,
		-1
	).SetKeyUsage(
		MBEDTLS_X509_KU_DIGITAL_SIGNATURE |
		MBEDTLS_X509_KU_CRL_SIGN |
		MBEDTLS_X509_KU_KEY_CERT_SIGN |
		MBEDTLS_X509_KU_KEY_AGREEMENT |
		MBEDTLS_X509_KU_NON_REPUDIATION
	).SetNsType(
		MBEDTLS_X509_NS_CERT_TYPE_SSL_CA |
		MBEDTLS_X509_NS_CERT_TYPE_EMAIL_CA |
		MBEDTLS_X509_NS_CERT_TYPE_OBJECT_SIGNING_CA
	).SetSerialNum(
		serialNum
	).SetV3Extension(
		DecentOid::GetVersionOid(),
		false, // Can't make these ext critical, it's not well supported by mbedTLS
		mbedTLScpp::CtnFullR(certConfig.m_ver)
	).SetV3Extension(
		DecentOid::GetEnclaveTypeOid(),
		false,
		mbedTLScpp::CtnFullR(certConfig.m_enclaveType)
	).SetV3Extension(
		DecentOid::GetAppHashOid(),
		false,
		mbedTLScpp::CtnFullR(enclaveHash)
	).SetV3Extension(
		DecentOid::GetAuthListOid(),
		false,
		mbedTLScpp::CtnFullR(authList)
	);

	auto der = certWriter.GetDer(rand);

	return mbedTLScpp::X509Cert::FromDER(mbedTLScpp::CtnFullR(der));
}


namespace Internal
{

using AppCertReqTupleCore = std::tuple<
	std::pair<
		Obj::StrKey<SIMOBJ_KSTR("KeyName")>,
		Obj::String
	>,
	std::pair<
		Obj::StrKey<SIMOBJ_KSTR("CSR")>,
		Obj::Bytes
	>,
	std::pair<
		Obj::StrKey<SIMOBJ_KSTR("AuthList")>,
		Obj::Bytes
	>
>;


using AppCertReqParserTp = std::tuple<
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("KeyName")>,
		Internal::AdvRlp::CatStringParser
	>,
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("CSR")>,
		Internal::AdvRlp::CatBytesParser
	>,
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("AuthList")>,
		Internal::AdvRlp::CatBytesParser
	>
>;

} // namespace Internal


class AppCertRequest :
	public Internal::Obj::StaticDict<Internal::AppCertReqTupleCore>
{
public: // static members:

	using Self = AppCertRequest;
	using Base = Internal::Obj::StaticDict<Internal::AppCertReqTupleCore>;

	template<typename _StrSeq>
	using _StrKey = Internal::Obj::StrKey<_StrSeq>;
	template<typename _StrSeq>
	using _RetRefType = typename Base::template GetRef<_StrKey<_StrSeq> >;
	template<typename _StrSeq>
	using _RetKRefType = typename Base::template GetConstRef<_StrKey<_StrSeq> >;

public:

	using Base::Base;

	_RetRefType<SIMOBJ_KSTR("KeyName")> get_KeyName()
	{
		return Base::template get<_StrKey<SIMOBJ_KSTR("KeyName")> >();
	}

	_RetKRefType<SIMOBJ_KSTR("KeyName")> get_KeyName() const
	{
		return Base::template get<_StrKey<SIMOBJ_KSTR("KeyName")> >();
	}

	std::string GetKeyName() const
	{
		return std::string(
			get_KeyName().data(),
			get_KeyName().data() + get_KeyName().size()
		);
	}

	_RetRefType<SIMOBJ_KSTR("CSR")> get_CSR()
	{
		return Base::template get<_StrKey<SIMOBJ_KSTR("CSR")> >();
	}

	_RetKRefType<SIMOBJ_KSTR("CSR")> get_CSR() const
	{
		return Base::template get<_StrKey<SIMOBJ_KSTR("CSR")> >();
	}

	std::vector<uint8_t> GetCSR() const
	{
		return std::vector<uint8_t>(
			get_CSR().data(),
			get_CSR().data() + get_CSR().size()
		);
	}

	_RetRefType<SIMOBJ_KSTR("AuthList")> get_AuthList()
	{
		return Base::template get<_StrKey<SIMOBJ_KSTR("AuthList")> >();
	}

	_RetKRefType<SIMOBJ_KSTR("AuthList")> get_AuthList() const
	{
		return Base::template get<_StrKey<SIMOBJ_KSTR("AuthList")> >();
	}

	std::vector<uint8_t> GetAuthList() const
	{
		return std::vector<uint8_t>(
			get_AuthList().data(),
			get_AuthList().data() + get_AuthList().size()
		);
	}

}; // class AppCertRequest


using AppCertRequestParser = Internal::AdvRlp::CatStaticDictParserT<
	Internal::AppCertReqParserTp,
	false,
	false,
	AppCertRequest
>;


} // namespace Common
} // namespace DecentEnclave
