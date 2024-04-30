// Copyright (c) 2023 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <array>


namespace EclipseMonitor
{

/**
 * @brief type for Trusted Timestamp, which received from a trusted source
 *
 */
using TrustedTimestamp = uint64_t;


/**
 * @brief type for Eclipse Monitor Session ID, which is a 128-bit unique random
 *        bytes, similar to UUID
 *
 */
using SessionID = std::array<uint8_t, 16>;


/**
 * @brief type for Sync Message Nonce, which is a 256-bit unique random bytes
 *
 */
using SyncNonce = std::array<uint8_t, 32>;


} // namespace EclipseMonitor
