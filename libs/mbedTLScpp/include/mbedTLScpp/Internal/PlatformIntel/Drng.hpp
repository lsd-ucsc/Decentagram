#pragma once

#include "Exceptions.hpp"
#include "DrngSeed.hpp"
#include "DrngRand.hpp"

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
			inline size_t ReadSeed(uint8_t* dest, size_t size)
			{
				return Internal::rdseed_get_bytes(size, dest, 0,
					gsk_rdSeedRcRetryPerStep * (
						(size / sizeof(MaxIntType)) + (size % sizeof(MaxIntType) == 0 ? 0 : 1)
					)
				);
			}

			inline void ReadRand(uint8_t* dest, size_t size)
			{
				return Internal::rdrand_get_bytes(size, dest);
			}
		}
	}
}
