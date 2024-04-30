// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


/**
 * @brief Usage of this file:
 *        Only define one DECENTENCLAVE_DEV_LEVEL_<NUM> macro with the compiler
 *        flag, where <NUM> is the development level number; all the other
 *        macros in this file will be automatically defined
 *
 *        The enabled/disabled functionalities that are not belong to any
 *        specific hardware platform will be documented in the following
 *        sections
 *
 *        For hardware platform specific functionalities, please refer to the
 *        corresponding header files
 *
 */

/**
 * @brief This switch indicates that the Decent Enclave built is expected to
 *        run in a development environment
 *
 *        This switch should be automatically enabled if one of the following
 *        development level switches is enabled
 *
 *        NOTE: this is different from the DEBUG switch, which is used to
 *        indicate that the Decent Enclave is built with debug information,
 *        while in development mode, the enclave could be built in release
 *        mode, but still run in a development environment
 *        (instead of production environment)
 *
 *
 * #define DECENTENCLAVE_DEV_MODE
 *
 */

/**
 * @brief This switch enables the DecentEnclave development mode
 *        level 0 and above
 *
 *        This is the most basic development level, so that it will
 *        automatically enables all the development levels above it
 *
 *        Under this level, the enclave is expecting to run within a simulated
 *        development environment, so any functionality requires a real
 *        hardware platform will be disabled
 *
 *        Affected features:
 *        TBD
 *
 * #define DECENTENCLAVE_DEV_LEVEL_0
 *
 */

/**
 * @brief This switch enables the DecentEnclave development mode
 *        level 1 and above
 *
 *        Affected features:
 *        TBD
 *
 * #define DECENTENCLAVE_DEV_LEVEL_1
 *
 */


//===== level 0 =====

#ifdef DECENTENCLAVE_DEV_LEVEL_0

#	ifndef DECENTENCLAVE_DEV_MODE
#		define DECENTENCLAVE_DEV_MODE
#	endif // DECENTENCLAVE_DEV_MODE

#	define DECENTENCLAVE_DEV_LEVEL_1

#endif // DECENTENCLAVE_DEV_LEVEL_0

//===== level 0 =====

#ifdef DECENTENCLAVE_DEV_LEVEL_1

#	ifndef DECENTENCLAVE_DEV_MODE
#		define DECENTENCLAVE_DEV_MODE
#	endif // DECENTENCLAVE_DEV_MODE

#endif // DECENTENCLAVE_DEV_LEVEL_1
