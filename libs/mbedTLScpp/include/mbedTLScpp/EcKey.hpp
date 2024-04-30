// Copyright (c) 2022 mbedTLScpp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#pragma once


#include <tuple>

#include <mbedtls/ecp.h>
#include <mbedtls/ecdh.h>
#include <mbedtls/ecdsa.h>

#include "BigNumber.hpp"
#include "EcKeyEnum.hpp"
#include "PKey.hpp"


#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{

/**
 * @brief Normal EC group allocator.
 *
 */
struct EcGroupAllocator : DefaultAllocBase
{
	typedef mbedtls_ecp_group      CObjType;

	using DefaultAllocBase::NewObject;
	using DefaultAllocBase::DelObject;

	static void Init(CObjType* ptr)
	{
		return mbedtls_ecp_group_init(ptr);
	}

	static void Free(CObjType* ptr) noexcept
	{
		return mbedtls_ecp_group_free(ptr);
	}
};


/**
 * @brief Normal EC Group Trait.
 *
 */
using DefaultEcGroupObjTrait = ObjTraitBase<EcGroupAllocator,
								false,
								false>;


/**
 * @brief Borrower EC Group Trait.
 *
 */
using BorrowedEcGroupTrait = ObjTraitBase<BorrowAllocBase<mbedtls_ecp_group>,
								true,
								false>;



template<
	typename _ObjTraits = DefaultEcGroupObjTrait,
	enable_if_t<
		std::is_same<typename _ObjTraits::CObjType, mbedtls_ecp_group>::value,
		int
	> = 0
>
class EcGroup : public ObjectBase<_ObjTraits>
{
public: // static member:

	using _Base = ObjectBase<_ObjTraits>;
	using _EcGroupOwnerType = EcGroup<DefaultEcGroupObjTrait>;

	template<typename _other_ObjTraits>
	static _EcGroupOwnerType FromDeepCopy(
		const EcGroup<_other_ObjTraits>& other
	)
	{
		_EcGroupOwnerType ret;
		ret.DeepCopy(other);
		return ret;
	}

public:

	template<// automated parts:
		typename _dummy_ObjTrait = _ObjTraits,
		enable_if_t<
			!_dummy_ObjTrait::sk_isBorrower &&
			!_dummy_ObjTrait::sk_isConst,
			int
		> = 0
	>
	EcGroup() :
		_Base::ObjectBase()
	{}


	template<// automated parts:
		typename _dummy_ObjTrait = _ObjTraits,
		enable_if_t<
			!_dummy_ObjTrait::sk_isBorrower &&
			!_dummy_ObjTrait::sk_isConst,
			int
		> = 0
	>
	explicit EcGroup(EcType type) :
		EcGroup()
	{
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			EcGroup::EcGroup,
			mbedtls_ecp_group_load,
			NonVirtualGet(),
			ToEcGroupId(type)
		);
	}


	EcGroup(EcGroup&& rhs) noexcept :
		_Base::ObjectBase(std::forward<_Base>(rhs)) //noexcept
	{}


	template<// automated parts:
		typename _dummy_ObjTrait = _ObjTraits,
		enable_if_t<
			!_dummy_ObjTrait::sk_isBorrower &&
			!_dummy_ObjTrait::sk_isConst,
			int
		> = 0
	>
	EcGroup(const mbedtls_ecp_group& other) :
		EcGroup()
	{
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			EcGroup::EcGroup,
			mbedtls_ecp_group_copy,
			NonVirtualGet(),
			&other
		);
	}


	template<// automated parts:
		typename _dummy_ObjTrait = _ObjTraits,
		enable_if_t<_dummy_ObjTrait::sk_isBorrower, int> = 0
	>
	explicit EcGroup(mbedtls_ecp_group& other) :
		_Base::ObjectBase(&other)
	{}


	// LCOV_EXCL_START
	virtual ~EcGroup() = default;
	// LCOV_EXCL_STOP


	/**
	 * @brief Move assignment. The `rhs` will be empty/null afterwards.
	 *
	 * @exception None No exception thrown
	 * @param rhs The other EcGroup instance.
	 * @return EcGroup& A reference to this instance.
	 */
	EcGroup& operator=(EcGroup&& rhs) noexcept
	{
		_Base::operator=(std::forward<_Base>(rhs)); //noexcept

		return *this;
	}


	using _Base::Get;
	using _Base::NonVirtualGet;
	using _Base::NullCheck;


	template<typename _other_ObjTraits>
	void DeepCopy(
		const EcGroup<_other_ObjTraits>& other
	)
	{
		if(other.IsNull())
		{
			_Base::FreeBaseObject();
		}
		else
		{
			MBEDTLSCPP_MAKE_C_FUNC_CALL(
				EcGroup::DeepCopy,
				mbedtls_ecp_group_copy,
				Get(),
				other.Get()
			);
		}
	}


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
		_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(EcGroup));
	}


	void Load(EcType type)
	{
		NullCheck();
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			EcGroup::Load,
			mbedtls_ecp_group_load,
			Get(),
			ToEcGroupId(type)
		);
	}

}; // class EcGroup


namespace Internal
{

template<typename _other_PKTrait>
inline PKeyBase<_other_PKTrait>&& PKeyBaseRRefFilter(
	PKeyBase<_other_PKTrait>&& other,
	PKeyAlgmCat algCat,
	PKeyType reqKeyType
)
{
	// Check algorithm category
	if (other.GetAlgorithmCat() != algCat)
	{
		throw InvalidArgumentException(
			"Internal::BaseRRefFilter"
			" - The algorithm of given PKeyBase doesn't match the required one"
		);
	}

	// Check key type
	switch (reqKeyType)
	{
	case PKeyType::Public:
	{
		if (!other.HasPubKey())
		{
			throw InvalidArgumentException(
				"Internal::BaseRRefFilter"
				" - The given PKeyBase doesn't have public key"
			);
		}
		break;
	}
	case PKeyType::Private:
	{
		if (other.GetKeyType() != PKeyType::Private)
		{
			throw InvalidArgumentException(
				"Internal::BaseRRefFilter"
				" - The given PKeyBase doesn't have private key"
			);
		}
		break;
	}
	default:
		throw InvalidArgumentException(
			"Internal::BaseRRefFilter"
			" - The given PKeyType is invalid"
		);
	}

	// All checks passed, return the given object
	return std::forward<PKeyBase<_other_PKTrait> >(other);
}


template<
	PKeyAlgmCat _AlgCat,
	PKeyType _ReqKeyType,
	typename _other_PKTrait
>
inline PKeyBase<_other_PKTrait>&& PKeyBaseRRefFilter(
	PKeyBase<_other_PKTrait>&& other
)
{
	return PKeyBaseRRefFilter(
		std::forward<PKeyBase<_other_PKTrait> >(other),
		_AlgCat,
		_ReqKeyType
	);
}


} // namespace Internal


template<
	typename _PKObjTrait = DefaultPKeyObjTrait,
	enable_if_t<
		std::is_same<
			typename _PKObjTrait::CObjType,
			mbedtls_pk_context
		>::value,
		int
	> = 0
>
class EcPublicKeyBase : public PKeyBase<_PKObjTrait>
{
public: // Types:

	using PKObjTrait = _PKObjTrait;
	using _Base = PKeyBase<_PKObjTrait>;

	using EcPublicKeyBaseOwnerType = EcPublicKeyBase<DefaultPKeyObjTrait>;

public: // Static members, and methods will be used in constructors:

	/**
	 * @brief	Move constructor that moves a general PKeyBase object to EC
	 *          Key pair. If it failed, the \c other will remain the same.
	 *
	 * @exception InvalidArgumentException Thrown when the given object is
	 *                                     not a EC private key.
	 * @exception mbedTLSRuntimeError Thrown when mbed TLS C function call failed.
	 *
	 * @param	other	The PKeyBase instance to convert.
	 */
	static EcPublicKeyBase<PKObjTrait> FromPKeyBase(
		PKeyBase<PKObjTrait>&& other
	)
	{
		return EcPublicKeyBase<PKObjTrait>(
			std::forward<PKeyBase<PKObjTrait> >(other)
		);
	}

	/**
	 * @brief Construct a EcPublicKeyBase object (public part) from a given PEM string.
	 *
	 * @param pem PEM string in std::string
	 */
	static EcPublicKeyBaseOwnerType FromPEM(const std::string& pem)
	{
		using _OwnerTrait = typename EcPublicKeyBaseOwnerType::PKObjTrait;
		return EcPublicKeyBaseOwnerType::FromPKeyBase(
			PKeyBase<_OwnerTrait>::FromPEM(pem)
		);
	}

	/**
	 * @brief Construct a EcPublicKeyBase object (public part) from a given DER bytes.
	 *
	 * @param der DER bytes referenced by ContCtnReadOnlyRef
	 */
	template<typename _SecCtnType>
	static EcPublicKeyBaseOwnerType FromDER(
		const ContCtnReadOnlyRef<_SecCtnType, false>& der
	)
	{
		using _OwnerTrait = typename EcPublicKeyBaseOwnerType::PKObjTrait;
		return EcPublicKeyBaseOwnerType::FromPKeyBase(
			PKeyBase<_OwnerTrait>::FromDER(der)
		);
	}


	template<typename _other_ObjTraits>
	static EcPublicKeyBaseOwnerType FromDeepCopy(
		const EcPublicKeyBase<_other_ObjTraits>& other
	)
	{
		if(other.IsNull())
		{
			EcPublicKeyBaseOwnerType ret;
			ret.FreeBaseObject();
			return ret;
		}
		else
		{
			const mbedtls_pk_context& otherPk = *other.Get();
			const mbedtls_ecp_keypair* otherEcCtx = mbedtls_pk_ec(otherPk);
			if (otherEcCtx == nullptr)
			{
				return EcPublicKeyBase();
			}
			else
			{
				const mbedtls_ecp_group& otherGrp =
					Internal::GetGroupFromEcPair(*otherEcCtx);

				EcPublicKeyBaseOwnerType ret(ToEcType(otherGrp.id));

				mbedtls_ecp_keypair& retEcCtx = *mbedtls_pk_ec(*(ret.Get()));

				MBEDTLSCPP_MAKE_C_FUNC_CALL(
					EcPublicKeyBase::FromDeepCopy,
					mbedtls_ecp_copy,
					&Internal::GetQFromEcPair(retEcCtx),
					&Internal::GetQFromEcPair(*otherEcCtx)
				);

				return ret;
			}
		}
	}


	/**
	 * @brief Construct a new EcPublicKeyBase object that borrows the C object.
	 *
	 * @param ptr pointer to the C object to be borrowed.
	 */
	template<
		typename _dummy_PKTrait = PKObjTrait,
		enable_if_t<_dummy_PKTrait::sk_isBorrower, int> = 0
	>
	static EcPublicKeyBase<_dummy_PKTrait> Borrow(mbedtls_pk_context* ptr)
	{
		auto pk = PKeyBase<_dummy_PKTrait>(ptr);

		return FromPKeyBase(std::move(pk));
	}


public:

	template<
		typename _dummy_PKTrait = PKObjTrait,
		enable_if_t<!_dummy_PKTrait::sk_isBorrower, int> = 0
	>
	EcPublicKeyBase() :
		_Base::PKeyBase()
	{}


	template<
		typename _dummy_PKTrait = PKObjTrait,
		enable_if_t<!_dummy_PKTrait::sk_isBorrower, int> = 0
	>
	explicit EcPublicKeyBase(mbedtls_pk_type_t) :
		EcPublicKeyBase()
	{
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			EcPublicKeyBase::EcPublicKeyBase,
			mbedtls_pk_setup,
			NonVirtualGet(),
			mbedtls_pk_info_from_type(mbedtls_pk_type_t::MBEDTLS_PK_ECKEY)
		);
	}


	template<
		typename _dummy_PKTrait = PKObjTrait,
		enable_if_t<!_dummy_PKTrait::sk_isBorrower, int> = 0
	>
	explicit EcPublicKeyBase(EcType type) :
		EcPublicKeyBase(MBEDTLS_PK_ECKEY)
	{
		mbedtls_ecp_keypair& ecCtx = *mbedtls_pk_ec(*NonVirtualGet());

		EcGroup<BorrowedEcGroupTrait> ecGrp(
			Internal::GetGroupFromEcPair(ecCtx)
		);
		ecGrp.Load(type);
	}


	/**
	 * @brief	Move constructor that moves a general PKeyBase object to EC
	 *          Key pair. If it failed, the \c other will remain the same.
	 *
	 * @exception InvalidArgumentException Thrown when the given object is
	 *                                     not a EC private key.
	 * @exception mbedTLSRuntimeError Thrown when mbed TLS C function call failed.
	 *
	 * @param	other	The PKeyBase instance to convert.
	 */
	explicit EcPublicKeyBase(PKeyBase<PKObjTrait>&& other) :
		EcPublicKeyBase(
			std::forward<_Base>(other),
			Internal::PKeyBaseRRefFilter<
				PKeyAlgmCat::EC,
				PKeyType::Public,
				PKObjTrait
			>
		)
	{}


	/**
	 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
	 *
	 * @exception None No exception thrown
	 * @param rhs The other EC public key instance.
	 */
	EcPublicKeyBase(EcPublicKeyBase&& rhs) noexcept :
		_Base::PKeyBase(std::forward<_Base>(rhs)) //noexcept
	{}


	// LCOV_EXCL_START
	virtual ~EcPublicKeyBase() = default;
	// LCOV_EXCL_STOP


	/**
	 * @brief Move assignment. The `rhs` will be empty/null afterwards.
	 *
	 * @exception None No exception thrown
	 * @param rhs The other EcPublicKeyBase instance.
	 * @return EcPublicKeyBase& A reference to this instance.
	 */
	EcPublicKeyBase& operator=(EcPublicKeyBase&& rhs) noexcept
	{
		_Base::operator=(std::forward<_Base>(rhs)); //noexcept

		return *this;
	}


	using _Base::Get;
	using _Base::NullCheck;
	using _Base::NonVirtualGet;
	using _Base::FreeBaseObject;

	using _Base::GetPublicDer;
	using _Base::GetPublicPem;


	/**
	 * @brief	Gets PKey algorithm type.
	 *
	 * @return	The PKey algorithm type.
	 */
	virtual PKeyAlgmCat GetAlgorithmCat() const override
	{
		return PKeyAlgmCat::EC;
	}


	/**
	 * @brief	Gets PKey type (either public or private).
	 *
	 * @return	The PKey type.
	 */
	virtual PKeyType GetKeyType() const override
	{
		return PKeyType::Public;
	}


	/**
	 * @brief Check if the current instance is holding a null pointer for
	 *        the mbedTLS object. If so, exception will be thrown. Helper
	 *        function to be called before accessing the mbedTLS object.
	 *
	 * @exception InvalidObjectException Thrown when the current instance is
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 */
	virtual void NullCheck() const override
	{
		_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(EcPublicKeyBase));
	}


	/**
	 * @brief	Gets mbed TLS's EC key pair context.
	 *
	 * @exception InvalidObjectException Thrown when the current instance is
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 *
	 * @return	The mbed TLS's EC key pair context.
	 */
	mbedtls_ecp_keypair* GetEcContext()
	{
		NullCheck();
		return mbedtls_pk_ec(*Get());;
	}


	/**
	 * @brief	Gets mbed TLS's EC key pair context.
	 *
	 * @exception InvalidObjectException Thrown when the current instance is
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 *
	 * @return	The mbed TLS's EC key pair context.
	 */
	const mbedtls_ecp_keypair* GetEcContext() const
	{
		NullCheck();
		return mbedtls_pk_ec(*Get());;
	}


	bool IsEcContextNull() const
	{
		return GetEcContext() == nullptr;
	}


	mbedtls_ecp_keypair& GetEcContextRef()
	{
		mbedtls_ecp_keypair* ctx = GetEcContext();
		if (ctx == nullptr)
		{
			throw InvalidObjectException(
				MBEDTLSCPP_CLASS_NAME_STR(EcPublicKeyBase)
			);
		}

		return *ctx;
	}


	const mbedtls_ecp_keypair& GetEcContextRef() const
	{
		const mbedtls_ecp_keypair* ctx = GetEcContext();
		if (ctx == nullptr)
		{
			throw InvalidObjectException(
				MBEDTLSCPP_CLASS_NAME_STR(EcPublicKeyBase)
			);
		}

		return *ctx;
	}


	/**
	 * @brief	Gets Elliptic Curve type
	 *
	 * @return	The Elliptic Curve type.
	 */
	virtual EcType GetEcType() const
	{
		return ToEcType(Internal::GetGroupFromEcPair(GetEcContextRef()).id);
	}


	EcGroup<> CopyGroup() const
	{
		const mbedtls_ecp_keypair& ctx = GetEcContextRef();

		EcGroup<> res;
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			EcPublicKeyBase::CopyGroup,
			mbedtls_ecp_group_copy,
			res.Get(),
			&Internal::GetGroupFromEcPair(ctx)
		);

		return res;
	}


	/**
	 * @brief	Verify signature.
	 *
	 * @tparam	containerType	Type of the container for the hash.
	 * @param	hash	The hash.
	 * @param	r   	Elliptic Curve signature's R value.
	 * @param	s   	Elliptic Curve signature's S value.
	 */
	template<
		typename _HashCtnType,
		bool _HashSecrecy,
		typename _r_Trait,
		typename _s_Trait
	>
	void VerifySign(
		const ContCtnReadOnlyRef<_HashCtnType, _HashSecrecy>& hash,
		const BigNumberBase<_r_Trait>& r,
		const BigNumberBase<_s_Trait>& s
	) const
	{
		const mbedtls_ecp_keypair& ecCtx = GetEcContextRef();

		EcGroup<> ecGrp = CopyGroup();

		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			EcPublicKeyBase::VerifySign,
			mbedtls_ecdsa_verify,
			ecGrp.Get(),
			static_cast<const unsigned char*>(hash.BeginPtr()),
			hash.GetRegionSize(),
			&Internal::GetQFromEcPair(ecCtx),
			r.Get(),
			s.Get()
		);
	}

protected:

	using _Base::GetPrivateDer;
	using _Base::GetPrivatePem;


	template<typename _FilterFunc>
	EcPublicKeyBase(
		PKeyBase<PKObjTrait>&& other,
		_FilterFunc filterFunc
	) :
		_Base::PKeyBase(
			filterFunc(
				std::forward<_Base>(other)
			)
		)
	{}


}; // class EcPublicKeyBase


template<
	typename _PKObjTrait = DefaultPKeyObjTrait,
	enable_if_t<
		std::is_same<
			typename _PKObjTrait::CObjType,
			mbedtls_pk_context
		>::value,
		int
	> = 0
>
class EcKeyPairBase : public EcPublicKeyBase<_PKObjTrait>
{
public: // Static members:

	using PKObjTrait = _PKObjTrait;
	using _Base = EcPublicKeyBase<_PKObjTrait>;
	using EcKeyPairBaseOwnerType = EcKeyPairBase<DefaultPKeyObjTrait>;

	/**
	 * @brief	Move constructor that moves a general PKeyBase object to EC
	 *          Key pair. If it failed, the \c other will remain the same.
	 *
	 * @exception InvalidArgumentException Thrown when the given object is
	 *                                     not a EC private key.
	 * @exception mbedTLSRuntimeError Thrown when mbed TLS C function call failed.
	 *
	 * @param	other	The PKeyBase instance to convert.
	 */
	static EcKeyPairBase<PKObjTrait> FromPKeyBase(
		PKeyBase<PKObjTrait>&& other
	)
	{
		return EcKeyPairBase<PKObjTrait>(
			std::forward<PKeyBase<PKObjTrait> >(other)
		);
	}

	/**
	 * @brief Construct a EC key pair from a given PEM string.
	 *
	 * @param pem PEM string in SecretString
	 */
	static EcKeyPairBaseOwnerType FromPEM(
		const SecretString& pem,
		RbgInterface& rand
	)
	{
		using _OwnerTrait = typename EcKeyPairBaseOwnerType::PKObjTrait;
		return EcKeyPairBaseOwnerType::FromPKeyBase(
			PKeyBase<_OwnerTrait>::FromPEM(pem, rand)
		);
	}

	/**
	 * @brief Construct a EC key pair from a given DER bytes.
	 *
	 * @param der DER bytes referenced by ContCtnReadOnlyRef
	 */
	template<typename _SecCtnType>
	static EcKeyPairBaseOwnerType FromDER(
		const ContCtnReadOnlyRef<_SecCtnType, true>& der,
		RbgInterface& rand
	)
	{
		using _OwnerTrait = typename EcKeyPairBaseOwnerType::PKObjTrait;
		return EcKeyPairBaseOwnerType::FromPKeyBase(
			PKeyBase<_OwnerTrait>::FromDER(der, rand)
		);
	}


	static EcKeyPairBaseOwnerType Generate(
		EcType ecType,
		RbgInterface& rand
	)
	{
		EcKeyPairBaseOwnerType res(ecType);

		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			EcKeyPairBase::EcKeyPairBase,
			mbedtls_ecp_gen_key,
			ToEcGroupId(ecType),
			res.GetEcContext(),
			&RbgInterface::CallBack,
			&rand
		);

		return res;
	}


	template<typename _other_ObjTraits>
	static EcKeyPairBaseOwnerType FromDeepCopy(
		const EcKeyPairBase<_other_ObjTraits>& other
	)
	{
		auto cpy = _Base::FromDeepCopy(other);

		if(other.IsNull())
		{
			return EcKeyPairBaseOwnerType(std::move(cpy));
		}
		else
		{
			const mbedtls_pk_context& otherPk = *other.Get();
			const mbedtls_ecp_keypair* otherEcCtx = mbedtls_pk_ec(otherPk);
			if (otherEcCtx == nullptr)
			{
				return EcKeyPairBaseOwnerType(std::move(cpy));
			}
			else
			{
				mbedtls_ecp_keypair& cpyEcCtx = *mbedtls_pk_ec(*(cpy.Get()));

				MBEDTLSCPP_MAKE_C_FUNC_CALL(
					EcKeyPairBase::FromDeepCopy,
					mbedtls_mpi_copy,
					&Internal::GetDFromEcPair(cpyEcCtx),
					&Internal::GetDFromEcPair(*otherEcCtx)
				);

				return EcKeyPairBaseOwnerType(std::move(cpy));
			}
		}
	}


	/**
	 * @brief Construct a new EC key object that borrows the C object.
	 *
	 * @param ptr pointer to the C object to be borrowed.
	 */
	template<
		typename _dummy_PKTrait = PKObjTrait,
		enable_if_t<_dummy_PKTrait::sk_isBorrower, int> = 0
	>
	static EcKeyPairBase<_dummy_PKTrait> Borrow(mbedtls_pk_context* ptr)
	{
		auto pk = PKeyBase<_dummy_PKTrait>(ptr);

		return FromPKeyBase(std::move(pk));
	}

public:

	template<
		typename _dummy_PKTrait = PKObjTrait,
		enable_if_t<!_dummy_PKTrait::sk_isBorrower, int> = 0
	>
	EcKeyPairBase() :
		_Base::EcPublicKeyBase()
	{}


	template<
		typename _dummy_PKTrait = PKObjTrait,
		enable_if_t<!_dummy_PKTrait::sk_isBorrower, int> = 0
	>
	explicit EcKeyPairBase(mbedtls_pk_type_t x) :
		_Base::EcPublicKeyBase(x)
	{}


	template<
		typename _dummy_PKTrait = PKObjTrait,
		enable_if_t<!_dummy_PKTrait::sk_isBorrower, int> = 0
	>
	explicit EcKeyPairBase(EcType type) :
		_Base::EcPublicKeyBase(type)
	{}


	/**
	 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
	 *
	 * @exception None No exception thrown
	 * @param rhs The other EC public key instance.
	 */
	EcKeyPairBase(EcKeyPairBase&& rhs) noexcept :
		_Base::EcPublicKeyBase(std::forward<_Base>(rhs)) //noexcept
	{}


	/**
	 * @brief	Move constructor that moves a general PKeyBase object to EC
	 *          Key pair. If it failed, the \c other will remain the same.
	 *
	 * @exception InvalidArgumentException Thrown when the given object is
	 *                                     not a EC private key.
	 * @exception mbedTLSRuntimeError Thrown when mbed TLS C function call failed.
	 *
	 * @param	other	The PKeyBase instance to convert.
	 */
	explicit EcKeyPairBase(PKeyBase<PKObjTrait>&& other) :
		_Base::EcPublicKeyBase(
			std::forward<PKeyBase<PKObjTrait> >(other),
			Internal::PKeyBaseRRefFilter<
				PKeyAlgmCat::EC,
				PKeyType::Private,
				PKObjTrait
			>
		)
	{}


	// LCOV_EXCL_START
	virtual ~EcKeyPairBase() = default;
	// LCOV_EXCL_STOP


	/**
	 * @brief Move assignment. The `rhs` will be empty/null afterwards.
	 *
	 * @exception None No exception thrown
	 * @param rhs The other EcKeyPairBase instance.
	 * @return EcKeyPairBase& A reference to this instance.
	 */
	EcKeyPairBase& operator=(EcKeyPairBase&& rhs) noexcept
	{
		_Base::operator=(std::forward<_Base>(rhs)); //noexcept

		return *this;
	}


	using _Base::Get;
	using _Base::NullCheck;
	using _Base::NonVirtualGet;
	using _Base::FreeBaseObject;

	using _Base::GetEcContext;
	using _Base::GetEcContextRef;
	using _Base::IsEcContextNull;
	using _Base::CopyGroup;

	using _Base::GetPublicDer;
	using _Base::GetPublicPem;

	using _Base::GetPrivateDer;
	using _Base::GetPrivatePem;


	/**
	 * @brief	Gets PKey type (either public or private).
	 *
	 * @return	The PKey type.
	 */
	virtual PKeyType GetKeyType() const override
	{
		return PKeyType::Private;
	}


	/**
	 * @brief	Make a signature.
	 *
	 * @tparam	_HashCtnType	Type of the container for the hash.
	 * @tparam	_HashSecrecy	Secrecy of the container for the hash.
	 * @param	hashType	The type of hash.
	 * @param	hash		The hash.
	 * @param	rand		The random bit generator.
	 * @return	A tuple of BigNum's. It's in the order of R and S value.
	 */
	template<typename _HashCtnType, bool _HashSecrecy>
	std::tuple<
		BigNum /* r */,
		BigNum /* s */
	>
	SignInBigNum(
		HashType hashType,
		const ContCtnReadOnlyRef<_HashCtnType, _HashSecrecy>& hash,
		RbgInterface& rand
	) const
	{
		const mbedtls_ecp_keypair& ecCtx = GetEcContextRef();
		EcGroup<> ecGrp = CopyGroup();

		BigNum r;
		BigNum s;

#ifdef MBEDTLS_ECDSA_DETERMINISTIC
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			EcKeyPairBase::SignInBigNum,
			mbedtls_ecdsa_sign_det_ext,
			ecGrp.Get(),
			r.Get(),
			s.Get(),
			&Internal::GetDFromEcPair(ecCtx),
			static_cast<const unsigned char*>(hash.BeginPtr()),
			hash.GetRegionSize(),
			GetMbedTlsMdType(hashType),
			&RbgInterface::CallBack,
			&rand
		);
#else
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			EcKeyPairBase::SignInBigNum,
			mbedtls_ecdsa_sign,
			ecGrp.Get(),
			r.Get(),
			s.Get(),
			&Internal::GetDFromEcPair(ecCtx),
			static_cast<const unsigned char*>(hash.BeginPtr()),
			hash.GetRegionSize(),
			&RbgInterface::CallBack,
			&rand
		);
#endif // MBEDTLS_ECDSA_DETERMINISTIC

		return std::make_tuple(r, s);
	}


	template<HashType _HashType>
	std::tuple<
		BigNum /* r */,
		BigNum /* s */
	>
	SignInBigNum(
		const Hash<_HashType>& hash,
		RbgInterface& rand
	) const
	{
		return SignInBigNum(_HashType, CtnFullR(hash), rand);
	}


	/**
	 * @brief	Derive shared key
	 *
	 * @param  	pubKey	The public key.
	 * @param	rand   	The random bit generator.
	 *
	 * @return The share key in BigNum
	 */
	template<typename _pub_Trait>
	BigNum DeriveSharedKeyInBigNum(
		const EcPublicKeyBase<_pub_Trait>& pubKey,
		RbgInterface& rand
	) const
	{
		const mbedtls_ecp_keypair& ecCtx    = GetEcContextRef();
		const mbedtls_ecp_keypair& pubEcCtx = pubKey.GetEcContextRef();

		EcGroup<> ecGrp = CopyGroup();

		BigNum res;
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			EcKeyPairBase::DeriveSharedKeyInBigNum,
			mbedtls_ecdh_compute_shared,
			ecGrp.Get(),
			res.Get(),
			&(Internal::GetQFromEcPair(pubEcCtx)),
			&(Internal::GetDFromEcPair(ecCtx)),
			&RbgInterface::CallBack,
			&rand
		);

		return res;
	}


protected:


	explicit EcKeyPairBase(_Base&& base) :
		_Base::EcPublicKeyBase(std::forward<_Base>(base))
	{}


	template<typename _FilterFunc>
	EcKeyPairBase(
		PKeyBase<PKObjTrait>&& other,
		_FilterFunc filterFunc
	) :
		_Base::EcPublicKeyBase(
			std::forward<PKeyBase<PKObjTrait> >(other),
			filterFunc
		)
	{}


}; // class EcKeyPairBase


namespace Internal
{


template<typename _PKObjTrait>
inline PKeyBase<_PKObjTrait>&& PKeyBaseRRefFilter(
	PKeyBase<_PKObjTrait>&& other,
	EcType reqEcType,
	PKeyType reqKeyType
)
{
	PKeyBase<_PKObjTrait>&& firstPass = PKeyBaseRRefFilter(
		std::forward<PKeyBase<_PKObjTrait> >(other),
		PKeyAlgmCat::EC,
		reqKeyType
	);

	const mbedtls_ecp_keypair* ecCtx = mbedtls_pk_ec(*(firstPass.Get()));
	if (ecCtx == nullptr)
	{
		throw InvalidArgumentException(
			"Internal::PKeyBaseRRefFilter"
			" - The given key does not contain a EC key context"
		);
	}

	const mbedtls_ecp_group& ecGrp = Internal::GetGroupFromEcPair(*ecCtx);
	if (ToEcType(ecGrp.id) != reqEcType)
	{
		throw InvalidArgumentException(
			"Internal::PKeyBaseRRefFilter"
			" - The curve type of given EC key doesn't match the required one"
		);
	}

	return std::forward<PKeyBase<_PKObjTrait> >(firstPass);
}


template<
	EcType   _ReqEcType,
	PKeyType _ReqKeyType,
	typename _PKObjTrait
>
inline PKeyBase<_PKObjTrait>&& PKeyBaseRRefFilter(
	PKeyBase<_PKObjTrait>&& other
)
{
	return PKeyBaseRRefFilter(
		std::forward<PKeyBase<_PKObjTrait> >(other),
		_ReqEcType,
		_ReqKeyType
	);
}


} // namespace Internal


template<
	EcType _ecType,
	typename _PKObjTrait = DefaultPKeyObjTrait,
	enable_if_t<
		std::is_same<
			typename _PKObjTrait::CObjType,
			mbedtls_pk_context
		>::value,
	int> = 0
>
class EcPublicKey : public EcPublicKeyBase<_PKObjTrait>
{
public: // Types and static members:

	using PKObjTrait = _PKObjTrait;
	using _Base = EcPublicKeyBase<_PKObjTrait>;
	using EcPublicKeyOwnerType = EcPublicKey<_ecType, DefaultPKeyObjTrait>;

	/**
	 * @brief The EC type used by this class.
	 *
	 */
	static constexpr EcType sk_ecType = _ecType;

	/**
	 * @brief The size of the key in Bytes.
	 *
	 */
	static constexpr size_t sk_kSize = GetCurveByteSize(sk_ecType);

	/**
	 * @brief The size of the key in Bytes that fits the MPI array.
	 *
	 */
	static constexpr size_t sk_kSizeFitsMpi =
		GetCurveByteSizeFitsMpi(sk_ecType);

	/**
	 * @brief The type of array used to store the EC Key components (key,
	 *        signature, and shared secret).
	 *
	 */
	using KArray = std::array<uint8_t, sk_kSize>;

	/**
	 * @brief The type of array used to store the EC Key components (key,
	 *        signature, and shared secret).
	 *
	 */
	using KArrayFitsMPI = std::array<uint8_t, sk_kSizeFitsMpi>;


	/**
	 * @brief	Move constructor that moves a general PKeyBase object to EC
	 *          Key pair. If it failed, the \c other will remain the same.
	 *
	 * @exception InvalidArgumentException Thrown when the given object is
	 *                                     not a EC private key.
	 * @exception mbedTLSRuntimeError Thrown when mbed TLS C function call failed.
	 *
	 * @param	other	The PKeyBase instance to convert.
	 */
	static EcPublicKey<sk_ecType, PKObjTrait> FromPKeyBase(
		PKeyBase<PKObjTrait>&& other
	)
	{
		return EcPublicKey<sk_ecType, PKObjTrait>(
			std::forward<PKeyBase<PKObjTrait> >(other)
		);
	}


	/**
	 * @brief Construct a EC public key from a given PEM string.
	 *
	 * @param pem PEM string in std::string
	 */
	static EcPublicKeyOwnerType FromPEM(const std::string& pem)
	{
		using _OwnerTrait = typename EcPublicKeyOwnerType::PKObjTrait;
		return EcPublicKeyOwnerType::FromPKeyBase(
			PKeyBase<_OwnerTrait>::FromPEM(pem)
		);
	}


	/**
	 * @brief Construct a EC public key from a given DER bytes.
	 *
	 * @param der DER bytes referenced by ContCtnReadOnlyRef
	 */
	template<typename _SecCtnType>
	static EcPublicKeyOwnerType FromDER(
		const ContCtnReadOnlyRef<_SecCtnType, false>& der
	)
	{
		using _OwnerTrait = typename EcPublicKeyOwnerType::PKObjTrait;
		return EcPublicKeyOwnerType::FromPKeyBase(
			PKeyBase<_OwnerTrait>::FromDER(der)
		);
	}


	template<typename _other_ObjTraits>
	static EcPublicKeyOwnerType FromDeepCopy(
		const EcPublicKey<sk_ecType, _other_ObjTraits>& other
	)
	{
		auto cpy = _Base::FromDeepCopy(other);

		if (
			!cpy.IsNull() &&
			!cpy.IsEcContextNull() &&
			cpy.GetEcType() != sk_ecType
		)
		{
			throw InvalidArgumentException(
				"EcPublicKey::FromDeepCopy - "
				"Given EC key type doesn't match the required one"
			);
		}

		return EcPublicKeyOwnerType(std::move(cpy));
	}


	/**
	 * @brief Construct a new EC key object that borrows the C object.
	 *
	 * @param ptr pointer to the C object to be borrowed.
	 */
	template<
		typename _dummy_PKTrait = PKObjTrait,
		enable_if_t<_dummy_PKTrait::sk_isBorrower, int> = 0
	>
	static EcPublicKey<sk_ecType, _dummy_PKTrait> Borrow(
		mbedtls_pk_context* ptr
	)
	{
		auto pk = PKeyBase<_dummy_PKTrait>(ptr);

		return FromPKeyBase(std::move(pk));
	}


public:

	template<
		typename _dummy_PKTrait = PKObjTrait,
		enable_if_t<!_dummy_PKTrait::sk_isBorrower, int> = 0
	>
	explicit EcPublicKey(EcType) :
		_Base::EcPublicKeyBase(sk_ecType)
	{}


	/**
	 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
	 *
	 * @exception None No exception thrown
	 * @param rhs The other EC public key instance.
	 */
	EcPublicKey(EcPublicKey&& rhs) noexcept :
		_Base::EcPublicKeyBase(std::forward<_Base>(rhs)) //noexcept
	{}


	/**
	 * @brief	Move constructor for PKeyBase.
	 *          If it failed, the \c other will remain the same.
	 *
	 * @exception InvalidArgumentException Thrown when the given object is
	 *                                     not a EC private key.
	 * @exception mbedTLSRuntimeError Thrown when mbed TLS C function call failed.
	 *
	 * @param	other	The PKeyBase instance to convert.
	 */
	explicit EcPublicKey(PKeyBase<PKObjTrait>&& other) :
		_Base::EcPublicKeyBase(
			std::forward<PKeyBase<PKObjTrait> >(other),
			Internal::PKeyBaseRRefFilter<
				sk_ecType,
				PKeyType::Public,
				PKObjTrait
			>
		)
	{}


	// LCOV_EXCL_START
	virtual ~EcPublicKey() = default;
	// LCOV_EXCL_STOP


	/**
	 * @brief	Move assignment operator
	 *
	 * @param	rhs	The right hand side.
	 *
	 * @return	A reference to this object.
	 */
	EcPublicKey& operator=(EcPublicKey&& rhs) noexcept
	{
		_Base::operator=(std::forward<_Base>(rhs)); //noexcept

		return *this;
	}


	using _Base::NullCheck;


	/**
	 * @brief Check if the current instance is holding a null pointer for
	 *        the mbedTLS object. If so, exception will be thrown. Helper
	 *        function to be called before accessing the mbedTLS object.
	 *
	 * @exception InvalidObjectException Thrown when the current instance is
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 */
	virtual void NullCheck() const override
	{
		_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(EcPublicKey));
	}


	/**
	 * @brief	Gets Elliptic Curve type
	 *
	 * @return	The Elliptic Curve type.
	 */
	virtual EcType GetEcType() const override
	{
		return sk_ecType;
	}


protected:


	explicit EcPublicKey(_Base&& base) :
		_Base::EcPublicKeyBase(std::forward<_Base>(base))
	{}


}; // class EcPublicKey



template<
	EcType _ecType,
	typename _PKObjTrait = DefaultPKeyObjTrait,
	enable_if_t<
		std::is_same<
			typename _PKObjTrait::CObjType,
			mbedtls_pk_context
		>::value,
		int
	> = 0
>
class EcKeyPair : public EcKeyPairBase<_PKObjTrait>
{
public: // Types and static members:

	using PKObjTrait = _PKObjTrait;
	using _Base = EcKeyPairBase<_PKObjTrait>;
	using EcKeyPairOwnerType = EcKeyPair<_ecType, _PKObjTrait>;

	/**
	 * @brief The EC type used by this class.
	 *
	 */
	static constexpr EcType sk_ecType = _ecType;

	/**
	 * @brief The size of the key in Bytes.
	 *
	 */
	static constexpr size_t sk_kSize = GetCurveByteSize(sk_ecType);

	/**
	 * @brief The size of the key in Bytes that fits the MPI array.
	 *
	 */
	static constexpr size_t sk_kSizeFitsMpi =
		GetCurveByteSizeFitsMpi(sk_ecType);

	/**
	 * @brief The type of array used to store the EC Key components (key,
	 *        signature, and shared secret).
	 *
	 */
	using KArray = std::array<uint8_t, sk_kSize>;

	/**
	 * @brief The type of array used to store the EC Key components (key,
	 *        signature, and shared secret).
	 *
	 */
	using KArrayFitsMPI = std::array<uint8_t, sk_kSizeFitsMpi>;

	/**
	 * @brief The type of array used to store the EC Key secret components (key,
	 *        signature, and shared secret).
	 *
	 */
	using KSecArray = SecretArray<uint8_t, sk_kSize>;


	/**
	 * @brief	Move constructor that moves a general PKeyBase object to EC
	 *          Key pair. If it failed, the \c other will remain the same.
	 *
	 * @exception InvalidArgumentException Thrown when the given object is
	 *                                     not a EC private key.
	 * @exception mbedTLSRuntimeError Thrown when mbed TLS C function call failed.
	 *
	 * @param	other	The PKeyBase instance to convert.
	 */
	static EcKeyPair<sk_ecType, PKObjTrait> FromPKeyBase(
		PKeyBase<PKObjTrait>&& other
	)
	{
		return EcKeyPair<sk_ecType, PKObjTrait>(
			std::forward<PKeyBase<PKObjTrait> >(other)
		);
	}


	/**
	 * @brief Construct a EC key pair from a given PEM string.
	 *
	 * @param pem PEM string in SecretString
	 */
	static EcKeyPairOwnerType FromPEM(
		const SecretString& pem,
		RbgInterface& rand
	)
	{
		using _OwnerTrait = typename EcKeyPairOwnerType::PKObjTrait;
		return EcKeyPairOwnerType::FromPKeyBase(
			PKeyBase<_OwnerTrait>::FromPEM(pem, rand)
		);
	}


	/**
	 * @brief Construct a EC key pair from a given DER bytes.
	 *
	 * @param der DER bytes referenced by ContCtnReadOnlyRef
	 */
	template<typename _SecCtnType>
	static EcKeyPairOwnerType FromDER(
		const ContCtnReadOnlyRef<_SecCtnType, true>& der,
		RbgInterface& rand
	)
	{
		using _OwnerTrait = typename EcKeyPairOwnerType::PKObjTrait;
		return EcKeyPairOwnerType::FromPKeyBase(
			PKeyBase<_OwnerTrait>::FromDER(der, rand)
		);
	}


	static EcKeyPairOwnerType Generate(
		RbgInterface& rand
	)
	{
		return EcKeyPairOwnerType(
			_Base::Generate(sk_ecType, rand)
		);
	}


	template<typename _other_ObjTraits>
	static EcKeyPairOwnerType FromDeepCopy(
		const EcKeyPair<sk_ecType, _other_ObjTraits>& other
	)
	{
		auto cpy = _Base::FromDeepCopy(other);

		if (
			!cpy.IsNull() &&
			!cpy.IsEcContextNull() &&
			cpy.GetEcType() != sk_ecType
		)
		{
			throw InvalidArgumentException(
				"EcKeyPair::FromDeepCopy - "
				"Given EC key type doesn't match the required one"
			);
		}

		return EcKeyPairOwnerType(std::move(cpy));
	}


	/**
	 * @brief Construct a new EC key object that borrows the C object.
	 *
	 * @param ptr pointer to the C object to be borrowed.
	 */
	template<
		typename _dummy_PKTrait = PKObjTrait,
		enable_if_t<_dummy_PKTrait::sk_isBorrower, int> = 0
	>
	static EcKeyPair<sk_ecType, _dummy_PKTrait> Borrow(
		mbedtls_pk_context* ptr
	)
	{
		auto pk = PKeyBase<_dummy_PKTrait>(ptr);

		return FromPKeyBase(std::move(pk));
	}


public:

	template<
		typename _dummy_PKTrait = PKObjTrait,
		enable_if_t<!_dummy_PKTrait::sk_isBorrower, int> = 0
	>
	explicit EcKeyPair(EcType) :
		_Base::EcKeyPairBase(sk_ecType)
	{}


	/**
	 * @brief	Move constructor for PKeyBase.
	 *          If it failed, the \c other will remain the same.
	 *
	 * @exception InvalidArgumentException Thrown when the given object is
	 *                                     not a EC private key.
	 * @exception mbedTLSRuntimeError Thrown when mbed TLS C function call failed.
	 *
	 * @param	other	The PKeyBase instance to convert.
	 */
	explicit EcKeyPair(PKeyBase<PKObjTrait>&& other) :
		_Base::EcKeyPairBase(
			std::forward<PKeyBase<PKObjTrait> >(other),
			Internal::PKeyBaseRRefFilter<
				sk_ecType,
				PKeyType::Private,
				PKObjTrait
			>
		)
	{}


	/**
	 * @brief	Move constructor
	 *
	 * @param	rhs	The right hand side.
	 */
	EcKeyPair(EcKeyPair&& rhs) noexcept :
		_Base::EcKeyPairBase(std::forward<_Base>(rhs)) //noexcept
	{}


	// LCOV_EXCL_START
	virtual ~EcKeyPair() = default;
	// LCOV_EXCL_STOP


	/**
	 * @brief	Move assignment operator
	 *
	 * @param	rhs	The right hand side.
	 *
	 * @return	A reference to this object.
	 */
	EcKeyPair& operator=(EcKeyPair&& rhs) noexcept
	{
		_Base::operator=(std::forward<_Base>(rhs)); //noexcept

		return *this;
	}


	using _Base::NullCheck;


	/**
	 * @brief Check if the current instance is holding a null pointer for
	 *        the mbedTLS object. If so, exception will be thrown. Helper
	 *        function to be called before accessing the mbedTLS object.
	 *
	 * @exception InvalidObjectException Thrown when the current instance is
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 */
	virtual void NullCheck() const override
	{
		_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(EcKeyPair));
	}

	/**
	 * @brief	Gets Elliptic Curve type
	 *
	 * @return	The Elliptic Curve type.
	 */
	virtual EcType GetEcType() const override
	{
		return sk_ecType;
	}


protected:


	explicit EcKeyPair(_Base&& base) :
		_Base::EcKeyPairBase(std::forward<_Base>(base))
	{}


}; // class EcKeyPair


} // namespace mbedTLScpp
