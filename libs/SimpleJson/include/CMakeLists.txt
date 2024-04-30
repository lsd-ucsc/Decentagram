# Copyright (c) 2022 SimpleJson
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

option(SIMPLEJSON_INSTALL_HEADERS "Install SimpleJson headers." ON)

if(SIMPLEJSON_INSTALL_HEADERS)

	file(GLOB headers "SimpleJson/*.hpp")

	install(FILES ${headers}
		DESTINATION include/SimpleJson
		PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

	add_subdirectory(SimpleJson)

endif(SIMPLEJSON_INSTALL_HEADERS)
