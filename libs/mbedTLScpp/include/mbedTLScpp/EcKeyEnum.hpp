// Copyright (c) 2022 mbedTLScpp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstddef>

#include <mbedtls/ecp.h>

#include "Exceptions.hpp"


#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{

/**
 * @brief Elliptic Curve types
 *
 */
enum class EcType
{
	SECP192R1,
	SECP224R1,
	SECP256R1,
	SECP384R1,
	SECP521R1,

	BrPo256R1,
	BrPo384R1,
	BrPo512R1,

	SECP192K1,
	SECP224K1,
	SECP256K1,

	CURVE25519,
	CURVE448,
};


/**
 * @brief	Gets Elliptic Curve size in Byte
 *
 * @exception	RuntimeException	Thrown when Invalid Elliptic Curve type is given.
 *
 * @param	type	The curve type.
 *
 * @return	The size in Byte.
 */
inline constexpr size_t GetCurveByteSize(EcType type)
{
	return
		(type == EcType::SECP192R1 ? 24UL :
		(type == EcType::SECP192K1 ? 24UL :

		(type == EcType::SECP224R1 ? 28UL :
		(type == EcType::SECP224K1 ? 28UL :

		(type == EcType::SECP256R1 ? 32UL :
		(type == EcType::SECP256K1 ? 32UL :
		(type == EcType::BrPo256R1 ? 32UL :

		(type == EcType::SECP384R1 ? 48UL :
		(type == EcType::BrPo384R1 ? 48UL :

		(type == EcType::BrPo512R1 ? 64UL :

		(type == EcType::SECP521R1 ? 66UL :

		(type == EcType::CURVE25519? 32UL :
		//(type == EcType::CURVE448  ? 56UL :

		(throw InvalidArgumentException(
			"Invalid Elliptic Curve type is given."
		))

		//)
		))))))))))));
}


/**
 * @brief	Gets Elliptic Curve size, in Byte, that can fit in the array of mbedtls_mpi_uint.
 *
 * @exception	RuntimeException	Thrown when Invalid Elliptic Curve type is given.
 *
 * @param	type	The curve type.
 *
 * @return	The size in Byte.
 */
inline constexpr size_t GetCurveByteSizeFitsMpi(EcType type)
{
	return
		(
			(GetCurveByteSize(type) + (sizeof(mbedtls_mpi_uint) - 1)) /
			sizeof(mbedtls_mpi_uint)
		) *
		sizeof(mbedtls_mpi_uint);
}
static_assert(GetCurveByteSizeFitsMpi(EcType::SECP192R1) == 24UL, "Programming Error");
static_assert(GetCurveByteSizeFitsMpi(EcType::SECP224R1) == 32UL, "Programming Error");
static_assert(GetCurveByteSizeFitsMpi(EcType::SECP256R1) == 32UL, "Programming Error");
static_assert(GetCurveByteSizeFitsMpi(EcType::SECP384R1) == 48UL, "Programming Error");
static_assert(GetCurveByteSizeFitsMpi(EcType::BrPo512R1) == 64UL, "Programming Error");
static_assert(GetCurveByteSizeFitsMpi(EcType::SECP521R1) == 72UL, "Programming Error");


/**
 * @brief Translate the EcKey type to the mbed TLS EC group ID.
 *
 * @param type The curve type.
 * @return constexpr mbedtls_ecp_group_id The mbed TLS EC group ID.
 */
inline constexpr mbedtls_ecp_group_id ToEcGroupId(EcType type)
{
	return
		(type == EcType::SECP192R1 ? mbedtls_ecp_group_id::MBEDTLS_ECP_DP_SECP192R1 :
		(type == EcType::SECP224R1 ? mbedtls_ecp_group_id::MBEDTLS_ECP_DP_SECP224R1 :
		(type == EcType::SECP256R1 ? mbedtls_ecp_group_id::MBEDTLS_ECP_DP_SECP256R1 :
		(type == EcType::SECP384R1 ? mbedtls_ecp_group_id::MBEDTLS_ECP_DP_SECP384R1 :
		(type == EcType::SECP521R1 ? mbedtls_ecp_group_id::MBEDTLS_ECP_DP_SECP521R1 :

		(type == EcType::BrPo256R1 ? mbedtls_ecp_group_id::MBEDTLS_ECP_DP_BP256R1 :
		(type == EcType::BrPo384R1 ? mbedtls_ecp_group_id::MBEDTLS_ECP_DP_BP384R1 :
		(type == EcType::BrPo512R1 ? mbedtls_ecp_group_id::MBEDTLS_ECP_DP_BP512R1 :

		(type == EcType::SECP192K1 ? mbedtls_ecp_group_id::MBEDTLS_ECP_DP_SECP192K1 :
		(type == EcType::SECP224K1 ? mbedtls_ecp_group_id::MBEDTLS_ECP_DP_SECP224K1 :
		(type == EcType::SECP256K1 ? mbedtls_ecp_group_id::MBEDTLS_ECP_DP_SECP256K1 :

		(type == EcType::CURVE25519? mbedtls_ecp_group_id::MBEDTLS_ECP_DP_CURVE25519:
		//(type == EcType::CURVE448  ? mbedtls_ecp_group_id::MBEDTLS_ECP_DP_CURVE448  :

		(throw InvalidArgumentException("Invalid Elliptic Curve type is given."))

		//)
		))))))))))));
}


/**
 * @brief Translate the mbed TLS EC group ID to the EcKey type.
 *
 * @param type The mbed TLS EC group ID.
 * @return constexpr mbedtls_ecp_group_id The curve type.
 */
inline constexpr EcType ToEcType(mbedtls_ecp_group_id type)
{
	return
		(type == mbedtls_ecp_group_id::MBEDTLS_ECP_DP_SECP192R1 ? EcType::SECP192R1 :
		(type == mbedtls_ecp_group_id::MBEDTLS_ECP_DP_SECP224R1 ? EcType::SECP224R1 :
		(type == mbedtls_ecp_group_id::MBEDTLS_ECP_DP_SECP256R1 ? EcType::SECP256R1 :
		(type == mbedtls_ecp_group_id::MBEDTLS_ECP_DP_SECP384R1 ? EcType::SECP384R1 :
		(type == mbedtls_ecp_group_id::MBEDTLS_ECP_DP_SECP521R1 ? EcType::SECP521R1 :

		(type == mbedtls_ecp_group_id::MBEDTLS_ECP_DP_BP256R1 ? EcType::BrPo256R1 :
		(type == mbedtls_ecp_group_id::MBEDTLS_ECP_DP_BP384R1 ? EcType::BrPo384R1 :
		(type == mbedtls_ecp_group_id::MBEDTLS_ECP_DP_BP512R1 ? EcType::BrPo512R1 :

		(type == mbedtls_ecp_group_id::MBEDTLS_ECP_DP_SECP192K1 ? EcType::SECP192K1 :
		(type == mbedtls_ecp_group_id::MBEDTLS_ECP_DP_SECP224K1 ? EcType::SECP224K1 :
		(type == mbedtls_ecp_group_id::MBEDTLS_ECP_DP_SECP256K1 ? EcType::SECP256K1 :

		(type == mbedtls_ecp_group_id::MBEDTLS_ECP_DP_CURVE25519? EcType::CURVE25519:
		//(type == mbedtls_ecp_group_id::MBEDTLS_ECP_DP_CURVE448 ? EcType::CURVE448   :

		(throw InvalidArgumentException("Invalid Elliptic Curve type is given."))

		//)
		))))))))))));
}


} // namespace mbedTLScpp
