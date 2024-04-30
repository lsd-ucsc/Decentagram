# Copyright (c) 2022 SimpleCMakeScripts
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.


cmake_minimum_required(VERSION 3.10)

project(SimpleCMakeScripts VERSION 0.3)


set(SIMPLECMAKESCRIPTS_PROJECT_NAME
	${PROJECT_NAME}
	CACHE STRING
	"The path to the directory containing the scripts"
	FORCE
)

set(SIMPLECMAKESCRIPTS_SCRIPTS_DIR
	${CMAKE_CURRENT_LIST_DIR}/scripts
	CACHE STRING
	"The path to the directory containing the scripts"
	FORCE
)

include(${CMAKE_CURRENT_LIST_DIR}/internal/basics.cmake)

simplecmakescripts_enable()
