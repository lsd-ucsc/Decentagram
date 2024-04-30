#pragma once

#include "CipherBase.hpp"

#include <mbedtls/cmac.h>

#include "Container.hpp"
#include "Container.hpp"
#include "Exceptions.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	/**
	 * @brief The container type used to store the CMAC result (for a known cipher type).
	 *
	 * @tparam _cipherType The type of the cipher.
	 * @tparam _bitSize    The size of the cipher key in bits.
	 * @tparam _cipherMode The cipher mode.
	 */
	template<CipherType _cipherType, size_t _bitSize, CipherMode _cipherMode>
	using Cmac = std::array<uint8_t, GetCipherBlockSize(_cipherType, _bitSize, _cipherMode)>;

	/**
	 * @brief The base class for CMAC calculator. It can accept some raw pointer
	 *        parameters, and cipher type can be specified at runtime.
	 *
	 */
	class CmacerBase : public CipherBase<>
	{
	public:

		CmacerBase() = delete;

		/**
		 * @brief Construct a new CMACer Base object
		 *
		 * @exception mbedTLSRuntimeError  Thrown when mbed TLS C function call failed.
		 * @exception std::bad_alloc       Thrown when memory allocation failed.
		 * @tparam ContainerType The container that used to store the key.
		 * @param cipherInfo The cipher info provided by mbed TLS library.
		 * @param key        The secret key for CMAC.
		 */
		template<typename ContainerType>
		CmacerBase(const mbedtls_cipher_info_t& cipherInfo, const ContCtnReadOnlyRef<ContainerType, true>& key) :
			CipherBase(cipherInfo)
		{
			MBEDTLSCPP_MAKE_C_FUNC_CALL(CmacerBase::CmacerBase,
				mbedtls_cipher_cmac_starts,
				NonVirtualGet(),
				static_cast<const unsigned char*>(key.BeginPtr()),
				key.GetRegionSize() * gsk_bitsPerByte);
		}

		/**
		 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
		 *
		 * @param rhs The other CmacerBase instance.
		 */
		CmacerBase(CmacerBase&& rhs) noexcept :
			CipherBase(std::forward<CipherBase>(rhs)) //noexcept
		{}

		CmacerBase(const CmacerBase& rhs) = delete;

		// LCOV_EXCL_START
		/** @brief Destructor */
		virtual ~CmacerBase() = default;
		// LCOV_EXCL_STOP

		/**
		 * @brief Move assignment. The `rhs` will be empty/null afterwards.
		 *
		 * @param rhs The other CmacerBase instance.
		 * @return CmacerBase& A reference to this instance.
		 */
		CmacerBase& operator=(CmacerBase&& rhs) noexcept
		{
			CipherBase::operator=(std::forward<CipherBase>(rhs)); //noexcept

			return *this;
		}

		CmacerBase& operator=(const CmacerBase& other) = delete;

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

			MBEDTLSCPP_MAKE_C_FUNC_CALL(CmacerBase::Update, mbedtls_cipher_cmac_update,
				Get(),
				static_cast<const unsigned char*>(data.BeginPtr()),
				data.GetRegionSize());
		}

		/**
		 * @brief Finishes the CMAC calculation and get the CMAC result.
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @return std::vector<uint8_t> The CMAC result.
		 */
		std::vector<uint8_t> Finish()
		{
			NullCheck();

			const size_t size = mbedtls_cipher_get_block_size(Get());
			if (size == 0)
			{
				throw UnexpectedErrorException("CMACerBase is not null, but mbedtls_cipher_get_block_size returns zero.");
			}

			std::vector<uint8_t> cmac(size);

			MBEDTLSCPP_MAKE_C_FUNC_CALL(CmacerBase::Finish, mbedtls_cipher_cmac_finish,
				Get(),
				static_cast<unsigned char*>(cmac.data()));

			return cmac;
		}

		/**
		 * @brief Restart the CMAC calculation, so that the previous CMAC state
		 *        will be wiped out. It's useful if you want to reuse the same
		 *        hmacer instance.
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 */
		void Restart()
		{
			NullCheck();

			MBEDTLSCPP_MAKE_C_FUNC_CALL(CmacerBase::Restart,
				mbedtls_cipher_cmac_reset,
				Get());
		}

	protected:

		void UpdateNoCheck(const void* data, size_t size)
		{
			MBEDTLSCPP_MAKE_C_FUNC_CALL(CmacerBase::UpdateNoCheck, mbedtls_cipher_cmac_update,
				Get(),
				static_cast<const unsigned char*>(data),
				size);
		}
	};

	/**
	 * @brief  The CMAC calculator. Only accept C++ objects as parameters, and
	 *        cipher type must be specified at compile time.
	 *
	 * @tparam _cipherType The type of the cipher.
	 * @tparam _bitSize    The size of the cipher key in bits.
	 * @tparam _cipherMode The cipher mode.
	 */
	template<CipherType _cipherType, size_t _bitSize, CipherMode _cipherMode>
	class Cmacer : public CmacerBase
	{
	public:

		/**
		 * @brief Construct a new Cmacer object
		 *
		 * @exception mbedTLSRuntimeError  Thrown when mbed TLS C function call failed.
		 * @exception std::bad_alloc       Thrown when memory allocation failed.
		 * @tparam ContainerType The container that used to store the key.
		 * @param key The secret key for CMAC.
		 */
		template<typename ContainerType>
		Cmacer(const ContCtnReadOnlyRef<ContainerType, true>& key) :
			CmacerBase(GetCipherInfo(_cipherType, _bitSize, _cipherMode), key)
		{}

		// LCOV_EXCL_START
		/**
		 * @brief Destroy the Cmacer object
		 *
		 */
		virtual ~Cmacer() = default;
		// LCOV_EXCL_STOP

		/**
		 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
		 *
		 * @param rhs The other Cmacer instance.
		 */
		Cmacer(Cmacer&& rhs) noexcept :
			CmacerBase(std::forward<CmacerBase>(rhs)) //noexcept
		{}

		Cmacer(const Cmacer& rhs) = delete;

		/**
		 * @brief Move assignment. The `rhs` will be empty/null afterwards.
		 *
		 * @param rhs The other Cmacer instance.
		 * @return Cmacer& A reference to this instance.
		 */
		Cmacer& operator=(Cmacer&& rhs) noexcept
		{
			CmacerBase::operator=(std::forward<CmacerBase>(rhs)); //noexcept

			return *this;
		}

		Cmacer& operator=(const Cmacer& other) = delete;

		/**
		 * @brief Finishes the CMAC calculation and get the CMAC result.
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @return Cmac<_HashTypeValue> The CMAC result.
		 */
		Cmac<_cipherType, _bitSize, _cipherMode> Finish()
		{
			NullCheck();

			return FinishNoCheck();
		}

		/**
		 * @brief Update the CMAC calculation with a list of Input Data Items.
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
		 * @return Cmac<_HashTypeValue> The CMAC result.
		 */
		template<size_t ListLen>
		Cmac<_cipherType, _bitSize, _cipherMode> CalcList(const InDataList<ListLen>& list)
		{
			NullCheck();

			for(auto it = list.begin(); it != list.end(); ++it)
			{
				UpdateNoCheck(it->m_data, it->m_size);
			}

			return FinishNoCheck();
		}

		/**
		 * @brief Update the CMAC calculation with a sequence of containers wrapped
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
		 * @return Cmac<_HashTypeValue> The CMAC result.
		 */
		template<class... Args>
		Cmac<_cipherType, _bitSize, _cipherMode> Calc(Args... args)
		{
			return CalcList(ConstructInDataList(args...));
		}

	private:

		Cmac<_cipherType, _bitSize, _cipherMode> FinishNoCheck()
		{
			Cmac<_cipherType, _bitSize, _cipherMode> cmac;

			MBEDTLSCPP_MAKE_C_FUNC_CALL(Cmacer::FinishNoCheck, mbedtls_cipher_cmac_finish,
				Get(),
				static_cast<unsigned char*>(cmac.data()));

			return cmac;
		}
	};
}