// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#if defined(DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED) || \
	defined(DECENT_ENCLAVE_PLATFORM_SGX_UNTRUSTED)


#include <mutex>
#include <memory>

#include <mbedtls/threading.h>
#include <mbedTLScpp/Internal/make_unique.hpp>

#include "../Platform/Print.hpp"


namespace DecentEnclave
{
namespace Common
{
namespace Sgx
{


struct InternalMutexStruct
{
	std::mutex m_mutex;
	std::unique_ptr<std::unique_lock<std::mutex> > m_lock;

	InternalMutexStruct() noexcept :
		m_mutex(), //noexcept
		m_lock() //noexcept
	{}
}; // struct InternalMutexStruct


struct MbedTlsMutex
{
	static void Init(mbedtls_threading_mutex_t *mutex)
	{
		if (mutex == nullptr)
		{
			return;
		}

		*mutex = new InternalMutexStruct();
	}


	static void Free(mbedtls_threading_mutex_t *mutex)
	{
		if (mutex == nullptr)
		{
			return;
		}

		InternalMutexStruct* internalPtr =
			static_cast<InternalMutexStruct*>(*mutex);
		delete internalPtr;
		*mutex = nullptr;
	}


	static int Lock(mbedtls_threading_mutex_t *mutex)
	{
		if (
			mutex == nullptr ||
			*mutex == nullptr
		)
		{
			return -1;
		}

		InternalMutexStruct* internalPtr =
			static_cast<InternalMutexStruct*>(*mutex);

		try
		{
			std::unique_ptr<std::unique_lock<std::mutex> > lockPtr =
				mbedTLScpp::Internal::make_unique<
					std::unique_lock<std::mutex>
				>(internalPtr->m_mutex);

			internalPtr->m_lock = std::move(lockPtr);
		}
		catch (const std::exception& e)
		{
			Platform::Print::StrErr(
				std::string("MbedTLS Failed to lock the mutex (errMsg=") +
				e.what() + ")"
			);
			return -1;
		}

		return 0;
	}


	static int Unlock(mbedtls_threading_mutex_t *mutex)
	{
		if (
			mutex == nullptr ||
			*mutex == nullptr
		)
		{
			return -1;
		}

		InternalMutexStruct* internalPtr =
			static_cast<InternalMutexStruct*>(*mutex);

		try
		{
			internalPtr->m_lock.reset();
		}
		catch (const std::exception& e)
		{
			Platform::Print::StrErr(
				std::string("MbedTLS Failed to lock the mutex (errMsg=") +
				e.what() + ")"
			);
			return -1;
		}

		return 0;
	}

}; // struct MbedTlsMutex


class MbedTlsInit
{
public: // static members:

	static const MbedTlsInit& Init()
	{
		static MbedTlsInit inst;
		return inst;
	}


public:

	MbedTlsInit() noexcept
	{
		mbedtls_threading_set_alt(
			MbedTlsMutex::Init,
			MbedTlsMutex::Free,
			MbedTlsMutex::Lock,
			MbedTlsMutex::Unlock);
	}


	~MbedTlsInit()
	{
		mbedtls_threading_free_alt();
	}

}; // class MbedTlsInit

} // namespace Sgx
} // namespace Common
} // namespace DecentEnclave

#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED || _UNTRUSTED
