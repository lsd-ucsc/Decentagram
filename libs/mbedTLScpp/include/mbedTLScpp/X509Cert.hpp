#pragma once

#include "ObjectBase.hpp"

#include <map>

#include <mbedtls/x509_crt.h>

#include "BigNumber.hpp"
#include "Common.hpp"
#include "Container.hpp"
#include "Exceptions.hpp"
#include "Hash.hpp"
#include "PKey.hpp"
#include "RandInterfaces.hpp"
#include "X509Crl.hpp"

#include "Internal/PemHelper.hpp"
#include "Internal/X509Helper.hpp"


#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{

/**
 * @brief X509 certificate writer object allocator.
 *
 */
struct X509CertWtrObjAllocator : DefaultAllocBase
{
	typedef mbedtls_x509write_cert      CObjType;

	using DefaultAllocBase::NewObject;
	using DefaultAllocBase::DelObject;

	static void Init(CObjType* ptr)
	{
		return mbedtls_x509write_crt_init(ptr);
	}

	static void Free(CObjType* ptr) noexcept
	{
		return mbedtls_x509write_crt_free(ptr);
	}
}; // struct X509CertWtrObjAllocator


/**
 * @brief X509 certificate writer object trait.
 *
 */
using DefaultX509CertWtrObjTrait = ObjTraitBase<X509CertWtrObjAllocator,
											false,
											false>;


/**
 * @brief X509 certificate object allocator.
 *
 */
struct X509CertObjAllocator : DefaultAllocBase
{
	typedef mbedtls_x509_crt      CObjType;

	using DefaultAllocBase::NewObject;
	using DefaultAllocBase::DelObject;

	static void Init(CObjType* ptr)
	{
		return mbedtls_x509_crt_init(ptr);
	}

	static void Free(CObjType* ptr) noexcept
	{
		return mbedtls_x509_crt_free(ptr);
	}
}; // struct X509CertObjAllocator


/**
 * @brief X509 certificate object trait.
 *
 */
using DefaultX509CertObjTrait = ObjTraitBase<X509CertObjAllocator,
											false,
											false>;

/**
 * @brief Borrower X509 Certificate object trait.
 *
 */
using BorrowedX509CertTrait =
	ObjTraitBase<BorrowAllocBase<mbedtls_x509_crt>,
								true,
								false>;


template<
	typename _X509CertObjTrait,
	enable_if_t<
		std::is_same<
			typename _X509CertObjTrait::CObjType,
			mbedtls_x509_crt
		>::value,
		int
	>
>
class X509CertBase;


class X509CertWriter : public ObjectBase<DefaultX509CertWtrObjTrait>
{
public: // Static members:

	using X509CertWtrTrait = DefaultX509CertWtrObjTrait;
	using _Base           = ObjectBase<X509CertWtrTrait>;


	template<typename _PKObjTrait>
	static X509CertWriter SelfSign(
		HashType hashType,
		const PKeyBase<_PKObjTrait> & prvKey,
		const std::string & subjName
	)
	{
		prvKey.NullCheck();

		X509CertWriter wrt;

		mbedtls_x509write_crt_set_version(
			wrt.Get(),
			MBEDTLS_X509_CRT_VERSION_3
		);

		mbedtls_x509write_crt_set_md_alg(wrt.Get(), GetMbedTlsMdType(hashType));

		mbedtls_x509write_crt_set_issuer_key(wrt.Get(), prvKey.MutableGet());
		mbedtls_x509write_crt_set_subject_key(wrt.Get(), prvKey.MutableGet());

		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			X509CertWriter::SelfSign,
			mbedtls_x509write_crt_set_subject_name,
			wrt.Get(),
			subjName.c_str()
		);

		// Self-Signed, thus issuer name is the same as subject name.
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			X509CertWriter::SelfSign,
			mbedtls_x509write_crt_set_issuer_name,
			wrt.Get(),
			subjName.c_str()
		);

		return wrt;
	}


	template<
		typename _CaCertObjTrait,
		typename _CaPKObjTrait,
		typename _SubPKObjTrait
	>
	static X509CertWriter CaSign(
		HashType hashType,
		const X509CertBase<_CaCertObjTrait, 0> & caCert,
		const PKeyBase<_CaPKObjTrait> & caKey,
		const PKeyBase<_SubPKObjTrait> & subjKey,
		const std::string & subjName
	);


public:


	/**
	 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
	 *
	 * @exception None No exception thrown
	 * @param rhs The other X509CertWriter instance.
	 */
	X509CertWriter(X509CertWriter&& rhs) noexcept :
		_Base::ObjectBase(std::forward<_Base>(rhs)) //noexcept
	{}


	X509CertWriter(const X509CertWriter& rhs) = delete;


	// LCOV_EXCL_START
	virtual ~X509CertWriter() = default;
	// LCOV_EXCL_STOP


	/**
	 * @brief Move assignment. The `rhs` will be empty/null afterwards.
	 *
	 * @exception None No exception thrown
	 * @param rhs The other X509CertWriter instance.
	 * @return X509CertWriter& A reference to this instance.
	 */
	X509CertWriter& operator=(X509CertWriter&& rhs) noexcept
	{
		_Base::operator=(std::forward<_Base>(rhs)); //noexcept

		return *this;
	}


	X509CertWriter& operator=(const X509CertWriter& other) = delete;


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
		_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(X509CertWriter));
	}

	using _Base::NullCheck;
	using _Base::Get;
	using _Base::Swap;


	/**
	 * @brief Generates a DER encoded X509 certificate.
	 *
	 * @param rand The Random Bit Generator.
	 *
	 * @return The DER encoded X509 certificate.
	 */
	std::vector<uint8_t> GetDer(RbgInterface& rand)
	{
		NullCheck();

		size_t bufSize = Internal::x509write_crt_der_est_size(*Get());
		std::vector<uint8_t> der(bufSize);

		int len = mbedtls_x509write_crt_der(
			Get(),
			der.data(),
			der.size(),
			&RbgInterface::CallBack,
			&rand
		);
		CheckMbedTlsLenRetVal(
			len,
			"X509CertWriter::GetDer",
			"mbedtls_x509write_crt_der"
		);

		der.erase(der.begin(), der.begin() + (der.size() - len));

		return der;
	}


	/**
	 * @brief Generates a PEM encoded X509 certificate.
	 *
	 * @param rand The Random Bit Generator.
	 *
	 * @return The PEM encoded X509 certificate.
	 */
	std::string GetPem(RbgInterface& rand)
	{
		std::vector<uint8_t> der = GetDer(rand);

		return Internal::DerToPem<std::string>(
			CtnFullR(der),
			Internal::GetPemHeaderCRT(),
			Internal::GetPemFooterCRT()
		);
	}


	template<typename _BigNumTrait>
	X509CertWriter& SetSerialNum(const BigNumberBase<_BigNumTrait> & serialNum)
	{
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			X509CertWriter::SetSerialNum,
			mbedtls_x509write_crt_set_serial,
			Get(),
			serialNum.Get()
		);

		return *this;
	}


	X509CertWriter& SetValidationTime(
		const std::string & validSince,
		const std::string & expireAfter
	)
	{
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			X509CertWriter::SetValidationTime,
			mbedtls_x509write_crt_set_validity,
			Get(),
			validSince.c_str(),
			expireAfter.c_str()
		);

		return *this;
	}


	X509CertWriter& SetBasicConstraints(bool isCa, int maxChainDepth)
	{
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			X509CertWriter::SetBasicConstraints,
			mbedtls_x509write_crt_set_basic_constraints,
			Get(),
			isCa,
			maxChainDepth
		);

		return *this;
	}


	X509CertWriter& SetKeyUsage(unsigned int keyUsage)
	{
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			X509CertWriter::SetKeyUsage,
			mbedtls_x509write_crt_set_key_usage,
			Get(),
			keyUsage
		);

		return *this;
	}


	X509CertWriter& SetNsType(unsigned char nsType)
	{
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			X509CertWriter::SetNsType,
			mbedtls_x509write_crt_set_ns_cert_type,
			Get(),
			nsType
		);

		return *this;
	}


	template<typename _CtnType, bool _CtnSec>
	X509CertWriter& SetV3Extension(
		const std::string& oid,
		bool isCritical,
		const ContCtnReadOnlyRef<_CtnType, _CtnSec>& data
	)
	{
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			X509CertWriter::SetV3Extensions,
			mbedtls_x509write_crt_set_extension,
			Get(),
			oid.data(), // OID
			oid.size(),
			isCritical, // Is critical?
			data.BeginBytePtr(), // Data
			data.GetRegionSize()
		);

		return *this;
	}


protected:


	X509CertWriter() :
		_Base::ObjectBase()
	{}


}; // class X509CertWriter

static_assert(
	IsCppObjOfCtype<X509CertWriter, mbedtls_x509write_cert>::value == true,
	"Implementation Error"
);





template<
	typename _X509CertObjTrait = DefaultX509CertObjTrait,
	enable_if_t<
		std::is_same<
			typename _X509CertObjTrait::CObjType,
			mbedtls_x509_crt
		>::value,
		int
	> = 0
>
class X509CertBase : public ObjectBase<_X509CertObjTrait>
{
public: // Static members:

	using X509CertTrait = _X509CertObjTrait;
	using _Base         = ObjectBase<X509CertTrait>;

	using CObjType    = typename _Base::CObjType;
	using CObjPtrType = typename std::add_pointer<CObjType>::type;

	friend class TlsConfig;


	/**
	 * @brief Defines an alias representing the VerifyFunc used for
	 *        certificate chain verification.
	 */
	typedef int(*VerifyFunc)(void *, mbedtls_x509_crt *, int, uint32_t *);


	/**
	 * @brief The callback function prototype for iterating over the
	 *        V3 extensions of a X509 certificate.
	 *
	 * @param oid      The OID of the extension.
	 * @param oidLen   Length of the OID.
	 * @param isCritical  True if the extension is critical.
	 * @param data     The data of the extension.
	 * @param dataLen  Length of the data.
	 *
	 * @return Keep iterating? True to continue iteration, false to stop.
	 *
	 */
	typedef bool(*V3ExtIterateCallback)(
		const uint8_t* oid,
		size_t oidLen,
		bool isCritical,
		const uint8_t* data,
		size_t dataLen
	);


	/**
	 * @brief Construct a X509 certificate (chain) from a given PEM string.
	 *
	 * @param pem PEM string in std::string
	 */
	static X509CertBase<DefaultX509CertObjTrait> FromPEM(const std::string& pem)
	{
		X509CertBase<DefaultX509CertObjTrait> cert;
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			X509CertBase::FromPEM,
			mbedtls_x509_crt_parse,
			cert.Get(),
			reinterpret_cast<const uint8_t*>(pem.c_str()),
			pem.size() + 1
		);
		return cert;
	}


	/**
	 * @brief Construct a X509 certificate from a given DER bytes.
	 *
	 * @param der DER bytes referenced by ContCtnReadOnlyRef
	 */
	template<typename _SecCtnType>
	static X509CertBase<DefaultX509CertObjTrait>
		FromDER(const ContCtnReadOnlyRef<_SecCtnType, false>& der)
	{
		X509CertBase<DefaultX509CertObjTrait> cert;
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			X509CertBase::FromDER,
			mbedtls_x509_crt_parse,
			cert.Get(),
			der.BeginBytePtr(),
			der.GetRegionSize()
		);
		return cert;
	}


	static X509CertBase<DefaultX509CertObjTrait> Empty()
	{
		X509CertBase<DefaultX509CertObjTrait> cert;
		return cert;
	}


public:

	/**
	 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
	 *
	 * @exception Unclear Depends on \c std::vector .
	 * @param rhs The other X509CertBase instance.
	 */
	X509CertBase(X509CertBase&& rhs) :
		_Base::ObjectBase(std::forward<_Base>(rhs)), //noexcept
		m_certStack(std::move(rhs.m_certStack)),
		m_currPtr(rhs.m_currPtr)
	{
		rhs.m_currPtr = nullptr;
	}


	/**
	 * @brief Construct a new X509CertBase object that borrows the C object.
	 *
	 * @tparam _dummy_PKTrait A dummy template parameter used to make sure
	 *                        the constructor is only available for borrowers.
	 * @param ptr pointer to the borrowed C object.
	 */
	template<
		typename _dummy_ObjTrait = X509CertTrait,
		enable_if_t<_dummy_ObjTrait::sk_isBorrower, int> = 0
	>
	X509CertBase(mbedtls_x509_crt* ptr) :
		_Base::ObjectBase(ptr),
		m_certStack(1, NonVirtualGet()),
		m_currPtr(NonVirtualGet())
	{}


	X509CertBase(const X509CertBase& rhs) = delete;


	// LCOV_EXCL_START
	virtual ~X509CertBase() = default;
	// LCOV_EXCL_STOP


	/**
	 * @brief Move assignment. The `rhs` will be empty/null afterwards.
	 *
	 * @exception Unclear Depends on \c std::vector .
	 * @param rhs The other X509CertBase instance.
	 * @return X509CertBase& A reference to this instance.
	 */
	X509CertBase& operator=(X509CertBase&& rhs)
	{
		_Base::operator=(std::forward<_Base>(rhs)); //noexcept

		if(this != &rhs)
		{
			m_certStack = std::move(rhs.m_certStack);
			m_currPtr = rhs.m_currPtr;

			rhs.m_currPtr = nullptr;
		}

		return *this;
	}


	X509CertBase& operator=(const X509CertBase& other) = delete;


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
		_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(X509CertBase));
	}


	virtual bool IsNull() const noexcept override
	{
		return _Base::IsNull() || (m_currPtr == nullptr);
	}


	using _Base::NullCheck;
	using _Base::Get;
	using _Base::NonVirtualGet;
	using _Base::Swap;


	std::vector<uint8_t> GetDer() const
	{
		NullCheck();

		const auto& curr = *m_currPtr;

		return std::vector<uint8_t>(
			curr.raw.p,
			curr.raw.p + curr.raw.len
		);
	}

	std::string GetPem() const
	{
		NullCheck();

		const auto& curr = *m_currPtr;

		return Internal::DerToPem<std::string>(
			CtnFullR(CDynArray<uint8_t>{curr.raw.p, curr.raw.len}),
			Internal::GetPemHeaderCRT(),
			Internal::GetPemFooterCRT()
		);
	}


	template<
		typename _dummy_CertTrait = X509CertTrait,
		enable_if_t<!_dummy_CertTrait::sk_isConst, int> = 0
	>
	PKeyBase<BorrowedPKeyTrait> BorrowPublicKey()
	{
		NullCheck();

		return PKeyBase<BorrowedPKeyTrait>(&(m_currPtr->pk));
	}


	const PKeyBase<BorrowedPKeyTrait> BorrowPublicKey() const
	{
		NullCheck();

		return PKeyBase<BorrowedPKeyTrait>(&(m_currPtr->pk));
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

		auto borrowed = PKeyBase<BorrowedPKeyTrait>(&(m_currPtr->pk));

		return _PKeyType::FromDER(
			CtnFullR(
				borrowed.GetPublicDer()
			)
		);
	}


	HashType GetSignHashType() const
	{
		NullCheck();

		mbedtls_md_type_t signMd = Internal::GetSignMdFromCrt(*m_currPtr);
		return GetHashType(signMd);
	}


	std::vector<uint8_t> GetSignHash() const
	{
		NullCheck();

		mbedtls_md_type_t signMd = Internal::GetSignMdFromCrt(*m_currPtr);
		auto mdInfo = mbedtls_md_info_from_type(signMd);
		size_t hashLen = mbedtls_md_get_size(mdInfo);
		std::vector<uint8_t> hash(hashLen);

		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			X509CertBase::GetSignHash,
			mbedtls_md,
			mdInfo,
			m_currPtr->tbs.p,
			m_currPtr->tbs.len,
			hash.data()
		);

		return hash;
	}


	std::string GetCommonName() const
	{
		NullCheck();

		const mbedtls_asn1_named_data* cnData =
			mbedtls_asn1_find_named_data(
				&m_currPtr->subject,
				MBEDTLS_OID_AT_CN,
				sizeof(MBEDTLS_OID_AT_CN) - 1
			);

		return std::string(
			reinterpret_cast<const char*>(cnData->val.p),
			cnData->val.len
		);
	}


	template<typename _ItCallbackFunc>
	void IterateV3Extensions(_ItCallbackFunc itCallbackFunc) const
	{
		NullCheck();

		int mbedRet = 0;
		int is_critical = 0;
		size_t len = 0;

		unsigned char *end_ext_data = nullptr;
		unsigned char *end_ext_octet = nullptr;

		unsigned char *begin = m_currPtr->v3_ext.p;
		const unsigned char *end = m_currPtr->v3_ext.p + m_currPtr->v3_ext.len;

		unsigned char **p = &begin;

		const uint8_t* oidPtr = nullptr;
		size_t oidSize = 0;

		const uint8_t* extDataPtr = nullptr;
		size_t extDataSize = 0;

		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			X509CertBase::GetV3Extension,
			mbedtls_asn1_get_tag,
			p,
			end,
			&len,
			MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE
		);
		if (*p + len != end)
		{
			throw RuntimeException(
				"mbedTLScpp::X509CertBase::GetV3Extension"
				" - Invalid length returned by ASN1."
			);
		}

		while (*p < end)
		{
			is_critical = 0; /* DEFAULT FALSE */

			MBEDTLSCPP_MAKE_C_FUNC_CALL(
				X509CertBase::GetV3Extension,
				mbedtls_asn1_get_tag,
				p,
				end,
				&len,
				MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE
			);

			end_ext_data = *p + len;

			/* Get extension ID */
			MBEDTLSCPP_MAKE_C_FUNC_CALL(
				X509CertBase::GetV3Extension,
				mbedtls_asn1_get_tag,
				p,
				end_ext_data,
				&len,
				MBEDTLS_ASN1_OID
			);

			oidPtr = *p;
			oidSize = len;

			// The extension with given OID is found.

			*p += len;

			/* Get optional critical */
			mbedRet = mbedtls_asn1_get_bool(p, end_ext_data, &is_critical);
			if (
				mbedRet != MBEDTLS_EXIT_SUCCESS &&
				mbedRet != MBEDTLS_ERR_ASN1_UNEXPECTED_TAG
			)
			{
				throw RuntimeException(
					"mbedTLScpp::X509CertBase::GetV3Extension"
					" - Invalid tag returned by ASN1."
				);
			}

			/* Data should be octet string type */
			MBEDTLSCPP_MAKE_C_FUNC_CALL(
				X509CertBase::GetV3Extension,
				mbedtls_asn1_get_tag,
				p,
				end_ext_data,
				&len,
				MBEDTLS_ASN1_OCTET_STRING
			);

			extDataPtr = *p;
			extDataSize = len;

			end_ext_octet = *p + len;

			if (end_ext_octet != end_ext_data)
			{
				throw RuntimeException(
					"mbedTLScpp::X509CertBase::GetV3Extension"
					" - Invalid length returned by ASN1."
				);
			}

			bool cbRes = itCallbackFunc(
				oidPtr,
				oidSize,
				is_critical,
				extDataPtr,
				extDataSize
			);
			if (!cbRes)
			{
				// The callback function wants to stop the iteration.
				return;
			}

			*p = end_ext_data;
		}

		return;
	}


	template<typename _RetCtnType, typename _CtnType, bool _CtnSec>
	std::tuple<bool, _RetCtnType> FindV3Extension(
		const ContCtnReadOnlyRef<_CtnType, _CtnSec>& oidCtn
	)
	{
		using _RetCtnKPtrType = typename _RetCtnType::const_pointer;

		std::tuple<bool, _RetCtnType> res;
		bool isFound = false;

		IterateV3Extensions(
			[&oidCtn, &res, & isFound](
				const uint8_t* oidPtr,
				size_t oidSize,
				int is_critical,
				const uint8_t* extDataPtr,
				size_t extDataSize
			) -> bool
			{
				if (
					oidSize == oidCtn.GetRegionSize() &&
					std::memcmp(oidPtr, oidCtn.BeginBytePtr(), oidSize) == 0
				)
				{
					// The extension with given OID is found.
					isFound = true;

					std::get<0>(res) = is_critical;
					std::get<1>(res) = _RetCtnType(
						reinterpret_cast<_RetCtnKPtrType>(extDataPtr),
						extDataSize
					);

					// stop iterating
					return false;
				}

				// The extension with given OID is not found.
				// keep iterating
				return true;
			}
		);

		if (isFound)
		{
			return res;
		}
		else
		{
			throw RuntimeException(
				"X509CertBase::FindV3Extension"
				" - The extension with given OID is not found"
			);
		}
	}


	template<typename _PKObjTrait>
	void VerifySignature(const PKeyBase<_PKObjTrait> & pubKey) const
	{
		NullCheck();
		pubKey.NullCheck();

		const mbedtls_x509_buf& sign =
			Internal::GetSignFromCrt(*m_currPtr);
		mbedtls_md_type_t signMd =
			Internal::GetSignMdFromCrt(*m_currPtr);
		mbedtls_pk_type_t signPkType =
			Internal::GetSignPkTypeFromCrt(*m_currPtr);
		const void* signOpts =
			Internal::GetSignOptsFromCrt(*m_currPtr);

		auto hash = GetSignHash();

		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			X509CertBase::VerifySignature,
			mbedtls_pk_verify_ext,
			signPkType,
			signOpts,
			pubKey.MutableGet(),
			signMd,
			hash.data(),
			hash.size(),
			sign.p,
			sign.len
		);
	}


	void VerifySignature() const
	{
		VerifySignature(BorrowPublicKey());
	}



	// =========================================================================
	// Certificate chain operations
	//==========================================================================


	/**
	 * @brief	Gets the pointer to the current certificate in the chain.
	 *
	 * @exception None No exception thrown
	 * @return	The pointer to the current certificate.
	 */
	const CObjType* GetCurr() const noexcept
	{
		return m_currPtr;
	}


	/**
	 * @brief	Gets the pointer to the current certificate in the chain.
	 *
	 * @exception None No exception thrown
	 * @return	The pointer to the current certificate.
	 */
	CObjType* GetCurr() noexcept
	{
		return m_currPtr;
	}


	bool HasNext() const
	{
		NullCheck();
		return HasNextNoCheck();
	}


	void NextCert()
	{
		if (HasNext())
		{
			NextCertNoCheck();
		}
		else
		{
			throw RuntimeException("There is no next certificate in the chain.");
		}
	}


	void PrevCert()
	{
		if (m_certStack.size() > 1)
		{
			m_certStack.pop_back();
			m_currPtr = m_certStack.back();
		}
		else
		{
			throw RuntimeException("There is no previous certificate in the chain.");
		}
	}


	void GoToFirstCert()
	{
		m_currPtr = Get();
		m_certStack.clear();
		m_certStack.push_back(m_currPtr);
	}


	void GoToLastCert()
	{
		NullCheck();

		while (HasNextNoCheck())
		{
			NextCertNoCheck();
		}
	}

	std::string GetPemChain() const
	{
		NullCheck();

		std::string pemChain;

		const mbedtls_x509_crt* curr = NonVirtualGet();

		while (curr != nullptr)
		{
			std::string currPem = Internal::DerToPem<std::string>(
				CtnFullR(CDynArray<uint8_t>{curr->raw.p, curr->raw.len}),
				Internal::GetPemHeaderCRT(),
				Internal::GetPemFooterCRT()
			);
			pemChain += currPem;
			curr = curr->next;
		}

		return pemChain;
	}


	template<typename _CaObjTrait>
	void VerifyChainWithCa(
		const X509CertBase<_CaObjTrait>& ca,
		const X509Crl* crl,
		const char* cn,
		uint32_t& flags,
		const mbedtls_x509_crt_profile& prof,
		VerifyFunc vrfyFunc,
		void* vrfyParam
	) const
	{
		NullCheck();
		mbedtls_x509_crl* crlPtr = nullptr;

		if(crl != nullptr)
		{
			crl->NullCheck();
			crlPtr = crl->MutableGet();
		}

		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			X509CertBase::VerifyChainWithCa,
			mbedtls_x509_crt_verify_with_profile,
			MutableGet(),
			ca.MutableGet(),
			crlPtr,
			&prof,
			cn,
			&flags,
			vrfyFunc,
			vrfyParam
		);
	}


	template<typename _CaObjTrait,
		typename _dummy_CertTrait = X509CertTrait,
		enable_if_t<!_dummy_CertTrait::sk_isConst, int> = 0>
	void ShrinkChain(const X509CertBase<_CaObjTrait> & ca)
	{
		NullCheck();
		ca.NullCheck();

		mbedtls_x509_crt* prev = nullptr;
		mbedtls_x509_crt* curr = NonVirtualGet();

		bool found = false;

		while (curr != nullptr)
		{
			const mbedtls_x509_crt* currCa = ca.Get();
			while (currCa != nullptr && !found)
			{
				if (
					curr->raw.len == currCa->raw.len &&
					std::memcmp(curr->raw.p, currCa->raw.p, currCa->raw.len) == 0
				)
				{
					// Found
					found = true;
				}

				currCa = currCa->next;
			}

			if (found)
			{
				// The current one is duplicated. Free it.

				mbedtls_x509_crt* toBeFree = curr;

				if (prev == nullptr)
				{
					// This is the first one on chain.

					SetPtr(curr->next);
					curr->next = nullptr;

					// Set the current to the next one, so the search can continue.
					curr = NonVirtualGet();
				}
				else
				{
					prev->next = curr->next;
					curr->next = nullptr;

					// Set the current to the next one, so the search can continue.
					curr = prev->next;
				}

				mbedtls_x509_crt_free(toBeFree);
				// Assuming this is allocated on heap, since it's true for both
				// mbedtls implementation and this mbedTLScpp implementation
				mbedtls_free(toBeFree);
				found = false;
			}
			else
			{
				prev = curr;
				curr = curr->next;
			}
		}

		GoToFirstCert();
	}



	template<
		typename _SecCtnType,
		typename _dummy_CertTrait = X509CertTrait,
		enable_if_t<
			!_dummy_CertTrait::sk_isBorrower && !_dummy_CertTrait::sk_isConst,
			int
		> = 0
	>
	void AppendDER(const ContCtnReadOnlyRef<_SecCtnType, false>& der)
	{
		X509CertBase* pThis = static_cast<X509CertBase*>(this);

		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			X509CertBase::AppendDER,
			mbedtls_x509_crt_parse_der_with_ext_cb,
			Get(),
			der.BeginBytePtr(),
			der.GetRegionSize(),
			1,
			mbedTLSParseExtCallback,
			pThis
		);
	}


protected:


	using _Base::MutableGet;
	using _Base::SetPtr;


	template<typename _dummy_CertTrait = X509CertTrait,
		enable_if_t<!_dummy_CertTrait::sk_isBorrower, int> = 0>
	X509CertBase() :
		_Base::ObjectBase(),
		m_certStack(1, NonVirtualGet()),
		m_currPtr(NonVirtualGet())
	{}


	bool HasNextNoCheck() const noexcept
	{
		return m_currPtr->next != nullptr;
	}


	void NextCertNoCheck()
	{
		m_certStack.push_back(m_currPtr->next);
		m_currPtr = m_currPtr->next;
	}


	virtual int mbedTLSParseExt(
		mbedtls_x509_crt const* /* crt */,
		mbedtls_x509_buf const* /* oid */,
		int /* critical */,
		const unsigned char* /* p */,
		const unsigned char* /* end */
	)
	{
		return MBEDTLS_ERROR_ADD(
			MBEDTLS_ERR_X509_INVALID_EXTENSIONS,
			MBEDTLS_ERR_ASN1_UNEXPECTED_TAG
		);
	}


private: // static members


	static int mbedTLSParseExtCallback(
		void *p_ctx,
		mbedtls_x509_crt const *crt,
		mbedtls_x509_buf const *oid,
		int critical,
		const unsigned char *p,
		const unsigned char *end
	)
	{
		X509CertBase* pThis = static_cast<X509CertBase*>(p_ctx);

		return pThis->mbedTLSParseExt(crt, oid, critical, p, end);
	}


private:


	std::vector<CObjPtrType> m_certStack;
	CObjPtrType m_currPtr; // For noexcept


}; // class X509CertBase


using X509Cert = X509CertBase<>;

template<typename _CaCertObjTrait,
	typename _CaPKObjTrait,
	typename _SubPKObjTrait>
inline X509CertWriter X509CertWriter::CaSign(
		HashType hashType,
		const X509CertBase<_CaCertObjTrait> & caCert,
		const PKeyBase<_CaPKObjTrait> & caKey,
		const PKeyBase<_SubPKObjTrait> & subjKey,
		const std::string & subjName)
{
	caCert.NullCheck();
	caKey.NullCheck();
	subjKey.NullCheck();

	X509CertWriter wrt;

	mbedtls_x509write_crt_set_version(wrt.Get(), MBEDTLS_X509_CRT_VERSION_3);

	mbedtls_x509write_crt_set_md_alg(wrt.Get(), GetMbedTlsMdType(hashType));

	mbedtls_x509write_crt_set_issuer_key(wrt.Get(), caKey.MutableGet());
	mbedtls_x509write_crt_set_subject_key(wrt.Get(), subjKey.MutableGet());

	MBEDTLSCPP_MAKE_C_FUNC_CALL(
		X509CertWriter::CaSign,
		mbedtls_x509write_crt_set_subject_name,
		wrt.Get(), subjName.c_str());

	Internal::Asn1DeepCopy(
		wrt.Get()->MBEDTLS_PRIVATE(issuer),
		&caCert.Get()->subject
	);

	// It is necessary to reverse the named list we've copied,
	// otherwise, x509_crt_check_parent will fail to find the parent
	Internal::Asn1ReverseNamedDataList(
		wrt.Get()->MBEDTLS_PRIVATE(issuer)
	);

	return wrt;
}


} // namespace mbedTLScpp
