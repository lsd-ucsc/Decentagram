#pragma once

#include <cstdint>

#ifdef MBEDTLSCPP_MEMORY_TEST
#include <atomic> //atomic_int64_t
#endif

#include "Common.hpp"
#include "LoadedFunctions.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
#ifdef MBEDTLSCPP_MEMORY_TEST
		/**
		 * @brief The count for secret memory allocation.
		 *        It's only used for secret memory leak testing.
		 *
		 */
		extern std::atomic_int64_t gs_secretAllocationLeft;
#endif

	template<typename _CStructType,
		enable_if_t<IsCTypeAlike<_CStructType>::value, int> = 0>
	struct SecretStruct
	{
		_CStructType m_data;

		using CStructType = _CStructType;

		~SecretStruct()
		{
			StaticLoadedFunctions::GetInstance().SecureZeroize(&m_data, sizeof(m_data));
#ifdef MBEDTLSCPP_MEMORY_TEST
			gs_secretAllocationLeft -= sizeof(m_data);
#endif
		}

		bool operator==(const SecretStruct& rhs) const
		{
			return StaticLoadedFunctions::GetInstance().ConstTimeMemEqual(&m_data, &rhs.m_data, sizeof(m_data));
		}

		bool operator!=(const SecretStruct& rhs) const
		{
			return StaticLoadedFunctions::GetInstance().ConstTimeMemNotEqual(&m_data, &rhs.m_data, sizeof(m_data));
		}
	};
}
