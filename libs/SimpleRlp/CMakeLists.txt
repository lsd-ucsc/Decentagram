# Copyright (c) 2022 SimpleRlp
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

cmake_minimum_required(VERSION 3.14)

project(SimpleRlp VERSION 0.0.1 LANGUAGES CXX)

OPTION(SIMPLERLP_TEST "Option to build SimpleRlp test executable." OFF)

add_subdirectory(include)

set(ENV{SIMPLERLP_HOME} ${CMAKE_CURRENT_LIST_DIR})
set(SIMPLERLP_HOME ${CMAKE_CURRENT_LIST_DIR})

set(ENV{SIMPLERLP_INCLUDE} $ENV{SIMPLERLP_HOME}/include)
set(SIMPLERLP_INCLUDE ${SIMPLERLP_HOME}/include)

add_library(SimpleRlp INTERFACE)

target_include_directories(SimpleRlp INTERFACE include)
