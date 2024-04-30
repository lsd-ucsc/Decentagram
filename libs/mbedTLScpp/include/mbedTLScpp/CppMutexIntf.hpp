#pragma once

#include <mbedtls/threading.h>

#ifdef MBEDTLS_THREADING_ALT

#include <mutex>
#include <memory>

#include <mbedtls/platform.h>

#include "Internal/Memory.hpp"
#include "Internal/make_unique.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	class CppMutexIntf
	{
	public:

		static void MutexInit(mbedtls_threading_mutex_t *mutex) noexcept
		{
			if (mutex == nullptr)
			{
				return;
			}

			try
			{
				*mutex = Internal::NewObject<CppMutexIntf>();
			}
			catch(...)
			{
				*mutex = nullptr;
			}
		}

		static void MutexFree(mbedtls_threading_mutex_t *mutex) noexcept
		{
			if (mutex == nullptr)
			{
				return;
			}

			CppMutexIntf* cppMutex = static_cast<CppMutexIntf*>(*mutex);
			Internal::DelObject(cppMutex);
			*mutex = nullptr;
		}

		static int MutexLock(mbedtls_threading_mutex_t *mutex) noexcept
		{
			if (mutex == nullptr ||
				*mutex == nullptr)
			{
				return MBEDTLS_ERR_THREADING_BAD_INPUT_DATA;
			}

			CppMutexIntf* cppMutex = static_cast<CppMutexIntf*>(*mutex);

			try
			{
				std::unique_ptr<std::unique_lock<std::mutex> > lockPtr =
					Internal::make_unique<std::unique_lock<std::mutex> >(cppMutex->m_mutex);
				cppMutex->m_lock = std::move(lockPtr);
			}
			catch (...)
			{
				return MBEDTLS_ERR_THREADING_MUTEX_ERROR;
			}

			return MBEDTLS_EXIT_SUCCESS;
		}

		static int MutexUnlock(mbedtls_threading_mutex_t *mutex) noexcept
		{
			if (mutex == nullptr ||
				*mutex == nullptr)
			{
				return MBEDTLS_ERR_THREADING_BAD_INPUT_DATA;
			}

			CppMutexIntf* cppMutex = static_cast<CppMutexIntf*>(*mutex);

			try
			{
				cppMutex->m_lock.reset();
			}
			catch (...)
			{
				return MBEDTLS_ERR_THREADING_MUTEX_ERROR;
			}

			return MBEDTLS_EXIT_SUCCESS;
		}

	public:
		CppMutexIntf() noexcept :
			m_mutex(), //noexcept
			m_lock() //noexcept
		{}

		~CppMutexIntf()
		{}

	private:
		std::mutex m_mutex;
		std::unique_ptr<std::unique_lock<std::mutex> > m_lock;
	};

	class CppMutexIntfInitializer
	{
	public:
		CppMutexIntfInitializer() noexcept = default;
		~CppMutexIntfInitializer()
		{}

		void Init() noexcept
		{
			mbedtls_threading_set_alt(
				&CppMutexIntf::MutexInit,
				&CppMutexIntf::MutexFree,
				&CppMutexIntf::MutexLock,
				&CppMutexIntf::MutexUnlock
			);
		}
	};

	using DefaultThreadingSubInitializer = CppMutexIntfInitializer;
}

#else

#include "DummySubInitializer.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	using DefaultThreadingSubInitializer = DummySubInitializer;
}

#endif //MBEDTLS_THREADING_ALT
