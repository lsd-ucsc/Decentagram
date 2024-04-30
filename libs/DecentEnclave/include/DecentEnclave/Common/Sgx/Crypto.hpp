// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#if defined(DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED) || \
	defined(DECENT_ENCLAVE_PLATFORM_SGX_UNTRUSTED)


#include <sgx_tcrypto.h>
#include <mbedTLScpp/Cmac.hpp>
#include <mbedTLScpp/EcKey.hpp>
#include <mbedTLScpp/Hash.hpp>
#include <mbedTLScpp/SKey.hpp>

#include "../Exceptions.hpp"


namespace DecentEnclave
{
namespace Common
{
namespace Sgx
{


template<typename _EcKeyObjTrait>
inline void ImportEcKey(
	sgx_ec256_public_t& outKey,
	const mbedTLScpp::EcPublicKeyBase<_EcKeyObjTrait>& inKey
)
{
	if (inKey.GetEcType() != mbedTLScpp::EcType::SECP256R1)
	{
		throw InvalidArgumentException(
			"SGX SDK only supports EC key of type SECP256R1"
		);
	}

	const mbedtls_ecp_keypair* ecCtx = inKey.GetEcContext();
	const mbedtls_ecp_point& ecPoint = ecCtx->MBEDTLS_PRIVATE(Q);
	const mbedtls_mpi& ecX = ecPoint.MBEDTLS_PRIVATE(X);
	const mbedtls_mpi& ecY = ecPoint.MBEDTLS_PRIVATE(Y);

	int mbedRet = 0;
	mbedRet = mbedtls_mpi_write_binary_le(&ecX, outKey.gx, sizeof(outKey.gx));
	mbedTLScpp::CheckMbedTlsIntRetVal(
		mbedRet,
		"mbedtls_mpi_write_binary_le",
		"DecentEnclave::Common::Sgx::ImportEcKey"
	);
	mbedRet = mbedtls_mpi_write_binary_le(&ecY, outKey.gy, sizeof(outKey.gy));
	mbedTLScpp::CheckMbedTlsIntRetVal(
		mbedRet,
		"mbedtls_mpi_write_binary_le",
		"DecentEnclave::Common::Sgx::ImportEcKey"
	);
}


template<typename _EcKeyObjTrait>
inline void ExportEcKey(
	mbedTLScpp::EcPublicKeyBase<_EcKeyObjTrait>& outKey,
	const sgx_ec256_public_t& inKey
)
{
	if (outKey.GetEcType() != mbedTLScpp::EcType::SECP256R1)
	{
		throw InvalidArgumentException(
			"SGX SDK only supports EC key of type SECP256R1"
		);
	}

	mbedtls_ecp_keypair* ecCtx = outKey.GetEcContext();
	mbedtls_ecp_point& ecPoint = ecCtx->MBEDTLS_PRIVATE(Q);
	mbedtls_mpi& ecX = ecPoint.MBEDTLS_PRIVATE(X);
	mbedtls_mpi& ecY = ecPoint.MBEDTLS_PRIVATE(Y);
	mbedtls_mpi& ecZ = ecPoint.MBEDTLS_PRIVATE(Z);

	int mbedRet = 0;
	mbedRet = mbedtls_mpi_read_binary_le(&ecX, inKey.gx, sizeof(inKey.gx));
	mbedTLScpp::CheckMbedTlsIntRetVal(
		mbedRet,
		"mbedtls_mpi_read_binary_le",
		"DecentEnclave::Common::Sgx::ExportEcKey"
	);
	mbedRet = mbedtls_mpi_read_binary_le(&ecY, inKey.gy, sizeof(inKey.gy));
	mbedTLScpp::CheckMbedTlsIntRetVal(
		mbedRet,
		"mbedtls_mpi_read_binary_le",
		"DecentEnclave::Common::Sgx::ExportEcKey"
	);
	mbedRet = mbedtls_mpi_lset(&ecZ, 1);
	mbedTLScpp::CheckMbedTlsIntRetVal(
		mbedRet,
		"mbedtls_mpi_lset",
		"DecentEnclave::Common::Sgx::ExportEcKey"
	);
}


template<typename _EcKeyObjTrait>
inline void ExportEcKey(
	mbedTLScpp::EcKeyPairBase<_EcKeyObjTrait>& outKey,
	const sgx_ec256_private_t& inKey
)
{
	if (outKey.GetEcType() != mbedTLScpp::EcType::SECP256R1)
	{
		throw InvalidArgumentException(
			"SGX SDK only supports EC key of type SECP256R1"
		);
	}

	mbedtls_ecp_keypair* ecCtx = outKey.GetEcContext();
	mbedtls_mpi& ecD = ecCtx->MBEDTLS_PRIVATE(d);

	int mbedRet = 0;
	mbedRet = mbedtls_mpi_read_binary_le(&ecD, inKey.r, sizeof(inKey.r));
	mbedTLScpp::CheckMbedTlsIntRetVal(
		mbedRet,
		"mbedtls_mpi_read_binary_le",
		"DecentEnclave::Common::Sgx::ExportEcKey"
	);
}


/**
 * \brief  Cipher-based Key Derivation Function (CKDF). Based on the the key
 *         derivation function used in SGX RA.
 *
 * \tparam _cipherType       Type of the cipher.
 * \tparam _reqKeySizeInBits Size of requested key. In bits.
 * \tparam _cipherMode       Mode of the cipher.
 *
 * \param inKey The input key.
 * \param label The label.
 *
 * \return The output key.
 */
template<mbedTLScpp::CipherType _cipherType,
	uint16_t                    _reqKeySizeInBits,
	mbedTLScpp::CipherMode      _cipherMode,
	typename                    _KeyCtnType>
inline mbedTLScpp::SKey<_reqKeySizeInBits>
	Ckdf(
		const mbedTLScpp::ContCtnReadOnlyRef<_KeyCtnType, true>& inKey,
		const std::string& label)
{
	using namespace mbedTLScpp;
	using _Cmacer = Cmacer<_cipherType, _reqKeySizeInBits, _cipherMode>;

	static constexpr std::array<uint8_t, 1> counter{0x01};
	static constexpr std::array<uint8_t, 1> nullTerm{0x00};
	static constexpr std::array<uint16_t, 1> keyBitSize{ _reqKeySizeInBits };

	int mbedRet = 0;
	SKey<_reqKeySizeInBits> cmacKey;
	SKey<_reqKeySizeInBits> deriveKey;

	// mbedTLScpp::Cmacer<_cipherType, _reqKeySizeInBits, _cipherMode>(cmacKey).
	// Calc(deriveKey.m_key, inKey.m_key);
	_Cmacer macer1(CtnFullR(cmacKey));
	mbedRet = mbedtls_cipher_cmac_update(
		macer1.Get(),
		inKey.BeginBytePtr(),
		inKey.GetRegionSize()
	);
	mbedTLScpp::CheckMbedTlsIntRetVal(
		mbedRet,
		"mbedtls_cipher_cmac_update",
		"DecentEnclave::Common::Sgx::Ckdf"
	);
	mbedRet = mbedtls_cipher_cmac_finish(
		macer1.Get(),
		static_cast<unsigned char*>(deriveKey.data())
	);
	mbedTLScpp::CheckMbedTlsIntRetVal(
		mbedRet,
		"mbedtls_cipher_cmac_finish",
		"DecentEnclave::Common::Sgx::Ckdf"
	);


	//CMACer<cType, cSize, cMode>(deriveKey).Calc(outKey.m_key,
	//	counter,     //Counter
	//	label,       //Label
	//	nullTerm,    //Null terminator?
	//	keyBitSize); //Bit length of the output key

	SKey<_reqKeySizeInBits> resKey;

	_Cmacer macer2(CtnFullR(deriveKey));

	// counter
	mbedRet = mbedtls_cipher_cmac_update(
		macer2.Get(),
		counter.data(),
		counter.size()
	);
	mbedTLScpp::CheckMbedTlsIntRetVal(
		mbedRet,
		"mbedtls_cipher_cmac_update",
		"DecentEnclave::Common::Sgx::Ckdf"
	);
	// label
	auto labelRef = CtnFullR(label);
	mbedRet = mbedtls_cipher_cmac_update(
		macer2.Get(),
		labelRef.BeginBytePtr(),
		labelRef.GetRegionSize()
	);
	mbedTLScpp::CheckMbedTlsIntRetVal(
		mbedRet,
		"mbedtls_cipher_cmac_update",
		"DecentEnclave::Common::Sgx::Ckdf"
	);
	// null terminator
	mbedRet = mbedtls_cipher_cmac_update(
		macer2.Get(),
		nullTerm.data(),
		nullTerm.size()
	);
	mbedTLScpp::CheckMbedTlsIntRetVal(
		mbedRet,
		"mbedtls_cipher_cmac_update",
		"DecentEnclave::Common::Sgx::Ckdf"
	);
	// keyBitSize
	auto keyBitSizeRef = CtnFullR(keyBitSize);
	mbedRet = mbedtls_cipher_cmac_update(
		macer2.Get(),
		keyBitSizeRef.BeginBytePtr(),
		keyBitSizeRef.GetRegionSize()
	);
	mbedTLScpp::CheckMbedTlsIntRetVal(
		mbedRet,
		"mbedtls_cipher_cmac_update",
		"DecentEnclave::Common::Sgx::Ckdf"
	);
	// finish
	mbedRet = mbedtls_cipher_cmac_finish(
		macer2.Get(),
		static_cast<unsigned char*>(resKey.data())
	);
	mbedTLScpp::CheckMbedTlsIntRetVal(
		mbedRet,
		"mbedtls_cipher_cmac_finish",
		"DecentEnclave::Common::Sgx::Ckdf"
	);

	return resKey;
}


inline sgx_report_data_t ReportDataFromHash(
	const mbedTLScpp::Hash<mbedTLScpp::HashType::SHA256>& inHash
)
{
	using _HashType = mbedTLScpp::Hash<mbedTLScpp::HashType::SHA256>;
	static constexpr size_t sk_inHashSize = _HashType::sk_size;
	static constexpr size_t sk_rpDataSize = 2 * sk_inHashSize;

	sgx_report_data_t res = { { 0 } };

	auto cpFunc = [](
		uint8_t (&dst)[sk_rpDataSize],
		const std::array<uint8_t, sk_inHashSize>& src
	)
	{
		std::copy(src.begin(), src.end(), std::begin(dst));
	};

	cpFunc(res.d, inHash.m_data);

	return res;
}


} // namespace Sgx
} // namespace Common
} // namespace DecentEnclave

#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED || _UNTRUSTED
