#pragma once

#include "ObjectBase.hpp"

#include <mbedtls/md.h>

#include "Common.hpp"
#include "Exceptions.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	/** @brief	Enum that represent hash types */
	enum class HashType
	{
		SHA224,
		SHA256,
		SHA384,
		SHA512,
	};

	/**
	 * @brief Get the size (in bytes) of a given Hash type.
	 *
	 * @param type The type of the hash
	 * @exception InvalidArgumentException Thrown when the given hash type is not supported.
	 * @return constexpr uint8_t The size in bytes
	 */
	inline constexpr uint8_t GetHashByteSize(HashType type)
	{
		return (type == HashType::SHA224 ?
		           (224 / gsk_bitsPerByte) :
			   (type == HashType::SHA256 ?
			       (256 / gsk_bitsPerByte) :
			   (type == HashType::SHA384 ?
			       (384 / gsk_bitsPerByte) :
			   (type == HashType::SHA512 ?
			       (512 / gsk_bitsPerByte) :
                   throw InvalidArgumentException("Hash type given is not supported.")
			   ))));
	}
	static_assert(GetHashByteSize(HashType::SHA224) == (224 / gsk_bitsPerByte), "Programming error.");
	static_assert(GetHashByteSize(HashType::SHA256) == (256 / gsk_bitsPerByte), "Programming error.");
	static_assert(GetHashByteSize(HashType::SHA384) == (384 / gsk_bitsPerByte), "Programming error.");
	static_assert(GetHashByteSize(HashType::SHA512) == (512 / gsk_bitsPerByte), "Programming error.");

	/**
	 * @brief Translating mbed TLS cpp's message digest type to mbed TLS's message
	 *        digest type.
	 *
	 * @param type mbed TLS cpp's message digest type.
	 * @exception InvalidArgumentException Thrown when the given hash type is not supported.
	 * @return constexpr mbedtls_md_type_t mbed TLS's message digest type.
	 */
	inline constexpr mbedtls_md_type_t GetMbedTlsMdType(HashType type)
	{
		return (type == HashType::SHA224 ?
		           mbedtls_md_type_t::MBEDTLS_MD_SHA224 :
			   (type == HashType::SHA256 ?
			       mbedtls_md_type_t::MBEDTLS_MD_SHA256 :
			   (type == HashType::SHA384 ?
			       mbedtls_md_type_t::MBEDTLS_MD_SHA384 :
			   (type == HashType::SHA512 ?
			       mbedtls_md_type_t::MBEDTLS_MD_SHA512 :
                   throw InvalidArgumentException("Hash type given is not supported.")
			   ))));
	}

	/**
	 * @brief Translating mbed TLS's message digest type to mbed TLS cpp's message
	 *        digest type.
	 *
	 * @param type mbed TLS's message digest type.
	 * @exception InvalidArgumentException Thrown when the given hash type is not supported.
	 * @return constexpr HashType mbed TLS cpp's message digest type.
	 */
	inline constexpr HashType GetHashType(mbedtls_md_type_t type)
	{
		return (type == mbedtls_md_type_t::MBEDTLS_MD_SHA224 ?
		           HashType::SHA224 :
			   (type == mbedtls_md_type_t::MBEDTLS_MD_SHA256 ?
			       HashType::SHA256 :
			   (type == mbedtls_md_type_t::MBEDTLS_MD_SHA384 ?
			       HashType::SHA384 :
			   (type == mbedtls_md_type_t::MBEDTLS_MD_SHA512 ?
			       HashType::SHA512 :
                   throw InvalidArgumentException("mbedtls_md_type_t given is not supported.")
			   ))));
	}

	/**
	 * @brief Get the md_info C object from mbed TLS, by using the HashType.
	 *
	 * @exception InvalidArgumentException Thrown when the given hash type is not supported.
	 * @param type The hash type
	 * @return const mbedtls_md_info_t& The reference to md_info C object from mbed TLS
	 */
	inline const mbedtls_md_info_t& GetMdInfo(HashType type)
	{
		const mbedtls_md_info_t* res = mbedtls_md_info_from_type(GetMbedTlsMdType(type));

		if (res != nullptr)
		{
			return *res;
		}
		throw InvalidArgumentException("Hash type given is not supported.");
	}

	/**
	 * @brief Normal MsgDigest allocator.
	 *
	 */
	struct MdAllocator : DefaultAllocBase
	{
		typedef mbedtls_md_context_t      CObjType;

		using DefaultAllocBase::NewObject;
		using DefaultAllocBase::DelObject;

		static void Init(CObjType* ptr)
		{
			return mbedtls_md_init(ptr);
		}

		static void Free(CObjType* ptr) noexcept
		{
			return mbedtls_md_free(ptr);
		}
	};

	/**
	 * @brief Normal MsgDigest Trait.
	 *
	 */
	using DefaultMdObjTrait = ObjTraitBase<MdAllocator,
									false,
									false>;

	/** @brief	Message Digest Base class. It will be further inherited by the
	 *          hash calculator and HMAC calculator.
	 */
	template<typename _MdObjTrait = DefaultMdObjTrait,
		enable_if_t<std::is_same<typename _MdObjTrait::CObjType, mbedtls_md_context_t>::value, int> = 0>
	class MsgDigestBase : public ObjectBase<_MdObjTrait>
	{
	public: // Static members:

		using MdBaseTrait = _MdObjTrait;
		using _Base       = ObjectBase<MdBaseTrait>;

	public:

		/**
		 * @brief Construct a new Msgessage Digest Base object.
		 *
		 * @exception mbedTLSRuntimeError  Thrown when mbed TLS C function call failed.
		 * @exception std::bad_alloc       Thrown when memory allocation failed.
		 * @param mdInfo   The md info provided by mbed TLS library.
		 * @param needHmac Is HMAC calculation needed?
		 */
		MsgDigestBase(const mbedtls_md_info_t& mdInfo, bool needHmac) :
			_Base::ObjectBase()
		{
			static_assert(false == 0, "The value of false is different with the one expected in mbedTLS.");

			MBEDTLSCPP_MAKE_C_FUNC_CALL(MsgDigestBase::MsgDigestBase, mbedtls_md_setup, NonVirtualGet(), &mdInfo, needHmac);
		}

		/**
		 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
		 *
		 * @param rhs The other MsgDigestBase instance.
		 */
		MsgDigestBase(MsgDigestBase&& rhs) noexcept :
			_Base::ObjectBase(std::forward<_Base>(rhs)) //noexcept
		{}

		MsgDigestBase(const MsgDigestBase& rhs) = delete;

		// LCOV_EXCL_START
		/** @brief	Destructor */
		virtual ~MsgDigestBase() = default;
		// LCOV_EXCL_STOP

		/**
		 * @brief Move assignment. The `rhs` will be empty/null afterwards.
		 *
		 * @param rhs The other MsgDigestBase instance.
		 * @return MsgDigestBase& A reference to this instance.
		 */
		MsgDigestBase& operator=(MsgDigestBase&& rhs) noexcept
		{
			_Base::operator=(std::forward<_Base>(rhs)); //noexcept

			return *this;
		}

		MsgDigestBase& operator=(const MsgDigestBase& other) = delete;


		using _Base::Get;
		using _Base::NonVirtualGet;
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
			_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(MsgDigestBase));
		}
	};

	static_assert(IsCppObjOfCtype<MsgDigestBase<>, mbedtls_md_context_t>::value == true, "Programming Error");
}
