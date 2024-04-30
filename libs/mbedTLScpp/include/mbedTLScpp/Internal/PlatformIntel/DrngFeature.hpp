#pragma once

#include <cstring>

#include "../CpuId.hpp"

#include "Exceptions.hpp"

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
			constexpr uint32_t gsk_rdSeedMask = 0x00040000U;
			constexpr uint32_t gsk_rdRandMask = 0x40000000U;

			/**
			 * @brief Recommended number of retries per step for RDSEED.
			 *
			 */
			constexpr size_t gsk_rdSeedRcRetryPerStep = 20;

			constexpr size_t gsk_rdRandRetryLimit = 10;

#ifdef MBEDTLSCPP_INTERNAL_INTEL_NO_UINT64
			using MaxIntType = uint32_t;
#else
			using MaxIntType = uint64_t;
#endif //MBEDTLSCPP_INTERNAL_INTEL_NO_UINT64

			inline bool IsIntelProcessor()
			{
				uint32_t ebx = 0;
				uint32_t ecx = 0;
				uint32_t edx = 0;

				std::tie(std::ignore, ebx, ecx, edx) = RunCpuid(0x00, 0x00);

#ifdef DEBUG
				char str[sizeof(uint32_t) * 3];
				std::memcpy(&str[sizeof(uint32_t) * 0], &ebx, sizeof(uint32_t));
				std::memcpy(&str[sizeof(uint32_t) * 1], &edx, sizeof(uint32_t));
				std::memcpy(&str[sizeof(uint32_t) * 2], &ecx, sizeof(uint32_t));
#endif
				return  std::memcmp(&ebx, "Genu", 4) == 0 &&
						std::memcmp(&edx, "ineI", 4) == 0 &&
						std::memcmp(&ecx, "ntel", 4) == 0;
			}

			inline bool IsRdSeedSupported()
			{
				if(!IsIntelProcessor())
				{
					return false;
				}

				uint32_t ebx = 0;

				std::tie(std::ignore, ebx, std::ignore, std::ignore) =
					RunCpuid(0x07, 0x00);

				return (ebx & gsk_rdSeedMask) == gsk_rdSeedMask;
			}

			inline bool IsRdSeedSupportedCached()
			{
				static bool isRdSeedSupported = IsRdSeedSupported();
				return isRdSeedSupported;
			}

			inline bool IsRdRandSupported()
			{
				if(!IsIntelProcessor())
				{
					return false;
				}

				uint32_t ecx = 0;

				std::tie(std::ignore, std::ignore, ecx, std::ignore) =
					RunCpuid(0x01, 0x00);

				return (ecx & gsk_rdRandMask) == gsk_rdRandMask;
			}

			inline bool IsRdRandSupportedCached()
			{
				static bool isRdRandSupported = IsRdRandSupported();
				return isRdRandSupported;
			}

		}
	}
}
