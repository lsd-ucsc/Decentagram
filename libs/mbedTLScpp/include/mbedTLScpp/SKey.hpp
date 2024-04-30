#pragma once

#include "SecretArray.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	/**
	 * @brief The alias data type for secret keys.
	 *
	 * @tparam _KeyLenInBits The length of the secret key in \b bits .
	 */
	template<size_t _KeyLenInBits>
	using SKey = SecretArray<uint8_t, _KeyLenInBits / 8>;
}