file(GLOB headers_internal "Internal/*.hpp")

install(FILES ${headers_internal}
	DESTINATION include/mbedTLScpp/Internal
	PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

add_subdirectory(mbedtls-headers)
