#pragma once

#include "ObjectBase.hpp"

#include <mbedtls/x509_crl.h>
#include <mbedtls/pem.h>

#include "Common.hpp"
#include "Exceptions.hpp"
#include "Container.hpp"

#include "Internal/PemHelper.hpp"
#include "Internal/X509Helper.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{

/**
 * @brief X509 certificate request object allocator.
 *
 */
struct X509CrlObjAllocator : DefaultAllocBase
{
	typedef mbedtls_x509_crl      CObjType;

	using DefaultAllocBase::NewObject;
	using DefaultAllocBase::DelObject;

	static void Init(CObjType* ptr)
	{
		return mbedtls_x509_crl_init(ptr);
	}

	static void Free(CObjType* ptr) noexcept
	{
		return mbedtls_x509_crl_free(ptr);
	}
}; // struct X509CrlObjAllocator


/**
 * @brief X509 certificate request object trait.
 *
 */
using DefaultX509CrlObjTrait = ObjTraitBase<X509CrlObjAllocator,
											false,
											false>;


class X509Crl : public ObjectBase<DefaultX509CrlObjTrait>
{
public: // Static members:

	using X509CrlTrait = DefaultX509CrlObjTrait;
	using _Base        = ObjectBase<X509CrlTrait>;

	friend class TlsConfig;

	template<typename T,
		enable_if_t<std::is_same<typename T::CObjType, mbedtls_x509_crt>::value, int> >
	friend class X509CertBase;

	/**
	 * @brief Construct a X509 certificate revocation list from a given PEM string.
	 *
	 * @param pem PEM string in std::string
	 */
	static X509Crl FromPEM(const std::string& pem)
	{
		X509Crl crl;
		MBEDTLSCPP_MAKE_C_FUNC_CALL(X509Crl::FromPEM,
			mbedtls_x509_crl_parse,
			crl.Get(),
			reinterpret_cast<const uint8_t*>(pem.c_str()), pem.size() + 1);
		return crl;
	}

	/**
	 * @brief Construct a X509 certificate revocation list from a given DER bytes.
	 *
	 * @param der DER bytes referenced by ContCtnReadOnlyRef
	 */
	template<typename _SecCtnType>
	static X509Crl FromDER(const ContCtnReadOnlyRef<_SecCtnType, false>& der)
	{
		X509Crl crl;
		MBEDTLSCPP_MAKE_C_FUNC_CALL(X509Crl::FromDER,
			mbedtls_x509_crl_parse_der,
			crl.Get(),
			der.BeginBytePtr(), der.GetRegionSize());
		return crl;
	}

public:

	/**
	 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
	 *
	 * @exception None No exception thrown
	 * @param rhs The other X509Crl instance.
	 */
	X509Crl(X509Crl&& rhs) noexcept :
		_Base::ObjectBase(std::forward<_Base>(rhs)) //noexcept
	{}

	X509Crl(const X509Crl& rhs) = delete;

	// LCOV_EXCL_START
	virtual ~X509Crl() = default;
	// LCOV_EXCL_STOP

	/**
	 * @brief Move assignment. The `rhs` will be empty/null afterwards.
	 *
	 * @exception None No exception thrown
	 * @param rhs The other X509Crl instance.
	 * @return X509Crl& A reference to this instance.
	 */
	X509Crl& operator=(X509Crl&& rhs) noexcept
	{
		_Base::operator=(std::forward<_Base>(rhs)); //noexcept

		return *this;
	}

	X509Crl& operator=(const X509Crl& other) = delete;

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
		_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(X509Crl));
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
			Internal::GetPemHeaderCRL(),
			Internal::GetPemFooterCRL()
		);
	}

protected:

	X509Crl() :
		_Base::ObjectBase()
	{}
}; // class X509Crl

} // namespace mbedTLScpp
