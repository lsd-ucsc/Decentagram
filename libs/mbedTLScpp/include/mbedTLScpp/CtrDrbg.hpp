#pragma once

#include "ObjectBase.hpp"
#include "RandInterfaces.hpp"

#include "Common.hpp"
#include "Entropy.hpp"

#include <mbedtls/ctr_drbg.h>

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{

	/**
	 * @brief Ctr-DRBG allocator.
	 *
	 */
	struct CtrDrbgAllocator : DefaultAllocBase
	{
		typedef mbedtls_ctr_drbg_context      CObjType;

		using DefaultAllocBase::NewObject;
		using DefaultAllocBase::DelObject;

		static void Init(CObjType* ptr)
		{
			return mbedtls_ctr_drbg_init(ptr);
		}

		static void Free(CObjType* ptr) noexcept
		{
			return mbedtls_ctr_drbg_free(ptr);
		}
	};

	/**
	 * @brief Ctr-DRBG Trait.
	 *
	 */
	using DefaultCtrDrbgTrait = ObjTraitBase<CtrDrbgAllocator,
									false,
									false>;

	/**
	 * @brief Class for Ctr-DRBG
	 *
	 * @tparam _PredResist   Turns prediction resistance on or off
	 *                       (default to \c false/off )
	 * @tparam _EntropyLen   The amount of entropy grabbed on each seed or reseed
	 *                       (default to \c MBEDTLS_CTR_DRBG_ENTROPY_LEN )
	 * @tparam _ReseedInterv The reseed interval
	 *                       (default to \c MBEDTLS_CTR_DRBG_RESEED_INTERVAL )
	 */
	template<bool  _PredResist = false,
			size_t _EntropyLen = MBEDTLS_CTR_DRBG_ENTROPY_LEN,
			int    _ReseedInterv = MBEDTLS_CTR_DRBG_RESEED_INTERVAL>
	class CtrDrbg : public ObjectBase<DefaultCtrDrbgTrait>, public RbgInterface
	{
	public: // Static members:

		using _Base     = ObjectBase<DefaultCtrDrbgTrait>;
		using _BaseIntf = RbgInterface;

	public:

		/**
		 * @brief Construct a new Ctr Drbg object with a shared entropy object
		 *
		 */
		CtrDrbg() :
			CtrDrbg(GetSharedEntropy())
		{}

		/**
		 * @brief Construct a new Ctr Drbg object with the given entropy
		 *
		 * @param entropy The entropy given to use
		 */
		CtrDrbg(std::unique_ptr<EntropyInterface> entropy) :
			_Base::ObjectBase(),
			m_entropy(std::move(entropy))
		{
			MBEDTLSCPP_MAKE_C_FUNC_CALL(CtrDrbg::CtrDrbg, mbedtls_ctr_drbg_seed, NonVirtualGet(), &EntropyInterface::CallBack, m_entropy.get(), nullptr, 0);

			mbedtls_ctr_drbg_set_prediction_resistance(NonVirtualGet(), _PredResist ? MBEDTLS_CTR_DRBG_PR_ON : MBEDTLS_CTR_DRBG_PR_OFF);
			mbedtls_ctr_drbg_set_entropy_len(NonVirtualGet(), _EntropyLen);
			mbedtls_ctr_drbg_set_reseed_interval(NonVirtualGet(), _ReseedInterv);
		}

		/**
		 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
		 *
		 * @param rhs The other CtrDrbg instance.
		 */
		CtrDrbg(CtrDrbg&& rhs) noexcept :
			_Base::ObjectBase(std::forward<_Base>(rhs)),
			m_entropy(std::move(rhs.m_entropy))
		{}

		CtrDrbg(const CtrDrbg& rhs) = delete;

		// LCOV_EXCL_START
		/** @brief	Destructor */
		virtual ~CtrDrbg() = default;
		// LCOV_EXCL_STOP

		/**
		 * @brief Move assignment. The `rhs` will be empty/null afterwards.
		 *
		 * @param rhs The other CtrDrbg instance.
		 * @return CtrDrbg& A reference to this instance.
		 */
		CtrDrbg& operator=(CtrDrbg&& rhs) noexcept
		{
			_Base::operator=(std::forward<_Base>(rhs));
			m_entropy = std::move(rhs.m_entropy);

			return *this;
		}

		CtrDrbg& operator=(const CtrDrbg& other) = delete;


		using _Base::NullCheck;


		/**
		 * @brief Fill random bits into the given memory region.
		 *
		 * @param buf  The pointer to the beginning of the memory region.
		 * @param size The size of the memory region.
		 */
		virtual void Rand(void* buf, const size_t size) override
		{
			NullCheck();
			MBEDTLSCPP_MAKE_C_FUNC_CALL(
				CtrDrbg::Rand,
				mbedtls_ctr_drbg_random,
				static_cast<void*>(Get()),
				static_cast<unsigned char *>(buf),
				size
			);
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
			_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(CtrDrbg));
		}

	private:

		std::unique_ptr<EntropyInterface> m_entropy;
	};
}
