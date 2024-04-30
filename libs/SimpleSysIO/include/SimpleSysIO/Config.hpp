// Copyright (c) 2022 SimpleSysIO
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

/**
 * @brief Enable all implementations that are relying on the system calls
 *
 */
#ifdef SIMPLESYSIO_ENABLE_SYSCALL

#	define SIMPLESYSIO_ENABLE_SYSCALL_NETWORKING
#	define SIMPLESYSIO_ENABLE_SYSCALL_FILESYSTEM

#endif // SIMPLESYSIO_ENABLE_SYSCALL
