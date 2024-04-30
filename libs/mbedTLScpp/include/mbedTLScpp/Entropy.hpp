#pragma once

#include "ObjectBase.hpp"
#include "EntropyInterfaces.hpp"

#include <mbedtls/entropy.h>

#include "Exceptions.hpp"

#include <memory>

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	/**
	 * @brief Normal Entropy allocator.
	 *
	 */
	struct EntropyAllocator : DefaultAllocBase
	{
		typedef mbedtls_entropy_context      CObjType;

		using DefaultAllocBase::NewObject;
		using DefaultAllocBase::DelObject;

		static void Init(CObjType* ptr)
		{
			return mbedtls_entropy_init(ptr);
		}

		static void Free(CObjType* ptr) noexcept
		{
			return mbedtls_entropy_free(ptr);
		}
	};

	/**
	 * @brief Normal Entropy Trait.
	 *
	 */
	using DefaultEntropyObjTrait = ObjTraitBase<EntropyAllocator,
									false,
									false>;

	/**
	 * @brief The Entropy object class
	 *
	 * @tparam _EntropyObjTrait The object trait used.
	 */
	template<typename _EntropyObjTrait = DefaultEntropyObjTrait,
			 enable_if_t<std::is_same<typename _EntropyObjTrait::CObjType, mbedtls_entropy_context>::value, int> = 0>
	class Entropy : public ObjectBase<_EntropyObjTrait>, public EntropyInterface
	{
	public: // Static members:

		using EntropyTrait = _EntropyObjTrait;
		using _Base        = ObjectBase<EntropyTrait>;
		using _BaseIntf    = EntropyInterface;

	public:

		/**
		 * @brief Construct a new Entropy object
		 *
		 */
		template<typename _dummy_trait = EntropyTrait, enable_if_t<!_dummy_trait::sk_isBorrower, int> = 0>
		Entropy() :
			_Base::ObjectBase()
		{}

		/**
		 * @brief Construct a new Entropy Object by borrowing a object pointer
		 *        owned by other. Thus, this object base DOES NOT own the object.
		 *
		 * @exception None No exception thrown
		 * @param ptr The pointer to the not null mbedTLS C object.
		 */
		template<typename _dummy_trait = EntropyTrait, enable_if_t<_dummy_trait::sk_isBorrower, int> = 0>
		Entropy(mbedtls_entropy_context* ptr) noexcept :
			_Base::ObjectBase(ptr)
		{}

		/**
		 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
		 *
		 * @param rhs The other Entropy instance.
		 */
		Entropy(Entropy&& rhs) noexcept :
			_Base::ObjectBase(std::forward<_Base>(rhs)) //noexcept
		{}

		Entropy(const Entropy& rhs) = delete;

		// LCOV_EXCL_START
		/** @brief	Destructor */
		virtual ~Entropy() = default;
		// LCOV_EXCL_STOP

		/**
		 * @brief Move assignment. The `rhs` will be empty/null afterwards.
		 *
		 * @param rhs The other Entropy instance.
		 * @return Entropy& A reference to this instance.
		 */
		Entropy& operator=(Entropy&& rhs) noexcept
		{
			_Base::operator=(std::forward<_Base>(rhs)); //noexcept

			return *this;
		}

		Entropy& operator=(const Entropy& other) = delete;


		using _Base::NullCheck;
		using _Base::Get;


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
			_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(Entropy));
		}

		/**
		 * @brief Get the internal pointer to the C mbed TLS object.
		 *
		 * @return mbedtls_entropy_context* The pointer to the C mbed TLS Object.
		 */
		virtual void* GetRawPtr() noexcept override
		{
			return _Base::Get();
		}

		/**
		 * @brief Get the internal const pointer to the C mbed TLS object.
		 *
		 * @return const mbedtls_entropy_context* The const pointer to the C mbed TLS Object.
		 */
		virtual const void* GetRawPtr() const noexcept override
		{
			return _Base::Get();
		}

		/**
		 * @brief Fill entropy into a given memory region.
		 *
		 * @param buf  The pointer to the beginning of the memory region.
		 * @param size The size of the memory region.
		 */
		virtual void FillEntropy(void* buf, const size_t size) override
		{
			NullCheck();

			MBEDTLSCPP_MAKE_C_FUNC_CALL(
				Entropy::FillEntropy,
				mbedtls_entropy_func,
				Get(), static_cast<unsigned char *>(buf), size
			);
		}
	};

	/**
	 * @brief The Entropy object trait for borrowed objects.
	 *
	 */
	using BorrowedEntropyTrait = ObjTraitBase<BorrowAllocBase<mbedtls_entropy_context>,
												true,
												false>;

	/**
	 * @brief Get the Shared Entropy object. Different DRBGs can share the same
	 *        entropy. This function will construct the static Entropy object at
	 *        the first call, and construct a borrowed entropy object and return
	 *        it in EntropyInterface pointer.
	 *
	 * @return std::unique_ptr<EntropyInterface> The pointer to the shared entropy object.
	 */
	inline std::unique_ptr<EntropyInterface> GetSharedEntropy()
	{
		static Entropy<> sharedEntropy;

		return Internal::make_unique<Entropy<BorrowedEntropyTrait> >(sharedEntropy.Get());
	}
}
