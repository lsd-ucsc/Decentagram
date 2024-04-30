// Copyright (c) 2022 mbedTLScpp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstddef>
#include <cstdint>

// #include <mbedtls/constant_time.h>

// Currently, the declaration of mbedtls_ct_memcmp() is not wrapped in
// an extern "C" block.
extern "C" int mbedtls_ct_memcmp( const void *a, const void *b, size_t n );

namespace mbedTLScpp
{
namespace Internal
{

/**
 * @brief Compare equality of memory content in constant time.
 *
 * @param a
 * @param b
 * @param n
 * @return int 1 for equal, 0 for not equal.
 */
inline int ConstTimeMemEqual( const void *a, const void *b, size_t n ) noexcept
{
	// mbedtls_ct_memcmp should return in range [0, 256).
	uint32_t initRes = static_cast<uint32_t>(mbedtls_ct_memcmp(a, b, n));

	// Map 0 to 1 and [1, 256) to 0 using only constant-time arithmetic.
	return static_cast<int>(0x1 & ((initRes - 1) >> 8));
}

/**
 * @brief Compare inequality of memory content in constant time.
 *
 * @param a
 * @param b
 * @param n
 * @return int 0 for equal, 1 for not equal.
 */
inline int ConstTimeMemNotEqual( const void *a, const void *b, size_t n ) noexcept
{
	// ConstTimeMemEqual should return in range [0, 1]
	uint8_t initRes = static_cast<uint8_t>(ConstTimeMemEqual(a, b, n));

	// Map 0 to 1 and 1 to 0 using only constant-time arithmetic.
	return static_cast<int>(0x1 ^ initRes);
}

} // namespace Internal
} // namespace mbedTLScpp
