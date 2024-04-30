#pragma once

#ifdef MBEDTLSCPP_CUSTOMIZED_THREADING_INIT
#include MBEDTLSCPP_CUSTOMIZED_THREADING_INIT
#else
#include "CppMutexIntf.hpp"
#endif

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	class LibInitializer
	{
	public:

		static LibInitializer& GetInst() noexcept
		{
			static LibInitializer inst;
			return inst;
		}

	public:

		~LibInitializer()
		{}

	private:

		LibInitializer() noexcept :
			m_thr() // noexcept
		{
			m_thr.Init(); // noexcept
		}

	private:

		DefaultThreadingSubInitializer m_thr;
	};

}
