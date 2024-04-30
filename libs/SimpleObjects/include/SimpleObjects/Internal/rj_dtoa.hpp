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
// source: https://github.com/Tencent/rapidjson/blob/master/include/rapidjson/internal/dtoa.h

#pragma once

#include <cstdint>
#include <cstring>

#include <tuple>
#include <utility>

#include "rj_ieee754.hpp"
#include "rj_diyfp.hpp"

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{
namespace Internal {

inline const char* GetDigitsLut() {
	static const char cDigitsLut[200] = {
		'0','0','0','1','0','2','0','3','0','4','0','5','0','6','0','7','0','8','0','9',
		'1','0','1','1','1','2','1','3','1','4','1','5','1','6','1','7','1','8','1','9',
		'2','0','2','1','2','2','2','3','2','4','2','5','2','6','2','7','2','8','2','9',
		'3','0','3','1','3','2','3','3','3','4','3','5','3','6','3','7','3','8','3','9',
		'4','0','4','1','4','2','4','3','4','4','4','5','4','6','4','7','4','8','4','9',
		'5','0','5','1','5','2','5','3','5','4','5','5','5','6','5','7','5','8','5','9',
		'6','0','6','1','6','2','6','3','6','4','6','5','6','6','6','7','6','8','6','9',
		'7','0','7','1','7','2','7','3','7','4','7','5','7','6','7','7','7','8','7','9',
		'8','0','8','1','8','2','8','3','8','4','8','5','8','6','8','7','8','8','8','9',
		'9','0','9','1','9','2','9','3','9','4','9','5','9','6','9','7','9','8','9','9'
	};
	return cDigitsLut;
}

template<typename ContainerType>
inline void GrisuRound(
	ContainerType& buffer,
	uint64_t delta,
	uint64_t rest,
	uint64_t ten_kappa,
	uint64_t wp_w
) {
	while (rest < wp_w && delta - rest >= ten_kappa &&
		(rest + ten_kappa < wp_w ||  /// closer
			wp_w - rest > rest + ten_kappa - wp_w)) {
		buffer.back()--;
		rest += ten_kappa;
	}
}

inline int CountDecimalDigit32(uint32_t n) {
	// Simple pure C++ implementation was faster than __builtin_clz version in this situation.
	if (n < 10) return 1;
	if (n < 100) return 2;
	if (n < 1000) return 3;
	if (n < 10000) return 4;
	if (n < 100000) return 5;
	if (n < 1000000) return 6;
	if (n < 10000000) return 7;
	if (n < 100000000) return 8;
	// Will not reach 10 digits in DigitGen()
	//if (n < 1000000000) return 9;
	//return 10;
	return 9;
}

template<typename ContainerType>
inline void DigitGen(
	const DiyFp& W,
	const DiyFp& Mp,
	uint64_t delta,
	ContainerType& buffer,
	int& K
) {
	static const uint64_t kPow10[] = {
		1U, 10U, 100U, 1000U, 10000U, 100000U, 1000000U, 10000000U, 100000000U,
		1000000000U, 10000000000U, 100000000000U, 1000000000000U,
		10000000000000U, 100000000000000U, 1000000000000000U,
		10000000000000000U, 100000000000000000U, 1000000000000000000U,
		10000000000000000000U
	};
	const DiyFp one(uint64_t(1) << -Mp.e, Mp.e);
	const DiyFp wp_w = Mp - W;
	uint32_t p1 = static_cast<uint32_t>(Mp.f >> -one.e);
	uint64_t p2 = Mp.f & (one.f - 1);
	int kappa = CountDecimalDigit32(p1); // kappa in [0, 9]

	//results
	int signAdjust = (buffer.size() > 0 && buffer[0] == '-') ? -1 : 0;

	while (kappa > 0) {
		uint32_t d = 0;
		switch (kappa) {
			case  9: d = p1 /  100000000; p1 %=  100000000; break;
			case  8: d = p1 /   10000000; p1 %=   10000000; break;
			case  7: d = p1 /    1000000; p1 %=    1000000; break;
			case  6: d = p1 /     100000; p1 %=     100000; break;
			case  5: d = p1 /      10000; p1 %=      10000; break;
			case  4: d = p1 /       1000; p1 %=       1000; break;
			case  3: d = p1 /        100; p1 %=        100; break;
			case  2: d = p1 /         10; p1 %=         10; break;
			case  1: d = p1;              p1 =           0; break;
			default:;
		}
		if (d || (buffer.size() + signAdjust))
			buffer.push_back(static_cast<char>('0' + static_cast<char>(d)));
		kappa--;
		uint64_t tmp = (static_cast<uint64_t>(p1) << -one.e) + p2;
		if (tmp <= delta) {
			K += kappa;
			GrisuRound(buffer, delta, tmp, kPow10[kappa] << -one.e, wp_w.f);
			return;
		}
	}

	// kappa = 0
	for (;;) {
		p2 *= 10;
		delta *= 10;
		char d = static_cast<char>(p2 >> -one.e);
		if (d || (buffer.size() + signAdjust))
			buffer.push_back(static_cast<char>('0' + d));
		p2 &= one.f - 1;
		kappa--;
		if (p2 < delta) {
			K += kappa;
			int index = -kappa;
			GrisuRound(buffer, delta, p2, one.f, wp_w.f * (index < 20 ? kPow10[index] : 0));
			return;
		}
	}

}

template<typename ContainerType>
inline void Grisu2(ContainerType& buf, int& K, double value) {
	const DiyFp v(value);
	DiyFp w_m, w_p;
	v.NormalizedBoundaries(&w_m, &w_p);

	DiyFp mk;
	std::tie(mk, K) = GetCachedPower(w_p.e);
	const DiyFp W = v.Normalize() * mk;
	DiyFp Wp = w_p * mk;
	DiyFp Wm = w_m * mk;
	Wm.f++;
	Wp.f--;
	DigitGen(W, Wp, Wp.f - Wm.f, buf, K);
}

template<typename ContainerType>
inline void WriteExponent(ContainerType& buf, int K) {
	if (K < 0) {
		buf.push_back('-');
		K = -K;
	}

	if (K >= 100) {
		buf.push_back(static_cast<char>('0' + static_cast<char>(K / 100)));
		K %= 100;
		const char* d = GetDigitsLut() + K * 2;
		buf.push_back(d[0]);
		buf.push_back(d[1]);
	}
	else if (K >= 10) {
		const char* d = GetDigitsLut() + K * 2;
		buf.push_back(d[0]);
		buf.push_back(d[1]);
	}
	else
		buf.push_back(static_cast<char>('0' + static_cast<char>(K)));
}

template<typename ContainerType>
inline void Prettify(ContainerType& buf, int k, int16_t maxDecimal) {
	static constexpr size_t maxSigWidth = 21;

	const int signAdjust = (buf.size() > 0 && buf[0] == '-') ? -1 : 0;
	const int sigLen = static_cast<int>(buf.size()) + signAdjust;
	const int kk     = static_cast<int>(buf.size()) + signAdjust + k;  // 10^(kk-1) <= v < 10^kk
	const int maxDecimalPlaces = maxDecimal;

	if (0 <= k && ((kk < 0) || (static_cast<size_t>(kk) <= maxSigWidth))) {
		// 1234e7 -> 12340000000
		buf.insert(buf.end(), k, '0');
		buf.push_back('.');
		buf.push_back('0');
		return;
	}
	else if (0 < kk && static_cast<size_t>(kk) <= maxSigWidth) {
		// implies
		//     (0 <= k) != true
		//     0 > k
		//     buf.size() + k < buf.size()
		//     0 < kk < buf.size()

		// 1234e-2 -> 12.34
		buf.insert(buf.begin() + kk - signAdjust, '.');
		if (0 > k + maxDecimalPlaces) {
			// implies
			//     buf.size() > buf.size() + k + maxDecimalPlaces
			//     buf.size() > kk + maxDecimalPlaces
			//     buf.size() > initial i

			// When maxDecimalPlaces = 2, 1.2345 -> 1.23, 1.102 -> 1.1
			// Remove extra trailing zeros (at least one) after truncation.
			for (int i = kk + maxDecimalPlaces - signAdjust; i > kk + 1 - signAdjust; i--)
				if (buf[i] != '0') {
					buf.resize(i + 1);
					return;
				}

			buf.resize(kk + 2 - signAdjust); // Reserve one zero
			return;
		}
		else
			return;
	}
	else if (-6 < kk && kk <= 0) {
		// 1234e-6 -> 0.001234
		const int offset = 2 - kk;
		buf.insert(buf.begin() - signAdjust, {'0', '.', });
		if(offset - 2 > 0) {
			buf.insert(buf.begin() + 2 - signAdjust, offset - 2, '0');
		}

		if (sigLen - kk > maxDecimalPlaces) {
			// When maxDecimalPlaces = 2, 0.123 -> 0.12, 0.102 -> 0.1
			// Remove extra trailing zeros (at least one) after truncation.
			for (int i = maxDecimalPlaces + 1 - signAdjust; i > 2; i--)
				if (buf[i] != '0') {
					buf.resize(i + 1);
					return;
				}
			buf.resize(3 - signAdjust);
			return; // Reserve one zero
		}
		else {
			return;
		}
	}
	// else if (kk < -maxDecimalPlaces) {
	// 	// Truncate to zero
	// 	buf = (buf.size() > 0 && buf[0] == '-') ? "-0.0" : "0.0";
	// 	return;
	// }
	else if (buf.size() + signAdjust == 1) {
		// 1e30
		buf.push_back('e');
		WriteExponent(buf, kk - 1);
		return ;
	}
	else {
		// 1234e30 -> 1.234e33
		buf.insert(buf.begin() + 1 - signAdjust, '.');
		buf.push_back('e');
		return WriteExponent(buf, kk - 1);
	}
}

/**
 * @brief Converts a double value into a string, using Grisu2 algorithm
 *
 * @tparam ContainerType The type of container
 *
 * @param value The double value needs to be converted
 * @param maxDecimalPlaces The maximum number of decimal digits;
 *                         A input of 0 should have the same effect as 1.
 * @return ContainerType The resulting string.
 */
template<typename ContainerType>
inline ContainerType dtoa(double value, int16_t maxDecimalPlaces = 324) {
	Double d(value);

	if (d.IsZero()) {
		if (d.Sign()) {
			return "-0.0";
		} else {
			return "0.0";
		}
	}
	else {
		ContainerType res;
		int K = 0;

		if (value < 0) {
			res += '-';
			value = -value;
		}

		Grisu2(res, K, value);
		Prettify(res, K, maxDecimalPlaces);

		return res;
	}
}

/**
 * @brief Converts a double value into a string and copy into a
 *        back_insert_iterator, using Grisu2 algorithm
 *
 * @tparam BackInserterType The type of back_insert_iterator
 *
 * @param it The back_insert_iterator
 * @param value The double value needs to be converted
 * @param maxDecimalPlaces The maximum number of decimal digits;
 *                         A input of 0 should have the same effect as 1.
 */
template<typename BackInserterType>
inline void dtoa(BackInserterType it, double value, int16_t maxDecimalPlaces = 324) {
	using ContainerType = typename BackInserterType::container_type;

	ContainerType str = dtoa<ContainerType>(value, maxDecimalPlaces);
	std::copy(str.begin(), str.end(), it);
}

} // namespace Internal
} // namespace SimpleObjects
