/* Copyright (C) 2015, Intel Corporation.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

-       Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.
-       Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.
-       Neither the name of Intel Corporation nor the names of its contributors
		may be used to endorse or promote products derived from this software
		without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL INTEL CORPORATION BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE. */

#pragma once

#include "../../Common.hpp"

#include "Exceptions.hpp"
#include "DrngFeature.hpp"

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
			namespace Internal
			{

#if defined(_WIN32) || defined(MBEDTLSCPP_INTERNAL_INTEL_NO_RDRAND_ASM_INST)

				inline uint8_t rdrand_step(uint16_t* x)
				{
#if defined(__GNUC__)
					uint8_t err = 0;

					asm volatile(".byte 0x66; .byte 0x0f; .byte 0xc7; .byte 0xf0; setc %1"
								: "=a"  (*x),
								  "=qm" (err)
					);

					return err;
#elif defined(_WIN32)
					return static_cast<uint8_t>(_rdrand16_step(x));
#else
#error "This platform is not supported."
#endif
				}

				inline uint8_t rdrand_step(uint32_t* x)
				{
#if defined(__GNUC__)
					uint8_t err = 0;

					asm volatile(".byte 0x0f; .byte 0xc7; .byte 0xf0; setc %1"
								: "=a"  (*x),
								  "=qm" (err)
					);

					return err;
#elif defined(_WIN32)
					return static_cast<uint8_t>(_rdrand32_step(x));
#else
#error "This platform is not supported."
#endif
				}

#ifndef MBEDTLSCPP_INTERNAL_INTEL_NO_UINT64
				inline uint8_t rdrand_step(uint64_t* x)
				{
#if defined(__GNUC__)
					uint8_t err = 0;

					asm volatile(".byte 0x48; .byte 0x0f; .byte 0xc7; .byte 0xf0; setc %1"
								: "=a"  (*x),
								  "=qm" (err)
					);

					return err;
#elif defined(_WIN32)
					return static_cast<uint8_t>(_rdrand64_step(x));
#else
#error "This platform is not supported."
#endif
				}
#endif //MBEDTLSCPP_INTERNAL_INTEL_NO_UINT64

#else
				template<typename T,
					enable_if_t<
						std::is_same<T, uint16_t>::value
						|| std::is_same<T, uint32_t>::value
						|| std::is_same<T, MaxIntType>::value
					, int> = 0>
				inline uint8_t rdrand_step(T* x)
				{
					uint8_t err = 0;

					asm volatile("rdrand %0; setc %1"
								: "=r"  (*x),
								  "=qm" (err)
					);

					return err;
				}
#endif //defined(_WIN32) || defined(MBEDTLSCPP_INTERNAL_INTEL_NO_RDRAND_ASM_INST)





				template<typename T,
					enable_if_t<
						std::is_same<T, uint16_t>::value
						|| std::is_same<T, uint32_t>::value
						|| std::is_same<T, MaxIntType>::value
					, int> = 0>
				inline void rdrand(T* x, bool retry)
				{
					if (!IsRdRandSupportedCached())
					{
						throw FeatureUnsupportedException("mbedTLScpp::Internal::PlatformIntel::Internal - "
							"RDRAND is not supported on this platform.");
					}

					if (retry)
					{
						for (size_t i = 0; i < gsk_rdRandRetryLimit; ++i)
						{
							if (rdrand_step(x) != 0)
							{
								return;
							}
						}

						throw PlatformBusyException("mbedTLScpp::Internal::PlatformIntel::Internal - "
							"RDRAND no response; try again later.");
					}
					else
					{
						if (rdrand_step(x) != 0)
						{
							return;
						}

						throw PlatformBusyException("mbedTLScpp::Internal::PlatformIntel::Internal - "
							"RDRAND no response; try again later.");
					}
				}





				template<typename T,
					enable_if_t<
						std::is_same<T, uint16_t>::value
						|| std::is_same<T, uint32_t>::value
						|| std::is_same<T, MaxIntType>::value
					, int> = 0>
				void rdrand_get_n(size_t n, T *dest)
				{
					for (size_t i = 0; i < n; ++i)
					{
						rdrand(dest, true);
						dest= &(dest[1]);
					}
				}





				void rdrand_get_bytes(size_t n, uint8_t *dest)
				{
					uint8_t *start         = nullptr;
					uint8_t *residualstart = nullptr;
					MaxIntType *blockstart = nullptr;
					MaxIntType temprand    = 0;
					size_t i               = 0;
					size_t count           = 0;
					size_t residual        = 0;
					size_t startlen        = 0;
					size_t length          = 0;

					/* Compute the address of the first 32- or 64- bit aligned
					block in the destination buffer, depending on whether we are
					in 32- or 64-bit mode */
					start = dest;
					if ( ( (MaxIntType)start % (MaxIntType)sizeof(MaxIntType) ) == 0)
					{
						blockstart = (MaxIntType*)start;
						count = n;
						startlen = 0;
					}
					else
					{
						blockstart = (MaxIntType*)(
							( (MaxIntType)start & ~(MaxIntType)(sizeof(MaxIntType) - 1) ) +
							(MaxIntType)sizeof(MaxIntType)
						);
						count = n - (
							sizeof(MaxIntType) - (size_t)( (MaxIntType)start % sizeof(MaxIntType) )
						);
						startlen = (size_t)(
							(MaxIntType)blockstart - (MaxIntType)start
						);
					}

					/* Compute the number of 32- or 64- bit blocks and the
					remaining number of bytes */
					residual = count % sizeof(MaxIntType);
					length   = count / sizeof(MaxIntType);
					if (residual != 0)
					{
						residualstart = (uint8_t*)(blockstart + length);
					}

					/* Get a temporary random number for use in the residuals.
					Failout if retry fails */
					if (startlen > 0)
					{
						rdrand(&temprand, true);
					}

					/* populate the starting misaligned block */
					for (i = 0; i < startlen; i++)
					{
						start[i] = (uint8_t)(temprand & 0xff);
						temprand = temprand >> 8;
					}

					/* populate the central aligned block. Fail out if retry fails */

					rdrand_get_n(length, blockstart);

					/* populate the final misaligned block */
					if (residual > 0)
					{
						rdrand(&temprand, true);

						for (i = 0; i < residual; i++)
						{
							residualstart[i] = (uint8_t)(temprand & 0xff);
							temprand = temprand >> 8;
						}
					}
				}

			}
		}
	}
}
