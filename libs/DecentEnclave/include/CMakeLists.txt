# Copyright (c) 2022 DecentEnclave
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

option(DECENTENCLAVE_INSTALL_HEADERS "Install DecentEnclave headers." ON)

if(DECENTENCLAVE_INSTALL_HEADERS)

	file(GLOB headers "DecentEnclave/*.hpp")

	install(
		FILES ${headers}
		DESTINATION include/DecentEnclave
		PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
	)

	add_subdirectory(DecentEnclave)

endif(DECENTENCLAVE_INSTALL_HEADERS)
