# Copyright (c) 2022 SimpleSysIO
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

option(SIMPLESYSIO_INSTALL_HEADERS "Install SimpleSysIO headers." ON)

if(SIMPLESYSIO_INSTALL_HEADERS)

	file(GLOB headers "SimpleSysIO/*.hpp")

	install(
		FILES ${headers}
		DESTINATION include/SimpleSysIO
		PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
	)

	add_subdirectory(SimpleSysIO)

endif(SIMPLESYSIO_INSTALL_HEADERS)
