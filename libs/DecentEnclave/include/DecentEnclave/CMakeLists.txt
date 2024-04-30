# Copyright (c) 2022 DecentEnclave
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

if(DECENTENCLAVE_INSTALL_HEADERS)

	file(GLOB headers "Common/*.hpp")

	install(
		FILES ${headers}
		DESTINATION include/DecentEnclave/Common
		PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
	)


	file(GLOB headers "Untrusted/*.hpp")

	install(
		FILES ${headers}
		DESTINATION include/DecentEnclave/Untrusted
		PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
	)

endif(DECENTENCLAVE_INSTALL_HEADERS)
