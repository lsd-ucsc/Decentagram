# Copyright 2022 SimpleObjects
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

option(SIMPLEOBJECTS_INSTALL_HEADERS "Install SimpleObjects headers." ON)

if(SIMPLEOBJECTS_INSTALL_HEADERS)

	file(GLOB headers "SimpleObjects/*.hpp")

	install(FILES ${headers}
		DESTINATION include/SimpleObjects
		PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

	add_subdirectory(SimpleObjects)

endif(SIMPLEOBJECTS_INSTALL_HEADERS)
