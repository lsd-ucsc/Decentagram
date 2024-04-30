# Copyright (c) 2022 DecentEnclave
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

cmake_minimum_required(VERSION 3.14)

project(DecentEnclave VERSION 0.0.1 LANGUAGES CXX)

# OPTION(DECENTENCLAVE_TEST "Option to build DecentEnclave test executable." OFF)

add_subdirectory(include)

set(
	DECENTENCLAVE_HOME
	${CMAKE_CURRENT_LIST_DIR}
	CACHE STRING
	"DecentEnclave home directory"
	FORCE
)

set(
	DECENTENCLAVE_INCLUDE
	${DECENTENCLAVE_HOME}/include
	CACHE STRING
	"DecentEnclave include directory"
	FORCE
)

add_library(DecentEnclave INTERFACE)

target_include_directories(DecentEnclave INTERFACE include)

# if(${DECENTENCLAVE_TEST})
# 	enable_testing()
# 	add_subdirectory(test)
# endif(${DECENTENCLAVE_TEST})
