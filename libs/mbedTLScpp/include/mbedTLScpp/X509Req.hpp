// Copyright (c) 2022 mbedTLScpp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#pragma once


#include "ObjectBase.hpp"

#include <mbedtls/x509_csr.h>
#include <mbedtls/pem.h>

#include "Common.hpp"
#include "Container.hpp"
#include "Exceptions.hpp"
#include "Hash.hpp"
#include "PKey.hpp"
#include "RandInterfaces.hpp"

#include "Internal/X509Helper.hpp"
#include "Internal/PemHelper.hpp"


#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{


/**
 * @brief X509 certificate request writer object allocator.
 *
 */
struct X509ReqWtrObjAllocator : DefaultAllocBase
{
	typedef mbedtls_x509write_csr      CObjType;

	using DefaultAllocBase::NewObject;
	using DefaultAllocBase::DelObject;

	static void Init(CObjType* ptr)
	{
		return mbedtls_x509write_csr_init(ptr);
	}

	static void Free(CObjType* ptr) noexcept
	{
		return mbedtls_x509write_csr_free(ptr);
	}
}; // struct X509ReqWtrObjAllocator


/**
 * @brief X509 certificate request writer object trait.
 *
 */
using DefaultX509ReqWtrObjTrait = ObjTraitBase<X509ReqWtrObjAllocator,
											false,
											false>;


class X509ReqWriter : public ObjectBase<DefaultX509ReqWtrObjTrait>
{
public: // Static members:

	using X509ReqWtrTrait = DefaultX509ReqWtrObjTrait;
	using _Base           = ObjectBase<X509ReqWtrTrait>;

public:


	template<typename _PKObjTrait>
	X509ReqWriter(
		HashType hashType,
		const PKeyBase<_PKObjTrait> & keyPair,
		const std::string& subjName
	) :
		_Base::ObjectBase()
	{
		keyPair.NullCheck();

		mbedtls_x509write_csr_set_key(
			NonVirtualGet(),
			keyPair.MutableGet()
		);
		mbedtls_x509write_csr_set_md_alg(
			NonVirtualGet(),
			GetMbedTlsMdType(hashType)
		);

		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			X509ReqWriter::X509ReqWriter,
			mbedtls_x509write_csr_set_subject_name,
			NonVirtualGet(),
			subjName.c_str()
		);
	}


	/**
	 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
	 *
	 * @exception None No exception thrown
	 * @param rhs The other PKeyBase instance.
	 */
	X509ReqWriter(X509ReqWriter&& rhs) noexcept :
		_Base::ObjectBase(std::forward<_Base>(rhs)) //noexcept
	{}


	X509ReqWriter(const X509ReqWriter& rhs) = delete;


	// LCOV_EXCL_START
	virtual ~X509ReqWriter() = default;
	// LCOV_EXCL_STOP


	/**
	 * @brief Move assignment. The `rhs` will be empty/null afterwards.
	 *
	 * @exception None No exception thrown
	 * @param rhs The other X509ReqWriter instance.
	 * @return X509ReqWriter& A reference to this instance.
	 */
	X509ReqWriter& operator=(X509ReqWriter&& rhs) noexcept
	{
		_Base::operator=(std::forward<_Base>(rhs)); //noexcept

		return *this;
	}


	X509ReqWriter& operator=(const X509ReqWriter& other) = delete;


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
		_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(X509ReqWriter));
	}

	using _Base::NullCheck;
	using _Base::Get;
	using _Base::NonVirtualGet;
	using _Base::Swap;


	/**
	 * @brief Generates a DER encoded X509 request.
	 *
	 * @param rand The Random Bit Generator.
	 *
	 * @return The DER encoded X509 request.
	 */
	std::vector<uint8_t> GetDer(RbgInterface& rand)
	{
		NullCheck();

		size_t bufSize = Internal::x509write_csr_der_est_size(*Get());
		std::vector<uint8_t> der(bufSize);

		int len = mbedtls_x509write_csr_der(
			Get(),
			der.data(),
			der.size(),
			&RbgInterface::CallBack,
			&rand
		);
		CheckMbedTlsLenRetVal(
			len,
			"X509ReqWriter::GetDer",
			"mbedtls_x509write_csr_der"
		);

		der.erase(der.begin(), der.begin() + (der.size() - len));

		return der;
	}


	/**
	 * @brief Generates a PEM encoded X509 request.
	 *
	 * @param rand The Random Bit Generator.
	 *
	 * @return The PEM encoded X509 request.
	 */
	std::string GetPem(RbgInterface& rand)
	{
		std::vector<uint8_t> der = GetDer(rand);

		return Internal::DerToPem<std::string>(
			CtnFullR(der),
			Internal::GetPemHeaderCSR(),
			Internal::GetPemFooterCSR()
		);
	}

}; // class X509ReqWriter

static_assert(
	IsCppObjOfCtype<X509ReqWriter, mbedtls_x509write_csr>::value == true,
	"Implementation Error"
);





/**
 * @brief X509 certificate request object allocator.
 *
 */
struct X509ReqObjAllocator : DefaultAllocBase
{
	typedef mbedtls_x509_csr      CObjType;

	using DefaultAllocBase::NewObject;
	using DefaultAllocBase::DelObject;

	static void Init(CObjType* ptr)
	{
		return mbedtls_x509_csr_init(ptr);
	}

	static void Free(CObjType* ptr) noexcept
	{
		return mbedtls_x509_csr_free(ptr);
	}
}; // struct X509ReqObjAllocator


/**
 * @brief X509 certificate request object trait.
 *
 */
using DefaultX509ReqObjTrait = ObjTraitBase<X509ReqObjAllocator,
											false,
											false>;


class X509Req : public ObjectBase<DefaultX509ReqObjTrait>
{
public: // Static members:

	using X509ReqTrait = DefaultX509ReqObjTrait;
	using _Base        = ObjectBase<X509ReqTrait>;

	/**
	 * @brief Construct a X509 certificate request from a given PEM string.
	 *
	 * @param pem PEM string in std::string
	 */
	static X509Req FromPEM(const std::string& pem)
	{
		X509Req req;
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			X509Req::FromPEM,
			mbedtls_x509_csr_parse,
			req.Get(),
			reinterpret_cast<const uint8_t*>(pem.c_str()),
			pem.size() + 1
		);
		return req;
	}

	/**
	 * @brief Construct a X509 certificate request from a given DER bytes.
	 *
	 * @param der DER bytes referenced by ContCtnReadOnlyRef
	 */
	template<typename _SecCtnType>
	static X509Req FromDER(const ContCtnReadOnlyRef<_SecCtnType, false>& der)
	{
		X509Req req;
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			X509Req::FromDER,
			mbedtls_x509_csr_parse,
			req.Get(),
			der.BeginBytePtr(),
			der.GetRegionSize()
		);
		return req;
	}

public:

	/**
	 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
	 *
	 * @exception None No exception thrown
	 * @param rhs The other X509Req instance.
	 */
	X509Req(X509Req&& rhs) noexcept :
		_Base::ObjectBase(std::forward<_Base>(rhs)) //noexcept
	{}

	X509Req(const X509Req& rhs) = delete;

	// LCOV_EXCL_START
	virtual ~X509Req() = default;
	// LCOV_EXCL_STOP

	/**
	 * @brief Move assignment. The `rhs` will be empty/null afterwards.
	 *
	 * @exception None No exception thrown
	 * @param rhs The other X509Req instance.
	 * @return X509Req& A reference to this instance.
	 */
	X509Req& operator=(X509Req&& rhs) noexcept
	{
		_Base::operator=(std::forward<_Base>(rhs)); //noexcept

		return *this;
	}

	X509Req& operator=(const X509Req& other) = delete;

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
		_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(X509Req));
	}

	using _Base::NullCheck;
	using _Base::Get;
	using _Base::Swap;

	std::vector<uint8_t> GetDer() const
	{
		NullCheck();

		return std::vector<uint8_t>(
			Get()->raw.p,
			Get()->raw.p + Get()->raw.len
		);
	}

	std::string GetPem() const
	{
		NullCheck();

		return Internal::DerToPem<std::string>(
			CtnFullR(CDynArray<uint8_t>{Get()->raw.p, Get()->raw.len}),
			Internal::GetPemHeaderCSR(),
			Internal::GetPemFooterCSR()
		);
	}

	PKeyBase<BorrowedPKeyTrait> BorrowPublicKey()
	{
		NullCheck();

		return PKeyBase<BorrowedPKeyTrait>(&(Get()->pk));
	}

	template<
		typename _PKeyType,
		enable_if_t<
			IsCppObjOfCtype<_PKeyType, mbedtls_pk_context>::value,
			int
		> = 0
	>
	_PKeyType GetPublicKey() const
	{
		NullCheck();

		auto borrowed = PKeyBase<BorrowedPKeyTrait>(
			&(MutableGet()->pk)
		);

		return _PKeyType::FromDER(
			CtnFullR(
				borrowed.GetPublicDer()
			)
		);
	}

	HashType GetSignHashType() const
	{
		NullCheck();

		return GetHashType(Internal::GetSignMdFromCsr(*Get()));
	}

	void VerifySignature()
	{
		NullCheck();

		const mbedtls_x509_buf& sign = Internal::GetSignFromCsr(*Get());
		mbedtls_md_type_t signMd = Internal::GetSignMdFromCsr(*Get());
		mbedtls_pk_type_t signPkType = Internal::GetSignPkTypeFromCsr(*Get());
		const void* signOpts = Internal::GetSignOptsFromCsr(*Get());

		const mbedtls_md_info_t *mdInfo = mbedtls_md_info_from_type(signMd);
		const size_t mdSize = mbedtls_md_get_size(mdInfo);
		std::vector<uint8_t> tmpHash(mdSize);

		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			X509Req::VerifySignature,
			mbedtls_md,
			mdInfo,
			Get()->cri.p,
			Get()->cri.len,
			tmpHash.data()
		);
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			X509Req::VerifySignature,
			mbedtls_pk_verify_ext,
			signPkType,
			signOpts,
			&Get()->pk,
			signMd,
			tmpHash.data(),
			tmpHash.size(),
			sign.p,
			sign.len
		);
	}

protected:

	X509Req() :
		_Base::ObjectBase()
	{}

}; // class X509Req


} // namespace mbedTLScpp
