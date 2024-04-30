# Copyright (c) 2022 SimpleConcurrency
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

cmake_minimum_required(VERSION 3.14)

project(SimpleConcurrency VERSION 0.0.1 LANGUAGES CXX)

OPTION(SIMPLECONCURRENCY_TEST "Option to build SimpleConcurrency test executable." OFF)

add_subdirectory(include)

set(
	SIMPLECONCURRENCY_HOME
	${CMAKE_CURRENT_LIST_DIR}
	CACHE STRING
	"SimpleConcurrency home directory"
	FORCE
)

set(
	SIMPLECONCURRENCY_INCLUDE
	${SIMPLECONCURRENCY_HOME}/include
	CACHE STRING
	"SimpleConcurrency include directory"
	FORCE
)

add_library(SimpleConcurrency INTERFACE)

target_include_directories(SimpleConcurrency INTERFACE include)
