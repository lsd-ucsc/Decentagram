// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


//#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED


#include <type_traits>
#include <utility>
#include <vector>

#include <cstring>

#include "../../Common/Sgx/DevModeDefs.hpp"
#ifdef DECENTENCLAVE_SGX_DEBUG_FLAG
#include "../../Common/Platform/Print.hpp"
#endif // DECENTENCLAVE_SGX_DEBUG_FLAG

#include <sgx_utils.h>
#include <sgx_key.h>

#include <mbedTLScpp/SKey.hpp>
#include <mbedTLScpp/Hash.hpp>
#include <SimpleObjects/Codec/Hex.hpp>

#include "../../Common/Internal/SimpleObj.hpp"
#include "../../Common/Sgx/Exceptions.hpp"
#include "EnclaveIdentity.hpp"


namespace DecentEnclave
{
namespace Trusted
{
namespace Sgx
{


template<size_t _keySizeBits>
struct DecentRootSealKey
{
public: // static members:

	static constexpr size_t sk_keySizeBits = _keySizeBits;
	static constexpr size_t sk_keyIdLen    = 24;
	static constexpr size_t sk_plainKeySizeBits = 128;
	static constexpr size_t sk_plainKeySizeBytes = sk_plainKeySizeBits / 8;
	static constexpr size_t sk_keyGenIterations =
		sk_keySizeBits / sk_plainKeySizeBits;


	static_assert(
		sk_keySizeBits % sk_plainKeySizeBits == 0,
		"Key size must be a multiple of 128 bits."
	);


	using KeyType = mbedTLScpp::SKey<sk_keySizeBits>;


	static std::array<uint8_t, SGX_KEYID_SIZE> BuildKeyId()
	{
		static constexpr char sk_idStr[] = "Decent Root Seal Key - ";
		static_assert(
			sizeof(sk_idStr) < SGX_KEYID_SIZE,
			"Key ID string size is too large"
		);
		static_assert(
			sizeof(sk_idStr) == sk_keyIdLen,
			"Key ID string size is not correct"
		);

		std::array<uint8_t, SGX_KEYID_SIZE> id;
		std::copy(std::begin(sk_idStr), std::end(sk_idStr), std::begin(id));

		return id;
	}

public:

	DecentRootSealKey() :
		m_keyReq(GenKeyRequest())
	{}

	DecentRootSealKey(const std::vector<uint8_t>& keyMeta) :
		m_keyReq(MetaToKeyReq(keyMeta))
	{}

	~DecentRootSealKey() = default;

	KeyType DeriveKey() const
	{
		KeyType key;

		sgx_key_request_t tmpKeyReq = m_keyReq;
		DeriveKey(key, tmpKeyReq);

		return key;
	}

	std::vector<uint8_t> GetKeyMeta() const
	{
		return KeyReqToMeta(m_keyReq);
	}

private: // static members:

	static void DeriveKey(KeyType& key, sgx_key_request_t& keyReq)
	{
		using _SgxKeyType = uint8_t[sk_plainKeySizeBytes];
		using _SgxKeyPtrType = typename std::add_pointer<_SgxKeyType>::type;

		keyReq.key_name = SGX_KEYSELECT_SEAL;
		keyReq.key_policy = SGX_KEYPOLICY_MRENCLAVE;

		for (size_t i = 0; i < sk_keyGenIterations; ++i)
		{
			keyReq.key_id.id[sk_keyIdLen] = static_cast<uint8_t>(i);

			sgx_status_t sgxRet = sgx_get_key(
				&keyReq,
				reinterpret_cast<_SgxKeyPtrType>(
					key.data() +
					(i * sk_plainKeySizeBytes)
				)
			);
			DECENTENCLAVE_CHECK_SGX_RUNTIME_ERROR(
				sgxRet,
				sgx_get_key
			);
		}
	}

	static sgx_key_request_t GenKeyRequest()
	{
		sgx_key_request_t keyReq;
		std::memset(&keyReq, 0, sizeof(sgx_key_request_t));


		keyReq.key_name = SGX_KEYSELECT_SEAL;
		keyReq.key_policy = SGX_KEYPOLICY_MRENCLAVE;


		sgx_report_t selfRep = EnclaveIdentity::GenSelfReport();

		keyReq.isv_svn = selfRep.body.isv_svn;
		keyReq.cpu_svn = selfRep.body.cpu_svn;

		keyReq.attribute_mask.flags =
			SGX_FLAGS_INITTED |
			SGX_FLAGS_DEBUG |
			SGX_FLAGS_MODE64BIT |
			SGX_FLAGS_PROVISION_KEY |
			SGX_FLAGS_EINITTOKEN_KEY |
			SGX_FLAGS_KSS;
		keyReq.attribute_mask.xfrm =
			SGX_XFRM_LEGACY |
			SGX_XFRM_AVX;

		auto id = BuildKeyId();
		std::copy(std::begin(id), std::end(id), std::begin(keyReq.key_id.id));

		keyReq.config_svn = selfRep.body.config_svn;

		return keyReq;
	}

	static std::vector<uint8_t> KeyReqToMeta(const sgx_key_request_t& keyReq)
	{
		std::vector<uint8_t> ret(sizeof(sgx_key_request_t));
		std::memcpy(ret.data(), &keyReq, sizeof(sgx_key_request_t));
		return ret;
	}

	static sgx_key_request_t MetaToKeyReq(const std::vector<uint8_t>& meta)
	{
		if (meta.size() != sizeof(sgx_key_request_t))
		{
			throw Common::Exception(
				"Invalid meta data size for DecentRootSealKey"
			);
		}

		sgx_key_request_t ret;
		std::memcpy(&ret, meta.data(), sizeof(sgx_key_request_t));
		return ret;
	}

private:

	sgx_key_request_t m_keyReq;


}; // struct DecentRootSealKey


struct PlatformId
{

	static constexpr size_t sk_idSizeBits = 256;
	static constexpr size_t sk_idSizeBytes = sk_idSizeBits / 8;
	static constexpr size_t sk_plainKeySizeBits = 128;
	static constexpr size_t sk_plainKeySizeBytes = sk_plainKeySizeBits / 8;

	static const std::array<uint8_t, sk_idSizeBytes>& GetId()
	{
		static std::array<uint8_t, sk_idSizeBytes> id = GenId();
		return id;
	}

private:

	static constexpr uint16_t sk_allowedProdId = 0;

	static void CheckEnvironment()
	{
		sgx_report_t selfRep = EnclaveIdentity::GenSelfReport();

		if (selfRep.body.isv_prod_id != sk_allowedProdId)
		{
			throw Common::Exception(
				"ProdID configured does not meet the requirement "
				"to generate platform ID"
			);
		}
	}

	static std::array<uint8_t, sk_idSizeBytes> GenId()
	{
		using _SgxKeyType = uint8_t[sk_plainKeySizeBytes];
		using _SgxKeyPtrType = typename std::add_pointer<_SgxKeyType>::type;

		CheckEnvironment();

		sgx_key_request_t keyReq;
		std::memset(&keyReq, 0, sizeof(sgx_key_request_t));

		keyReq.key_name = SGX_KEYSELECT_SEAL;
		keyReq.key_policy = 0;

		mbedTLScpp::SKey<sk_plainKeySizeBits> key;

		sgx_status_t sgxRet = sgx_get_key(
			&keyReq,
			reinterpret_cast<_SgxKeyPtrType>(key.data())
		);
		DECENTENCLAVE_CHECK_SGX_RUNTIME_ERROR(
			sgxRet,
			sgx_get_key
		);

#ifdef DECENTENCLAVE_SGX_DEBUG_FLAG
		std::string keyHex = Common::Internal::Obj::Codec::HEX::
			template Encode<std::string>(key);
		Common::Platform::Print::StrDebug(
			"Key generated for PlatformId  : " + keyHex
		);
#endif // DECENTENCLAVE_SGX_DEBUG_FLAG

		mbedTLScpp::Hasher<mbedTLScpp::HashType::SHA256> hasher;
		auto hash = hasher.Calc(mbedTLScpp::CtnFullR(key));

		return hash.m_data;
	}


}; // struct PlatformId


} // namespace Sgx
} // namespace Trusted
} // namespace DecentEnclave

//#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
