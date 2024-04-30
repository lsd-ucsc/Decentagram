#pragma once

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	class DummySubInitializer
	{
	public:
		DummySubInitializer() noexcept = default;
		~DummySubInitializer()
		{}

		void Init() noexcept {}
	};
}
