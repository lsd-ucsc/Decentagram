# Copyright (c) 2022 SimpleSysIO
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

cmake_minimum_required(VERSION 3.14)

project(SimpleSysIO VERSION 0.0.1 LANGUAGES CXX)

OPTION(SIMPLESYSIOTEST "Option to build SimpleSysIO test executable." OFF)

add_subdirectory(include)

set(ENV{SIMPLESYSIO_HOME} ${CMAKE_CURRENT_LIST_DIR})
set(
	SIMPLESYSIO_HOME
	${CMAKE_CURRENT_LIST_DIR}
	CACHE STRING
	"SimpleSysIO home directory"
	FORCE
)

set(ENV{SIMPLESYSIO_INCLUDE} $ENV{SIMPLESYSIO_HOME}/include)
set(
	SIMPLESYSIO_INCLUDE
	${SIMPLESYSIO_HOME}/include
	CACHE STRING
	"SimpleSysIO include directory"
	FORCE
)

add_library(SimpleSysIO INTERFACE)

target_include_directories(SimpleSysIO INTERFACE include)
