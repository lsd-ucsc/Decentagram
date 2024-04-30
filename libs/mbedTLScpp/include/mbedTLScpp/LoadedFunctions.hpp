#pragma once

#include <cstddef>

#include <type_traits>

#include <mbedtls/platform_util.h>

#include "Internal/ConstantTimeFunc.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	/**
	 * @brief Manages functions that are loaded by pointers, and the pointer will
	 *        not change during the rumtime. It's used to ensure compiler will not
	 *        optimize the function call.
	 *
	 */
	class StaticLoadedFunctions
	{
	public: // types

		typedef void (*SecureZeroizeFunc)(void*, size_t);
		typedef int (*MemEqualFunc)(const void*, const void*, size_t);

	public: // Static members

		static StaticLoadedFunctions& GetInstance()
		{
			static StaticLoadedFunctions inst;
			return inst;
		}

	public:
		StaticLoadedFunctions() :
			m_zeroize(mbedtls_platform_zeroize),        // noexcept
			m_memEqual(Internal::ConstTimeMemEqual),    // noexcept
			m_memNEqual(Internal::ConstTimeMemNotEqual) // noexcept
		{}

		// LCOV_EXCL_START
		virtual ~StaticLoadedFunctions() = default;
		// LCOV_EXCL_STOP

		void SecureZeroize(void* ptr, size_t size) noexcept
		{
			SecureZeroizeFunc tmpZeroize = m_zeroize;
			(*tmpZeroize)(ptr, size);
		}

		bool ConstTimeMemEqual(const void* a, const void* b, size_t size) noexcept
		{
			MemEqualFunc tmpMemEqual = m_memEqual;
			int res = (*tmpMemEqual)(a, b, size);

			return bool(res);
		}

		bool ConstTimeMemNotEqual(const void* a, const void* b, size_t size) noexcept
		{
			MemEqualFunc tmpMemNEqual = m_memNEqual;
			int res = (*tmpMemNEqual)(a, b, size);

			return bool(res);
		}

	private:

		volatile SecureZeroizeFunc m_zeroize;

		volatile MemEqualFunc m_memEqual;
		volatile MemEqualFunc m_memNEqual;

		static_assert(std::is_volatile<decltype(m_zeroize)>::value && std::is_pointer<decltype(m_zeroize)>::value, "m_zeroize must be a volatile pointer to the function.");
		static_assert(std::is_volatile<decltype(m_memEqual)>::value && std::is_pointer<decltype(m_memEqual)>::value, "m_memEqual must be a volatile pointer to the function.");
		static_assert(std::is_volatile<decltype(m_memNEqual)>::value && std::is_pointer<decltype(m_memNEqual)>::value, "m_memNEqual must be a volatile pointer to the function.");
	};
}
