# Copyright (c) 2022 SimpleConcurrency
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

option(
	SIMPLECONCURRENCY_INSTALL_HEADERS
	"Install SimpleConcurrency headers."
	ON
)

if(SIMPLECONCURRENCY_INSTALL_HEADERS)

	file(GLOB headers "SimpleConcurrency/*.hpp")

	install(
		FILES ${headers}
		DESTINATION include/SimpleConcurrency
		PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
	)

	add_subdirectory(SimpleConcurrency)

endif(SIMPLECONCURRENCY_INSTALL_HEADERS)
