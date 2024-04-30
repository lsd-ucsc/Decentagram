# Copyright (c) 2022 SimpleJson
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

cmake_minimum_required(VERSION 3.14)

project(SimpleJson VERSION 0.0.1 LANGUAGES CXX)

OPTION(SIMPLEJSON_TEST "Option to build SimpleJson test executable." OFF)

add_subdirectory(include)

set(ENV{SIMPLEJSON_HOME} ${CMAKE_CURRENT_LIST_DIR})
set(SIMPLEJSON_HOME ${CMAKE_CURRENT_LIST_DIR})

set(ENV{SIMPLEJSON_INCLUDE} $ENV{SIMPLEJSON_HOME}/include)
set(SIMPLEJSON_INCLUDE ${SIMPLEJSON_HOME}/include)

add_library(SimpleJson INTERFACE)

target_include_directories(SimpleJson INTERFACE include)
