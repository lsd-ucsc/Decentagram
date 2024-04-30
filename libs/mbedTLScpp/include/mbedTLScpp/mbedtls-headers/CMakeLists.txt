file(GLOB headers_common "common/*.h")

install(FILES ${headers_common}
	DESTINATION include/mbedTLScpp/mbedtls-headers/common
	PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

file(GLOB headers_normal "normal/*.h")

install(FILES ${headers_normal}
	DESTINATION include/mbedTLScpp/mbedtls-headers/normal
	PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

file(GLOB headers_enclave "enclave/*.h")

install(FILES ${headers_enclave}
	DESTINATION include/mbedTLScpp/mbedtls-headers/enclave
	PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
