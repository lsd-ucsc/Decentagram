// Tencent is pleased to support the open source community by making RapidJSON available.
//
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip.
//
// Licensed under the MIT License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

// This is a C++ header-only implementation of Grisu2 algorithm from the publication:
// Loitsch, Florian. "Printing floating-point numbers quickly and accurately with
// integers." ACM Sigplan Notices 45.6 (2010): 233-243.

// imported & modified by SimpleObjects , 2022
// source: https://github.com/Tencent/rapidjson/blob/master/include/rapidjson/internal/diyfp.h

#pragma once

#include <cstddef>
#include <cstdint>

#include <utility>
#include <stdexcept>

#include "rj_common.hpp"

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{
namespace Internal {

/**
 * @brief Count number of leading 0 bits
 *
 * @param x testing number
 * @return uint32_t number of leading zero bits
 */
inline uint32_t clzll(uint64_t x) {
	// Passing 0 to __builtin_clzll is UB in GCC and results in an
	// infinite loop in the software implementation.
	if(x == 0) {
		return 64;
	}

	// naive version
	static constexpr uint64_t leadingOne = (static_cast<uint64_t>(1) << 63);
	static_assert(leadingOne == 0x8000000000000000ULL, "Incorrect leading 1 constant");

	uint32_t r = 0;
	while (!(x & leadingOne)) {
		x <<= 1;
		++r;
	}

	return r;
}

struct DiyFp {
	// static members:
	static constexpr int kDiySignificandSize     = 64;
	static constexpr int kDpSignificandSize      = 52;
	static constexpr int kDpExponentBias         = 0x3FF + kDpSignificandSize;
	static constexpr int kDpMaxExponent          = 0x7FF - kDpExponentBias;
	static constexpr int kDpMinExponent          = -kDpExponentBias;
	static constexpr int kDpDenormalExponent     = -kDpExponentBias + 1;
	static constexpr uint64_t kDpExponentMask    = RjUint64C2(0x7FF00000, 0x00000000);
	static constexpr uint64_t kDpSignificandMask = RjUint64C2(0x000FFFFF, 0xFFFFFFFF);
	static constexpr uint64_t kDpHiddenBit       = RjUint64C2(0x00100000, 0x00000000);

	DiyFp() : f(), e() {}

	DiyFp(uint64_t fp, int exp) : f(fp), e(exp) {}

	explicit DiyFp(double d) {
		union {
			double d;
			uint64_t u64;
		} u = { d };

		int biased_e = static_cast<int>((u.u64 & kDpExponentMask) >> kDpSignificandSize);
		uint64_t significand = (u.u64 & kDpSignificandMask);
		if (biased_e != 0) {
			f = significand + kDpHiddenBit;
			e = biased_e - kDpExponentBias;
		}
		else {
			f = significand;
			e = kDpMinExponent + 1;
		}
	}

	DiyFp operator-(const DiyFp& rhs) const {
		return DiyFp(f - rhs.f, e);
	}

	DiyFp operator*(const DiyFp& rhs) const {
#if defined(_MSC_VER) && defined(_M_AMD64) && !defined(DECENT_ENCLAVE_TRUSTED)
		uint64_t h;
		uint64_t l = _umul128(f, rhs.f, &h);
		if (l & (uint64_t(1) << 63)) // rounding
			h++;
		return DiyFp(h, e + rhs.e + 64);
#elif (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) && defined(__x86_64__)
		__extension__ typedef unsigned __int128 uint128;
		uint128 p = static_cast<uint128>(f) * static_cast<uint128>(rhs.f);
		uint64_t h = static_cast<uint64_t>(p >> 64);
		uint64_t l = static_cast<uint64_t>(p);
		if (l & (uint64_t(1) << 63)) // rounding
			h++;
		return DiyFp(h, e + rhs.e + 64);
#else
		const uint64_t M32 = 0xFFFFFFFF;
		const uint64_t a = f >> 32;
		const uint64_t b = f & M32;
		const uint64_t c = rhs.f >> 32;
		const uint64_t d = rhs.f & M32;
		const uint64_t ac = a * c;
		const uint64_t bc = b * c;
		const uint64_t ad = a * d;
		const uint64_t bd = b * d;
		uint64_t tmp = (bd >> 32) + (ad & M32) + (bc & M32);
		tmp += 1U << 31;  /// mult_round
		return DiyFp(ac + (ad >> 32) + (bc >> 32) + (tmp >> 32), e + rhs.e + 64);
#endif
	}

	DiyFp Normalize() const {
		int s = static_cast<int>(clzll(f));
		return DiyFp(f << s, e - s);
	}

	DiyFp NormalizeBoundary() const {
		DiyFp res = *this;
		while (!(res.f & (kDpHiddenBit << 1))) {
			res.f <<= 1;
			res.e--;
		}
		res.f <<= (kDiySignificandSize - kDpSignificandSize - 2);
		res.e = res.e - (kDiySignificandSize - kDpSignificandSize - 2);
		return res;
	}

	void NormalizedBoundaries(DiyFp* minus, DiyFp* plus) const {
		DiyFp pl = DiyFp((f << 1) + 1, e - 1).NormalizeBoundary();
		DiyFp mi = (f == kDpHiddenBit) ? DiyFp((f << 2) - 1, e - 2) : DiyFp((f << 1) - 1, e - 1);
		mi.f <<= mi.e - pl.e;
		mi.e = pl.e;
		*plus = pl;
		*minus = mi;
	}

	uint64_t f;
	int e;
};

inline DiyFp GetCachedPowerByIndex(size_t index) {
	// 10^-348, 10^-340, ..., 10^340
	static constexpr uint64_t kCachedPowers_F[] = {
		RjUint64C2(0xfa8fd5a0, 0x081c0288), RjUint64C2(0xbaaee17f, 0xa23ebf76),
		RjUint64C2(0x8b16fb20, 0x3055ac76), RjUint64C2(0xcf42894a, 0x5dce35ea),
		RjUint64C2(0x9a6bb0aa, 0x55653b2d), RjUint64C2(0xe61acf03, 0x3d1a45df),
		RjUint64C2(0xab70fe17, 0xc79ac6ca), RjUint64C2(0xff77b1fc, 0xbebcdc4f),
		RjUint64C2(0xbe5691ef, 0x416bd60c), RjUint64C2(0x8dd01fad, 0x907ffc3c),
		RjUint64C2(0xd3515c28, 0x31559a83), RjUint64C2(0x9d71ac8f, 0xada6c9b5),
		RjUint64C2(0xea9c2277, 0x23ee8bcb), RjUint64C2(0xaecc4991, 0x4078536d),
		RjUint64C2(0x823c1279, 0x5db6ce57), RjUint64C2(0xc2109436, 0x4dfb5637),
		RjUint64C2(0x9096ea6f, 0x3848984f), RjUint64C2(0xd77485cb, 0x25823ac7),
		RjUint64C2(0xa086cfcd, 0x97bf97f4), RjUint64C2(0xef340a98, 0x172aace5),
		RjUint64C2(0xb23867fb, 0x2a35b28e), RjUint64C2(0x84c8d4df, 0xd2c63f3b),
		RjUint64C2(0xc5dd4427, 0x1ad3cdba), RjUint64C2(0x936b9fce, 0xbb25c996),
		RjUint64C2(0xdbac6c24, 0x7d62a584), RjUint64C2(0xa3ab6658, 0x0d5fdaf6),
		RjUint64C2(0xf3e2f893, 0xdec3f126), RjUint64C2(0xb5b5ada8, 0xaaff80b8),
		RjUint64C2(0x87625f05, 0x6c7c4a8b), RjUint64C2(0xc9bcff60, 0x34c13053),
		RjUint64C2(0x964e858c, 0x91ba2655), RjUint64C2(0xdff97724, 0x70297ebd),
		RjUint64C2(0xa6dfbd9f, 0xb8e5b88f), RjUint64C2(0xf8a95fcf, 0x88747d94),
		RjUint64C2(0xb9447093, 0x8fa89bcf), RjUint64C2(0x8a08f0f8, 0xbf0f156b),
		RjUint64C2(0xcdb02555, 0x653131b6), RjUint64C2(0x993fe2c6, 0xd07b7fac),
		RjUint64C2(0xe45c10c4, 0x2a2b3b06), RjUint64C2(0xaa242499, 0x697392d3),
		RjUint64C2(0xfd87b5f2, 0x8300ca0e), RjUint64C2(0xbce50864, 0x92111aeb),
		RjUint64C2(0x8cbccc09, 0x6f5088cc), RjUint64C2(0xd1b71758, 0xe219652c),
		RjUint64C2(0x9c400000, 0x00000000), RjUint64C2(0xe8d4a510, 0x00000000),
		RjUint64C2(0xad78ebc5, 0xac620000), RjUint64C2(0x813f3978, 0xf8940984),
		RjUint64C2(0xc097ce7b, 0xc90715b3), RjUint64C2(0x8f7e32ce, 0x7bea5c70),
		RjUint64C2(0xd5d238a4, 0xabe98068), RjUint64C2(0x9f4f2726, 0x179a2245),
		RjUint64C2(0xed63a231, 0xd4c4fb27), RjUint64C2(0xb0de6538, 0x8cc8ada8),
		RjUint64C2(0x83c7088e, 0x1aab65db), RjUint64C2(0xc45d1df9, 0x42711d9a),
		RjUint64C2(0x924d692c, 0xa61be758), RjUint64C2(0xda01ee64, 0x1a708dea),
		RjUint64C2(0xa26da399, 0x9aef774a), RjUint64C2(0xf209787b, 0xb47d6b85),
		RjUint64C2(0xb454e4a1, 0x79dd1877), RjUint64C2(0x865b8692, 0x5b9bc5c2),
		RjUint64C2(0xc83553c5, 0xc8965d3d), RjUint64C2(0x952ab45c, 0xfa97a0b3),
		RjUint64C2(0xde469fbd, 0x99a05fe3), RjUint64C2(0xa59bc234, 0xdb398c25),
		RjUint64C2(0xf6c69a72, 0xa3989f5c), RjUint64C2(0xb7dcbf53, 0x54e9bece),
		RjUint64C2(0x88fcf317, 0xf22241e2), RjUint64C2(0xcc20ce9b, 0xd35c78a5),
		RjUint64C2(0x98165af3, 0x7b2153df), RjUint64C2(0xe2a0b5dc, 0x971f303a),
		RjUint64C2(0xa8d9d153, 0x5ce3b396), RjUint64C2(0xfb9b7cd9, 0xa4a7443c),
		RjUint64C2(0xbb764c4c, 0xa7a44410), RjUint64C2(0x8bab8eef, 0xb6409c1a),
		RjUint64C2(0xd01fef10, 0xa657842c), RjUint64C2(0x9b10a4e5, 0xe9913129),
		RjUint64C2(0xe7109bfb, 0xa19c0c9d), RjUint64C2(0xac2820d9, 0x623bf429),
		RjUint64C2(0x80444b5e, 0x7aa7cf85), RjUint64C2(0xbf21e440, 0x03acdd2d),
		RjUint64C2(0x8e679c2f, 0x5e44ff8f), RjUint64C2(0xd433179d, 0x9c8cb841),
		RjUint64C2(0x9e19db92, 0xb4e31ba9), RjUint64C2(0xeb96bf6e, 0xbadf77d9),
		RjUint64C2(0xaf87023b, 0x9bf0ee6b)
	};
	static constexpr size_t kCachedPowers_F_Len = sizeof(kCachedPowers_F) / sizeof(uint64_t);
	static constexpr int16_t kCachedPowers_E[] = {
		-1220, -1193, -1166, -1140, -1113, -1087, -1060, -1034, -1007,  -980,
		 -954,  -927,  -901,  -874,  -847,  -821,  -794,  -768,  -741,  -715,
		 -688,  -661,  -635,  -608,  -582,  -555,  -529,  -502,  -475,  -449,
		 -422,  -396,  -369,  -343,  -316,  -289,  -263,  -236,  -210,  -183,
		 -157,  -130,  -103,   -77,   -50,   -24,     3,    30,    56,    83,
		  109,   136,   162,   189,   216,   242,   269,   295,   322,   348,
		  375,   402,   428,   455,   481,   508,   534,   561,   588,   614,
		  641,   667,   694,   720,   747,   774,   800,   827,   853,   880,
		  907,   933,   960,   986,  1013,  1039,  1066
	};
	static constexpr size_t kCachedPowers_E_Len = sizeof(kCachedPowers_E) / sizeof(int16_t);
	static_assert(kCachedPowers_F_Len == kCachedPowers_E_Len, "The length of cached table does not match.");

	if (index >= kCachedPowers_F_Len) {
		throw std::invalid_argument("The requested result is over the length of the cached powers table.");
	}
	return DiyFp(kCachedPowers_F[index], kCachedPowers_E[index]);
}

inline std::pair<DiyFp, int> GetCachedPower(int e) {
	//int k = static_cast<int>(ceil((-61 - e) * 0.30102999566398114)) + 374;
	double dk = (-61 - e) * 0.30102999566398114 + 347;  // dk must be positive, so can do ceiling in positive
	int k = static_cast<int>(dk);
	if (dk - k > 0.0)
		k++;

	unsigned index = static_cast<unsigned>((k >> 3) + 1);
	int K = -(-348 + static_cast<int>(index << 3));    // decimal exponent no need lookup table

	return std::make_pair(GetCachedPowerByIndex(index), K);
}

} // namespace Internal
} // namespace SimpleObjects
