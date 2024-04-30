#pragma once

#include <string>
#include <stdexcept>
#include <utility>
#include <memory>
#include <new>

#include <mbedtls/error.h>
#include <mbedtls/platform.h>

#include "Internal/make_unique.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	/**
	 * @brief The exception base class for all mbed TLS cpp classes' exceptions
	 *        (except those in Container header).
	 *        This class is inherited from std::runtime_error class.
	 *
	 */
	class RuntimeException : public std::runtime_error
	{
	public:

		/**
		 * @brief Construct a new Runtime Exception object.
		 *        Same usage as the one in std::runtime_error.
		 *
		 * @param what_arg explanatory string
		 */
		RuntimeException(const std::string& what_arg) :
			std::runtime_error(what_arg)
		{}

		/**
		 * @brief Construct a new Runtime Exception object.
		 *        Same usage as the one in std::runtime_error.
		 *
		 * @param what_arg explanatory string
		 */
		RuntimeException(const char* what_arg) :
			std::runtime_error(what_arg)
		{}

		/**
		 * @brief Construct a new Runtime Exception object.
		 *        Same usage as the one in std::runtime_error.
		 *
		 * @param other another exception object to copy
		 */
		RuntimeException(const RuntimeException& other) noexcept :
			std::runtime_error(other)
		{}

		// LCOV_EXCL_START
		/**
		 * @brief Destroy the Runtime Exception object
		 *
		 */
		virtual ~RuntimeException() = default;
		// LCOV_EXCL_STOP
	};

	/**
	 * @brief The exception class that encapsulates all mbed TLS built-in error
	 *        codes. In mbed TLS cpp, all C-style error code will be encapsulated
	 *        and thrown by this exception class.
	 *
	 */
	class mbedTLSRuntimeError : public RuntimeException
	{
	public: //static members

		static std::string ErrorCodeToString(int errorCode)
		{
			static constexpr char sk_alphabet[] = "0123456789ABCDEF";
			static constexpr size_t sk_errCodeDigits = 4;

			int posErrCode = errorCode < 0 ? -errorCode : errorCode;
			std::string errCodeHex(sk_errCodeDigits, '\0');
			for (size_t i = 0; i < sk_errCodeDigits; ++i)
			{
				errCodeHex[sk_errCodeDigits - i - 1] =
					sk_alphabet[posErrCode & 0x0F];
				posErrCode >>= 4;
			}

			return std::string("-0x") + errCodeHex;
		}

		/**
		 * @brief The helper function to construct the explanatory string.
		 *
		 * @param errorCode Error code returned by mbed TLS.
		 * @param caller    The name of mbed TLS cpp function that calls the mbed TLS C function.
		 * @param callee    The name of mbed TLS C function being called.
		 * @return std::string The explanatory string.
		 */
		static std::string ConstructWhatMsg(int errorCode, const char* caller, const char* callee)
		{
			const char* highErrCStr = mbedtls_high_level_strerr(errorCode);
			const char* lowErrCStr = mbedtls_low_level_strerr(errorCode);
			std::string highErrStr(highErrCStr != nullptr ? highErrCStr : "N/A");
			std::string lowErrStr(lowErrCStr != nullptr ? lowErrCStr : "N/A");

			std::string errCodeHex = ErrorCodeToString(errorCode);

			return std::string("embed TLS returned error ") + errCodeHex +
			       " (" + highErrStr + " : " + lowErrStr + ") when function " +
				   caller + " called function " + callee;
		}

	public:

		/**
		 * @brief Construct a new mbed TLS Runtime Error object.
		 *        Same usage as the one in RuntimeException.
		 *
		 * @param errorCode The error code returned by mbed TLS library
		 * @param what_arg  explanatory string
		 */
		mbedTLSRuntimeError(int errorCode, const char* what_arg) :
			RuntimeException(what_arg),
			m_errorCode(errorCode)
		{}

		/**
		 * @brief Construct a new mbed TLS Runtime Error object.
		 *        Same usage as the one in RuntimeException.
		 *
		 * @param errorCode The error code returned by mbed TLS library
		 * @param what_arg  explanatory string
		 */
		mbedTLSRuntimeError(int errorCode, const std::string& what_arg) :
			RuntimeException(what_arg),
			m_errorCode(errorCode)
		{}

		/**
		 * @brief Construct a new mbed TLS Runtime Error object.
		 *        Same usage as the one in RuntimeException.
		 *
		 * @param other another exception object to copy
		 */
		mbedTLSRuntimeError(const mbedTLSRuntimeError& other) noexcept :
			RuntimeException(other),
			m_errorCode(other.m_errorCode)
		{}

		// LCOV_EXCL_START
		/**
		 * @brief Destroy the mbed TLS Runtime Error object
		 *
		 */
		virtual ~mbedTLSRuntimeError() = default;
		// LCOV_EXCL_STOP

		/**
		 * @brief Get the error code
		 *
		 * @return int The error code
		 */
		int GetErrorCode() const noexcept
		{
			return m_errorCode;
		}

	private:

		int m_errorCode;
	};

	/**
	 * @brief An exception class that is thrown when the program is trying to
	 *        access an null instance of a mbed TLS cpp object.
	 *
	 */
	class InvalidObjectException : public RuntimeException
	{
	public:

		InvalidObjectException(const std::string& objTypeName) :
			RuntimeException("A invalid/null instance of the mbed TLS cpp object type " + objTypeName + " is accessed.")
		{}

		InvalidObjectException(const InvalidObjectException& other) noexcept :
			RuntimeException(other)
		{}

		// LCOV_EXCL_START
		/**
		 * @brief Destroy the Invalid Object Exception object
		 *
		 */
		virtual ~InvalidObjectException() = default;
		// LCOV_EXCL_STOP
	};

	/**
	 * @brief An exception class that is thrown when the argument given to the
	 *        function call is invalid.
	 *
	 */
	class InvalidArgumentException : public RuntimeException
	{
	public:

		/**
		 * @brief Construct a new Invalid Argument Exception object
		 *
		 * @param what_arg explanatory string
		 */
		InvalidArgumentException(const char* what_arg) :
			RuntimeException(what_arg)
		{}

		/**
		 * @brief Construct a new Invalid Argument Exception object
		 *
		 * @param what_arg explanatory string
		 */
		InvalidArgumentException(const std::string& what_arg) :
			RuntimeException(what_arg)
		{}

		InvalidArgumentException(const InvalidArgumentException& other) noexcept :
			RuntimeException(other)
		{}

		// LCOV_EXCL_START
		/**
		 * @brief Destroy the Invalid Argument Exception object
		 *
		 */
		virtual ~InvalidArgumentException() = default;
		// LCOV_EXCL_STOP
	};

	/**
	 * @brief An exception class that is thrown when unexpected error occurs.
	 *        An unexpected error should not occurs if both mbed TLS and mbed TLS
	 *        cpp are implemented correctly. But just in case if there might be bugs
	 *        in the library.
	 *
	 */
	class UnexpectedErrorException : public RuntimeException
	{
	public:

		/**
		 * @brief Construct a new Unexpected Error Exception object
		 *
		 * @param what_arg explanatory string
		 */
		UnexpectedErrorException(const char* what_arg) :
			RuntimeException(what_arg)
		{}

		/**
		 * @brief Construct a new Unexpected Error Exception object
		 *
		 * @param what_arg explanatory string
		 */
		UnexpectedErrorException(const std::string& what_arg) :
			RuntimeException(what_arg)
		{}

		UnexpectedErrorException(const UnexpectedErrorException& other) noexcept :
			RuntimeException(other)
		{}

		// LCOV_EXCL_START
		/**
		 * @brief Destroy the Unexpected Error Exception object
		 *
		 */
		virtual ~UnexpectedErrorException() = default;
		// LCOV_EXCL_STOP
	};

} // namespace mbedTLScpp





/** ============================================================================
 *   Helper functions
 *  ============================================================================
 */


#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{

inline void CheckMbedTlsLenRetVal(
	int lenVal,
	const char* callerName,
	const char* calleeName
)
{
	if (lenVal < 0)
	{
		throw mbedTLSRuntimeError(lenVal,
			mbedTLSRuntimeError::ConstructWhatMsg(
				lenVal,
				callerName,
				calleeName
			)
		);
	}
}

inline void CheckMbedTlsIntRetVal(
	int lenVal,
	const char* callerName,
	const char* calleeName
)
{
	if (lenVal != 0)
	{
		throw mbedTLSRuntimeError(lenVal,
			mbedTLSRuntimeError::ConstructWhatMsg(
				lenVal,
				callerName,
				calleeName
			)
		);
	}
}

} // namespace mbedTLScpp

/**
 * @brief Check if the error code given is MBEDTLS_EXIT_SUCCESS or not.
 *        If not, it will construct and throw the mbedTLSRuntimeError exception.
 *
 */
#define MBEDTLSCPP_THROW_IF_ERROR_CODE_NON_SUCCESS(ERROR_CODE, CALLER, CALLEE) { \
	if ((ERROR_CODE) != MBEDTLS_EXIT_SUCCESS) { \
		std::unique_ptr<std::string> errorStrPtr; \
		errorStrPtr = Internal::make_unique<std::string>(mbedTLSRuntimeError::ConstructWhatMsg(ERROR_CODE, #CALLER, #CALLEE)); \
		throw mbedTLSRuntimeError(ERROR_CODE, *errorStrPtr); \
	} \
}

/**
 * @brief Check if the mbed TLS C function call given returns MBEDTLS_EXIT_SUCCESS or not.
 *        If not, it will construct and throw the mbedTLSRuntimeError exception.
 *
 */
#define MBEDTLSCPP_C_FUNC_CALL(CALLER, CALLEE, CALL_STATEMENT) { \
	int retVal = (CALL_STATEMENT); \
	MBEDTLSCPP_THROW_IF_ERROR_CODE_NON_SUCCESS(retVal, CALLER, CALLEE); \
}

/**
 * @brief Make the call to a specified mbed TLS C function, and check if it returns
 *        MBEDTLS_EXIT_SUCCESS or not.
 *        If not, it will construct and throw the mbedTLSRuntimeError exception.
 *
 */
#define MBEDTLSCPP_MAKE_C_FUNC_CALL(CALLER, CALLEE, ...) { \
	MBEDTLSCPP_C_FUNC_CALL(CALLER, CALLEE, CALLEE(__VA_ARGS__)); \
}

#define MBEDTLSCPP_MEMALLOC_NULLPTR_CHECK(PTR) { \
	if ((PTR) == nullptr) { \
		throw std::bad_alloc(); \
	} \
}
