// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#if defined(DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED) || \
	defined(DECENT_ENCLAVE_PLATFORM_SGX_UNTRUSTED)


#include <string>

#include <sgx_quote.h>

#include <SimpleObjects/Codec/Hex.hpp>

#include "../Exceptions.hpp"
#include "../Internal/SimpleObj.hpp"
#include "EpidRaMessages.hpp"


namespace DecentEnclave
{
namespace Common
{
namespace Sgx
{


class IasRequester
{
public: // static members:

	static const std::string& GetIasUrlDev()
	{
		static const std::string url =
			"https://api.trustedservices.intel.com/sgx/dev";
		return url;
	}

	static const std::string& GetIasUrlProd()
	{
		static const std::string url =
			"https://api.trustedservices.intel.com/sgx";
		return url;
	}

	static const std::string& GetIasSigrlUri()
	{
		static const std::string uri = "/attestation/v5/sigrl/";
		return uri;
	}

	static const std::string& GetIasReportUri()
	{
		static const std::string uri = "/attestation/v5/report";
		return uri;
	}

	static const std::string& GetHdrLabelSubKey()
	{
		static const std::string label = "Ocp-Apim-Subscription-Key";
		return label;
	}

	static const std::string& GetHdrLabelReqId()
	{
		static const std::string label = "Request-ID";
		return label;
	}

	static const std::string& GetHdrLabelSign()
	{
		static const std::string label = "X-IASReport-Signature";
		return label;
	}

	static const std::string& GetHdrLabelCert()
	{
		static const std::string label = "X-IASReport-Signing-Certificate";
		return label;
	}

	static sgx_spid_t ParseSpid(const std::string& spidStr)
	{
		if (sizeof(sgx_spid_t) != (spidStr.size() / 2))
		{
			throw InvalidArgumentException("Invalid SPID string");
		}

		sgx_spid_t res = sgx_spid_t();

		std::vector<uint8_t> parsed = Internal::Obj::Codec::HEX::
			template Decode<std::vector<uint8_t> >(spidStr);
		static_assert(
			std::is_same<decltype(res.id[0]), uint8_t&>::value,
			"SPID value type mismatch"
		);
		std::copy(parsed.begin(), parsed.end(), res.id);
		return res;
	}

public:

	IasRequester() = default;

	virtual ~IasRequester() = default;

	virtual std::string GetSigrl(const sgx_epid_group_id_t& gid) const = 0;

	virtual IasReportSet GetReport(const std::string& reqBody) const = 0;

}; // class IasRequester


} // namespace Sgx
} // namespace Common
} // namespace DecentEnclave

#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED || _UNTRUSTED

