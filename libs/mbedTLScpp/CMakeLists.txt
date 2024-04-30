# Copyright (c) 2022 mbedTLScpp
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.


cmake_minimum_required(VERSION 3.14)

project(mbedTLScpp VERSION 3.2.1.1 LANGUAGES CXX)

OPTION(MBEDTLSCPP_TEST "Option to build mbedTLScpp test executable." OFF)

add_subdirectory(include)

set(
	MBEDTLSCPP_HOME
	${CMAKE_CURRENT_LIST_DIR}
	CACHE PATH
	"mbedTLScpp home directory"
	FORCE
)

set(
	MBEDTLSCPP_INCLUDE
	${MBEDTLSCPP_HOME}/include
	CACHE PATH
	"mbedTLScpp include directory"
	FORCE
)

set(
	MBEDTLSCPP_MBEDTLS_HEADERS
	${MBEDTLSCPP_INCLUDE}/mbedTLScpp/mbedtls-headers
	CACHE PATH
	"mbedTLScpp directory containing headers manipulate MbedTLS configuration"
	FORCE
)

include(${MBEDTLSCPP_HOME}/cmake/mbedTLScpp_UseMbedtlsHeaders.cmake)

add_library(mbedTLScpp INTERFACE)

target_include_directories(mbedTLScpp INTERFACE include)
