// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <array>
#include <utility>
#include <vector>

#include <mbedTLScpp/Container.hpp>
#include <mbedTLScpp/SecretVector.hpp>
#include <mbedTLScpp/SKey.hpp>

#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
#include <sgx_tcrypto.h>
#include "../Sgx/Exceptions.hpp"
#else
#include <mbedTLScpp/Gcm.hpp>
#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED


namespace DecentEnclave
{
namespace Common
{
namespace Platform
{

template<size_t _keyBitSize>
class AesGcmOneGoNative;

#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED

template<>
class AesGcmOneGoNative<128>
{
public: // static members:

	static constexpr size_t sk_keyBitSize = 128;
	static constexpr size_t sk_keyByteSize = sk_keyBitSize / 8;

	using KeyType = mbedTLScpp::SKey<sk_keyBitSize>;

public:

	AesGcmOneGoNative(KeyType key) :
		m_key(std::move(key))
	{}

	AesGcmOneGoNative(const AesGcmOneGoNative& other) :
		m_key(other.m_key)
	{}

	AesGcmOneGoNative(AesGcmOneGoNative&& other) :
		m_key(std::move(other.m_key))
	{}

	~AesGcmOneGoNative() = default;

	template<
		typename _IvCtnType,   bool _IvCtnSecrecy,
		typename _AadCtnType,  bool _AadCtnSecrecy,
		typename _DataCtnType, bool _DataCtnSecrecy
	>
	std::pair<
		std::vector<uint8_t>,
		std::array<uint8_t, 16>
	>
	Encrypt(
		const mbedTLScpp::ContCtnReadOnlyRef<_IvCtnType,   _IvCtnSecrecy>& iv,
		const mbedTLScpp::ContCtnReadOnlyRef<_AadCtnType,  _AadCtnSecrecy>& aad,
		const mbedTLScpp::ContCtnReadOnlyRef<_DataCtnType, _DataCtnSecrecy>& data
	)
	{
		std::vector<uint8_t> res(data.GetRegionSize());
		std::array<uint8_t, 16> tag;

		const sgx_aes_gcm_128bit_key_t* keyPtr =
			reinterpret_cast<const sgx_aes_gcm_128bit_key_t*>(m_key.data());
		sgx_aes_gcm_128bit_tag_t* tagPtr =
			reinterpret_cast<sgx_aes_gcm_128bit_tag_t*>(tag.data());

		sgx_status_t sgxRet = sgx_rijndael128GCM_encrypt(
			keyPtr,
			data.BeginBytePtr(),
			static_cast<uint32_t>(data.GetRegionSize()),
			res.data(),
			iv.BeginBytePtr(),
			static_cast<uint32_t>(iv.GetRegionSize()),
			aad.BeginBytePtr(),
			static_cast<uint32_t>(aad.GetRegionSize()),
			tagPtr
		);
		DECENTENCLAVE_CHECK_SGX_RUNTIME_ERROR(
			sgxRet,
			sgx_rijndael128GCM_encrypt
		);

		return std::make_pair(std::move(res), std::move(tag));
	}

	template<
		typename _IvCtnType,   bool _IvCtnSecrecy,
		typename _AadCtnType,  bool _AadCtnSecrecy,
		typename _DataCtnType, bool _DataCtnSecrecy,
		typename _TagCtnType,  bool _TagCtnSecrecy
	>
	mbedTLScpp::SecretVector<uint8_t>
	Decrypt(
		const mbedTLScpp::ContCtnReadOnlyRef<_IvCtnType,   _IvCtnSecrecy>& iv,
		const mbedTLScpp::ContCtnReadOnlyRef<_AadCtnType,  _AadCtnSecrecy>& aad,
		const mbedTLScpp::ContCtnReadOnlyRef<_DataCtnType, _DataCtnSecrecy>& data,
		const mbedTLScpp::ContCtnReadOnlyRef<_TagCtnType, _TagCtnSecrecy>& tag
	)
	{
		mbedTLScpp::SecretVector<uint8_t> res(data.GetRegionSize());

		const sgx_aes_gcm_128bit_key_t* keyPtr =
			reinterpret_cast<const sgx_aes_gcm_128bit_key_t*>(m_key.data());
		const sgx_aes_gcm_128bit_tag_t* tagPtr =
			reinterpret_cast<const sgx_aes_gcm_128bit_tag_t*>(tag.BeginBytePtr());

		sgx_status_t sgxRet = sgx_rijndael128GCM_decrypt(
			keyPtr,
			data.BeginBytePtr(),
			static_cast<uint32_t>(data.GetRegionSize()),
			res.data(),
			iv.BeginBytePtr(),
			static_cast<uint32_t>(iv.GetRegionSize()),
			aad.BeginBytePtr(),
			static_cast<uint32_t>(aad.GetRegionSize()),
			tagPtr
		);
		DECENTENCLAVE_CHECK_SGX_RUNTIME_ERROR(
			sgxRet,
			sgx_rijndael128GCM_decrypt
		);

		return res;
	}

private:

	KeyType m_key;

}; // class AesGcmOneGoNative<128>

#else //#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED

template<size_t _keyBitSize>
class AesGcmOneGoNative
{
public: // static members:

	static constexpr size_t sk_keyBitSize = _keyBitSize;
	static constexpr size_t sk_keyByteSize = sk_keyBitSize / 8;

	using KeyType = mbedTLScpp::SKey<sk_keyBitSize>;

	using GcmCryptorType =
		mbedTLScpp::Gcm<mbedTLScpp::CipherType::AES, sk_keyBitSize>;

public:

	AesGcmOneGoNative(KeyType key) :
		m_cryptor(mbedTLScpp::CtnFullR(key))
	{}

	AesGcmOneGoNative(const AesGcmOneGoNative& other) :
		m_cryptor(other.m_cryptor)
	{}

	AesGcmOneGoNative(AesGcmOneGoNative&& other) :
		m_cryptor(std::move(other.m_cryptor))
	{}

	~AesGcmOneGoNative() = default;

	template<
		typename _IvCtnType,   bool _IvCtnSecrecy,
		typename _AadCtnType,  bool _AadCtnSecrecy,
		typename _DataCtnType, bool _DataCtnSecrecy
	>
	std::pair<
		std::vector<uint8_t>,
		std::array<uint8_t, 16>
	>
	Encrypt(
		const mbedTLScpp::ContCtnReadOnlyRef<_IvCtnType,   _IvCtnSecrecy>& iv,
		const mbedTLScpp::ContCtnReadOnlyRef<_AadCtnType,  _AadCtnSecrecy>& aad,
		const mbedTLScpp::ContCtnReadOnlyRef<_DataCtnType, _DataCtnSecrecy>& data
	)
	{
		return m_cryptor.Encrypt(
			data,
			iv,
			aad
		);
	}

	template<
		typename _IvCtnType,   bool _IvCtnSecrecy,
		typename _AadCtnType,  bool _AadCtnSecrecy,
		typename _DataCtnType, bool _DataCtnSecrecy,
		typename _TagCtnType,  bool _TagCtnSecrecy
	>
	mbedTLScpp::SecretVector<uint8_t>
	Decrypt(
		const mbedTLScpp::ContCtnReadOnlyRef<_IvCtnType,   _IvCtnSecrecy>& iv,
		const mbedTLScpp::ContCtnReadOnlyRef<_AadCtnType,  _AadCtnSecrecy>& aad,
		const mbedTLScpp::ContCtnReadOnlyRef<_DataCtnType, _DataCtnSecrecy>& data,
		const mbedTLScpp::ContCtnReadOnlyRef<_TagCtnType, _TagCtnSecrecy>& tag
	)
	{
		return m_cryptor.Decrypt(
			data,
			iv,
			aad,
			tag
		);
	}

private:

	GcmCryptorType m_cryptor;

}; // class AesGcmOneGoNative

#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED


} // namespace Platform
} // namespace Common
} // namespace DecentEnclave
