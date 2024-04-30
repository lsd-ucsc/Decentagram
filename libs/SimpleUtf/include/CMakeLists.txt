# Copyright (c) 2022 SimpleUtf
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

option(SIMPLEUTF_INSTALL_HEADERS "Install SimpleUtf headers." ON)

if(SIMPLEUTF_INSTALL_HEADERS)

	file(GLOB headers "SimpleUtf/*.hpp")

	install(FILES ${headers}
		DESTINATION include/SimpleUtf
		PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

	#add_subdirectory(SimpleUtf)

endif(SIMPLEUTF_INSTALL_HEADERS)
