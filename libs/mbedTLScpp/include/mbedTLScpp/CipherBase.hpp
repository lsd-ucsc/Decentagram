#pragma once

#include "ObjectBase.hpp"

#include <mbedtls/cipher.h>

#include "Common.hpp"
#include "Exceptions.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{

	/** @brief	Values that represent cipher types */
	enum class CipherType
	{
		AES,
	};

	/** @brief	Values that represent cipher types combined with different key size */
	enum class CipherSizedType
	{
		AES_128,
		AES_192,
		AES_256,
	};

	/** @brief	Values that represent cipher modes */
	enum class CipherMode
	{
		ECB,
		CBC,
		CTR,
		GCM,
	};

	/**
	 * @brief Get the Cipher Sized Type
	 *
	 * @param type    The type of the cipher.
	 * @param bitSize The size of the cipher key.
	 * @return constexpr CipherSizedType
	 */
	inline constexpr CipherSizedType GetCipherSizedType(CipherType type, size_t bitSize)
	{
		return (type == CipherType::AES ?
					(bitSize == 128 ?
						(CipherSizedType::AES_128) :
					(bitSize == 192 ?
						(CipherSizedType::AES_192) :
					(bitSize == 256 ?
						(CipherSizedType::AES_256) :
						throw InvalidArgumentException("The given bit size is not supported by AES cipher.")
					))) :
					throw InvalidArgumentException("The given cipher type is not supported."));
	}
	static_assert(GetCipherSizedType(CipherType::AES, 128) == CipherSizedType::AES_128, "Programming error.");
	static_assert(GetCipherSizedType(CipherType::AES, 192) == CipherSizedType::AES_192, "Programming error.");
	static_assert(GetCipherSizedType(CipherType::AES, 256) == CipherSizedType::AES_256, "Programming error.");

	/**
	 * @brief Get the mbed TLS Cipher Type
	 *
	 * @param sizedType The cipher types combined with key size
	 * @param mode      The cipher mode
	 * @return mbedtls_cipher_type_t the mbedtls_cipher_type_t in mbed TLS.
	 */
	inline constexpr mbedtls_cipher_type_t GetMbedTlsCipherType(CipherSizedType sizedType, CipherMode mode)
	{
		return (sizedType == CipherSizedType::AES_128 ?
					(mode == CipherMode::ECB ?
						mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_128_ECB :
					(mode == CipherMode::CBC ?
						mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_128_CBC :
					(mode == CipherMode::CTR ?
						mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_128_CTR :
					(mode == CipherMode::GCM ?
						mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_128_GCM :
						throw InvalidArgumentException("The given cipher mode is not supported by AES_128")
					)))) :
				(sizedType == CipherSizedType::AES_192 ?
					(mode == CipherMode::ECB ?
						mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_192_ECB :
					(mode == CipherMode::CBC ?
						mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_192_CBC :
					(mode == CipherMode::CTR ?
						mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_192_CTR :
					(mode == CipherMode::GCM ?
						mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_192_GCM :
						throw InvalidArgumentException("The given cipher mode is not supported by AES_192")
					)))) :
				(sizedType == CipherSizedType::AES_256 ?
					(mode == CipherMode::ECB ?
						mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_256_ECB :
					(mode == CipherMode::CBC ?
						mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_256_CBC :
					(mode == CipherMode::CTR ?
						mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_256_CTR :
					(mode == CipherMode::GCM ?
						mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_256_GCM :
						throw InvalidArgumentException("The given cipher mode is not supported by AES_256")
					)))) :
				throw InvalidArgumentException("The given sized type is not supported")
				)));
	}
	static_assert(GetMbedTlsCipherType(CipherSizedType::AES_128, CipherMode::ECB) == mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_128_ECB, "Programming error.");
	static_assert(GetMbedTlsCipherType(CipherSizedType::AES_128, CipherMode::CBC) == mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_128_CBC, "Programming error.");
	static_assert(GetMbedTlsCipherType(CipherSizedType::AES_128, CipherMode::CTR) == mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_128_CTR, "Programming error.");
	static_assert(GetMbedTlsCipherType(CipherSizedType::AES_128, CipherMode::GCM) == mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_128_GCM, "Programming error.");
	static_assert(GetMbedTlsCipherType(CipherSizedType::AES_192, CipherMode::ECB) == mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_192_ECB, "Programming error.");
	static_assert(GetMbedTlsCipherType(CipherSizedType::AES_192, CipherMode::CBC) == mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_192_CBC, "Programming error.");
	static_assert(GetMbedTlsCipherType(CipherSizedType::AES_192, CipherMode::CTR) == mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_192_CTR, "Programming error.");
	static_assert(GetMbedTlsCipherType(CipherSizedType::AES_192, CipherMode::GCM) == mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_192_GCM, "Programming error.");
	static_assert(GetMbedTlsCipherType(CipherSizedType::AES_256, CipherMode::ECB) == mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_256_ECB, "Programming error.");
	static_assert(GetMbedTlsCipherType(CipherSizedType::AES_256, CipherMode::CBC) == mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_256_CBC, "Programming error.");
	static_assert(GetMbedTlsCipherType(CipherSizedType::AES_256, CipherMode::CTR) == mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_256_CTR, "Programming error.");
	static_assert(GetMbedTlsCipherType(CipherSizedType::AES_256, CipherMode::GCM) == mbedtls_cipher_type_t::MBEDTLS_CIPHER_AES_256_GCM, "Programming error.");

	/**
	 * @brief Get the Cipher Info provided by the mbed TLS.
	 *
	 * @param type    The type of the cipher.
	 * @param bitSize The size of the cipher key.
	 * @param mode    The cipher mode
	 * @return const mbedtls_cipher_info_t& The cipher info provided by mbed TLS.
	 */
	inline const mbedtls_cipher_info_t& GetCipherInfo(CipherType type, size_t bitSize, CipherMode mode)
	{
		const mbedtls_cipher_info_t* res = mbedtls_cipher_info_from_type(GetMbedTlsCipherType(GetCipherSizedType(type, bitSize), mode));

		if (res != nullptr)
		{
			return *res;
		}
		throw InvalidArgumentException("Cipher type given is not supported.");
	}

	/**
	 * @brief Get the Cipher Block Size
	 *
	 * @param type    The type of the cipher.
	 * @param bitSize The size of the cipher key.
	 * @param mode    The cipher mode
	 * @return size_t The size of the cipher block.
	 */
	inline constexpr size_t GetCipherBlockSize(
		CipherType type,
		size_t     /* bitSize */,
		CipherMode /* mode */
	)
	{
		return (type == CipherType::AES ?
					(16) :
					throw InvalidArgumentException(
						"The given cipher type is not supported."
					)
				);
	}
	static_assert(
		GetCipherBlockSize(CipherType::AES, 128, CipherMode::ECB) == 16,
		"Implementation error."
	);
	static_assert(
		GetCipherBlockSize(CipherType::AES, 192, CipherMode::ECB) == 16,
		"Implementation error."
	);
	static_assert(
		GetCipherBlockSize(CipherType::AES, 256, CipherMode::ECB) == 16,
		"Implementation error."
	);
	static_assert(
		GetCipherBlockSize(CipherType::AES, 128, CipherMode::GCM) == 16,
		"Implementation error."
	);
	static_assert(
		GetCipherBlockSize(CipherType::AES, 192, CipherMode::GCM) == 16,
		"Implementation error."
	);
	static_assert(
		GetCipherBlockSize(CipherType::AES, 256, CipherMode::GCM) == 16,
		"Implementation error."
	);

	/**
	 * @brief The allocator of cipher objects.
	 *
	 */
	struct CipherAllocator : DefaultAllocBase
	{
		typedef mbedtls_cipher_context_t      CObjType;

		using DefaultAllocBase::NewObject;
		using DefaultAllocBase::DelObject;

		static void Init(CObjType* ptr)
		{
			return mbedtls_cipher_init(ptr);
		}

		static void Free(CObjType* ptr) noexcept
		{
			return mbedtls_cipher_free(ptr);
		}
	};

	/**
	 * @brief The trait for normal cipher objects.
	 *
	 */
	using DefaultCipherObjTrait = ObjTraitBase<CipherAllocator,
									false,
									false>;

	/**
	 * @brief The base class for cipher objects.
	 *
	 * @tparam _CipherTrait The trait used for the cipher object.
	 */
	template<typename _CipherTrait = DefaultCipherObjTrait,
		enable_if_t<std::is_same<typename _CipherTrait::CObjType, mbedtls_cipher_context_t>::value, int> = 0>
	class CipherBase : public ObjectBase<_CipherTrait>
	{
	public: // Static members:

		using CipherBaseTrait = _CipherTrait;
		using _Base           = ObjectBase<CipherBaseTrait>;

	public:

		CipherBase() = delete;

		/**
		 * @brief Construct a new Cipher Base object
		 *
		 * @exception mbedTLSRuntimeError  Thrown when mbed TLS C function call failed.
		 * @exception std::bad_alloc       Thrown when memory allocation failed.
		 * @param cipherInfo The cipher info provided by mbed TLS library.
		 */
		CipherBase(const mbedtls_cipher_info_t& cipherInfo) :
			_Base::ObjectBase()
		{
			MBEDTLSCPP_MAKE_C_FUNC_CALL(CipherBase::CipherBase, mbedtls_cipher_setup, NonVirtualGet(), &cipherInfo);
		}

		/**
		 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
		 *
		 * @param rhs The other CipherBase instance.
		 */
		CipherBase(CipherBase&& rhs) noexcept :
			_Base::ObjectBase(std::forward<_Base>(rhs)) //noexcept
		{}

		CipherBase(const CipherBase& rhs) = delete;

		// LCOV_EXCL_START
		/**
		 * @brief Destroy the Cipher Base object
		 *
		 */
		virtual ~CipherBase() = default;
		// LCOV_EXCL_STOP

		/**
		 * @brief Move assignment. The `rhs` will be empty/null afterwards.
		 *
		 * @param rhs The other CipherBase instance.
		 * @return CipherBase& A reference to this instance.
		 */
		CipherBase& operator=(CipherBase&& rhs) noexcept
		{
			_Base::operator=(std::forward<_Base>(rhs)); //noexcept

			return *this;
		}

		CipherBase& operator=(const CipherBase& other) = delete;


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
			_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(CipherBase));
		}

		using _Base::Get;
		using _Base::NonVirtualGet;
	};
}
