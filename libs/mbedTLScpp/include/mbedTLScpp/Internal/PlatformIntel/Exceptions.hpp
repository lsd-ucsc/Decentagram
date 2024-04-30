#pragma once

#include "../../Exceptions.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	namespace Internal
	{
		namespace PlatformIntel
		{
			/**
			 * @brief An exception class that is thrown when the requesting
			 *        feature is not supported.
			 *
			 */
			class FeatureUnsupportedException : public RuntimeException
			{
			public:

				/**
				 * @brief Construct a new FeatureUnsupported Exception object
				 *
				 * @param what_arg explanatory string
				 */
				FeatureUnsupportedException(const char* what_arg) :
					RuntimeException(what_arg)
				{}

				/**
				 * @brief Construct a new FeatureUnsupported Exception object
				 *
				 * @param what_arg explanatory string
				 */
				FeatureUnsupportedException(const std::string& what_arg) :
					RuntimeException(what_arg)
				{}

				FeatureUnsupportedException(const FeatureUnsupportedException& other) noexcept :
					RuntimeException(other)
				{}

				// LCOV_EXCL_START
				/**
				 * @brief Destroy the FeatureUnsupported Exception object
				 *
				 */
				virtual ~FeatureUnsupportedException() = default;
				// LCOV_EXCL_STOP
			};

			/**
			 * @brief An exception class that is thrown when the platform
			 *        couldn't respond to a function call immediately.
			 *
			 */
			class PlatformBusyException : public RuntimeException
			{
			public:

				/**
				 * @brief Construct a new PlatformBusy Exception object
				 *
				 * @param what_arg explanatory string
				 */
				PlatformBusyException(const char* what_arg) :
					RuntimeException(what_arg)
				{}

				/**
				 * @brief Construct a new PlatformBusy Exception object
				 *
				 * @param what_arg explanatory string
				 */
				PlatformBusyException(const std::string& what_arg) :
					RuntimeException(what_arg)
				{}

				PlatformBusyException(const PlatformBusyException& other) noexcept :
					RuntimeException(other)
				{}

				// LCOV_EXCL_START
				/**
				 * @brief Destroy the PlatformBusy Exception object
				 *
				 */
				virtual ~PlatformBusyException() = default;
				// LCOV_EXCL_STOP
			};
		}
	}
}
