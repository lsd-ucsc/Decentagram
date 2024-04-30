# Copyright (c) 2022 SimpleCMakeScripts
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.


cmake_minimum_required(VERSION 3.10)


include_guard()


include(DecentEnclaveIntelSgxPath)


message(STATUS "Adding Intel SGX Targets...")


##################################################
# SGX libraries
##################################################

####################
# Windows OS
####################
if(WIN32)

	#######
	# Untrusted
	#######
	if(NOT TARGET IntelSGX::Untrusted::service)
		add_library(IntelSGX::Untrusted::service STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Untrusted::service PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Untrusted::service
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
		)
		set_target_properties(IntelSGX::Untrusted::service PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_uae_service.lib"
			IMPORTED_LOCATION_DEBUG           "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_uae_service.lib"
			IMPORTED_LOCATION_RELEASE         "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_uae_service.lib"
			IMPORTED_LOCATION_DEBUGSIMULATION "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_uae_service_sim.lib"
		)
	endif()
	if(NOT TARGET IntelSGX::Untrusted::key_exchange)
		add_library(IntelSGX::Untrusted::key_exchange STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Untrusted::key_exchange PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Untrusted::key_exchange
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
		)
		set_target_properties(IntelSGX::Untrusted::key_exchange PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_ukey_exchange.lib"
			IMPORTED_LOCATION_DEBUG           "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_ukey_exchange.lib"
			IMPORTED_LOCATION_RELEASE         "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_ukey_exchange.lib"
			IMPORTED_LOCATION_DEBUGSIMULATION "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_ukey_exchange.lib"
		)
	endif()
	if(NOT TARGET IntelSGX::Untrusted::rts)
		add_library(IntelSGX::Untrusted::rts STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Untrusted::rts PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Untrusted::rts
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
		)
		set_target_properties(IntelSGX::Untrusted::rts PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_urts.lib"
			IMPORTED_LOCATION_DEBUG           "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_urts.lib"
			IMPORTED_LOCATION_RELEASE         "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_urts.lib"
			IMPORTED_LOCATION_DEBUGSIMULATION "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_urts_sim.lib"
		)
	endif()
	if(NOT TARGET IntelSGX::Untrusted::file_system)
		add_library(IntelSGX::Untrusted::file_system STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Untrusted::file_system PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Untrusted::file_system
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
		)
		set_target_properties(IntelSGX::Untrusted::file_system PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_uprotected_fs.lib"
			IMPORTED_LOCATION_DEBUG           "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_uprotected_fs.lib"
			IMPORTED_LOCATION_RELEASE         "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_uprotected_fs.lib"
			IMPORTED_LOCATION_DEBUGSIMULATION "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_uprotected_fs.lib"
		)
	endif()
	if(NOT TARGET IntelSGX::Untrusted::capable)
		add_library(IntelSGX::Untrusted::capable STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Untrusted::capable PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Untrusted::capable
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
		)
		set_target_properties(IntelSGX::Untrusted::capable PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_capable.lib"
			IMPORTED_LOCATION_DEBUG           "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_capable.lib"
			IMPORTED_LOCATION_RELEASE         "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_capable.lib"
			IMPORTED_LOCATION_DEBUGSIMULATION "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_capable.lib"
		)
	endif()

	#######
	# Trusted
	#######
	if(NOT TARGET IntelSGX::Trusted::rts)
		add_library(IntelSGX::Trusted::rts STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Trusted::rts PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Trusted::rts
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
		)
		set_target_properties(IntelSGX::Trusted::rts PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_trts.lib"
			IMPORTED_LOCATION_DEBUG           "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_trts.lib"
			IMPORTED_LOCATION_RELEASE         "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_trts.lib"
			IMPORTED_LOCATION_DEBUGSIMULATION "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_trts_sim.lib"
		)
	endif()
	if(NOT TARGET IntelSGX::Trusted::service)
		add_library(IntelSGX::Trusted::service STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Trusted::service PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Trusted::service
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
		)
		set_target_properties(IntelSGX::Trusted::service PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_tservice.lib"
			IMPORTED_LOCATION_DEBUG           "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_tservice.lib"
			IMPORTED_LOCATION_RELEASE         "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_tservice.lib"
			IMPORTED_LOCATION_DEBUGSIMULATION "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_tservice_sim.lib"
		)
	endif()
	if(NOT TARGET IntelSGX::Trusted::stdc)
		add_library(IntelSGX::Trusted::stdc STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Trusted::stdc PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Trusted::stdc
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}/tlibc
		)
		set_target_properties(IntelSGX::Trusted::stdc PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_tstdc.lib"
			IMPORTED_LOCATION_DEBUG           "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_tstdc.lib"
			IMPORTED_LOCATION_RELEASE         "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_tstdc.lib"
			IMPORTED_LOCATION_DEBUGSIMULATION "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_tstdc.lib"
		)
	endif()
	if(NOT TARGET IntelSGX::Trusted::cxx)
		add_library(IntelSGX::Trusted::cxx STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Trusted::cxx PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Trusted::cxx
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}/libc++
		)
		set_target_properties(IntelSGX::Trusted::cxx PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_tcxx.lib"
			IMPORTED_LOCATION_DEBUG           "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_tcxx.lib"
			IMPORTED_LOCATION_RELEASE         "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_tcxx.lib"
			IMPORTED_LOCATION_DEBUGSIMULATION "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_tcxx.lib"
		)
	endif()
	if(NOT TARGET IntelSGX::Trusted::key_exchange)
		add_library(IntelSGX::Trusted::key_exchange STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Trusted::key_exchange PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Trusted::key_exchange
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
		)
		set_target_properties(IntelSGX::Trusted::key_exchange PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_tkey_exchange.lib"
			IMPORTED_LOCATION_DEBUG           "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_tkey_exchange.lib"
			IMPORTED_LOCATION_RELEASE         "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_tkey_exchange.lib"
			IMPORTED_LOCATION_DEBUGSIMULATION "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_tkey_exchange.lib"
		)
	endif()
	if(NOT TARGET IntelSGX::Trusted::crypto)
		add_library(IntelSGX::Trusted::crypto STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Trusted::crypto PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Trusted::crypto
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
		)
		set_target_properties(IntelSGX::Trusted::crypto PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_tcrypto.lib"
			IMPORTED_LOCATION_DEBUG           "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_tcrypto.lib"
			IMPORTED_LOCATION_RELEASE         "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_tcrypto.lib"
			IMPORTED_LOCATION_DEBUGSIMULATION "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_tcrypto.lib"
		)
	endif()
	if(NOT TARGET IntelSGX::Trusted::file_system)
		add_library(IntelSGX::Trusted::file_system STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Trusted::file_system PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Trusted::file_system
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
		)
		set_target_properties(IntelSGX::Trusted::file_system PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_tprotected_fs.lib"
			IMPORTED_LOCATION_DEBUG           "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_tprotected_fs.lib"
			IMPORTED_LOCATION_RELEASE         "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_tprotected_fs.lib"
			IMPORTED_LOCATION_DEBUGSIMULATION "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_tprotected_fs.lib"
		)
	endif()
	if(NOT TARGET IntelSGX::Trusted::switchless)
		#add_library(IntelSGX::Trusted::switchless STATIC IMPORTED GLOBAL)
		add_library(IntelSGX::Trusted::switchless INTERFACE IMPORTED GLOBAL)
		#set_target_properties(IntelSGX::Trusted::switchless PROPERTIES
		#	IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Trusted::switchless
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
		)
		#set_target_properties(IntelSGX::Trusted::switchless PROPERTIES
		#	IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_tswitchless.lib"
		#	IMPORTED_LOCATION_DEBUG           "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_tswitchless.lib"
		#	IMPORTED_LOCATION_RELEASE         "${DECENTENCLAVE_SGXSDK_LIB}/Release/sgx_tswitchless.lib"
		#	IMPORTED_LOCATION_DEBUGSIMULATION "${DECENTENCLAVE_SGXSDK_LIB}/Debug/sgx_tswitchless.lib"
		#)
	endif()

####################
# Linux OS
####################

elseif(UNIX)

	#######
	# Untrusted
	#######
	if(NOT TARGET IntelSGX::Untrusted::service)
		add_library(IntelSGX::Untrusted::service SHARED IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Untrusted::service PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Untrusted::service
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
		)
		set_target_properties(IntelSGX::Untrusted::service PROPERTIES
			IMPORTED_LOCATION                 "-lsgx_uae_service"
			IMPORTED_LOCATION_DEBUG           "-lsgx_uae_service"
			IMPORTED_LOCATION_RELEASE         "-lsgx_uae_service"
			IMPORTED_LOCATION_DEBUGSIMULATION "${DECENTENCLAVE_SGXSDK_LIB}/libsgx_uae_service_sim.so"
		)
	endif()
	if(NOT TARGET IntelSGX::Untrusted::key_exchange)
		add_library(IntelSGX::Untrusted::key_exchange STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Untrusted::key_exchange PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Untrusted::key_exchange
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
		)
		set_target_properties(IntelSGX::Untrusted::key_exchange PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/libsgx_ukey_exchange.a"
		)
	endif()
	if(NOT TARGET IntelSGX::Untrusted::rts)
		add_library(IntelSGX::Untrusted::rts SHARED IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Untrusted::rts PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Untrusted::rts
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
		)
		set_target_properties(IntelSGX::Untrusted::rts PROPERTIES
			IMPORTED_LOCATION                 "-lsgx_urts"
			IMPORTED_LOCATION_DEBUG           "-lsgx_urts"
			IMPORTED_LOCATION_RELEASE         "-lsgx_urts"
			IMPORTED_LOCATION_DEBUGSIMULATION "${DECENTENCLAVE_SGXSDK_LIB}/libsgx_urts_sim.so"
		)
	endif()
	if(NOT TARGET IntelSGX::Untrusted::file_system)
		add_library(IntelSGX::Untrusted::file_system STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Untrusted::file_system PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Untrusted::file_system
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
		)
		set_target_properties(IntelSGX::Untrusted::file_system PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/libsgx_uprotected_fs.a"
		)
	endif()

	#######
	# Trusted
	#######
	if(NOT TARGET IntelSGX::Trusted::rts)
		add_library(IntelSGX::Trusted::rts STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Trusted::rts PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Trusted::rts
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
		)
		set_target_properties(IntelSGX::Trusted::rts PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/libsgx_trts.a"
			IMPORTED_LOCATION_DEBUG           "${DECENTENCLAVE_SGXSDK_LIB}/libsgx_trts.a"
			IMPORTED_LOCATION_RELEASE         "${DECENTENCLAVE_SGXSDK_LIB}/libsgx_trts.a"
			IMPORTED_LOCATION_DEBUGSIMULATION "${DECENTENCLAVE_SGXSDK_LIB}/libsgx_trts_sim.a"
		)
	endif()
	if(NOT TARGET IntelSGX::Trusted::service)
		add_library(IntelSGX::Trusted::service STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Trusted::service PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Trusted::service
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
		)
		set_target_properties(IntelSGX::Trusted::service PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/libsgx_tservice.a"
			IMPORTED_LOCATION_DEBUG           "${DECENTENCLAVE_SGXSDK_LIB}/libsgx_tservice.a"
			IMPORTED_LOCATION_RELEASE         "${DECENTENCLAVE_SGXSDK_LIB}/libsgx_tservice.a"
			IMPORTED_LOCATION_DEBUGSIMULATION "${DECENTENCLAVE_SGXSDK_LIB}/libsgx_tservice_sim.a"
		)
	endif()
	if(NOT TARGET IntelSGX::Trusted::stdc)
		add_library(IntelSGX::Trusted::stdc STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Trusted::stdc PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Trusted::stdc
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}/tlibc
		)
		set_target_properties(IntelSGX::Trusted::stdc PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/libsgx_tstdc.a"
		)
	endif()
	if(NOT TARGET IntelSGX::Trusted::cxx)
		add_library(IntelSGX::Trusted::cxx STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Trusted::cxx PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Trusted::cxx
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}/libcxx
		)
		set_target_properties(IntelSGX::Trusted::cxx PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/libsgx_tcxx.a"
		)
	endif()
	if(NOT TARGET IntelSGX::Trusted::key_exchange)
		add_library(IntelSGX::Trusted::key_exchange STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Trusted::key_exchange PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Trusted::key_exchange
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
		)
		set_target_properties(IntelSGX::Trusted::key_exchange PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/libsgx_tkey_exchange.a"
		)
	endif()
	if(NOT TARGET IntelSGX::Trusted::crypto)
		add_library(IntelSGX::Trusted::crypto STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Trusted::crypto PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Trusted::crypto
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
		)
		set_target_properties(IntelSGX::Trusted::crypto PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/libsgx_tcrypto.a"
		)
	endif()
	if(NOT TARGET IntelSGX::Trusted::file_system)
		add_library(IntelSGX::Trusted::file_system STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Trusted::file_system PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		target_include_directories(
			IntelSGX::Trusted::file_system
			BEFORE
			INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
		)
		set_target_properties(IntelSGX::Trusted::file_system PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/libsgx_tprotected_fs.a"
		)
	endif()
	if(NOT TARGET IntelSGX::Trusted::switchless)
		add_library(IntelSGX::Trusted::switchless STATIC IMPORTED GLOBAL)
		set_target_properties(IntelSGX::Trusted::switchless PROPERTIES
			IMPORTED_CONFIGURATIONS Release Debug DebugSimulation)
		set_target_properties(IntelSGX::Trusted::switchless PROPERTIES
			IMPORTED_LOCATION                 "${DECENTENCLAVE_SGXSDK_LIB}/libsgx_tswitchless.a"
		)
	endif()

endif()

####################
# Interfaces
####################

if(NOT TARGET IntelSGX::Trusted::WholeArcLibs)
	add_library(IntelSGX::Trusted::WholeArcLibs INTERFACE IMPORTED GLOBAL)
	target_link_libraries(IntelSGX::Trusted::WholeArcLibs INTERFACE
		IntelSGX::Trusted::switchless
		IntelSGX::Trusted::rts
	)
endif()

if(NOT TARGET IntelSGX::Trusted::GroupLibs)
	add_library(IntelSGX::Trusted::GroupLibs INTERFACE IMPORTED GLOBAL)
	target_link_libraries(IntelSGX::Trusted::GroupLibs INTERFACE
		IntelSGX::Trusted::stdc
		IntelSGX::Trusted::cxx
		IntelSGX::Trusted::service
		IntelSGX::Trusted::key_exchange
		IntelSGX::Trusted::crypto
		IntelSGX::Trusted::file_system
	)
endif()

if(NOT TARGET IntelSGX::Untrusted::Libs)
	add_library(IntelSGX::Untrusted::Libs INTERFACE IMPORTED GLOBAL)
	target_include_directories(
		IntelSGX::Untrusted::Libs
		INTERFACE ${DECENTENCLAVE_SGXSDK_INCLUDE}
	)
	target_link_libraries(IntelSGX::Untrusted::Libs INTERFACE
		IntelSGX::Untrusted::service
		IntelSGX::Untrusted::rts
		IntelSGX::Untrusted::key_exchange
		IntelSGX::Untrusted::file_system
	)
endif()

message(STATUS "Finished Adding Intel SGX Targets.")
message(STATUS "")
