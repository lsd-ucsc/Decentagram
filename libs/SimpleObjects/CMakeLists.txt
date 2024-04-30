# Copyright 2022 SimpleObjects
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

cmake_minimum_required(VERSION 3.14)

project(SimpleObjects VERSION 0.0.1 LANGUAGES CXX)

add_subdirectory(include)

OPTION(SIMPLEOBJECTS_TEST "Option to build SimpleObjects test executable." OFF)
OPTION(SIMPLEOBJECTS_TEST_LCOV "Option to turn on test code coverage." OFF)
# SET(SIMPLEOBJECTS_TEST ON CACHE BOOL "Option to build SimpleObjects test executable." FORCE)

set(ENV{SIMPLEOBJECTS_HOME} ${CMAKE_CURRENT_LIST_DIR})
set(SIMPLEOBJECTS_HOME ${CMAKE_CURRENT_LIST_DIR})

set(ENV{SIMPLEOBJECTS_INCLUDE} $ENV{SIMPLEOBJECTS_HOME}/include)
set(SIMPLEOBJECTS_INCLUDE ${SIMPLEOBJECTS_HOME}/include)

add_library(SimpleObjects INTERFACE)

target_include_directories(SimpleObjects INTERFACE include)
