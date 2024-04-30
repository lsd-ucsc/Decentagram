# Copyright (c) 2022 SimpleCMakeScripts
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.


cmake_minimum_required(VERSION 3.13)


include_guard()


include(DecentEnclaveIntelSgxLib)


##
# [Private] [Macro] _decent_enclave_add_target_sgx_helper_vars
#
# Some helper variables for platform dependent configurations
##
macro(_decent_enclave_add_target_sgx_helper_vars)
	if(MSVC)
		set(BINARY_SUB_DIR "$<CONFIG>")
		set(WHOLE_ARCHIVE_FLAG_BEGIN "")
		set(WHOLE_ARCHIVE_FLAG_END "")
		set(GROUP_FLAG_BEGIN "")
		set(GROUP_FLAG_END "")
	else()
		set(BINARY_SUB_DIR "")
		set(WHOLE_ARCHIVE_FLAG_BEGIN -Wl,--whole-archive)
		set(WHOLE_ARCHIVE_FLAG_END -Wl,--no-whole-archive)
		set(GROUP_FLAG_BEGIN -Wl,--start-group)
		set(GROUP_FLAG_END -Wl,--end-group)
	endif()
endmacro(_decent_enclave_add_target_sgx_helper_vars)


##
# [Private] [Macro] _decent_encalve_add_target_sgx_print_target_info
#
# Print the target configurations
##
macro(_decent_encalve_add_target_sgx_print_target_info)
	message(STATUS "==================== INTEL SGX TARGET ====================")
	# message(STATUS "ARGN = ${ARGN}")
	message(STATUS "TARGET_NAME        = ${_SGX_TARGET_TARGET_NAME}")
	message(STATUS "UNTRUSTED_SOURCE   = ${_SGX_TARGET_UNTRUSTED_SOURCE}")
	message(STATUS "UNTRUSTED_DEF      = ${_SGX_TARGET_UNTRUSTED_DEF}")
	message(STATUS "UNTRUSTED_INCL_DIR = ${_SGX_TARGET_UNTRUSTED_INCL_DIR}")
	message(STATUS "UNTRUSTED_COMP_OPT = ${_SGX_TARGET_UNTRUSTED_COMP_OPT}")
	message(STATUS "UNTRUSTED_LINK_OPT = ${_SGX_TARGET_UNTRUSTED_LINK_OPT}")
	message(STATUS "UNTRUSTED_LINK_LIB = ${_SGX_TARGET_UNTRUSTED_LINK_LIB}")
	message(STATUS "TRUSTED_SOURCE     = ${_SGX_TARGET_TRUSTED_SOURCE}")
	message(STATUS "TRUSTED_DEF        = ${_SGX_TARGET_TRUSTED_DEF}")
	message(STATUS "TRUSTED_INCL_DIR   = ${_SGX_TARGET_TRUSTED_INCL_DIR}")
	message(STATUS "TRUSTED_COMP_OPT   = ${_SGX_TARGET_TRUSTED_COMP_OPT}")
	message(STATUS "TRUSTED_LINK_OPT   = ${_SGX_TARGET_TRUSTED_LINK_OPT}")
	message(STATUS "TRUSTED_LINK_LIB   = ${_SGX_TARGET_TRUSTED_LINK_LIB}")
	message(STATUS "EDL_PATH           = ${_SGX_TARGET_EDL_PATH}")
	message(STATUS "EDL_INCLUDE        = ${_SGX_TARGET_EDL_INCLUDE}")
	message(STATUS "EDL_OUTPUT_DIR     = ${_SGX_TARGET_EDL_OUTPUT_DIR}")
	message(STATUS "SIGN_CONFIG        = ${_SGX_TARGET_SIGN_CONFIG}")
	message(STATUS "SIGN_KEY           = ${_SGX_TARGET_SIGN_KEY}")
	message(STATUS "==========================================================")
	message(STATUS "")
endmacro(_decent_encalve_add_target_sgx_print_target_info)


##
# [Private] [Macro] _decent_enclave_add_target_sgx_add_edl
#
# Add the EDL target of an intel SGX program
##
macro(_decent_enclave_add_target_sgx_add_edl)

	##################################################
	# EDL target
	##################################################
	set(_SGX_TARGET_EDL_TRUSTED_OUTPUT
		${_SGX_TARGET_EDL_OUTPUT_DIR}/Enclave_t.c)
	set(_SGX_TARGET_EDL_UNTRUSTED_OUTPUT
		${_SGX_TARGET_EDL_OUTPUT_DIR}/Enclave_u.c)

	set(_SGX_TARGET_EDL_SEARCH_PATH_ARG "")
	foreach(item IN LISTS DECENTENCLAVE_SGXSDK_INCLUDE _SGX_TARGET_EDL_INCLUDE)
		set(_SGX_TARGET_EDL_SEARCH_PATH_ARG
			${_SGX_TARGET_EDL_SEARCH_PATH_ARG} --search-path "${item}")
	endforeach()

	execute_process(
		COMMAND "${DECENTENCLAVE_SGXSDK_EDGER}"
		--trusted "${_SGX_TARGET_EDL_PATH}"
		${_SGX_TARGET_EDL_SEARCH_PATH_ARG}
		WORKING_DIRECTORY "${_SGX_TARGET_EDL_OUTPUT_DIR}"
	)

	execute_process(
		COMMAND "${DECENTENCLAVE_SGXSDK_EDGER}"
		--untrusted "${_SGX_TARGET_EDL_PATH}"
		${_SGX_TARGET_EDL_SEARCH_PATH_ARG}
		WORKING_DIRECTORY "${_SGX_TARGET_EDL_OUTPUT_DIR}"
	)

	add_custom_command(OUTPUT ${_SGX_TARGET_EDL_TRUSTED_OUTPUT}
		COMMAND "${DECENTENCLAVE_SGXSDK_EDGER}"
		--trusted "${_SGX_TARGET_EDL_PATH}"
		${_SGX_TARGET_EDL_SEARCH_PATH_ARG}
		WORKING_DIRECTORY "${_SGX_TARGET_EDL_OUTPUT_DIR}"
		DEPENDS "${_SGX_TARGET_EDL_PATH}"
		COMMENT "Processing EDL for trusted part..."
	)

	add_custom_command(OUTPUT ${_SGX_TARGET_EDL_UNTRUSTED_OUTPUT}
		COMMAND "${DECENTENCLAVE_SGXSDK_EDGER}"
		--untrusted "${_SGX_TARGET_EDL_PATH}"
		${_SGX_TARGET_EDL_SEARCH_PATH_ARG}
		WORKING_DIRECTORY "${_SGX_TARGET_EDL_OUTPUT_DIR}"
		DEPENDS "${_SGX_TARGET_EDL_PATH}"
		COMMENT "Processing EDL for untrusted part..."
	)

	add_custom_target(${_SGX_TARGET_TARGET_NAME}_edl
		DEPENDS ${_SGX_TARGET_EDL_TRUSTED_OUTPUT}
				${_SGX_TARGET_EDL_UNTRUSTED_OUTPUT})

	set_target_properties(${_SGX_TARGET_TARGET_NAME}_edl
		PROPERTIES FOLDER "${_SGX_TARGET_TARGET_NAME}")

endmacro(_decent_enclave_add_target_sgx_add_edl)


##
# [Private] [Macro] _decent_enclave_add_target_sgx_add_trusted
#
# Add the trusted target of an intel SGX program
##
macro(_decent_enclave_add_target_sgx_add_trusted)

	##################################################
	# Enclave
	##################################################
	list(APPEND _SGX_TARGET_TRUSTED_SOURCE ${_SGX_TARGET_EDL_TRUSTED_OUTPUT})

	add_library(${_SGX_TARGET_TARGET_NAME}_trusted SHARED
		${_SGX_TARGET_TRUSTED_SOURCE})

	#C++ standard
	set_property(TARGET ${_SGX_TARGET_TARGET_NAME}_trusted
		PROPERTY CXX_STANDARD 11)

	#defines:
	target_compile_definitions(
		${_SGX_TARGET_TARGET_NAME}_trusted
		PRIVATE
			DECENT_ENCLAVE_TRUSTED
			DECENT_ENCLAVE_PLATFORM_SGX
			DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
			${_SGX_TARGET_TRUSTED_DEF}
	)

	#include directories:
	target_include_directories(
		${_SGX_TARGET_TARGET_NAME}_trusted
		PRIVATE
			${_SGX_TARGET_TRUSTED_INCL_DIR}
	)

	#compiler flags:
	target_compile_options(${_SGX_TARGET_TARGET_NAME}_trusted
		PRIVATE ${DECENTENCLAVE_SGXSDK_TRUSTED_C_FLAGS}
				$<$<COMPILE_LANGUAGE:CXX>:${DECENTENCLAVE_SGXSDK_TRUSTED_CXX_FLAGS}>
				${_SGX_TARGET_TRUSTED_COMP_OPT})

	#linker flags:
	target_link_options(${_SGX_TARGET_TARGET_NAME}_trusted
		PRIVATE ${DECENTENCLAVE_SGXSDK_TRUSTED_LINKER_FLAGS}
				${_SGX_TARGET_TRUSTED_LINK_OPT})

	#folder
	set_target_properties(${_SGX_TARGET_TARGET_NAME}_trusted
		PROPERTIES FOLDER "${_SGX_TARGET_TARGET_NAME}")

	set(_SGX_TARGET_TRUSTED_LIB_NAME
		"${CMAKE_SHARED_LIBRARY_PREFIX}${_SGX_TARGET_TARGET_NAME}_trusted$<$<CONFIG:Debug>:${CMAKE_DEBUG_POSTFIX}>")
	set(_SGX_TARGET_TRUSTED_LIB
		"${_SGX_TARGET_TRUSTED_LIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
	set(_SGX_TARGET_TRUSTED_LIB_SIGNED
		"${_SGX_TARGET_TRUSTED_LIB_NAME}.signed${CMAKE_SHARED_LIBRARY_SUFFIX}")

	add_custom_command(TARGET ${_SGX_TARGET_TARGET_NAME}_trusted
		POST_BUILD
		COMMAND  "${DECENTENCLAVE_SGXSDK_SIGNER}" sign
		-config  "${_SGX_TARGET_SIGN_CONFIG}"
		-key     "${_SGX_TARGET_SIGN_KEY}"
		-enclave "${CMAKE_CURRENT_BINARY_DIR}/${BINARY_SUB_DIR}/${_SGX_TARGET_TRUSTED_LIB}"
		-out     "${CMAKE_CURRENT_BINARY_DIR}/${_SGX_TARGET_TRUSTED_LIB_SIGNED}"
	)

	target_link_libraries(${_SGX_TARGET_TARGET_NAME}_trusted
		${WHOLE_ARCHIVE_FLAG_BEGIN}
		IntelSGX::Trusted::switchless
		IntelSGX::Trusted::rts
		${WHOLE_ARCHIVE_FLAG_END}
		${GROUP_FLAG_BEGIN}
		IntelSGX::Trusted::stdc
		IntelSGX::Trusted::cxx
		IntelSGX::Trusted::service
		IntelSGX::Trusted::key_exchange
		IntelSGX::Trusted::crypto
		IntelSGX::Trusted::file_system
		${_SGX_TARGET_TRUSTED_LINK_LIB}
		${GROUP_FLAG_END}
	)

	add_dependencies(${_SGX_TARGET_TARGET_NAME}_trusted
		${_SGX_TARGET_TARGET_NAME}_edl)

endmacro(_decent_enclave_add_target_sgx_add_trusted)


##
# [Private] [Macro] _decent_enclave_add_target_sgx_add_untrusted
#
# Add the untrusted target of an intel SGX program
##
macro(_decent_enclave_add_target_sgx_add_untrusted)

	##################################################
	# Untrusted
	##################################################
	list(APPEND _SGX_TARGET_UNTRUSTED_SOURCE ${_SGX_TARGET_EDL_UNTRUSTED_OUTPUT})

	add_executable(${_SGX_TARGET_TARGET_NAME}
		${_SGX_TARGET_UNTRUSTED_SOURCE})

	#C++ standard
	set_property(TARGET ${_SGX_TARGET_TARGET_NAME}
		PROPERTY CXX_STANDARD 11)

	#defines:
	target_compile_definitions(
		${_SGX_TARGET_TARGET_NAME}
		PRIVATE
			DECENT_ENCLAVE_PLATFORM_SGX
			DECENT_ENCLAVE_PLATFORM_SGX_UNTRUSTED
			DECENT_ENCLAVE_PLATFORM_SGX_IMAGE="${_SGX_TARGET_TRUSTED_LIB_SIGNED}"
			DECENT_ENCLAVE_PLATFORM_SGX_TOKEN="${_SGX_TARGET_TARGET_NAME}_Enclave.token"
			${_SGX_TARGET_UNTRUSTED_DEF}
	)

	#include directories:
	target_include_directories(
		${_SGX_TARGET_TARGET_NAME}
		PRIVATE
			${_SGX_TARGET_UNTRUSTED_INCL_DIR}
	)

	#compiler flags:
	target_compile_options(${_SGX_TARGET_TARGET_NAME}
		PRIVATE ${_SGX_TARGET_UNTRUSTED_COMP_OPT})

	#linker flags:
	target_link_options(${_SGX_TARGET_TARGET_NAME}
		PRIVATE ${_SGX_TARGET_UNTRUSTED_LINK_OPT})

	#linker flags:
	set_target_properties(${_SGX_TARGET_TARGET_NAME}
		PROPERTIES FOLDER "${_SGX_TARGET_TARGET_NAME}")

	target_link_libraries(${_SGX_TARGET_TARGET_NAME}
		IntelSGX::Untrusted::Libs
		${_SGX_TARGET_UNTRUSTED_LINK_LIB}
	)

	add_dependencies(${_SGX_TARGET_TARGET_NAME}
		${_SGX_TARGET_TARGET_NAME}_trusted)

endmacro(_decent_enclave_add_target_sgx_add_untrusted)



##
# [Public] [Function] decent_enclave_add_target_sgx
#
# Add an intel SGX program consisting of a EDL target, a trusted target,
# and an untrusted target
#
# Function call example:
#
# decent_enclave_add_target_sgx(<TARGET_NAME>
# 		UNTRUSTED_SOURCE   <src1>;<src2>;...
# 		UNTRUSTED_DEF      <def1>;<def2>;...
# 		UNTRUSTED_INCL_DIR <dir1>;<dir2>;...
# 		UNTRUSTED_COMP_OPT <opt1>;<opt2>;...
# 		UNTRUSTED_LINK_OPT <opt1>;<opt2>;...
# 		UNTRUSTED_LINK_LIB <lib1>;<lib2>;...
# 		TRUSTED_SOURCE     <src1>;<src2>;...
# 		TRUSTED_DEF        <def1>;<def2>;...
# 		TRUSTED_INCL_DIR   <dir1>;<dir2>;...
# 		TRUSTED_COMP_OPT   <opt1>;<opt2>;...
# 		TRUSTED_LINK_OPT   <opt1>;<opt2>;...
# 		TRUSTED_LINK_LIB   <lib1>;<lib2>;...
# 		EDL_PATH           <path>
# 		EDL_INCLUDE        <path1>;<path2>;...
# 		EDL_OUTPUT_DIR     <dir>
# 		SIGN_CONFIG        <path>
# 		SIGN_KEY           <path>
# )
##
function(decent_enclave_add_target_sgx)

	# Parse function arguments
	if (${ARGC} LESS 31)
		message(FATAL "Not enough arguments passed to add_sgx_target function")
	endif()

	# Argument 0 is the target name
	set(_SGX_TARGET_TARGET_NAME "${ARGV0}")

	# other function arguments to be parsed
	set(options "")
	set(oneValueArgs EDL_PATH EDL_OUTPUT_DIR SIGN_CONFIG SIGN_KEY)
	set(multiValueArgs
		UNTRUSTED_SOURCE UNTRUSTED_DEF UNTRUSTED_INCL_DIR
		UNTRUSTED_COMP_OPT UNTRUSTED_LINK_OPT UNTRUSTED_LINK_LIB
		TRUSTED_SOURCE   TRUSTED_DEF   TRUSTED_INCL_DIR
		TRUSTED_COMP_OPT   TRUSTED_LINK_OPT   TRUSTED_LINK_LIB
		EDL_INCLUDE
	)

	# parse
	cmake_parse_arguments(
		PARSE_ARGV
		1 # The parsing starts with 1st argument, since 0 is target name
		_SGX_TARGET
		"${options}"
		"${oneValueArgs}"
		"${multiValueArgs}"
	)

	_decent_enclave_add_target_sgx_helper_vars()
	_decent_encalve_add_target_sgx_print_target_info()

	_decent_enclave_add_target_sgx_add_edl()

	_decent_enclave_add_target_sgx_add_trusted()

	_decent_enclave_add_target_sgx_add_untrusted()

endfunction(decent_enclave_add_target_sgx)
