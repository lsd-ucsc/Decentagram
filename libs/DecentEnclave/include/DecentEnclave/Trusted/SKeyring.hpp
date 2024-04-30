// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <atomic>
#include <string>
#include <unordered_map>
#include <vector>

#include <mbedTLScpp/Hash.hpp>
#include <mbedTLScpp/Hkdf.hpp>
#include <mbedTLScpp/SecretVector.hpp>
#include <mbedTLScpp/SKey.hpp>
#include <SimpleObjects/Codec/Hex.hpp>

#include "../Common/Exceptions.hpp"

#ifdef DECENTENCLAVE_SGX_DEBUG_FLAG
#include "../Common/Internal/SimpleObj.hpp"
#include "../Common/Platform/Print.hpp"
#endif // DECENTENCLAVE_SGX_DEBUG_FLAG


#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
#include "Sgx/SealKey.hpp"

namespace DecentEnclave
{
namespace Trusted
{

using DecentRootSealKeyGenerator = Sgx::DecentRootSealKey<512>;

} // namespace Trusted
} // namespace DecentEnclave

#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED

namespace DecentEnclave
{
namespace Trusted
{


class SKeyring
{
public: // static members:

	using RootKeyGenerator = DecentRootSealKeyGenerator;
	using RootKeyType = typename RootKeyGenerator::KeyType;
	using ChildKeyType = mbedTLScpp::SecretVector<uint8_t>;
	using AuthIDsHashType = mbedTLScpp::Hash<mbedTLScpp::HashType::SHA256>;

	template<size_t _keyBitSize>
	using ChildSKeyType = mbedTLScpp::SKey<_keyBitSize>;


	/**
	 * @brief Get the singleton instance of SKeyring
	 *        There can only be one instance of SKeyring
	 *
	 * @return SKeyring&
	 */
	static SKeyring& GetMutableInstance(
		const std::vector<uint8_t>& rootKeyMeta = std::vector<uint8_t>(),
		const AuthIDsHashType& authIDsHash = AuthIDsHashType()
	)
	{
		static SKeyring s_inst(rootKeyMeta, authIDsHash);
		return s_inst;
	}


	static const SKeyring& GetInstance()
	{
		return GetMutableInstance();
	}


public:

	SKeyring(
		const std::vector<uint8_t>& rootKeyMeta,
		const AuthIDsHashType& authIDsHash
	) :
		SKeyring(
			rootKeyMeta.empty() ?
				RootKeyGenerator() :
				RootKeyGenerator(rootKeyMeta),
			authIDsHash
		)
	{}

	SKeyring(
		const RootKeyGenerator& rootKeyGen,
		const AuthIDsHashType& authIDsHash
	) :
		m_isLocked(false),
		m_rootKeyMeta(rootKeyGen.GetKeyMeta()),
		m_rootKey(rootKeyGen.DeriveKey()),
		m_authIDsHash(authIDsHash),
		m_keyMap()
	{
#ifdef DECENTENCLAVE_SGX_DEBUG_FLAG
		std::string keyHex = Common::Internal::Obj::Codec::HEX::
			template Encode<std::string>(m_rootKey);
		Common::Platform::Print::StrDebug(
			"Decent root seal key          : " + keyHex
		);
#endif // DECENTENCLAVE_SGX_DEBUG_FLAG
	}

	SKeyring& RegisterKey(const std::string& keyName, size_t keySize)
	{
		AssertUnlocked("register key");

		if (m_keyMap.find(keyName) != m_keyMap.end())
		{
			throw Common::Exception("Key name already exists.");
		}

		m_keyMap.emplace(
			std::string(keyName),
			mbedTLScpp::Hkdf<mbedTLScpp::HashType::SHA256>(
				keySize,
				mbedTLScpp::CtnFullR(m_rootKey),
				mbedTLScpp::CtnFullR(keyName),
				mbedTLScpp::CtnFullR(m_authIDsHash)
			)
		);

		return *this;
	}

	const ChildKeyType& GetKey(const std::string& keyName) const
	{
		AssertLocked("getting key");

		auto iter = m_keyMap.find(keyName);
		if (iter == m_keyMap.end())
		{
			throw Common::Exception(
				"The skey named " + keyName + " is not found"
			);
		}

		return iter->second;
	}

	template<size_t _keyBitSize>
	ChildSKeyType<_keyBitSize> GetSKey(const std::string& keyName) const
	{
		static constexpr size_t keySize = _keyBitSize / 8;

		const auto& key = GetKey(keyName);
		if (key.size() < keySize)
		{
			throw Common::Exception("source key size is too small.");
		}

		ChildSKeyType<_keyBitSize> res;
		// read first keySize bytes from key to res
		std::copy(key.begin(), key.begin() + keySize, res.begin());

		return res;
	}

	void Lock()
	{
		m_isLocked = true;
	}

	std::vector<uint8_t> GetKeyHashList() const
	{
		AssertLocked("getting key hash list");

		std::vector<uint8_t> ret;
		ret.reserve(m_keyMap.size() * AuthIDsHashType::sk_size);

		for (const auto& key : m_keyMap)
		{
			mbedTLScpp::Hasher<mbedTLScpp::HashType::SHA256> hasher;
			auto hash = hasher.Calc(mbedTLScpp::CtnFullR(key.second));
			ret.insert(ret.end(), hash.m_data.begin(), hash.m_data.end());
		}

		return ret;
	}

private:

	void AssertLocked(const std::string& op) const
	{
		if (!m_isLocked)
		{
			throw Common::Exception("SKeyring must be locked before " + op);
		}
	}

	void AssertUnlocked(const std::string& op) const
	{
		if (m_isLocked)
		{
			throw Common::Exception(
				"SKeyring must remain unlocked in order to " + op
			);
		}
	}

	std::atomic_bool m_isLocked;
	std::vector<uint8_t> m_rootKeyMeta;
	RootKeyType m_rootKey;
	AuthIDsHashType m_authIDsHash;
	std::unordered_map<std::string, ChildKeyType> m_keyMap;

};// class SKeyring


} // namespace Trusted
} // namespace DecentEnclave
