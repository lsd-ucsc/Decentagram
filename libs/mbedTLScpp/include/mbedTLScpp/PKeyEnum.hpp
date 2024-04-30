// Copyright (c) 2022 mbedTLScpp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <mbedtls/pk.h>

#include "Exceptions.hpp"


#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{


/**
 * @brief	Values that represent public key (asymmetric key)
 *          algorithm categories. For now, it could be EC, or RSA.
 */
enum class PKeyAlgmCat
{
	EC,
	RSA,
}; // enum class PKeyAlgmCat


/**
 * @brief	Values that represent public key (asymmetric key) types.
 *          It's either public or private
 */
enum class PKeyType
{
	Public,
	Private,
}; // enum class PKeyType


inline PKeyAlgmCat GetAlgmCat(const mbedtls_pk_context & ctx)
{
	mbedtls_pk_type_t type = mbedtls_pk_get_type(&ctx);

	switch (type)
	{
	case mbedtls_pk_type_t::MBEDTLS_PK_ECKEY:
	case mbedtls_pk_type_t::MBEDTLS_PK_ECKEY_DH:
	case mbedtls_pk_type_t::MBEDTLS_PK_ECDSA:
		return PKeyAlgmCat::EC;
	case mbedtls_pk_type_t::MBEDTLS_PK_RSA:
	case mbedtls_pk_type_t::MBEDTLS_PK_RSA_ALT:
	case mbedtls_pk_type_t::MBEDTLS_PK_RSASSA_PSS:
		return PKeyAlgmCat::RSA;
	case mbedtls_pk_type_t::MBEDTLS_PK_NONE:
		throw InvalidArgumentException(
			"PKeyBase::GetAlgmCat - The given PKey has no type; it's empty."
		);
	default:
		throw InvalidArgumentException(
			"PKeyBase::GetAlgmCat - The given PKey type isn't supported."
		);
	}
}

} // namespace mbedTLScpp
