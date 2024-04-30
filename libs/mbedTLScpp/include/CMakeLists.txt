option(MBEDTLSCPP_INSTALL_HEADERS "Install mbed TLS cpp headers." ON)

if(MBEDTLSCPP_INSTALL_HEADERS)

	file(GLOB headers "mbedTLScpp/*.hpp")

	install(FILES ${headers}
		DESTINATION include/mbedTLScpp
		PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

	add_subdirectory(mbedTLScpp)

endif(MBEDTLSCPP_INSTALL_HEADERS)
