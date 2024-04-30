// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#if defined(DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED) || \
	defined(DECENT_ENCLAVE_PLATFORM_SGX_UNTRUSTED)


#include <cstdint>

#include <memory>
#include <type_traits>

#include <AdvancedRlp/AdvancedRlp.hpp>
#include <mbedTLScpp/EcKey.hpp>
#include <mbedTLScpp/Hkdf.hpp>
#include <mbedTLScpp/SecretArray.hpp>
#include <sgx_ukey_exchange.h>
#include <SimpleJson/SimpleJson.hpp>
#include <SimpleObjects/Codec/Base64.hpp>
#include <SimpleObjects/Codec/Hex.hpp>
#include <SimpleObjects/SimpleObjects.hpp>

#include "../Internal/SimpleObj.hpp"
#include "../Internal/SimpleRlp.hpp"
#include "../Platform/Print.hpp"
#include "AttestationConfig.hpp"
#include "Crypto.hpp"
#include "EpidRaMessages.hpp"
#include "IasReportVerifier.hpp"
#include "IasRequester.hpp"


namespace DecentEnclave
{
namespace Common
{
namespace Sgx
{


class EpidRaSvcProvCore
{
public: // static members:

	enum class HSState : uint8_t
	{
		Initial,
		Msg0rSent,
		Msg2Sent,
		HandshakeDone,
		HandshakeRefused,
	};

	using SKey128Bit = mbedTLScpp::SecretArray<uint8_t, 16>;
	using SKey256Bit = mbedTLScpp::SecretArray<uint8_t, 32>;
	using EcKeyPairType = mbedTLScpp::EcKeyPair<mbedTLScpp::EcType::SECP256R1>;
	using EcPubKeyType = mbedTLScpp::EcPublicKey<mbedTLScpp::EcType::SECP256R1>;

	static constexpr size_t sk_iasNonceLen = 32;


	static std::string BuildNonce(
		mbedTLScpp::RbgInterface& randGen,
		size_t len = sk_iasNonceLen
	)
	{
		// NOTE: since the length of the nonce field is defined as the number
		//       of characters in the nonce field of the *JSON* message,
		//       thus, we can't put binary data into the nonce field.

		std::vector<uint8_t> randBytes(len / 2);
		randGen.Rand(randBytes.data(), randBytes.size());

		std::string res = Internal::Obj::Codec::HEX::
			template Encode<std::string>(randBytes);

		return res;
	}


public:

	EpidRaSvcProvCore(
		std::shared_ptr<EcKeyPairType> mySignKey,
		sgx_spid_t spid,
		std::unique_ptr<IasRequester> iasReq,
		std::unique_ptr<IasEpidReportVerifier> iasReportVrfy,
		std::unique_ptr<EpidQuoteVerifier> epidQuoteVrfy,
		mbedTLScpp::RbgInterface& randGen
	) :
		m_mySignKey(std::move(mySignKey)),
		m_myEncKey(EcKeyPairType::Generate(randGen)),
		m_peerEncKey(),
		m_smk(),
		m_mk(),
		m_sk(),
		m_vk(),
		m_spid(spid),
		m_nonce(BuildNonce(randGen)),
		m_iasReq(std::move(iasReq)),
		m_iasReportVrfy(std::move(iasReportVrfy)),
		m_epidQuoteVrfy(std::move(epidQuoteVrfy)),
		m_iasReportSet(),
		m_handshakeState(HSState::Initial)
	{
		if (m_mySignKey == nullptr)
		{
			throw InvalidArgumentException("The given key pair is null");
		}
	}


	virtual ~EpidRaSvcProvCore() = default;


	EpidRaSvcProvCore(const EpidRaSvcProvCore& rhs) = delete;


	EpidRaSvcProvCore(EpidRaSvcProvCore&& rhs) :
		m_mySignKey(std::move(rhs.m_mySignKey)),
		m_myEncKey(std::move(rhs.m_myEncKey)),
		m_peerEncKey(std::move(rhs.m_peerEncKey)),
		m_smk(std::move(rhs.m_smk)),
		m_mk(std::move(rhs.m_mk)),
		m_sk(std::move(rhs.m_sk)),
		m_vk(std::move(rhs.m_vk)),
		m_spid(std::move(rhs.m_spid)),
		m_nonce(std::move(rhs.m_nonce)),
		m_iasReq(std::move(rhs.m_iasReq)),
		m_iasReportVrfy(std::move(rhs.m_iasReportVrfy)),
		m_epidQuoteVrfy(std::move(rhs.m_epidQuoteVrfy)),
		m_iasReportSet(std::move(rhs.m_iasReportSet)),
		m_handshakeState(rhs.m_handshakeState)
	{
		rhs.m_handshakeState = HSState::Initial;
	}


	virtual bool IsHandshakeDone() const
	{
		return m_handshakeState == HSState::HandshakeDone;
	}


	virtual bool IsHandshakeRefused() const
	{
		return m_handshakeState == HSState::HandshakeRefused;
	}


	const std::string& GetNonce() const
	{
		return m_nonce;
	}


	const IasReportSet& GetIasReportSet() const
	{
		return m_iasReportSet;
	}


	const EpidQuoteVerifier& GetEpidQuoteVerifier() const
	{
		return *m_epidQuoteVrfy;
	}


//==========
// EPID protocol messages
//==========


	virtual decent_ra_msg0r_t GetMsg0r(const decent_ra_msg0s_t& msg0s)
	{
		if (!ValidateExtGrpId(msg0s.extended_grp_id))
		{
			throw InvalidArgumentException(
				"The given extended group ID is unsupported"
			);
		}

		decent_ra_msg0r_t res;
		ImportEcKey(res.sp_pub_key, *m_mySignKey);

		m_handshakeState = HSState::Msg0rSent;

		return res;
	}


	virtual std::vector<uint8_t> GetMsg2(
		const sgx_ra_msg1_t& msg1,
		mbedTLScpp::RbgInterface& randGen
	)
	{
		using _Hasher = mbedTLScpp::Hasher<mbedTLScpp::HashType::SHA256>;
		using _Cmacer = mbedTLScpp::Cmacer<
			mbedTLScpp::CipherType::AES,
			128,
			mbedTLScpp::CipherMode::ECB
		>;

		int mbedRet = 0;
		std::vector<uint8_t> res;

		sgx_ec256_public_t myEncSgxKey;
		ImportEcKey(myEncSgxKey, m_myEncKey);

		SetPeerEncrPubKey(msg1.g_a, randGen);
		m_epidQuoteVrfy->SetStdReportData(GenStdReportData(myEncSgxKey));

		res.resize(sizeof(sgx_ra_msg2_t));
		sgx_ra_msg2_t& msg2Ref = *reinterpret_cast<sgx_ra_msg2_t*>(res.data());

		msg2Ref.g_b = myEncSgxKey;
		msg2Ref.spid = m_spid;
		msg2Ref.quote_type = AttestationConfig::sk_quoteTypeLinkable;
		msg2Ref.kdf_id = AttestationConfig::sk_kdfIdDefault;

		auto hashToBeSigned = _Hasher().Calc(
			mbedTLScpp::CtnFullR(myEncSgxKey.gx),
			mbedTLScpp::CtnFullR(myEncSgxKey.gy),
			mbedTLScpp::CtnFullR(m_peerEncKey.gx),
			mbedTLScpp::CtnFullR(m_peerEncKey.gy)
		);

		mbedTLScpp::BigNum rBN;
		mbedTLScpp::BigNum sBN;
		std::tie(rBN, sBN) = m_mySignKey->SignInBigNum(hashToBeSigned, randGen);

		mbedRet = mbedtls_mpi_write_binary_le(
			rBN.Get(),
			reinterpret_cast<uint8_t*>(msg2Ref.sign_gb_ga.x),
			sizeof(msg2Ref.sign_gb_ga.x)
		);
		mbedTLScpp::CheckMbedTlsIntRetVal(
			mbedRet,
			"mbedtls_mpi_write_binary_le",
			"DecentEnclave::Common::Sgx::EpidRaSvcProvCore::GetMsg2"
		);
		mbedRet = mbedtls_mpi_write_binary_le(
			sBN.Get(),
			reinterpret_cast<uint8_t*>(msg2Ref.sign_gb_ga.y),
			sizeof(msg2Ref.sign_gb_ga.y)
		);
		mbedTLScpp::CheckMbedTlsIntRetVal(
			mbedRet,
			"mbedtls_mpi_write_binary_le",
			"DecentEnclave::Common::Sgx::EpidRaSvcProvCore::GetMsg2"
		);

		const size_t cmac_size = offsetof(sgx_ra_msg2_t, mac);
		std::vector<uint8_t> tmpCmacData(cmac_size);
		std::memcpy(tmpCmacData.data(), &(msg2Ref.g_b), tmpCmacData.size());

		auto cmacRes = _Cmacer(mbedTLScpp::CtnFullR(m_smk)).Calc(
			mbedTLScpp::CtnFullR(tmpCmacData)
		);
		static_assert(
			std::tuple_size<decltype(cmacRes)>::value == sizeof(msg2Ref.mac),
			"CMAC result size doesn't match"
		);
		std::memcpy(msg2Ref.mac, cmacRes.data(), cmacRes.size());

		std::string sigrlB64 = m_iasReq->GetSigrl(msg1.gid);
		Platform::Print::StrDebug("SigRL: " + sigrlB64);
		std::vector<uint8_t> sigRL =
			Internal::Obj::Codec::Base64::
				template Decode<std::vector<uint8_t> >(sigrlB64);

		msg2Ref.sig_rl_size = static_cast<uint32_t>(sigRL.size());
		res.insert(res.end(), sigRL.begin(), sigRL.end());

		m_handshakeState = HSState::Msg2Sent;
		return res;
	}


	virtual std::vector<uint8_t> GetMsg4(
		const std::vector<uint8_t>& msg3
	)
	{
		if (msg3.size() < sizeof(sgx_ra_msg3_t))
		{
			throw InvalidArgumentException(
				"msg3 is too short (size = " + std::to_string(msg3.size()) + ")"
			);
		}

		const sgx_ra_msg3_t& msg3Ref =
			*reinterpret_cast<const sgx_ra_msg3_t*>(msg3.data());

		auto iasReqBody = BuildIasReportReqBody(msg3Ref, msg3.size(), m_nonce);
		Platform::Print::StrDebug("IAS report request: " + iasReqBody);

		m_iasReportSet = m_iasReq->GetReport(iasReqBody);

		Platform::Print::StrDebug(
			"IAS report: " +
			GetStrFromSimpleBytes(m_iasReportSet.get_Report())
		);

		bool vrfySucc = false;
		try
		{
			m_iasReportVrfy->VerifyAndReduceReportSet(
				m_iasReportSet,
				*m_epidQuoteVrfy,
				&m_nonce
			);
			vrfySucc = true;
		}
		catch(const Exception&)
		{}

		std::vector<uint8_t> msg4 = GenMsg4(vrfySucc);

		if (vrfySucc)
		{
			m_handshakeState = HSState::HandshakeDone;
		}
		else
		{
			m_handshakeState = HSState::HandshakeRefused;
		}

		return msg4;
	}


protected:


	virtual bool ValidateExtGrpId(uint32_t extGrpId) const
	{
		return extGrpId == 0;
	}


	void SetPeerEncrPubKey(
		const sgx_ec256_public_t & inEncPubKey,
		mbedTLScpp::RbgInterface& randGen
	)
	{
		m_peerEncKey = inEncPubKey;

		EcPubKeyType peerEncKey(mbedTLScpp::EcType::SECP256R1);
		ExportEcKey(peerEncKey, m_peerEncKey);

		auto sharedKeyInt =
			m_myEncKey.DeriveSharedKeyInBigNum(peerEncKey, randGen);

		SKey256Bit sharedKey;
		int mbedRet = mbedtls_mpi_write_binary_le(
			sharedKeyInt.Get(),
			sharedKey.data(),
			sharedKey.size()
		);
		mbedTLScpp::CheckMbedTlsIntRetVal(
			mbedRet,
			"mbedtls_mpi_write_binary_le",
			"DecentEnclave::Common::Sgx::EpidRaSvcProvCore::SetPeerEncrPubKey"
		);


		// Reference: https://community.intel.com/t5/Intel-Software-Guard-Extensions/Key-Derivation-MK-SK-VK-SMK/m-p/1085912
		// Reference: https://www.intel.com/content/www/us/en/developer/articles/code-sample/software-guard-extensions-remote-attestation-end-to-end-example.html
		// SMK (SIGMA protocol)
		// SK (Signing Key/Symmetric Key)
		// MK (Master Key/Masking Key)
		// VK (Verification Key)
		m_smk = Ckdf<mbedTLScpp::CipherType::AES, 128, mbedTLScpp::CipherMode::ECB>(
			CtnFullR(sharedKey), "SMK"
		);
		m_mk  = Ckdf<mbedTLScpp::CipherType::AES, 128, mbedTLScpp::CipherMode::ECB>(
			CtnFullR(sharedKey), "MK"
		);
		m_sk  = Ckdf<mbedTLScpp::CipherType::AES, 128, mbedTLScpp::CipherMode::ECB>(
			CtnFullR(sharedKey), "SK"
		);
		m_vk  = Ckdf<mbedTLScpp::CipherType::AES, 128, mbedTLScpp::CipherMode::ECB>(
			CtnFullR(sharedKey), "VK"
		);
	}


	sgx_report_data_t GenStdReportData(const sgx_ec256_public_t& myEncSgxKey)
	{
		using _Hasher = mbedTLScpp::Hasher<mbedTLScpp::HashType::SHA256>;

		// Verify the report_data in the Quote matches the expected value.
		// The first 32 bytes of report_data are SHA256 HASH of {ga|gb|vk}.
		// The second 32 bytes of report_data are set to zero.
		auto reportDataHash = _Hasher().Calc(
			mbedTLScpp::CtnFullR(m_peerEncKey.gx),
			mbedTLScpp::CtnFullR(m_peerEncKey.gy),
			mbedTLScpp::CtnFullR(myEncSgxKey.gx),
			mbedTLScpp::CtnFullR(myEncSgxKey.gy),
			mbedTLScpp::CtnFullR(m_vk)
		);

		return ReportDataFromHash(reportDataHash);
	}


	static std::string BuildIasReportReqBody(
		const sgx_ra_msg3_t& msg3,
		const size_t msg3Size,
		const std::string& nonce
	)
	{
		using _PsSecDescType =
			typename std::remove_reference<
				decltype(msg3.ps_sec_prop.sgx_ps_sec_prop_desc)
			>::type;
		static_assert(
			std::is_same<uint8_t[256], _PsSecDescType>::value,
			"Unexpected type of sgx_ps_sec_prop_desc_t"
		);

		static const std::array<uint8_t, sizeof(sgx_ps_sec_prop_desc_t)>
			sk_zeroSecPropDesc = { 0 };
		static const Internal::Obj::String sk_labelQuote = "isvEnclaveQuote";
		static const Internal::Obj::String sk_labelNonce = "nonce";
		static const Internal::Obj::String sk_labelPseMa = "pseManifest";


		const uint8_t* quotePtr = reinterpret_cast<const uint8_t*>(&msg3.quote);
		std::vector<uint8_t> quote(
			quotePtr,
			quotePtr + (msg3Size - sizeof(sgx_ra_msg3_t))
		);

		// check msg3.ps_sec_prop.sgx_ps_sec_prop_desc to see if was PSE enabled
		std::string pseManifestStr;
		if (
			!std::equal(
				sk_zeroSecPropDesc.cbegin(),
				sk_zeroSecPropDesc.cend(),
				std::begin(msg3.ps_sec_prop.sgx_ps_sec_prop_desc)
			)
		)
		{
			// PSE manifest presents
			pseManifestStr = Internal::Obj::Codec::Base64::
				template Encode<std::string>(
					msg3.ps_sec_prop.sgx_ps_sec_prop_desc
				);
		}
		else
		{
			Platform::Print::StrDebug("PSE is not enabled during RA");
		}

		auto jsonObj = Internal::Obj::Dict();
		jsonObj[sk_labelQuote] = Internal::Obj::Codec::Base64::
			template Encode<Internal::Obj::String>(quote);
		jsonObj[sk_labelNonce] = Internal::Obj::String(nonce);
		if (!pseManifestStr.empty())
		{
			jsonObj[sk_labelPseMa] = Internal::Obj::String(pseManifestStr);
		}

		std::string json = SimpleJson::DumpStr(jsonObj);

		return json;
	}


	std::vector<uint8_t> GenMsg4(
		bool vrfyRes
	)
	{
		using _Cmacer = mbedTLScpp::Cmacer<
			mbedTLScpp::CipherType::AES,
			128,
			mbedTLScpp::CipherMode::ECB
		>;

		static const Internal::Obj::String sk_labelVRes = "VerifyResult";
		static const Internal::Obj::String sk_labelRepSet = "ReportSet";
		static const Internal::Obj::String sk_labelMsgBody = "MsgBody";
		static const Internal::Obj::String sk_labelMac = "MAC";

		auto msg4Body = Internal::Obj::Dict();
		msg4Body[sk_labelVRes] = Internal::Obj::Bool(vrfyRes);
		msg4Body[sk_labelRepSet] = Internal::Obj::Bytes(
			Internal::Rlp::WriterGeneric::Write(m_iasReportSet)
		);
		auto msg4BodyBytes = AdvancedRlp::GenericWriter::Write(msg4Body);

		auto cmacRes = _Cmacer(mbedTLScpp::CtnFullR(m_sk)).Calc(
			mbedTLScpp::CtnFullR(msg4BodyBytes)
		);

		auto msg4 = Internal::Obj::Dict();
		msg4[sk_labelMsgBody] = Internal::Obj::Bytes(msg4BodyBytes);
		msg4[sk_labelMac] = Internal::Obj::Bytes(
			cmacRes.begin(),
			cmacRes.end()
		);

		return AdvancedRlp::GenericWriter::Write(msg4);
	}


private:


	std::shared_ptr<EcKeyPairType> m_mySignKey;
	EcKeyPairType m_myEncKey;
	sgx_ec256_public_t m_peerEncKey;
	SKey128Bit m_smk;
	SKey128Bit m_mk;
	SKey128Bit m_sk;
	SKey128Bit m_vk;
	sgx_spid_t m_spid;
	std::string m_nonce;
	std::unique_ptr<IasRequester> m_iasReq;
	std::unique_ptr<IasEpidReportVerifier> m_iasReportVrfy;
	std::unique_ptr<EpidQuoteVerifier> m_epidQuoteVrfy;
	IasReportSet m_iasReportSet;
	HSState m_handshakeState;


}; // class EpidRaSvcProvCore


} // namespace Sgx
} // namespace Common
} // namespace DecentEnclave

#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED || _UNTRUSTED
