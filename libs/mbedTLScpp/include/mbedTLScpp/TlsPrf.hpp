// Copyright (c) 2022 mbedTLScpp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <mbedtls/ssl.h>

#include "SKey.hpp"
#include "Container.hpp"

#include "Exceptions.hpp"


#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{

/**
 * @brief Enum for the type of TLS-PRF
 *
 */
enum class TlsPrfType
{
	TlsPrfSha256,
	TlsPrfSha384,
	HkdfSha256,
	HkdfSha384,
};

/**
 * @brief Get the mbed TLS' TLS-PRF type.
 *
 * @param prfType The TLS-PRF type in mbed TLS cpp.
 * @return Enum of mbedtls_tls_prf_types
 */
inline constexpr mbedtls_tls_prf_types GetMbedTlsPrfType(TlsPrfType prfType)
{
	return
			prfType == TlsPrfType::TlsPrfSha256 ?
			mbedtls_tls_prf_types::MBEDTLS_SSL_TLS_PRF_SHA256 :
		(prfType == TlsPrfType::TlsPrfSha384 ?
			mbedtls_tls_prf_types::MBEDTLS_SSL_TLS_PRF_SHA384 :
		(prfType == TlsPrfType::HkdfSha256 ?
			mbedtls_tls_prf_types::MBEDTLS_SSL_HKDF_EXPAND_SHA256 :
		(prfType == TlsPrfType::HkdfSha384 ?
			mbedtls_tls_prf_types::MBEDTLS_SSL_HKDF_EXPAND_SHA384 :
		throw InvalidArgumentException(
			"The given TLS PRF type is not supported."
		))));
}

/**
 * @brief TLS-PRF function for key derivation.
 *
 * @tparam _prfType               Type of the TLS-PRF.
 * @tparam _keyLenInBits          Length of the key to be generated.
 * @tparam _SContainerType        Type of the container for secret key.
 * @tparam _RandContainerType     Type of the container for random bytes.
 * @tparam _RandContainerSecrecy  Secrecy of the type of the container for random bytes.
 * @param skey  Secret key for the key derivation function.
 * @param label String label for the key derivation function.
 * @param rand  Random bytes.
 *
 * @exception InvalidArgumentException Thrown when given TLS PRF type is not supported.
 * @exception mbedTLSRuntimeError      Thrown when mbed TLS C function call failed.
 *
 * @return The derived key
 */
template<TlsPrfType _prfType,
			size_t     _keyLenInBits,
			typename   _SContainerType,
			typename   _RandContainerType,  bool _RandContainerSecrecy>
inline SKey<_keyLenInBits> TlsPrf(
	const ContCtnReadOnlyRef<_SContainerType,     true>& skey,
	const std::string& label,
	const ContCtnReadOnlyRef<_RandContainerType,  _RandContainerSecrecy >& rand)
{
	constexpr mbedtls_tls_prf_types prfCType = GetMbedTlsPrfType(_prfType);
	SKey<_keyLenInBits> res;

	MBEDTLSCPP_MAKE_C_FUNC_CALL(
		mbedTLScpp::TlsPrf,
		mbedtls_ssl_tls_prf,
		prfCType,
		skey.BeginBytePtr(), skey.GetRegionSize(),
		label.c_str(),
		rand.BeginBytePtr(), rand.GetRegionSize(),
		res.data(), res.size()
	);

	return res;
}

} // namespace mbedTLScpp
