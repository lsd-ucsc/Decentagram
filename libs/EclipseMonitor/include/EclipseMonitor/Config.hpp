// Copyright (c) 2023 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#ifdef ECLIPSEMONITOR_DEV_MODE
#	define ECLIPSEMONITOR_DEV_USE_DEV_SESSION_ID
#	define ECLIPSEMONITOR_DEV_USE_DEV_SYNC_NONCE
#	define ECLIPSEMONITOR_DEV_DISABLE_REFRESH_SYNC
#endif
