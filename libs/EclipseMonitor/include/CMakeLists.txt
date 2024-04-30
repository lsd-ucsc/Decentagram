# Copyright (c) 2022 EclipseMonitor
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

option(ECLIPSEMONITOR_INSTALL_HEADERS "Install EclipseMonitor headers." ON)

if(ECLIPSEMONITOR_INSTALL_HEADERS)

	file(GLOB headers "EclipseMonitor/*.hpp")

	install(FILES ${headers}
		DESTINATION include/EclipseMonitor
		PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

	# add_subdirectory(EclipseMonitor)

endif(ECLIPSEMONITOR_INSTALL_HEADERS)
