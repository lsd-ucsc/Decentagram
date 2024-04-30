// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#if defined(DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED) || \
	defined(DECENT_ENCLAVE_PLATFORM_SGX_UNTRUSTED)


#include <mbedTLScpp/Hash.hpp>
#include <mbedTLScpp/X509Cert.hpp>
#include <sgx_quote.h>
#include <SimpleJson/SimpleJson.hpp>
#include <SimpleObjects/Codec/Base64.hpp>
#include <SimpleObjects/Codec/Hex.hpp>

#include "../Internal/SimpleObj.hpp"
#include "../Platform/Print.hpp"
#include "Crypto.hpp"
#include "EpidRaMessages.hpp"


namespace DecentEnclave
{
namespace Common
{
namespace Sgx
{


class EpidQuoteVerifier
{
public: // static members:

	static sgx_quote_t ParseQuoteFromB64(const char* b64Str, size_t b64StrLen)
	{
		auto decoded = Internal::Obj::Codec::Base64::
			template Decode<std::vector<uint8_t> >(b64Str, b64Str + b64StrLen);

		// Base 64-encoded BODY of QUOTE structure (i.e., QUOTE
		// structure without signature related fields: SIG_LEN and SIG)
		if (decoded.size() != (sizeof(sgx_quote_t) - sizeof(uint32_t)))
		{
			throw Exception("Failed to decode the quote body");
		}

		sgx_quote_t res;
		res.signature_len = 0;

		uint8_t* resPtr = reinterpret_cast<uint8_t*>(&res);
		std::memcpy(resPtr, decoded.data(), decoded.size());

		return res;
	}


public:


	virtual void Verify(const sgx_quote_t& quote)
	{
		VerifyReportBody(quote.report_body);
	}


	virtual void VerifyReportBody(const sgx_report_body_t& body)
	{
		VerifyMrEnclave(body.mr_enclave);

		VerifyReportData(body.report_data);
	}


	virtual void SetStdReportData(const sgx_report_data_t& data)
	{
		m_stdReportData = data;
	}


	virtual void SetAddReportData(std::vector<uint8_t> data)
	{
		m_addReportData = std::move(data);
	}


	virtual void SetAuthorizedEnclave(
		std::vector<std::vector<uint8_t> > idList
	)
	{
		m_authEnclaves = std::move(idList);
	}


	virtual const sgx_report_data_t& GetStdReportData() const
	{
		return m_stdReportData;
	}


protected:


	virtual void VerifyMrEnclave(const sgx_measurement_t& mrEnclave)
	{
		std::vector<uint8_t> mrEnclaveBytes(
			mrEnclave.m,
			mrEnclave.m + sizeof(mrEnclave.m)
		);
		for (const auto& authId : m_authEnclaves)
		{
			if (authId == mrEnclaveBytes)
			{
				Platform::Print::StrDebug(
					"Verified enclave ID: " +
					Internal::Obj::Codec::HEX::
						template Encode<std::string>(mrEnclaveBytes)
				);
				return;
			}
		}

		throw Exception("The enclave being verified is not authorized.");
	}


	virtual void VerifyReportData(const sgx_report_data_t& reportData)
	{
		sgx_report_data_t expData = { { 0 } };

		if (m_addReportData.size() == 0)
		{
			expData = m_stdReportData;
		}
		else
		{
			auto hash = mbedTLScpp::Hasher<mbedTLScpp::HashType::SHA256>().Calc(
				mbedTLScpp::CtnFullR(m_stdReportData.d),
				mbedTLScpp::CtnFullR(m_addReportData)
			);

			expData = ReportDataFromHash(hash);
		}

		if (
			std::equal(
				std::begin(reportData.d),
				std::end(reportData.d),
				std::begin(expData.d)
			)
		)
		{
			Platform::Print::StrDebug("Verified report data");
			return;
		}

		throw Exception("The report data does not match the expected one");
	}


private:

	std::vector<std::vector<uint8_t> > m_authEnclaves;
	std::vector<uint8_t> m_addReportData;
	sgx_report_data_t m_stdReportData;

}; // class EpidQuoteVerifier


class IasEpidReportVerifier
{
public:

	IasEpidReportVerifier() = default;


	virtual ~IasEpidReportVerifier() = default;


	virtual void VerifyAndReduceReportSet(
		IasReportSet& reportSet,
		EpidQuoteVerifier& quoteVerifier,
		const std::string* nonce = nullptr
	)
	{
		ShrinkCertChain(reportSet);

		mbedTLScpp::X509Cert certChain =
			X509CertFromDERList(reportSet.get_IasCert());

		VerifyCert(reportSet, certChain);

		VerifySign(reportSet, certChain);

		std::string reportStr = GetStrFromSimpleBytes(reportSet.get_Report());
		auto report = SimpleJson::GenericObjectParser().Parse(reportStr);
		const auto& reportDict = report.AsDict();

		VerifyReport(reportSet, reportDict, nonce);

		VerifyEncQuote(reportSet, reportDict, quoteVerifier);
	}


	virtual const mbedTLScpp::X509Cert& GetRootCaCert() const
	{
		static constexpr const char sk_rootCaPem[] =
"-----BEGIN CERTIFICATE-----\n\
MIIFSzCCA7OgAwIBAgIJANEHdl0yo7CUMA0GCSqGSIb3DQEBCwUAMH4xCzAJBgNV\n\
BAYTAlVTMQswCQYDVQQIDAJDQTEUMBIGA1UEBwwLU2FudGEgQ2xhcmExGjAYBgNV\n\
BAoMEUludGVsIENvcnBvcmF0aW9uMTAwLgYDVQQDDCdJbnRlbCBTR1ggQXR0ZXN0\n\
YXRpb24gUmVwb3J0IFNpZ25pbmcgQ0EwIBcNMTYxMTE0MTUzNzMxWhgPMjA0OTEy\n\
MzEyMzU5NTlaMH4xCzAJBgNVBAYTAlVTMQswCQYDVQQIDAJDQTEUMBIGA1UEBwwL\n\
U2FudGEgQ2xhcmExGjAYBgNVBAoMEUludGVsIENvcnBvcmF0aW9uMTAwLgYDVQQD\n\
DCdJbnRlbCBTR1ggQXR0ZXN0YXRpb24gUmVwb3J0IFNpZ25pbmcgQ0EwggGiMA0G\n\
CSqGSIb3DQEBAQUAA4IBjwAwggGKAoIBgQCfPGR+tXc8u1EtJzLA10Feu1Wg+p7e\n\
LmSRmeaCHbkQ1TF3Nwl3RmpqXkeGzNLd69QUnWovYyVSndEMyYc3sHecGgfinEeh\n\
rgBJSEdsSJ9FpaFdesjsxqzGRa20PYdnnfWcCTvFoulpbFR4VBuXnnVLVzkUvlXT\n\
L/TAnd8nIZk0zZkFJ7P5LtePvykkar7LcSQO85wtcQe0R1Raf/sQ6wYKaKmFgCGe\n\
NpEJUmg4ktal4qgIAxk+QHUxQE42sxViN5mqglB0QJdUot/o9a/V/mMeH8KvOAiQ\n\
byinkNndn+Bgk5sSV5DFgF0DffVqmVMblt5p3jPtImzBIH0QQrXJq39AT8cRwP5H\n\
afuVeLHcDsRp6hol4P+ZFIhu8mmbI1u0hH3W/0C2BuYXB5PC+5izFFh/nP0lc2Lf\n\
6rELO9LZdnOhpL1ExFOq9H/B8tPQ84T3Sgb4nAifDabNt/zu6MmCGo5U8lwEFtGM\n\
RoOaX4AS+909x00lYnmtwsDVWv9vBiJCXRsCAwEAAaOByTCBxjBgBgNVHR8EWTBX\n\
MFWgU6BRhk9odHRwOi8vdHJ1c3RlZHNlcnZpY2VzLmludGVsLmNvbS9jb250ZW50\n\
L0NSTC9TR1gvQXR0ZXN0YXRpb25SZXBvcnRTaWduaW5nQ0EuY3JsMB0GA1UdDgQW\n\
BBR4Q3t2pn680K9+QjfrNXw7hwFRPDAfBgNVHSMEGDAWgBR4Q3t2pn680K9+Qjfr\n\
NXw7hwFRPDAOBgNVHQ8BAf8EBAMCAQYwEgYDVR0TAQH/BAgwBgEB/wIBADANBgkq\n\
hkiG9w0BAQsFAAOCAYEAeF8tYMXICvQqeXYQITkV2oLJsp6J4JAqJabHWxYJHGir\n\
IEqucRiJSSx+HjIJEUVaj8E0QjEud6Y5lNmXlcjqRXaCPOqK0eGRz6hi+ripMtPZ\n\
sFNaBwLQVV905SDjAzDzNIDnrcnXyB4gcDFCvwDFKKgLRjOB/WAqgscDUoGq5ZVi\n\
zLUzTqiQPmULAQaB9c6Oti6snEFJiCQ67JLyW/E83/frzCmO5Ru6WjU4tmsmy8Ra\n\
Ud4APK0wZTGtfPXU7w+IBdG5Ez0kE1qzxGQaL4gINJ1zMyleDnbuS8UicjJijvqA\n\
152Sq049ESDz+1rRGc2NVEqh1KaGXmtXvqxXcTB+Ljy5Bw2ke0v8iGngFBPqCTVB\n\
3op5KBG3RjbF6RRSzwzuWfL7QErNC8WEy5yDVARzTA5+xmBc388v9Dm21HGfcC8O\n\
DD+gT9sSpssq0ascmvH49MOgjt1yoysLtdCtJW/9FZpoOypaHx0R+mJTLwPXVMrv\n\
DaVzWh5aiEx+idkSGMnX\n\
-----END CERTIFICATE-----\n\
";

		static mbedTLScpp::X509Cert s_caCert =
			mbedTLScpp::X509Cert::FromPEM(sk_rootCaPem);

		return s_caCert;
	}


	virtual const mbedtls_x509_crt_profile& GetCertVrfyProfile() const
	{
		static mbedtls_x509_crt_profile s_profile =
		{
			// IAS cert uses SHA256
			MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA256),

			// IAS cert are in RSA 3072 & 2048
			MBEDTLS_X509_ID_FLAG(MBEDTLS_PK_RSA),

			// IAS cert uses no EC curve
			0x00000000,

			// IAS cert are in RSA 3072 & 2048
			2048
		};

		return s_profile;
	}


protected:


	virtual void ShrinkCertChain(IasReportSet& reportSet) const
	{
		mbedTLScpp::X509Cert certChain =
			X509CertFromDERList(reportSet.get_IasCert());

		certChain.ShrinkChain(GetRootCaCert());

		reportSet.get_IasCert().clear();
		X509Cert2DERList(reportSet.get_IasCert(), certChain);
	}


	virtual void VerifyCert(
		const IasReportSet& /* reportSet */,
		const mbedTLScpp::X509Cert& parsedCertChain
	)
	{
		uint32_t vrfyFlags = 0;
		parsedCertChain.VerifyChainWithCa(
			GetRootCaCert(),
			nullptr, /* no CRL to verify with */
			nullptr, /* We don't care about common name */
			vrfyFlags,
			GetCertVrfyProfile(),
			VerifyCertCallback,
			this
		);
		if (vrfyFlags != 0)
		{
			std::string flagStr = Internal::Obj::Codec::HEX::
				template Encode<std::string>(vrfyFlags);

			throw Exception(
				"IAS certificate verification failed (flags=" + flagStr + ")"
			);
		}
	}


	virtual int VerifyCertCallbackImpl(
		mbedtls_x509_crt* /* cert */,
		int               /* depth */,
		uint32_t*         /* flags */
	)
	{
		// We don't need to do anything here
		return 0;
	}


	virtual void VerifySign(
		const IasReportSet& reportSet,
		mbedTLScpp::X509Cert& parsedCertChain
	)
	{
		using _Hasher = mbedTLScpp::Hasher<mbedTLScpp::HashType::SHA256>;

		auto hash = _Hasher().Calc(
			mbedTLScpp::CtnFullR(reportSet.get_Report().GetVal())
		);

		parsedCertChain.GoToFirstCert();
		do
		{
			try
			{
				parsedCertChain.BorrowPublicKey().VerifyDerSign(
					hash,
					mbedTLScpp::CtnFullR(reportSet.get_ReportSign().GetVal())
				);
				// At this point, signature is verified successfully
				return;
			}
			catch(const mbedTLScpp::mbedTLSRuntimeError&)
			{}
			// At this point, signature has failed to verify; try next cert
			parsedCertChain.NextCert();
		} while (parsedCertChain.HasNext());

		// At this point, we have tried all certs and none of them verified
		// the signature
		throw Exception("IAS report signature verification failed");
	}


	virtual void VerifyReport(
		const IasReportSet& /* reportSet */,
		const Internal::Obj::DictBaseObj& parsedReport,
		const std::string* nonce
	)
	{
		static const Internal::Obj::String sk_labelNonce = "nonce";
		static const Internal::Obj::String sk_labelStatus =
			"isvEnclaveQuoteStatus";

		if (nonce != nullptr)
		{
			// Nonce verification requested
			auto it = parsedReport.FindVal(sk_labelNonce);
			if (it == parsedReport.ValsCEnd())
			{
				throw Exception("nonce field is missing from IAS report");
			}

			Internal::Obj::String nonceStr = *nonce;
			if (
				(*it).AsString() !=
				static_cast<const Internal::Obj::StringBaseObj&>(nonceStr)
			)
			{
				throw Exception("IAS report nonce does not match");
			}
		}

		auto it = parsedReport.FindVal(sk_labelStatus);
		if (it == parsedReport.ValsCEnd())
		{
			throw Exception(
				"Enclave quote status field is missing from IAS report"
			);
		}
		VerifyEncQuoteStatus((*it).AsString());
	}


	virtual void VerifyEncQuoteStatus(
		const Internal::Obj::StringBaseObj& statusStr
	)
	{
		using _StrBase = Internal::Obj::StringBaseObj;

		static const Internal::Obj::String sk_statusOK =
			"OK";
		static const Internal::Obj::String sk_statusGrpOutdate =
			"GROUP_OUT_OF_DATE";
		static const Internal::Obj::String sk_statusConfNeed =
			"CONFIGURATION_NEEDED";
		static const Internal::Obj::String sk_statusSwNeed =
			"SW_HARDENING_NEEDED";
		static const Internal::Obj::String sk_statusConfSwNeed =
			"CONFIGURATION_AND_SW_HARDENING_NEEDED";

		if (
			statusStr != static_cast<const _StrBase&>(sk_statusOK) &&
			statusStr != static_cast<const _StrBase&>(sk_statusGrpOutdate) &&
			statusStr != static_cast<const _StrBase&>(sk_statusConfNeed) &&
			statusStr != static_cast<const _StrBase&>(sk_statusSwNeed) &&
			statusStr != static_cast<const _StrBase&>(sk_statusConfSwNeed)
		)
		{
			throw Exception(
				"Enclave quote status is not allowed (status=" +
				statusStr.ToString() + ")"
			);
		}
	}


	virtual void VerifyEncQuote(
		const IasReportSet& /* reportSet */,
		const Internal::Obj::DictBaseObj& parsedReport,
		EpidQuoteVerifier& epidQuoteVerifier
	)
	{
		static const Internal::Obj::String sk_labelQuote =
			"isvEnclaveQuoteBody";

		auto it = parsedReport.FindVal(sk_labelQuote);
		if (it == parsedReport.ValsCEnd())
		{
			throw Exception(
				"Enclave quote body field is missing from IAS report"
			);
		}

		sgx_quote_t quote = EpidQuoteVerifier::ParseQuoteFromB64(
			((*it).AsString()).c_str(),
			((*it).AsString()).size()
		);

		epidQuoteVerifier.Verify(quote);
	}


private:

	static int VerifyCertCallback(
		void*             userData,
		mbedtls_x509_crt* cert,
		int               depth,
		uint32_t*         flags
	)
	{
		IasEpidReportVerifier* inst =
			static_cast<IasEpidReportVerifier*>(userData);
		return inst->VerifyCertCallbackImpl(cert, depth, flags);
	}

}; // class IasEpidReportVerifier


} // namespace Sgx
} // namespace Common
} // namespace DecentEnclave

#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED || _UNTRUSTED
