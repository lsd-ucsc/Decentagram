// Copyright (c) 2022 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <DecentEnclave/Common/Keyring.hpp>
#include <DecentEnclave/Common/Sgx/Crypto.hpp>
#include <DecentEnclave/Common/Sgx/Exceptions.hpp>
#include <DecentEnclave/Trusted/Sgx/Random.hpp>

#include <mbedTLScpp/EcKey.hpp>
#include <mbedTLScpp/DefaultRbg.hpp>

#include <sgx_tcrypto.h>


namespace Revoker
{


DECENTENCLAVE_KEYRING_KEY(
	Secp256r1,
	mbedTLScpp::EcKeyPair<mbedTLScpp::EcType::SECP256R1>,
	mbedTLScpp::EcPublicKeyBase<>
)
{
	using namespace mbedTLScpp;

	sgx_ecc_state_handle_t eccHlr;
	auto sgxRet = sgx_ecc256_open_context(&eccHlr);
	DECENTENCLAVE_CHECK_SGX_RUNTIME_ERROR(
		sgxRet,
		sgx_ecc256_open_context
	);

	sgx_ec256_private_t priv;
	sgx_ec256_public_t pub;
	sgxRet = sgx_ecc256_create_key_pair(&priv, &pub, eccHlr);
	sgx_ecc256_close_context(eccHlr);
	DECENTENCLAVE_CHECK_SGX_RUNTIME_ERROR(
		sgxRet,
		sgx_ecc256_create_key_pair
	);

	mbedTLScpp::EcKeyPair<mbedTLScpp::EcType::SECP256R1> keyPair(
		mbedTLScpp::EcType::SECP256R1
	);

	DecentEnclave::Common::Sgx::ExportEcKey(keyPair, pub);
	DecentEnclave::Common::Sgx::ExportEcKey(keyPair, priv);

	return keyPair;
}


DECENTENCLAVE_KEYRING_KEY(
	Secp256k1,
	mbedTLScpp::EcKeyPair<mbedTLScpp::EcType::SECP256K1>,
	mbedTLScpp::EcPublicKeyBase<>
)
{
	using namespace mbedTLScpp;

	DecentEnclave::Trusted::Sgx::RandGenerator rand;
	return EcKeyPair<EcType::SECP256K1>::Generate(rand);
}


} // namespace Revoker
