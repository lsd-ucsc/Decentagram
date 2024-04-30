# Copyright (c) 2022 SimpleRlp
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

option(SIMPLERLP_INSTALL_HEADERS "Install SimpleRlp headers." ON)

if(SIMPLERLP_INSTALL_HEADERS)

	file(GLOB headers "SimpleRlp/*.hpp")

	install(FILES ${headers}
		DESTINATION include/SimpleRlp
		PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

	add_subdirectory(SimpleRlp)

endif(SIMPLERLP_INSTALL_HEADERS)
