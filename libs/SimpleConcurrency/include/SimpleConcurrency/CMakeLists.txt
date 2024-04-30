# Copyright (c) 2022 SimpleConcurrency
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.


if(SIMPLECONCURRENCY_INSTALL_HEADERS)

	file(GLOB headers "Threading/*.hpp")

	install(
		FILES ${headers}
		DESTINATION include/SimpleConcurrency/Threading
		PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
	)

endif(SIMPLECONCURRENCY_INSTALL_HEADERS)
