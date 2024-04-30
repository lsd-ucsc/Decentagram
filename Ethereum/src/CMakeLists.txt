# Copyright (c) 2022 Decentagram
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.


include(DecentEnclaveIntelSgx)


decent_enclave_print_config_sgx()


decent_enclave_add_target_sgx(EthereumClient
	UNTRUSTED_SOURCE
		${DECENTENCLAVE_INCLUDE}/DecentEnclave/SgxEdgeSources/NetIO_u.cpp
		${DECENTENCLAVE_INCLUDE}/DecentEnclave/SgxEdgeSources/SysIO_u.cpp
		${CMAKE_CURRENT_LIST_DIR}/Untrusted/Main.cpp
	UNTRUSTED_DEF
		DECENTENCLAVE_DEV_LEVEL_0
		SIMPLESYSIO_ENABLE_SYSCALL
		CURL_STATICLIB
	UNTRUSTED_INCL_DIR
		${CMAKE_CURRENT_LIST_DIR}/../include
	UNTRUSTED_COMP_OPT
		$<$<CONFIG:Debug>:${DEBUG_OPTIONS}>
		$<$<CONFIG:DebugSimulation>:${DEBUG_OPTIONS}>
		$<$<CONFIG:Release>:${RELEASE_OPTIONS}>
	UNTRUSTED_LINK_OPT ""
	UNTRUSTED_LINK_LIB
		SimpleUtf
		SimpleObjects
		SimpleJson
		SimpleRlp
		SimpleSysIO
		SimpleConcurrency
		DecentEnclave
		EclipseMonitor
		mbedTLScpp
		mbedcrypto
		mbedx509
		mbedtls
		libcurl
		Boost::asio
	TRUSTED_SOURCE
		${DECENTENCLAVE_INCLUDE}/DecentEnclave/SgxEdgeSources/AppLambdaHandler_t.cpp
		${DECENTENCLAVE_INCLUDE}/DecentEnclave/SgxEdgeSources/Attestation_t.cpp
		${DECENTENCLAVE_INCLUDE}/DecentEnclave/SgxEdgeSources/Crypto_t.cpp
		${DECENTENCLAVE_INCLUDE}/DecentEnclave/SgxEdgeSources/SysIO_t.cpp
		${CMAKE_CURRENT_LIST_DIR}/Trusted/Enclave.cpp
	TRUSTED_DEF
		DECENTENCLAVE_DEV_LEVEL_0
		ECLIPSEMONITOR_DEV_MODE
		ECLIPSEMONITOR_LOGGING_HEADER=<EthereumClt/Common/SubmoduleLogging.hpp>
	TRUSTED_INCL_DIR
		${CMAKE_CURRENT_LIST_DIR}/../include
	TRUSTED_COMP_OPT
		$<$<CONFIG:Debug>:${DEBUG_OPTIONS}>
		$<$<CONFIG:DebugSimulation>:${DEBUG_OPTIONS}>
		$<$<CONFIG:Release>:${RELEASE_OPTIONS}>
	TRUSTED_LINK_OPT   ""
	TRUSTED_LINK_LIB
		SimpleUtf
		SimpleObjects
		SimpleJson
		SimpleRlp
		SimpleSysIO
		DecentEnclave
		EclipseMonitor
		mbedTLScpp
		mbedcrypto_enclave
		mbedx509_enclave
		mbedtls_enclave
	EDL_PATH
		${CMAKE_CURRENT_LIST_DIR}/Trusted/Enclave.edl
	EDL_INCLUDE
		${DECENTENCLAVE_INCLUDE}
	EDL_OUTPUT_DIR
		${CMAKE_CURRENT_LIST_DIR}
	SIGN_CONFIG
		${CMAKE_CURRENT_LIST_DIR}/Enclave.config.xml
	SIGN_KEY
		${CMAKE_CURRENT_LIST_DIR}/Enclave_private.pem
)
