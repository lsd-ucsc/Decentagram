# Copyright 2022 SimpleCMakeScripts
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.


include_guard()


macro(simple_curl_https_pre_configuration)
	set(
		BUILD_CURL_EXE OFF CACHE BOOL
		"Set to ON to build CURL executable."
		FORCE
	)
	set(
		BUILD_TESTING OFF CACHE BOOL
		"Set to ON to build libcurl test libraries."
		FORCE
	)
	set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
	set(
		CURL_ENABLE_EXPORT_TARGET OFF CACHE BOOL
		"to enable cmake export target"
		FORCE
	)
	set(
		CURL_STATICLIB ON CACHE BOOL
		"Set to ON to build libcurl with static linking."
		FORCE
	)
	if (MSVC)
		set(
			CURL_STATIC_CRT ON CACHE BOOL
			"Set to ON to build libcurl with static CRT on Windows (/MT)."
			FORCE
		)
	endif()
	set(
		CURL_ZLIB OFF CACHE BOOL
		"Set to ON to enable building curl with zlib support."
		FORCE
	)
	set(
		CURL_USE_MBEDTLS ON CACHE BOOL
		"Set to ON to build libcurl with MbedTLS."
		FORCE
	)
	set(
		CURL_USE_LIBSSH2 OFF CACHE BOOL
		"Set to ON to enable building curl with libSSH2 support."
		FORCE
	)
	set(
		CURL_USE_OPENSSL OFF CACHE BOOL
		"Set to ON to build libcurl with OpenSSL."
		FORCE
	)
	set(
		HTTP_ONLY ON CACHE BOOL
		"Set to ON to build libcurl with only http functionalities."
		FORCE
	)
	set(
		ENABLE_MANUAL OFF CACHE BOOL
		"Set to ON to build libcurl manual."
		FORCE
	)
	set(
		ENABLE_UNIX_SOCKETS OFF CACHE BOOL
		"Define if you want Unix domain sockets support"
		FORCE
	)
endmacro(simple_curl_https_pre_configuration)
