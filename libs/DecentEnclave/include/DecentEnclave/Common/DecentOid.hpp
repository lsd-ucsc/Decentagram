// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <string>
#include <vector>

#include <mbedTLScpp/Internal/Asn1Helper.hpp>


namespace DecentEnclave
{
namespace Common
{

struct DecentOid
{

	// Reference for OID to ASN.1 encoding rules:
	// https://learn.microsoft.com/en-us/windows/win32/seccertenroll/about-object-identifier

	static const std::string& GetAsn1OidUuid()
	{
		static const std::string oidUuid = {
			static_cast<char>(2 * 40 + 25) // 2.25
		};
		return oidUuid;
	}


	static std::string BuildDecentEnclaveOid()
	{
		// This is the OID generated from UUID
		// bac83453-fdf5-4ac2-9182-d7bc2ee0981e
		// based on standard https://oidref.com/2.25
		std::vector<uint8_t> uuid = {
			0xbaU, 0xc8U, 0x34U, 0x53U,
			0xfdU, 0xf5U,
			0x4aU, 0xc2U,
			0x91U, 0x82U,
			0xd7U, 0xbcU, 0x2eU, 0xe0U, 0x98U, 0x1eU,
		};
		std::string res;

		mbedTLScpp::Internal::Asn1MultiBytesOidEncode<char>(
			std::back_inserter(res),
			uuid.cbegin(),
			uuid.cend(),
			uuid.size()
		);

		return GetAsn1OidUuid() + res;
	}


	static const std::string& GetDecentEnclaveOid()
	{
		static const std::string oid = BuildDecentEnclaveOid();
		return oid;
	}


	//==========
	// Root.*
	//==========


	static const std::string& GetVersionOid()
	{
		static const std::string oid = GetDecentEnclaveOid() + '\x01';

		return oid;
	}


	static const std::string& GetEnclaveTypeOid()
	{
		static const std::string oid = GetDecentEnclaveOid() + '\x02';

		return oid;
	}


	static const std::string& GetEnclaveTypeSpecRootOid()
	{
		static const std::string oid = GetDecentEnclaveOid() + '\x03';

		return oid;
	}


	static const std::string& GetKeyringHashOid()
	{
		static const std::string oid = GetDecentEnclaveOid() + '\x04';

		return oid;
	}


	static const std::string& GetAppHashOid()
	{
		static const std::string oid = GetDecentEnclaveOid() + '\x05';

		return oid;
	}


	static const std::string& GetAuthListOid()
	{
		static const std::string oid = GetDecentEnclaveOid() + '\x06';

		return oid;
	}


	//==========
	// Root.3.* - For platform specific data
	//==========


	static const std::string& GetSgxDataRootOid()
	{
		static const std::string oid = GetEnclaveTypeSpecRootOid() + '\x01';

		return oid;
	}


	//==========
	// Root.3.1.* - For SGX EPID platform data
	//==========


	static const std::string& GetSgxStdReportDataOid()
	{
		static const std::string oid = GetSgxDataRootOid() + '\x01';

		return oid;
	}


	static const std::string& GetSgxSelfRaReportOid()
	{
		static const std::string oid = GetSgxDataRootOid() + '\x02';

		return oid;
	}


}; // struct DecentOid

} // namespace Common
} // namespace DecentEnclave
