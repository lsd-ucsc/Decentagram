#pragma once

#include <cstddef>

#include "Exceptions.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	/**
	 * @brief The interface class for entropy that will be used by Deterministic
	 *        Random Bit Generators (DRBGs).
	 *
	 */
	class EntropyInterface
	{
	public: // Static members:

		/**
		 * @brief	Call back function provided for mbedTLS library call back needs.
		 *
		 * @param [in,out]	ctx	The pointer point to a EntropyInterface instance. Must not null.
		 * @param [out]	  	buf	The buffer to be filled with entropy.
		 * @param 		  	len	The length of the buffer.
		 *
		 * @return	mbedTLS errorcode.
		 */
		static int CallBack(void * ctx, unsigned char * buf, size_t len) noexcept
		{
			if (ctx == nullptr)
			{
				return MBEDTLS_ERR_ERROR_GENERIC_ERROR;
				// Or MBEDTLS_ERR_CIPHER_BAD_INPUT_DATA ? which one is better?
			}

			try
			{
				EntropyInterface* entropy = static_cast<EntropyInterface*>(ctx);
				entropy->FillEntropy(buf, len);
				return MBEDTLS_EXIT_SUCCESS;
			}
			catch (const mbedTLSRuntimeError& e)
			{
				return e.GetErrorCode();
			}
			catch (...)
			{
				return MBEDTLS_ERR_ERROR_GENERIC_ERROR;
			}
		}

	public:

		EntropyInterface() = default;

		// LCOV_EXCL_START
		virtual ~EntropyInterface() = default;
		// LCOV_EXCL_STOP

		/**
		 * @brief Get the raw C pointer to the object which can uniquely
		 *        identify the inner entropy object.
		 *
		 * @return void* The pointer
		 */
		virtual void* GetRawPtr() = 0;

		/**
		 * @brief Get the const raw C pointer to the object which can uniquely
		 *        identify the inner entropy object.
		 *
		 * @return const void* The pointer.
		 */
		virtual const void* GetRawPtr() const = 0;

		/**
		 * @brief	Generate Entropy to fill the given buffer.
		 *
		 * @param [in,out]	buf 	The buffer to store the generated random bits.
		 * @param 		  	size	The size.
		 */
		virtual void FillEntropy(void* buf, const size_t size) = 0;

		/**
		 * @brief Get a random number
		 *
		 * @tparam ResultType The type of the number.
		 * @return ResultType The generated random number.
		 */
		template<typename ResultType,
			enable_if_t<std::is_signed<ResultType>::value || std::is_unsigned<ResultType>::value, int> = 0>
		ResultType GetEntropy()
		{
			ResultType res;
			FillEntropy(&res, sizeof(ResultType));
			return res;
		}
	};
}
