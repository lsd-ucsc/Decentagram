# Copyright (c) 2022 Decentagram
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.


cmake_minimum_required(VERSION 3.18)

project(Revoker LANGUAGES C CXX VERSION 0.0.1)


################################################################################
# Set compile options
################################################################################

if(MSVC)
	set(COMMON_OPTIONS /W4 /WX /EHsc /MP /GR /Zc:__cplusplus
		/wd4815 # Mitigate issue in SGXSDK about zero-sized array
		/wd4310 # Mitigate issue in SGXSDK about cast truncates in #include <limits>
	)
	set(DEBUG_OPTIONS /MTd /Od /Zi /DDEBUG)
	set(RELEASE_OPTIONS /MT /Ox /Oi /Ob2 /fp:fast)# /DNDEBUG
#	set(CMAKE_EXE_LINKER_FLAGS_DEBUG ${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG)
else()
	set(COMMON_OPTIONS -pthread -Wall -Wextra -Werror)
	set(DEBUG_OPTIONS -O0 -g -DDEBUG)
	set(RELEASE_OPTIONS -O2) #-DNDEBUG defined by default
endif()

set(DEBUG_OPTIONS ${COMMON_OPTIONS} ${DEBUG_OPTIONS})
set(RELEASE_OPTIONS ${COMMON_OPTIONS} ${RELEASE_OPTIONS})

if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG")
endif()

################################################################################
# Setup CMake environment for enclave targets
################################################################################

#Removed Basic Runtime Checks in MSVC
if(MSVC)
	STRING (REGEX REPLACE "/RTC(su|[1su])" ""
		CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
	STRING (REGEX REPLACE "/RTC(su|[1su])" ""
		CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
	STRING (REGEX REPLACE "/RTC(su|[1su])" ""
		CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
	STRING (REGEX REPLACE "/RTC(su|[1su])" ""
		CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
endif()

#Remove all standard libraries dependency here so that enclave DLL can be
# compiled properly. And it will be added back later for non-enclave apps.
set(UNTRUSTED_CXX_STANDARD_LIBRARIES ${CMAKE_CXX_STANDARD_LIBRARIES_INIT})
set(UNTRUSTED_C_STANDARD_LIBRARIES ${CMAKE_C_STANDARD_LIBRARIES_INIT})
set(CMAKE_CXX_STANDARD_LIBRARIES "")
set(CMAKE_C_STANDARD_LIBRARIES "")

# Add DebugSimulation to CMake configuration types
set(CMAKE_CONFIGURATION_TYPES Release Debug DebugSimulation)
set_property(GLOBAL PROPERTY DEBUG_CONFIGURATIONS Debug DebugSimulation)

set(CMAKE_CXX_FLAGS_DEBUGSIMULATION           ${CMAKE_CXX_FLAGS_DEBUG})
set(CMAKE_C_FLAGS_DEBUGSIMULATION             ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_EXE_LINKER_FLAGS_DEBUGSIMULATION    ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
set(CMAKE_SHARED_LINKER_FLAGS_DEBUGSIMULATION ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
set(CMAKE_STATIC_LINKER_FLAGS_DEBUGSIMULATION ${CMAKE_STATIC_LINKER_FLAGS_DEBUG})

################################################################################
# Fetch external dependencies
################################################################################

include(FetchContent)

add_subdirectory(libs/SimpleCMakeScripts)
simplecmakescripts_enable()

add_subdirectory(libs/SimpleUtf)
add_subdirectory(libs/SimpleObjects)
add_subdirectory(libs/SimpleRlp)
add_subdirectory(libs/SimpleJson)
add_subdirectory(libs/SimpleSysIO)
add_subdirectory(libs/SimpleConcurrency)

set(SIMPLEBOOST_BOOST_VER 1.81.0 CACHE STRING "Boost version to use." FORCE)
add_subdirectory(libs/SimpleBoostSelector)
simpleboost_enable_sublib(asio)

add_subdirectory(libs/EclipseMonitor)
add_subdirectory(libs/DecentEnclave)

add_subdirectory(libs/mbedTLScpp)

## MbedTLS
set(ENABLE_TESTING OFF CACHE BOOL "Build mbed TLS tests." FORCE)
set(ENABLE_PROGRAMS OFF CACHE BOOL "Build mbed TLS programs." FORCE)
if (MSVC)
	set(
		MSVC_STATIC_RUNTIME
		ON
		CACHE BOOL
		"Build the libraries with /MT compiler flag"
		FORCE
	)
endif()
add_subdirectory(libs/mbedtls)
mbedTLScpp_Decentize_Normal(mbedcrypto)
mbedTLScpp_Decentize_Normal(mbedx509)
mbedTLScpp_Decentize_Normal(mbedtls)
mbedTLScpp_Decentize_Enclave(mbedcrypto_enclave)
mbedTLScpp_Decentize_Enclave(mbedx509_enclave)
mbedTLScpp_Decentize_Enclave(mbedtls_enclave)
set(MBEDTLS_INCLUDE_DIRS ${MBEDTLS_DIR}/include/)
set(MBEDCRYPTO_LIBRARY mbedcrypto)
set(MBEDX509_LIBRARY mbedx509)
set(MBEDTLS_LIBRARY mbedtls)


################################################################################
# Add source directories
################################################################################

add_subdirectory(src)
add_subdirectory(tests)
