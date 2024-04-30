# Copyright (c) 2022 SimpleBoostSelector
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

cmake_minimum_required(VERSION 3.14)

project(SimpleBoost VERSION 0.0.1 LANGUAGES CXX)

set(SIMPLEBOOST_BOOST_VER
	"1.70.0"
	CACHE STRING
	"The version number of the selected boost library"
)

set(SIMPLEBOOST_SCRIPTS_HOME
	"${CMAKE_CURRENT_LIST_DIR}/cmake"
	CACHE STRING
	"The path to the SimpleBoostSelector's scripts directory"
	FORCE
)

option(SIMPLEBOOST_TEST
	"Option to build SimpleBoost test executable."
	OFF)

string(REGEX REPLACE
	"\\."
	"_"
	SIMPLEBOOST_BOOST_VER_US
	${SIMPLEBOOST_BOOST_VER})

set(SIMPLEBOOST_BOOST_VER_CMAKE_FILE_PATH
	${SIMPLEBOOST_SCRIPTS_HOME}/versions/boost_${SIMPLEBOOST_BOOST_VER_US}.cmake
)

include(${SIMPLEBOOST_SCRIPTS_HOME}/functions.cmake)

simpleboost_msg_status("Boost version selected: ${SIMPLEBOOST_BOOST_VER}")
simpleboost_msg_status(
	"Boost version file: ${SIMPLEBOOST_BOOST_VER_CMAKE_FILE_PATH}"
)

simpleboost_include_ver_file(${SIMPLEBOOST_BOOST_VER_CMAKE_FILE_PATH})
include(${SIMPLEBOOST_SCRIPTS_HOME}/sub_lib_meta.cmake)
