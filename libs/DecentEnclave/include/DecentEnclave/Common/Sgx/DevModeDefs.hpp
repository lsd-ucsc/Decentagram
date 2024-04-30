// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include "../DevModeDefs.hpp"


/**
 * @brief Affected features:
 *       1. Ensure the enclave image is loaded with debug flag
 *
 *
 * #define DECENTENCLAVE_DEV_MODE
 *
 */

/**
 * @brief Affected features:
 *        TBD
 *
 * #define DECENTENCLAVE_DEV_LEVEL_0
 *
 */


#if defined(DECENTENCLAVE_DEV_MODE)
#	define DECENTENCLAVE_SGX_DEBUG_FLAG 1
#elif !defined(NDEBUG) || defined(EDEBUG) || defined(DEBUG)
#	define DECENTENCLAVE_SGX_DEBUG_FLAG 1
#else
#	define DECENTENCLAVE_SGX_DEBUG_FLAG 0
#endif // DECENTENCLAVE_DEV_MODE
