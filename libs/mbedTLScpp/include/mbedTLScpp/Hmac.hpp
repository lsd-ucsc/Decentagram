#pragma once

#include "MsgDigestBase.hpp"

#include "Container.hpp"
#include "Exceptions.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	/**
	 * @brief The container type used to store the HMAC result (for a known hash type).
	 *
	 * @tparam _HashTypeValue The type of the hash.
	 */
	template<HashType _HashTypeValue>
	using Hmac = std::array<uint8_t, GetHashByteSize(_HashTypeValue)>;

	/**
	 * @brief The base class for HMAC calculator. It can accept some raw pointer
	 *        parameters, and hash type can be specified at runtime.
	 *
	 */
	class HmacerBase : public MsgDigestBase<>
	{
	public:

		HmacerBase() = delete;

		/**
		 * @brief Construct a new HMACer Base object
		 *
		 * @exception mbedTLSRuntimeError  Thrown when mbed TLS C function call failed.
		 * @exception std::bad_alloc       Thrown when memory allocation failed.
		 * @tparam ContainerType The container that used to store the key.
		 * @param mdInfo The md info provided by mbed TLS library.
		 * @param key    The secret key for HMAC.
		 */
		template<typename ContainerType>
		HmacerBase(const mbedtls_md_info_t& mdInfo, const ContCtnReadOnlyRef<ContainerType, true>& key) :
			MsgDigestBase(mdInfo, true)
		{
			MBEDTLSCPP_MAKE_C_FUNC_CALL(HmacerBase::HmacerBase,
				mbedtls_md_hmac_starts,
				NonVirtualGet(),
				static_cast<const unsigned char*>(key.BeginPtr()),
				key.GetRegionSize());
		}

		/**
		 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
		 *
		 * @param rhs The other HmacerBase instance.
		 */
		HmacerBase(HmacerBase&& rhs) noexcept :
			MsgDigestBase(std::forward<MsgDigestBase>(rhs)) //noexcept
		{}

		HmacerBase(const HmacerBase& rhs) = delete;

		// LCOV_EXCL_START
		/** @brief Destructor */
		virtual ~HmacerBase() = default;
		// LCOV_EXCL_STOP

		/**
		 * @brief Move assignment. The `rhs` will be empty/null afterwards.
		 *
		 * @param rhs The other HmacerBase instance.
		 * @return HmacerBase& A reference to this instance.
		 */
		HmacerBase& operator=(HmacerBase&& rhs) noexcept
		{
			MsgDigestBase::operator=(std::forward<MsgDigestBase>(rhs)); //noexcept

			return *this;
		}

		HmacerBase& operator=(const HmacerBase& other) = delete;

		/**
		 * @brief Updates the calculation with the given data.
		 *
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @tparam ContainerType The type of the container that stores the data.
		 * @param data The data to be hmaced.
		 */
		template<typename ContainerType, bool ContainerSecrecy>
		void Update(const ContCtnReadOnlyRef<ContainerType, ContainerSecrecy>& data)
		{
			NullCheck();

			MBEDTLSCPP_MAKE_C_FUNC_CALL(HmacerBase::Update, mbedtls_md_hmac_update,
				Get(),
				static_cast<const unsigned char*>(data.BeginPtr()),
				data.GetRegionSize());
		}

		/**
		 * @brief Finishes the HMAC calculation and get the HMAC result.
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @return std::vector<uint8_t> The HMAC result.
		 */
		std::vector<uint8_t> Finish()
		{
			NullCheck();

			const mbedtls_md_info_t* mdInfo = mbedtls_md_info_from_ctx(Get());
			const size_t size = mbedtls_md_get_size(mdInfo);
			if (size == 0)
			{
				throw UnexpectedErrorException("HMACerBase is not null, but mbedtls_md_get_size returns zero.");
			}

			std::vector<uint8_t> hmac(size);

			MBEDTLSCPP_MAKE_C_FUNC_CALL(HmacerBase::Finish, mbedtls_md_hmac_finish,
				Get(),
				static_cast<unsigned char*>(hmac.data()));

			return hmac;
		}

		/**
		 * @brief Restart the hmac calculation, so that the previous hmac state
		 *        will be wiped out. It's useful if you want to reuse the same
		 *        hmacer instance.
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @tparam ContainerType The container that used to store the key.
		 * @param key    The secret key for HMAC.
		 */
		template<typename ContainerType>
		void Restart(const ContCtnReadOnlyRef<ContainerType, true>& key)
		{
			NullCheck();

			MBEDTLSCPP_MAKE_C_FUNC_CALL(HmacerBase::Restart,
				mbedtls_md_hmac_starts,
				Get(),
				static_cast<const unsigned char*>(key.BeginPtr()),
				key.GetRegionSize());
		}

	protected:

		void UpdateNoCheck(const void* data, size_t size)
		{
			MBEDTLSCPP_MAKE_C_FUNC_CALL(HmacerBase::UpdateNoCheck, mbedtls_md_hmac_update,
				Get(),
				static_cast<const unsigned char*>(data),
				size);
		}
	};

	/**
	 * @brief The HMAC calculator. Only accept C++ objects as parameters, and
	 *        hash type must be specified at compile time.
	 *
	 * @tparam _HashTypeValue Type of the hash
	 */
	template<HashType _HashTypeValue>
	class Hmacer : public HmacerBase
	{
	public: //static members:
		static constexpr size_t sk_hashByteSize = GetHashByteSize(_HashTypeValue);

	public:

		/**
		 * @brief Construct a new Hmacer object
		 *
		 * @exception mbedTLSRuntimeError  Thrown when mbed TLS C function call failed.
		 * @exception std::bad_alloc       Thrown when memory allocation failed.
		 * @tparam ContainerType The container that used to store the key.
		 * @param key The secret key for HMAC.
		 */
		template<typename ContainerType>
		Hmacer(const ContCtnReadOnlyRef<ContainerType, true>& key) :
			HmacerBase(GetMdInfo(_HashTypeValue), key)
		{}

		// LCOV_EXCL_START
		/**
		 * @brief Destroy the Hmacer object
		 *
		 */
		virtual ~Hmacer() = default;
		// LCOV_EXCL_STOP

		/**
		 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
		 *
		 * @param rhs The other Hmacer instance.
		 */
		Hmacer(Hmacer&& rhs) noexcept :
			HmacerBase(std::forward<HmacerBase>(rhs)) //noexcept
		{}

		Hmacer(const Hmacer& rhs) = delete;

		/**
		 * @brief Move assignment. The `rhs` will be empty/null afterwards.
		 *
		 * @param rhs The other Hmacer instance.
		 * @return Hmacer& A reference to this instance.
		 */
		Hmacer& operator=(Hmacer&& rhs) noexcept
		{
			HmacerBase::operator=(std::forward<HmacerBase>(rhs)); //noexcept

			return *this;
		}

		Hmacer& operator=(const Hmacer& other) = delete;

		/**
		 * @brief Finishes the HMAC calculation and get the HMAC result.
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @return Hmac<_HashTypeValue> The HMAC result.
		 */
		Hmac<_HashTypeValue> Finish()
		{
			NullCheck();

			return FinishNoCheck();
		}

		/**
		 * @brief Update the HMAC calculation with a list of Input Data Items.
		 *        NOTE: This function will not clean the previous state, thus,
		 *        it will update the calculation state based on the existing state;
		 *        Thus, you may need to call restart first.
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @tparam ListLen The length of the list.
		 * @param list The list of Input Data Items.
		 * @return Hmac<_HashTypeValue> The HMAC result.
		 */
		template<size_t ListLen>
		Hmac<_HashTypeValue> CalcList(const InDataList<ListLen>& list)
		{
			NullCheck();

			for(auto it = list.begin(); it != list.end(); ++it)
			{
				UpdateNoCheck(it->m_data, it->m_size);
			}

			return FinishNoCheck();
		}

		/**
		 * @brief Update the HMAC calculation with a sequence of containers wrapped
		 *        by ContCtnReadOnlyRef. The sequence of containers can be in any
		 *        length.
		 *        NOTE: This function will not clean the previous state, thus,
		 *        it will update the calculation state based on the existing state;
		 *        Thus, you may need to call restart first.
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @tparam Args The type of the container wrapped by ContCtnReadOnlyRef
		 * @param args The container.
		 * @return Hmac<_HashTypeValue> The HMAC result.
		 */
		template<class... Args>
		Hmac<_HashTypeValue> Calc(Args... args)
		{
			return CalcList(ConstructInDataList(args...));
		}

	private:

		Hmac<_HashTypeValue> FinishNoCheck()
		{
			Hmac<_HashTypeValue> hmac;

			MBEDTLSCPP_MAKE_C_FUNC_CALL(Hmacer::FinishNoCheck, mbedtls_md_hmac_finish,
				Get(),
				static_cast<unsigned char*>(hmac.data()));

			return hmac;
		}
	};
}