# Copyright (c) 2022 SimpleCMakeScripts
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.


cmake_minimum_required(VERSION 3.10)


include_guard()


message(STATUS "Setting Intel SGX Variables...")


################################################################################
# SGX SDK Root Path
################################################################################

####################
# Windows OS
####################
If(WIN32)

	if((NOT DEFINED ENV{SGXSDKInstallPath}))
		message(FATAL_ERROR "Intel SGX SDK is not installed properly!")
	else()
		set(_DECENTENCLAVE_SGXSDK_HOME "$ENV{SGXSDKInstallPath}")
	endif()


####################
# Linux OS
####################
elseif(UNIX AND NOT APPLE)

	if((DEFINED ENV{SGX_SDK}))
		set(_DECENTENCLAVE_SGXSDK_HOME "$ENV{SGX_SDK}")
	else()
		set(_DECENTENCLAVE_SGXSDK_HOME "/opt/intel/sgxsdk")
	endif()


####################
# Other OS (unsupported OS)
####################
else()
	message(FATAL_ERROR "OS not supported by Intel SGX!")


endif()


get_filename_component(
	_DECENTENCLAVE_SGXSDK_HOME
	${_DECENTENCLAVE_SGXSDK_HOME}
	ABSOLUTE
)

if(NOT EXISTS ${_DECENTENCLAVE_SGXSDK_HOME})
	message(FATAL_ERROR "Intel SGX SDK is not installed properly!")
endif()

set(
	DECENTENCLAVE_SGXSDK_HOME
	"${_DECENTENCLAVE_SGXSDK_HOME}"
	CACHE STRING
	"The path to the Intel SGX SDK directory"
	FORCE
)


################################################################################
# Internal variables for convenience
################################################################################

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
	set(
		_DECENTENCLAVE_SGXSDK_ARCHI_WIN
		"win32"
	)
	set(
		_DECENTENCLAVE_SGXSDK_ARCHI_LINUX_LIB #for libs
		"lib"
	)
	set(
		_DECENTENCLAVE_SGXSDK_ARCHI_LINUX_EXE #for executable, e.g. signing tool.
		"x86"
	)
else()
	set(
		_DECENTENCLAVE_SGXSDK_ARCHI_WIN
		"x64"
	)
	set(
		_DECENTENCLAVE_SGXSDK_ARCHI_LINUX_LIB #for libs
		"lib64"
	)
	set(
		_DECENTENCLAVE_SGXSDK_ARCHI_LINUX_EXE #for executable, e.g. signing tool.
		"x64"
	)
endif()

set(
	DECENTENCLAVE_SGXSDK_ARCHI_WIN
	${_DECENTENCLAVE_SGXSDK_ARCHI_WIN}
	CACHE INTERNAL
	"Architecture name used in SGXSDK for Windows"
	FORCE
)
set(
	DECENTENCLAVE_SGXSDK_ARCHI_LINUX_LIB #for libs
	${_DECENTENCLAVE_SGXSDK_ARCHI_LINUX_LIB}
	CACHE INTERNAL
	"Architecture name used in SGXSDK library for Linux"
	FORCE
)
set(
	DECENTENCLAVE_SGXSDK_ARCHI_LINUX_EXE #for executable, e.g. signing tool.
	${_DECENTENCLAVE_SGXSDK_ARCHI_LINUX_EXE}
	CACHE INTERNAL
	"Architecture name used in SGXSDK executable for Linux"
	FORCE
)


if (WIN32)
	set(
		_DECENTENCLAVE_SGXSDK_LIB
		"${DECENTENCLAVE_SGXSDK_HOME}/bin/${DECENTENCLAVE_SGXSDK_ARCHI_WIN}"
	)
	set(
		_DECENTENCLAVE_SGXSDK_TOOL
		"${DECENTENCLAVE_SGXSDK_HOME}/bin/win32/Release"
	)
else()
	set(
		_DECENTENCLAVE_SGXSDK_LIB
		"${DECENTENCLAVE_SGXSDK_HOME}/${DECENTENCLAVE_SGXSDK_ARCHI_LINUX_LIB}"
	)
	set(
		_DECENTENCLAVE_SGXSDK_TOOL
		"${DECENTENCLAVE_SGXSDK_HOME}/bin/${DECENTENCLAVE_SGXSDK_ARCHI_LINUX_EXE}"
	)
endif()

set(
	DECENTENCLAVE_SGXSDK_LIB
	${_DECENTENCLAVE_SGXSDK_LIB}
	CACHE INTERNAL
	"The path to the Intel SGX SDK library directory"
	FORCE
)
set(
	DECENTENCLAVE_SGXSDK_TOOL
	${_DECENTENCLAVE_SGXSDK_TOOL}
	CACHE INTERNAL
	"The path to the directory containing Intel SGX SDK tools "
	FORCE
)


################################################################################
# SGX Tools Path
################################################################################

if(WIN32)
	set(
		_DECENTENCLAVE_SGXSDK_EDGER
		"${DECENTENCLAVE_SGXSDK_TOOL}/sgx_edger8r.exe"
	)
	set(
		_DECENTENCLAVE_SGXSDK_SIGNER
		"${DECENTENCLAVE_SGXSDK_TOOL}/sgx_sign.exe"
	)
else()
	set(
		_DECENTENCLAVE_SGXSDK_EDGER
		"${DECENTENCLAVE_SGXSDK_TOOL}/sgx_edger8r"
	)
	set(
		_DECENTENCLAVE_SGXSDK_SIGNER
		"${DECENTENCLAVE_SGXSDK_TOOL}/sgx_sign"
	)
endif()

get_filename_component(
	_DECENTENCLAVE_SGXSDK_EDGER
	${_DECENTENCLAVE_SGXSDK_EDGER}
	ABSOLUTE
)
get_filename_component(
	_DECENTENCLAVE_SGXSDK_SIGNER
	${_DECENTENCLAVE_SGXSDK_SIGNER}
	ABSOLUTE
)

set(
	DECENTENCLAVE_SGXSDK_EDGER
	${_DECENTENCLAVE_SGXSDK_EDGER}
	CACHE FILEPATH
	"The path to the Intel SGXSDK edger8r tool"
	FORCE
)
set(
	DECENTENCLAVE_SGXSDK_SIGNER
	${_DECENTENCLAVE_SGXSDK_SIGNER}
	CACHE FILEPATH
	"The path to the Intel SGXSDK signing tool"
	FORCE
)


################################################################################
# SGX SDK library headers
################################################################################

#headers:
set(
	DECENTENCLAVE_SGXSDK_INCLUDE
	"${DECENTENCLAVE_SGXSDK_HOME}/include"
	CACHE PATH
	"The path to the Intel SGX SDK include directory"
	FORCE
)

if(WIN32)
	set(
		_DECENTENCLAVE_SGXSDK_INCLUDE_C
		"${DECENTENCLAVE_SGXSDK_INCLUDE}/tlibc"
	)
	set(
		_DECENTENCLAVE_SGXSDK_INCLUDE_CXX
		"${DECENTENCLAVE_SGXSDK_INCLUDE}/libc++"
	)
else()
	set(
		_DECENTENCLAVE_SGXSDK_INCLUDE_C
		"${DECENTENCLAVE_SGXSDK_INCLUDE}/tlibc"
	)
	set(
		_DECENTENCLAVE_SGXSDK_INCLUDE_CXX
		"${DECENTENCLAVE_SGXSDK_INCLUDE}/libcxx"
	)
endif()

set(
	DECENTENCLAVE_SGXSDK_INCLUDE_C
	${_DECENTENCLAVE_SGXSDK_INCLUDE_C}
	CACHE PATH
	"The path to the Intel SGX SDK include directory for standard C library"
	FORCE
)
set(
	DECENTENCLAVE_SGXSDK_INCLUDE_CXX
	${_DECENTENCLAVE_SGXSDK_INCLUDE_CXX}
	CACHE PATH
	"The path to the Intel SGX SDK include directory for standard C++ library"
	FORCE
)


################################################################################
# Compile flags
################################################################################

if(WIN32)

	# NOTE: need to use /d2FH4- to disable FH4 to avoid undefined __CxxFrameHandler4
	# https://community.intel.com/t5/Intel-Software-Guard-Extensions/Problem-Cannot-find-SGX-Debugger-in-Visual-Studio/m-p/1126761/thread-id/1785
	# https://devblogs.microsoft.com/cppblog/making-cpp-exception-handling-smaller-x64/

	#C Flags:
	set(
		_DECENTENCLAVE_SGXSDK_TRUSTED_C_FLAGS
		/X
	)

	#CXX Flags:
	set(
		_DECENTENCLAVE_SGXSDK_TRUSTED_CXX_FLAGS
		/Zc:__cplusplus /d2FH4-
	)

else()

	#C Flags:
	set(
		_DECENTENCLAVE_SGXSDK_TRUSTED_C_FLAGS
		-nostdinc -fvisibility=hidden -fpie -fstack-protector
	)

	#CXX Flags:
	set(
		_DECENTENCLAVE_SGXSDK_TRUSTED_CXX_FLAGS
		-nostdinc++
	)

endif()

set(
	DECENTENCLAVE_SGXSDK_TRUSTED_C_FLAGS
	${_DECENTENCLAVE_SGXSDK_TRUSTED_C_FLAGS}
	CACHE INTERNAL
	"Intel SGX SDK C (and C++) compiler flags for the trusted part"
	FORCE
)
set(
	DECENTENCLAVE_SGXSDK_TRUSTED_CXX_FLAGS
	${_DECENTENCLAVE_SGXSDK_TRUSTED_CXX_FLAGS}
	CACHE INTERNAL
	"Intel SGX SDK C++ compiler flags for the trusted part"
	FORCE
)


################################################################################
# Linker flags
################################################################################
if(WIN32)

	#Linker Flags:
	set(
		_DECENTENCLAVE_SGXSDK_TRUSTED_LINKER_FLAGS
		/NODEFAULTLIB /NOENTRY
	)

else()

	#Linker Flags:
	set(
		_DECENTENCLAVE_SGXSDK_TRUSTED_LINKER_FLAGS
		"-Wl,--no-undefined" "-nostdlib" "-nodefaultlibs" "-nostartfiles"
		"-Wl,-Bstatic" "-Wl,-Bsymbolic" "-Wl,--no-undefined"
		"-Wl,-pie,-eenclave_entry" "-Wl,--export-dynamic"
		"-Wl,--defsym,__ImageBase=0"
	)

endif()

set(
	DECENTENCLAVE_SGXSDK_TRUSTED_LINKER_FLAGS
	${_DECENTENCLAVE_SGXSDK_TRUSTED_LINKER_FLAGS}
	CACHE INTERNAL
	"Intel SGX SDK linker flags for the trusted part"
	FORCE
)


################################################################################
# Finished
################################################################################

macro(decent_enclave_print_config_sgx)
	message(STATUS "==================== INTEL SGX CONFIG ====================")
	message(STATUS "Intel SGX SDK home:         "
		"${DECENTENCLAVE_SGXSDK_HOME}")
	message(STATUS "Intel SGX SDK include:      "
		"${DECENTENCLAVE_SGXSDK_INCLUDE}")
	message(STATUS "Intel SGX SDK include C:    "
		"${DECENTENCLAVE_SGXSDK_INCLUDE_C}")
	message(STATUS "Intel SGX SDK include C++:  "
		"${DECENTENCLAVE_SGXSDK_INCLUDE_CXX}")
	message(STATUS "Intel SGX SDK C flags:      "
		"${DECENTENCLAVE_SGXSDK_TRUSTED_C_FLAGS}")
	message(STATUS "Intel SGX SDK C++ flags:    "
		"${DECENTENCLAVE_SGXSDK_TRUSTED_CXX_FLAGS}")
	message(STATUS "Intel SGX SDK linker flags: "
		"${DECENTENCLAVE_SGXSDK_TRUSTED_LINKER_FLAGS}")
	message(STATUS "Intel SGX SDK edger8r:      "
		"${DECENTENCLAVE_SGXSDK_EDGER}")
	message(STATUS "Intel SGX SDK signer:       "
		"${DECENTENCLAVE_SGXSDK_SIGNER}")
	message(STATUS "==========================================================")
endmacro(decent_enclave_print_config_sgx)

message(STATUS "Finished setting Intel SGX Variables.")
message(STATUS "")
