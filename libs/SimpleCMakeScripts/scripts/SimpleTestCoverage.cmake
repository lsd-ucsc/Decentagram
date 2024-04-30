# Copyright 2022 SimpleCMakeScripts
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.


include_guard()


################################################################################
# Test Coverage
################################################################################

macro(simple_cxx_test_coverage TARGET_NAME BASE_DIR)
	if(UNIX AND NOT APPLE)
		message(STATUS "Setting up C++ test coverage for target "
				"${TARGET_NAME}, with base directory at ${BASE_DIR}.")

		set(${TARGET_NAME}_OBJ_OUTPUT_DIR
				"${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET_NAME}.dir/src/")

		set(${TARGET_NAME}_EXECUTABLE
				"${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}")
		set(${TARGET_NAME}_LCOV_OUTPUT_DIR
				"${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.lcov/")
		file(MAKE_DIRECTORY ${${TARGET_NAME}_LCOV_OUTPUT_DIR})
		set(${TARGET_NAME}_LCOV_OUTPUT_ZIP
				"${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.lcov.zip")

		find_program(GCOV_BIN gcov)
		find_program(LCOV_BIN lcov)
		find_program(GENHTML_BIN genhtml)
		find_program(ZIP_BIN zip)
		if(NOT GCOV_BIN)
			message(FATAL_ERROR "gcov program is not found.")
		endif()
		if(NOT LCOV_BIN)
			message(FATAL_ERROR "lcov program is not found.")
		endif()
		if(NOT GENHTML_BIN)
			message(FATAL_ERROR "genhtml program is not found.")
		endif()
		if(NOT ZIP_BIN)
			message(FATAL_ERROR "zip program is not found.")
		endif()

		target_compile_options(${TARGET_NAME}
			PUBLIC "-g"
			PUBLIC "-fprofile-arcs"
			PUBLIC "-ftest-coverage")
		target_link_libraries(${TARGET_NAME}
			gcov)

		add_custom_target(${TARGET_NAME}_lcov
			COMMAND ${LCOV_BIN} --gcov-tool ${GCOV_BIN} --quiet
			--directory ${${TARGET_NAME}_OBJ_OUTPUT_DIR}
			--base-directory ${BASE_DIR}
			--zerocounters
			COMMAND ${LCOV_BIN} --gcov-tool ${GCOV_BIN} --quiet
				--directory ${${TARGET_NAME}_OBJ_OUTPUT_DIR}
				--base-directory ${BASE_DIR}
				--initial --capture
				--output-file ${TARGET_NAME}.lcov.init
			COMMAND ${${TARGET_NAME}_EXECUTABLE}
			COMMAND ${LCOV_BIN} --gcov-tool ${GCOV_BIN} --quiet
				--directory ${${TARGET_NAME}_OBJ_OUTPUT_DIR}
				--base-directory ${BASE_DIR}
				--capture
				--output-file ${TARGET_NAME}.lcov.capture
			COMMAND ${LCOV_BIN} --gcov-tool ${GCOV_BIN} --quiet
				-a ${TARGET_NAME}.lcov.init
				-a ${TARGET_NAME}.lcov.capture
				--output-file ${TARGET_NAME}.lcov.count
			COMMAND ${LCOV_BIN} --gcov-tool ${GCOV_BIN}
				--extract ${TARGET_NAME}.lcov.count "\"${BASE_DIR}/*\""
				--output-file ${TARGET_NAME}.lcov.extract
			COMMAND ${GENHTML_BIN} --quiet --demangle-cpp
				--output-directory ${${TARGET_NAME}_LCOV_OUTPUT_DIR}
				${TARGET_NAME}.lcov.extract
			COMMAND ${ZIP_BIN} --recurse-paths --quiet #--move
				${${TARGET_NAME}_LCOV_OUTPUT_ZIP}
				${${TARGET_NAME}_LCOV_OUTPUT_DIR}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMENT "Generating testing code coverage report...")
		add_dependencies(${TARGET_NAME}_lcov ${TARGET_NAME})
	else()
		message(WARNING "Test coverage setup is ignored in platforms "
				"other than Linux.")
	endif()
endmacro(simple_cxx_test_coverage)
