#pragma once

#include <cstdint>

#include <tuple>
#include <type_traits>

#ifdef _WIN32
#include <cstring>
#include <intrin.h>
#include <immintrin.h>
#endif

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	namespace Internal
	{
		std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>
			RunCpuid(uint32_t func, uint32_t subfunc)
		{
			static_assert(std::is_same<uint32_t, unsigned int>::value, "Programming Error.");

#if defined(__GNUC__)
			uint32_t eax = 0;
			uint32_t ebx = 0;
			uint32_t ecx = 0;
			uint32_t edx = 0;

			asm volatile("cpuid"
						: "=a" (eax),
						  "=b" (ebx),
						  "=c" (ecx),
						  "=d" (edx)
						: "a"  (func),
						  "c"  (subfunc)
			);

			return std::make_tuple(eax, ebx, ecx, edx);
#elif defined(_WIN32)
			static_assert(sizeof(int) == sizeof(uint32_t), "Programming Error.");

			uint32_t info[4] = { 0 };

			int infoWin[4] = { 0 };
			int funcWin = 0;
			int subfuncWin = 0;
			std::memcpy(&funcWin, &func, sizeof(int));
			std::memcpy(&subfuncWin, &subfunc, sizeof(int));

			__cpuidex(infoWin, funcWin, subfuncWin);

			std::memcpy(info, infoWin, sizeof(info));
			return std::make_tuple(info[0], info[1], info[2], info[3]);
#else
#error "This platform is not supported."
#endif
		}
	}
}
