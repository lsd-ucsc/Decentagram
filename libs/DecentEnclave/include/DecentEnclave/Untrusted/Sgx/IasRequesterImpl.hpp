// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


// TODO: #ifdef DECENT_ENCLAVE_PLATFORM_SGX_UNTRUSTED


#include <iterator>
#include <string>
#include <vector>

#include <sgx_report.h>

#include <SimpleObjects/Codec/Base64.hpp>
#include <SimpleObjects/Codec/Hex.hpp>

#include "../CUrl.hpp"
#include "../../Common/Exceptions.hpp"
#include "../../Common/Internal/SimpleObj.hpp"
#include "../../Common/Sgx/IasRequester.hpp"


namespace DecentEnclave
{
namespace Untrusted
{
namespace Sgx
{


class IasRequesterImpl :
	public Common::Sgx::IasRequester
{
public: // static members:


	using Base = Common::Sgx::IasRequester;

	using Base::GetIasUrlDev;
	using Base::GetIasUrlProd;
	using Base::GetIasSigrlUri;
	using Base::GetIasReportUri;

	using Base::GetHdrLabelSubKey;
	using Base::GetHdrLabelReqId;
	using Base::GetHdrLabelSign;
	using Base::GetHdrLabelCert;

	using Base::ParseSpid;


public:

	IasRequesterImpl(
		const std::string& iasUrl,
		const std::string& subscriptionKey
	) :
		Base(),
		m_iasUrl(iasUrl),
		m_subscriptionKey(subscriptionKey)
	{}

	virtual ~IasRequesterImpl() = default;


	virtual std::string GetSigrl(const sgx_epid_group_id_t& gid) const override
	{
		std::string reqFullUrl = m_iasUrl + GetIasSigrlUri();

		std::string gidStr = EncodeGroupId(gid);
		reqFullUrl += gidStr;

		std::string requestId;
		CUrlHeaderCallBack headerCallback =
			[&requestId]
			(char* ptr, size_t size, size_t nitems, void*) -> size_t
			{
				static std::string tmp;
				tmp = std::string(ptr, size * nitems);
				if (tmp.find(GetHdrLabelReqId()) == 0)
				{
					requestId = CUrlParseHeaderValue(tmp);
				}

				// If returned amount differs from the amount passed in,
				// it will signal an error to the library and cause the transfer
				// to get aborted
				// - https://curl.se/libcurl/c/CURLOPT_HEADERFUNCTION.html
				return size * nitems;
			};

		std::string outRevcList;
		CUrlContentCallBack contentCallback =
			[&outRevcList]
			(char* ptr, size_t size, size_t nmemb, void*) -> size_t
			{
				outRevcList += std::string(ptr, size * nmemb);

				// If returned amount differs from the amount passed in,
				// it will signal an error to the library and cause the transfer
				// to get aborted
				// - https://curl.se/libcurl/c/CURLOPT_HEADERFUNCTION.html
				return size * nmemb;
			};

		std::string hdrSubKey = GetHdrLabelSubKey() + ": " + m_subscriptionKey;

		Untrusted::CUrlRequestExpectRespCode(
			reqFullUrl,
			"GET",
			{
				"Cache-Control: no-cache",
				hdrSubKey,
			},
			std::string(),
			&headerCallback,
			&contentCallback,
			200
		);

		return outRevcList;
	}


	virtual Common::Sgx::IasReportSet GetReport(
		const std::string& reqBody
	) const override
	{
		std::string reqFullUrl = m_iasUrl + GetIasReportUri();


		std::string requestId;
		std::string iasSign;
		std::string iasCert;
		CUrlHeaderCallBack headerCallback =
			[&requestId, &iasSign, &iasCert]
			(char* ptr, size_t size, size_t nitems, void*) -> size_t
			{
				static std::string tmp;
				tmp = std::string(ptr, size * nitems);
				if (tmp.find(GetHdrLabelReqId()) == 0)
				{
					requestId = CUrlParseHeaderValue(tmp);
				}
				else if (tmp.find(GetHdrLabelSign()) == 0)
				{
					iasSign = CUrlParseHeaderValue(tmp);
				}
				else if (tmp.find(GetHdrLabelCert()) == 0)
				{
					iasCert = CUrlParseHeaderValue(tmp);
					CUrlUnescape(iasCert);
				}

				// If returned amount differs from the amount passed in,
				// it will signal an error to the library and cause the transfer
				// to get aborted
				// - https://curl.se/libcurl/c/CURLOPT_HEADERFUNCTION.html
				return size * nitems;
			};

		std::string respBody;
		CUrlContentCallBack contentCallback =
			[&respBody]
			(char* ptr, size_t size, size_t nmemb, void*) -> size_t
			{
				respBody += std::string(ptr, size * nmemb);

				// If returned amount differs from the amount passed in,
				// it will signal an error to the library and cause the transfer
				// to get aborted
				// - https://curl.se/libcurl/c/CURLOPT_HEADERFUNCTION.html
				return size * nmemb;
			};

		std::string hdrSubKey = GetHdrLabelSubKey() + ": " + m_subscriptionKey;

		Untrusted::CUrlRequestExpectRespCode(
			reqFullUrl,
			"POST",
			{
				"Cache-Control: no-cache",
				"Content-Type: application/json",
				hdrSubKey,
			},
			reqBody,
			&headerCallback,
			&contentCallback,
			200
		);

		Common::Sgx::IasReportSet reportSet;

		reportSet.get_Report() = Common::Sgx::GetSimpleBytesFromStr(respBody);

		reportSet.get_ReportSign() = Common::Internal::Obj::Codec::Base64::
			template Decode<Common::Internal::Obj::Bytes>(iasSign);

		Common::Sgx::CertPEM2DERList(reportSet.get_IasCert(), iasCert);

		return reportSet;
	}

private:

	static std::string EncodeGroupId(const sgx_epid_group_id_t& gid)
	{
		std::vector<uint8_t> gidLitEnd(std::begin(gid), std::end(gid));
		std::vector<uint8_t> gidBigEnd(gidLitEnd.rbegin(), gidLitEnd.rend());
		return Common::Internal::Obj::Codec::Hex::Encode<std::string>(gidBigEnd);
	}

	std::string m_iasUrl;
	std::string m_subscriptionKey;

}; // class IasRequesterImpl


} // namespace Sgx
} // namespace Untrusted
} // namespace DecentEnclave

// TODO: #endif // DECENT_ENCLAVE_PLATFORM_SGX_UNTRUSTED
