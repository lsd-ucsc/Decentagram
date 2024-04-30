#pragma once

#include "ObjectBase.hpp"

#include <mbedtls/gcm.h>

#include "Common.hpp"
#include "Exceptions.hpp"
#include "Container.hpp"
#include "CipherBase.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	/**
	 * @brief GCM object allocator.
	 *
	 */
	struct GcmObjAllocator : DefaultAllocBase
	{
		typedef mbedtls_gcm_context      CObjType;

		using DefaultAllocBase::NewObject;
		using DefaultAllocBase::DelObject;

		static void Init(CObjType* ptr)
		{
			return mbedtls_gcm_init(ptr);
		}

		static void Free(CObjType* ptr) noexcept
		{
			return mbedtls_gcm_free(ptr);
		}
	};

	/**
	 * @brief GCM object trait.
	 *
	 */
	using DefaultGcmObjTrait = ObjTraitBase<GcmObjAllocator,
											 false,
											 false>;

	template<typename _GcmObjTrait = DefaultGcmObjTrait,
		enable_if_t<std::is_same<typename _GcmObjTrait::CObjType, mbedtls_gcm_context>::value, int> = 0>
	class GcmBase : public ObjectBase<_GcmObjTrait>
	{
	public: // Static members:

		using GcmObjTrait = _GcmObjTrait;
		using _Base        = ObjectBase<GcmObjTrait>;

	public:

		template<typename _SecCtnType>
		GcmBase(const ContCtnReadOnlyRef<_SecCtnType, true>& key, CipherType cType) :
			_Base::ObjectBase()
		{
			mbedtls_cipher_id_t cipherId;
			switch (cType)
			{
			case CipherType::AES:
				cipherId = mbedtls_cipher_id_t::MBEDTLS_CIPHER_ID_AES;
				break;
			default:
				throw InvalidArgumentException("mbedTLScpp::GcmBase::GcmBase - Invalid cipher type.");
			}

			MBEDTLSCPP_MAKE_C_FUNC_CALL(
				GcmBase::GcmBase,
				mbedtls_gcm_setkey,
				NonVirtualGet(), cipherId,
				key.BeginBytePtr(), static_cast<unsigned int>(key.GetRegionSize() * gsk_bitsPerByte));
		}

		/**
		 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
		 *
		 * @exception None No exception thrown
		 * @param rhs The other GcmBase instance.
		 */
		GcmBase(GcmBase&& rhs) noexcept :
			_Base::ObjectBase(std::forward<_Base>(rhs)) //noexcept
		{}

		GcmBase(const GcmBase& rhs) = delete;

		// LCOV_EXCL_START
		virtual ~GcmBase() = default;
		// LCOV_EXCL_STOP

		/**
		 * @brief Move assignment. The `rhs` will be empty/null afterwards.
		 *
		 * @exception None No exception thrown
		 * @param rhs The other GcmBase instance.
		 * @return GcmBase& A reference to this instance.
		 */
		GcmBase& operator=(GcmBase&& rhs) noexcept
		{
			_Base::operator=(std::forward<_Base>(rhs)); //noexcept

			return *this;
		}

		GcmBase& operator=(const GcmBase& other) = delete;


		using _Base::Get;
		using _Base::NonVirtualGet;
		using _Base::NullCheck;
		using _Base::Swap;


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
			_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(GcmBase));
		}

		template<typename _DataCtnType, bool _DataSec,
			typename _IvCtnType, bool _IvSec,
			typename _AddCtnType, bool _AddSec>
		std::pair<std::vector<uint8_t>, std::array<uint8_t, 16> > Encrypt(
			const ContCtnReadOnlyRef<_DataCtnType, _DataSec>& data,
			const ContCtnReadOnlyRef<_IvCtnType,   _IvSec  >& iv,
			const ContCtnReadOnlyRef<_AddCtnType,  _AddSec >& add
		)
		{
			NullCheck();

			std::vector<uint8_t> encRes(data.GetRegionSize());
			std::array<uint8_t, 16> tag;

			MBEDTLSCPP_MAKE_C_FUNC_CALL(
				Gcm::Encrypt,
				mbedtls_gcm_crypt_and_tag,
				Get(),
				MBEDTLS_GCM_ENCRYPT, data.GetRegionSize(),
				iv.BeginBytePtr()  , iv.GetRegionSize(),
				add.BeginBytePtr() , add.GetRegionSize(),
				data.BeginBytePtr(),
				encRes.data(),
				tag.size(), tag.data()
			);

			return std::make_pair(encRes, tag);
		}

		template<typename _DataCtnType, bool _DataSec,
			typename _IvCtnType, bool _IvSec,
			typename _AddCtnType, bool _AddSec,
			typename _TagCtnType>
		SecretVector<uint8_t> Decrypt(
			const ContCtnReadOnlyRef<_DataCtnType, _DataSec>& data,
			const ContCtnReadOnlyRef<_IvCtnType,   _IvSec  >& iv,
			const ContCtnReadOnlyRef<_AddCtnType,  _AddSec >& add,
			const ContCtnReadOnlyRef<_TagCtnType,  false   >& tag
		)
		{
			NullCheck();

			SecretVector<uint8_t> decRes(data.GetRegionSize());

			MBEDTLSCPP_MAKE_C_FUNC_CALL(
				Gcm::Decrypt,
				mbedtls_gcm_auth_decrypt,
				Get(),
				data.GetRegionSize(),
				iv.BeginBytePtr()  , iv.GetRegionSize(),
				add.BeginBytePtr() , add.GetRegionSize(),
				tag.BeginBytePtr() , tag.GetRegionSize(),
				data.BeginBytePtr(),
				decRes.data()
			);

			return decRes;
		}

	};

	template<CipherType _cipherType, size_t _bitSize>
	class Gcm : public GcmBase<DefaultGcmObjTrait>
	{
	public: // static member:

		using _Base = GcmBase<DefaultGcmObjTrait>;

		static constexpr CipherType sk_cipherType = _cipherType;
		static constexpr size_t sk_keyBitSize = _bitSize;
		static constexpr CipherSizedType sk_cipherSizedType = GetCipherSizedType(sk_cipherType, sk_keyBitSize);

		static_assert(
			sk_cipherSizedType == CipherSizedType::AES_128 ||
			sk_cipherSizedType == CipherSizedType::AES_192 ||
			sk_cipherSizedType == CipherSizedType::AES_256,
			"The given cipher type or key size is not supported."
		);

	public:

		template<typename _SecCtnType>
		Gcm(const ContCtnReadOnlyRef<_SecCtnType, true>& key) :
			_Base::GcmBase(CheckInputKey(key), sk_cipherType)
		{}

		/**
		 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
		 *
		 * @exception None No exception thrown
		 * @param rhs The other Gcm instance.
		 */
		Gcm(Gcm&& rhs) noexcept :
			_Base::GcmBase(std::forward<_Base>(rhs)) //noexcept
		{}

		Gcm(const Gcm& rhs) = delete;

		// LCOV_EXCL_START
		virtual ~Gcm() = default;
		// LCOV_EXCL_STOP

		/**
		 * @brief Move assignment. The `rhs` will be empty/null afterwards.
		 *
		 * @exception None No exception thrown
		 * @param rhs The other Gcm instance.
		 * @return Gcm& A reference to this instance.
		 */
		Gcm& operator=(Gcm&& rhs) noexcept
		{
			_Base::operator=(std::forward<_Base>(rhs)); //noexcept

			return *this;
		}

		Gcm& operator=(const Gcm& other) = delete;


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
		virtual void NullCheck() const
		{
			_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(Gcm));
		}

	private:

		template<typename _SecCtnType>
		static const ContCtnReadOnlyRef<_SecCtnType, true>& CheckInputKey(const ContCtnReadOnlyRef<_SecCtnType, true>& key)
		{
			if (key.GetRegionSize() * gsk_bitsPerByte != sk_keyBitSize)
			{
				throw InvalidArgumentException("The given key size doesn't match the declared cipher size.");
			}
			return key;
		}
	};
}
