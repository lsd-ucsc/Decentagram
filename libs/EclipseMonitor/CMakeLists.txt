# Copyright (c) 2022 EclipseMonitor
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

cmake_minimum_required(VERSION 3.14)

project(EclipseMonitor VERSION 0.0.1 LANGUAGES CXX)

OPTION(ECLIPSEMONITOR_TEST "Option to build EclipseMonitor test executable." OFF)

add_subdirectory(include)

set(ECLIPSEMONITOR_HOME
	${CMAKE_CURRENT_LIST_DIR}
	CACHE STRING
	"Path to the eclipse monitor home directory"
	FORCE)

set(ECLIPSEMONITOR_INCLUDE
	${CMAKE_CURRENT_LIST_DIR}/include
	CACHE STRING
	"Path to the eclipse monitor include directory"
	FORCE)

add_library(EclipseMonitor INTERFACE)

target_include_directories(EclipseMonitor INTERFACE include)
