#pragma once

#include <cstddef>

#ifdef MBEDTLSCPP_MEMORY_TEST
#include <atomic> //atomic_int64_t
#endif
#include <memory>

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

	template<class T>
	class SecretAllocator : public std::allocator<T>
	{
	public: // Member types:

		typedef size_t    size_type;
		typedef ptrdiff_t difference_type;
		typedef T*        pointer;
		typedef const T*  const_pointer;
		typedef T&        reference;
		typedef const T&  const_reference;
		typedef T         value_type;

		template<class U>
		struct rebind
		{
			typedef SecretAllocator<U> other;
		};

	public:

		using std::allocator<T>::allocator;

#if __cplusplus < 201703L
	pointer allocate(size_type n, const void * hint)
	{
		pointer res = std::allocator<T>::allocate(n, hint);

#ifdef MBEDTLSCPP_MEMORY_TEST
		gs_secretAllocationLeft += n;
#endif

		return res;
	}
#endif

	pointer allocate(size_type n)
	{
		pointer res = std::allocator<T>::allocate(n);

#ifdef MBEDTLSCPP_MEMORY_TEST
		gs_secretAllocationLeft += n;
#endif

		return res;
	}

	void SecureZeroize(pointer p, size_type n) noexcept
	{
		if (p != nullptr)
		{
			StaticLoadedFunctions::GetInstance().SecureZeroize(p, n * sizeof(value_type));
		}
	}

	void deallocate(pointer p, size_type n)
	{
		SecureZeroize(p, n);

#ifdef MBEDTLSCPP_MEMORY_TEST
		if (p != nullptr)
		{
			gs_secretAllocationLeft -= n;
		}
#endif

		std::allocator<T>::deallocate(p, n);
	}

	};
}
