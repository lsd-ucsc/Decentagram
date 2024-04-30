// Copyright (c) 2022 mbedTLScpp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include "ObjectBase.hpp"

#include <mbedtls/pk.h>
#include <mbedtls/x509_crt.h>

#include "Common.hpp"
#include "Container.hpp"
#include "Exceptions.hpp"
#include "Hash.hpp"
#include "RandInterfaces.hpp"

#include "Internal/PKeyHelper.hpp"
#include "Internal/PemHelper.hpp"


#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{

/**
 * @brief Public Key object allocator.
 *
 */
struct PKeyObjAllocator : DefaultAllocBase
{
	typedef mbedtls_pk_context      CObjType;

	using DefaultAllocBase::NewObject;
	using DefaultAllocBase::DelObject;

	static void Init(CObjType* ptr)
	{
		return mbedtls_pk_init(ptr);
	}

	static void Free(CObjType* ptr) noexcept
	{
		return mbedtls_pk_free(ptr);
	}
}; // struct PKeyObjAllocator


/**
 * @brief Public Key object trait.
 *
 */
using DefaultPKeyObjTrait = ObjTraitBase<PKeyObjAllocator,
											false,
											false>;


/**
 * @brief Borrower Public Key object trait.
 *
 */
using BorrowedPKeyTrait = ObjTraitBase<BorrowAllocBase<mbedtls_pk_context>,
								true,
								false>;


template<
	typename _PKeyObjTrait = DefaultPKeyObjTrait,
	enable_if_t<
		std::is_same<
			typename _PKeyObjTrait::CObjType,
			mbedtls_pk_context
		>::value,
		int
	> = 0
>
class PKeyBase : public ObjectBase<_PKeyObjTrait>
{
public: // static member:

	using PKObjTrait = _PKeyObjTrait;
	using _Base      = ObjectBase<PKObjTrait>;

	using _PKeyOwnerType = PKeyBase<DefaultPKeyObjTrait>;


	friend class X509ReqWriter;
	friend class X509CertWriter;
	friend class TlsConfig;

	template<
		typename T,
		enable_if_t<
			std::is_same<typename T::CObjType, mbedtls_x509_crt>::value,
			int
		>
	>
	friend class X509CertBase;


	/**
	 * @brief Construct a PKeyBase object (public part) from a given PEM string.
	 *
	 * @param pem PEM string in std::string
	 */
	static _PKeyOwnerType FromPEM(const std::string& pem)
	{
		_PKeyOwnerType res;
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			PKeyBase::FromPEM,
			mbedtls_pk_parse_public_key,
			res.Get(),
			reinterpret_cast<const unsigned char*>(pem.c_str()),
			pem.size() + 1
		);
		return res;
	}


	/**
	 * @brief Construct a PKeyBase object from a given PEM string.
	 *
	 * @param pem PEM string in std::string
	 */
	static _PKeyOwnerType FromPEM(
		const SecretString& pem,
		RbgInterface& rand
	)
	{
		_PKeyOwnerType res;
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			PKeyBase::FromPEM,
			mbedtls_pk_parse_key,
			res.Get(),
			reinterpret_cast<const unsigned char*>(pem.c_str()),
			pem.size() + 1,
			nullptr,
			0,
			RbgInterface::CallBack,
			&rand
		);
		return res;
	}


	/**
	 * @brief Construct a PKeyBase object (public part) from a given DER bytes.
	 *
	 * @param der DER bytes referenced by ContCtnReadOnlyRef
	 */
	template<typename _SecCtnType>
	static _PKeyOwnerType FromDER(
		const ContCtnReadOnlyRef<_SecCtnType, false>& der
	)
	{
		_PKeyOwnerType res;
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			PKeyBase::FromPEM,
			mbedtls_pk_parse_public_key,
			res.Get(),
			static_cast<const unsigned char*>(der.BeginPtr()),
			der.GetRegionSize()
		);
		return res;
	}


	/**
	 * @brief Construct a PKeyBase object from a given DER bytes.
	 *
	 * @param der DER bytes referenced by ContCtnReadOnlyRef
	 */
	template<typename _SecCtnType>
	static _PKeyOwnerType FromDER(
		const ContCtnReadOnlyRef<_SecCtnType, true>& der,
		RbgInterface& rand
	)
	{
		_PKeyOwnerType res;
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			PKeyBase::FromPEM,
			mbedtls_pk_parse_key,
			res.Get(),
			static_cast<const unsigned char*>(der.BeginPtr()),
			der.GetRegionSize(),
			nullptr,
			0,
			RbgInterface::CallBack,
			&rand
		);
		return res;
	}


public:

	/**
	 * @brief Construct a new \em empty PKeyBase object
	 *
	 * @tparam _dummy_PKTrait A dummy template parameter used to make sure
	 *                        the constructor is not available for borrowers.
	 */
	template<
		typename _dummy_PKTrait = PKObjTrait,
		enable_if_t<!_dummy_PKTrait::sk_isBorrower, int> = 0
	>
	PKeyBase() :
		_Base::ObjectBase()
	{}

	/**
	 * @brief Construct a new PKeyBase object that borrows the C object.
	 *
	 * @tparam _dummy_PKTrait A dummy template parameter used to make sure
	 *                        the constructor is only available for borrowers.
	 * @param ptr pointer to the borrowed C object.
	 */
	template<typename _dummy_ObjTrait = PKObjTrait,
		enable_if_t<_dummy_ObjTrait::sk_isBorrower, int> = 0>
	PKeyBase(mbedtls_pk_context* ptr) noexcept :
		_Base::ObjectBase(ptr)
	{}

	/**
	 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
	 *
	 * @exception None No exception thrown
	 * @param rhs The other PKeyBase instance.
	 */
	PKeyBase(PKeyBase&& rhs) noexcept :
		_Base::ObjectBase(std::forward<_Base>(rhs)) //noexcept
	{}

	PKeyBase(const PKeyBase& rhs) = delete;

	// LCOV_EXCL_START
	virtual ~PKeyBase() = default;
	// LCOV_EXCL_STOP

	/**
	 * @brief Move assignment. The `rhs` will be empty/null afterwards.
	 *
	 * @exception None No exception thrown
	 * @param rhs The other PKeyBase instance.
	 * @return PKeyBase& A reference to this instance.
	 */
	PKeyBase& operator=(PKeyBase&& rhs) noexcept
	{
		_Base::operator=(std::forward<_Base>(rhs)); //noexcept

		return *this;
	}

	PKeyBase& operator=(const PKeyBase& other) = delete;

	/**
	 * @brief Check if the current instance is holding a null pointer for
	 *        the mbedTLS object. If so, exception will be thrown. Helper
	 *        function to be called before accessing the mbedTLS object.
	 *
	 * @exception InvalidObjectException Thrown when the current instance is
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 */
	virtual void NullCheck() const
	{
		_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(PKeyBase));
	}

	using _Base::NullCheck;
	using _Base::Get;
	using _Base::NonVirtualGet;
	using _Base::Swap;

	/**
	 * @brief	Gets PKey algorithm categories. For now, it could be EC, or RSA.
	 *
	 * @return	The enum value represents the PKey algorithm category.
	 */
	virtual PKeyAlgmCat GetAlgorithmCat() const
	{
		NullCheck();
		return GetAlgmCat(*Get());
	}

	/**
	 * @brief	Gets PKey type (either public or private).
	 *
	 * @return	The asymmetric key type.
	 */
	virtual PKeyType GetKeyType() const
	{
		NullCheck();
		return Internal::GetKeyType(*Get());
	}

	/**
	 * @brief Check if this PKey context has public part.
	 *
	 * @return true if it has, otherwise, false.
	 */
	virtual bool HasPubKey() const
	{
		NullCheck();
		return Internal::CallFuncBasedOnAlgCat<bool>(
			*Get(),
			Internal::HasPubKeyImpl()
		);
	}

	std::vector<uint8_t> GetPublicDer() const
	{
		size_t bufSize = EstPublicDerSize();

		std::vector<uint8_t> der(bufSize);

		int len = mbedtls_pk_write_pubkey_der(
			Get(),
			der.data(),
			der.size()
		);
		CheckMbedTlsLenRetVal(
			len,
			"PKeyBase::GetPublicDer",
			"mbedtls_pk_write_pubkey_der"
		);

		der.erase(der.begin(), der.begin() + (der.size() - len));

		return der;
	}

	SecretVector<uint8_t> GetPrivateDer() const
	{
		size_t bufSize = EstPrivateDerSize();

		SecretVector<uint8_t> der(bufSize);

		int len = mbedtls_pk_write_key_der(
			Get(),
			der.data(),
			der.size()
		);
		CheckMbedTlsLenRetVal(
			len,
			"PKeyBase::GetPrivateDer",
			"mbedtls_pk_write_key_der"
		);

		der.erase(der.begin(), der.begin() + (der.size() - len));

		return der;
	}

	std::string GetPublicPem() const
	{
		std::vector<uint8_t> der = GetPublicDer();

		return Internal::DerToPem<std::string>(
			CtnFullR(der),
			Internal::GetPemHeaderPubKey(),
			Internal::GetPemFooterPubKey()
		);
	}

	SecretString GetPrivatePem() const
	{
		SecretVector<uint8_t> der = GetPrivateDer();

#if defined(MBEDTLS_RSA_C)
		if (mbedtls_pk_get_type(Get()) == MBEDTLS_PK_RSA)
		{
			return Internal::DerToPem<SecretString>(
				CtnFullR(der),
				Internal::GetPemHeaderRsaPrivKey(),
				Internal::GetPemFooterRsaPrivKey()
			);
		}
		else
#endif
#if defined(MBEDTLS_ECP_C)
		if (mbedtls_pk_get_type(Get()) == MBEDTLS_PK_ECKEY)
		{
			return Internal::DerToPem<SecretString>(
				CtnFullR(der),
				Internal::GetPemHeaderEcPrivKey(),
				Internal::GetPemFooterEcPrivKey()
			);
		}
		else
#endif
		{
			throw InvalidArgumentException(
				"PKeyBase::GetPrivatePem - Invalid PKey type is given."
			);
		}
	}


	template<HashType _HashTypeVal>
	std::vector<uint8_t> SignInDer(
		const Hash<_HashTypeVal>& hash,
		RbgInterface& rand
	) const
	{
		size_t bufSize = EstDerSignSize(_HashTypeVal);

		std::vector<uint8_t> der(bufSize);

		size_t olen = 0;
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			PKeyBase::SignInDer,
			mbedtls_pk_sign,
			_Base::MutableGet(),
			GetMbedTlsMdType(_HashTypeVal),
			hash.data(),
			hash.size(),
			der.data(),
			der.size(),
			&olen,
			&RbgInterface::CallBack,
			&rand
		);

		der.resize(olen);

		return der;
	}


	template<
		HashType _HashTypeVal,
		typename _SignCtnType,
		bool _SignCtnSecrecy
	>
	void VerifyDerSign(
		const Hash<_HashTypeVal>& hash,
		const ContCtnReadOnlyRef<_SignCtnType, _SignCtnSecrecy>& sign
	) const
	{
		NullCheck();

		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			PKeyBase::VerifyDerSign,
			mbedtls_pk_verify,
			_Base::MutableGet(),
			GetMbedTlsMdType(_HashTypeVal),
			hash.data(),
			hash.size(),
			sign.BeginBytePtr(),
			sign.GetRegionSize()
		);
	}

protected:

	virtual size_t EstPublicDerSize() const
	{
		NullCheck();
		return Internal::pk_write_pubkey_der_est_size(*Get());
	}

	virtual size_t EstPrivateDerSize() const
	{
		NullCheck();
		return Internal::pk_write_prvkey_der_est_size(*Get());
	}

	virtual size_t EstDerSignSize(HashType hashType) const
	{
		NullCheck();
		return Internal::pk_write_sign_der_est_size(
			*Get(),
			GetHashByteSize(hashType)
		);
	}

}; // class PKeyBase

} // namespace mbedTLScpp
