// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <tuple>

#include <mbedTLScpp/X509Cert.hpp>
#include <sgx_tcrypto.h>
#include <SimpleRlp/SimpleRlp.hpp>

#include "../Internal/SimpleObj.hpp"
#include "../Internal/SimpleRlp.hpp"


#if defined(DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED) || \
	defined(DECENT_ENCLAVE_PLATFORM_SGX_UNTRUSTED)

#pragma pack(push, 1)

typedef struct _decent_ra_msg0s_t
{
	uint32_t  extended_grp_id;
} decent_ra_msg0s_t;

typedef struct _decent_ra_msg0r_t
{
	sgx_ec256_public_t  sp_pub_key;
} decent_ra_msg0r_t;

#pragma pack(pop)





namespace DecentEnclave
{
namespace Common
{
namespace Sgx
{


//==========
// IAS Report Set
//==========


using IasReportSetCore = std::tuple<
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("IasCert")>,
		Internal::Obj::ListT<
			Internal::Obj::Bytes
		>
	>,
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("Report")>,
		Internal::Obj::Bytes
	>,
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("ReportSign")>,
		Internal::Obj::Bytes
	>
>;


class IasReportSet :
	public Internal::Obj::StaticDict<IasReportSetCore>
{
public: // static members:

	using Self = IasReportSet;
	using Base = Internal::Obj::StaticDict<IasReportSetCore>;

	using IasCertType = Internal::Obj::ListT<
		Internal::Obj::Bytes
	>;

public:

	using Base::Base;

	Internal::Obj::Bytes& get_ReportSign()
	{
		return Base::get<Internal::Obj::StrKey<SIMOBJ_KSTR("ReportSign")> >();
	}

	const Internal::Obj::Bytes& get_ReportSign() const
	{
		return Base::get<Internal::Obj::StrKey<SIMOBJ_KSTR("ReportSign")> >();
	}

	IasCertType& get_IasCert()
	{
		return Base::get<Internal::Obj::StrKey<SIMOBJ_KSTR("IasCert")> >();
	}

	const IasCertType& get_IasCert() const
	{
		return Base::get<Internal::Obj::StrKey<SIMOBJ_KSTR("IasCert")> >();
	}

	Internal::Obj::Bytes& get_Report()
	{
		return Base::get<Internal::Obj::StrKey<SIMOBJ_KSTR("Report")> >();
	}

	const Internal::Obj::Bytes& get_Report() const
	{
		return Base::get<Internal::Obj::StrKey<SIMOBJ_KSTR("Report")> >();
	}

}; // class IasReportSet


using IasReportSetParserCore = std::tuple<
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("IasCert")>,
		Internal::Rlp::ListParserT<
			Internal::Rlp::BytesParser,
			Internal::Rlp::FailingParserBytes,
			Internal::Obj::ListT<
				Internal::Obj::Bytes
			>
		>
	>,
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("Report")>,
		Internal::Rlp::BytesParser
	>,
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("ReportSign")>,
		Internal::Rlp::BytesParser
	>
>;


using IasReportSetParser = Internal::Rlp::StaticDictParserT<
	IasReportSetParserCore,
	false, /* No missing items allowed */
	false, /* No extra items allowed */
	IasReportSet
>;


inline std::string GetStrFromSimpleBytes(const Internal::Obj::Bytes& b)
{
	return std::string(
		reinterpret_cast<const char*>(b.data()),
		reinterpret_cast<const char*>(b.data() + b.size())
	);
}


inline Internal::Obj::Bytes GetSimpleBytesFromStr(const std::string& s)
{
	return Internal::Obj::Bytes(
		reinterpret_cast<const uint8_t*>(s.data()),
		reinterpret_cast<const uint8_t*>(s.data() + s.size())
	);
}


inline void X509Cert2DERList(
	Internal::Obj::ListT<Internal::Obj::Bytes>& derList,
	mbedTLScpp::X509Cert& cert
)
{
	bool hasNext = cert.HasNext();
	do
	{
		derList.push_back(
			Internal::Obj::Bytes(
				cert.GetDer()
			)
		);

		hasNext = cert.HasNext();

		if (hasNext)
		{
			cert.NextCert();
		}
	} while (hasNext);
}


inline void CertPEM2DERList(
	Internal::Obj::ListT<Internal::Obj::Bytes>& derList,
	const std::string& pemStr
)
{
	mbedTLScpp::X509Cert iasCertObj =
		mbedTLScpp::X509Cert::FromPEM(pemStr);
	X509Cert2DERList(derList, iasCertObj);
}


inline mbedTLScpp::X509Cert X509CertFromDERList(
	Internal::Obj::ListT<Internal::Obj::Bytes>& derList
)
{
	auto cert = mbedTLScpp::X509Cert::Empty();
	for (const auto& der : derList)
	{
		cert.AppendDER(mbedTLScpp::CtnFullR(der.GetVal()));
	}
	return cert;
}


} // namespace Sgx
} // namespace Common
} // namespace DecentEnclave


#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED || _UNTRUSTED
