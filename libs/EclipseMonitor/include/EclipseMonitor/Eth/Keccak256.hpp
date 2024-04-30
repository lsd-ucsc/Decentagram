// Copyright (c) 2022 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

// Original source: https://github.com/ethereum/solidity/blob/develop/libsolutil/Keccak256.cpp
// Code changes from original source:
// - removed dependencies on external libraries
// - replaced macros with constexpr variables and inline functions
// - changed namespace name

/*
	This file is part of solidity.
	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0
/** @file SHA3.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <cstdint>
#include <cstring>

#include <array>
#include <vector>

namespace EclipseMonitor
{
/** libkeccak-tiny
 *
 * A single-file implementation of SHA-3 and SHAKE.
 *
 * Implementor: David Leon Gil
 * License: CC0, attribution kindly requested. Blame taken too,
 * but not liability.
 */

/******** The Keccak-f[1600] permutation ********/

namespace Internal
{

namespace EthKeccak256
{

/*** Constants. ***/
inline constexpr std::array<uint8_t, 24> GetRho()
{
	return {1,  3,   6, 10, 15, 21,
			28, 36, 45, 55,  2, 14,
			27, 41, 56,  8, 25, 43,
			62, 18, 39, 61, 20, 44};
}

inline constexpr std::array<uint8_t, 24> GetPi()
{
	return {10,  7, 11, 17, 18, 3,
			 5, 16,  8, 21, 24, 4,
			15, 23, 19, 13, 12, 2,
			20, 14, 22,  9, 6,  1};
}

inline constexpr std::array<uint64_t, 24> GetRC()
{
	return {1ULL, 0x8082ULL, 0x800000000000808aULL, 0x8000000080008000ULL,
	0x808bULL, 0x80000001ULL, 0x8000000080008081ULL, 0x8000000000008009ULL,
	0x8aULL, 0x88ULL, 0x80008009ULL, 0x8000000aULL,
	0x8000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL, 0x8000000000008003ULL,
	0x8000000000008002ULL, 0x8000000000000080ULL, 0x800aULL, 0x800000008000000aULL,
	0x8000000080008081ULL, 0x8000000000008080ULL, 0x80000001ULL, 0x8000000080008008ULL};
}

/*** Helper to unroll the permutation. ***/

template<typename _XT, typename _ST>
inline constexpr _XT Rol(_XT x, _ST s)
{
	return (((x) << s) | ((x) >> (64 - s)));
}

template<typename _FuncT>
inline void Repeat5(_FuncT e)
{
	e();e();e();
	e();e();
}

template<typename _FuncT>
inline void Repeat6(_FuncT e)
{
	e();e();e();
	e();e();e();
}

template<typename _FuncT>
inline void Repeat24(_FuncT e)
{
	Repeat6(e);
	Repeat6(e);
	Repeat6(e);
	Repeat6(e);
}

template<typename _VT, typename _ST, typename _FuncT>
inline void For5(_VT& v, _ST s, _FuncT e)
{
	v = 0;
	Repeat5([&](){
		e();
		v = static_cast<_VT>(v + s);
	});
}

/*** Keccak-f[1600] ***/
inline void Keccakf(void* state)
{
	auto* a = static_cast<uint64_t*>(state);
	uint64_t b[5] = {0};

	for (int i = 0; i < 24; i++)
	{
		uint8_t x, y;

		// Theta
		For5<uint8_t>(x, 1, [&](){
			b[x] = 0;
			For5<uint8_t>(y, 5, [&](){
				b[x] ^= a[x + y];
			});
		});

		For5<uint8_t>(x, 1, [&](){
			For5<uint8_t>(y, 5, [&](){
				a[y + x] ^= b[(x + 4) % 5] ^ Rol(b[(x + 1) % 5], 1);
			});
		});

		// Rho and pi
		uint64_t t = a[1];
		x = 0;
		Repeat24([&](){
			b[0] = a[GetPi()[x]];
			a[GetPi()[x]] = Rol(t, GetRho()[x]);
			t = b[0];
			x++;
		});

		// Chi
		For5<uint8_t>(y, 5, [&](){
			For5<uint8_t>(x, 1, [&](){
				b[x] = a[y + x];
			});
			For5<uint8_t>(x, 1, [&](){
				a[y + x] = b[x] ^ ((~b[(x + 1) % 5]) & b[(x + 2) % 5]);
			});
		});

		// Iota
		a[0] ^= GetRC()[i];
	}
}

/******** The FIPS202-defined functions. ********/

/*** Some helper functions. ***/
template<typename _FuncT>
inline void ForHelper(size_t& i, size_t step, size_t limit, _FuncT s)
{
	do {
		for (i = 0; i < limit; i += step) { s(); }
	} while (0);
}

inline void Xorin(uint8_t* dst, uint8_t const* src, size_t len)
{
	size_t i = 0;
	ForHelper(i, 1, len, [&](){
		dst[i] ^= src[i];
	});
}

inline void Setout(uint8_t const* src, uint8_t* dst, size_t len)
{
	size_t i = 0;
	ForHelper(i, 1, len, [&](){
		dst[i] = src[i];
	});
}

inline constexpr size_t GetPlen()
{
	return 200;
}

// Fold P*F over the full blocks of an input.
template<typename _AT, typename _IT, typename _LT, typename _RateT, typename _FuncT>
inline void FoldP(_AT& a, _IT& i, _LT& l, _RateT rate, _FuncT f)
{
	while (l >= rate)
	{
		f(a, i, rate);
		Keccakf(a);
		i += rate;
		l -= rate;
	}
}

/** The sponge-based hash construction. **/
inline void Hash(
	uint8_t* out,
	size_t outlen,
	uint8_t const* in,
	size_t inlen,
	size_t rate,
	uint8_t delim
)
{
	uint8_t a[GetPlen()] = {0};
	// Absorb input.
	FoldP(a, in, inlen, rate, Xorin);
	// Xor in the DS and pad frame.
	a[inlen] ^= delim;
	a[rate - 1] ^= 0x80;
	// Xor in the last block.
	Xorin(a, in, inlen);
	// Apply P
	Keccakf(a);
	// Squeeze output.
	FoldP(a, out, outlen, rate, Setout);
	Setout(a, out, outlen);
	std::memset(a, 0, GetPlen());
}

} // namespace EthKeccak256

} // namespace Internal


namespace Eth
{


/**
 * @brief Compute the Ethereum Keccak-256 hash of a given input.
 *
 * @param input     pointer to the input data
 * @param inputSize size of the input data
 * @return Keccak-256 hash of the input
 */
inline std::array<uint8_t, 32> Keccak256(
	const uint8_t* input,
	size_t inputSize
)
{
	std::array<uint8_t, 32> output;
	// Parameters used:
	// The 0x01 is the specific padding for keccak (sha3 uses 0x06) and
	// the way the round size (or window or whatever it was) is calculated.
	// 200 - (256 / 4) is the "rate"
	static constexpr uint8_t padding = 0x01U;

	Internal::EthKeccak256::Hash(
		output.data(), output.size(),
		input, inputSize,
		200 - (256 / 4),
		padding);
	return output;
}

/**
 * @brief Compute the Ethereum Keccak-256 hash of a given input.
 *
 * @tparam _ContainerT type of the container holding the input data
 * @param _input       input data
 * @return Keccak-256 hash of the input
 */
template<typename _ContainerT>
inline std::array<uint8_t, 32> Keccak256(const _ContainerT& _input)
{
	static constexpr size_t sk_valueSize =
		sizeof(typename _ContainerT::value_type);

	const uint8_t* inputPtr = reinterpret_cast<const uint8_t*>(_input.data());
	size_t inputSize = _input.size() * sk_valueSize;

	return Keccak256(inputPtr, inputSize);
}


} // namespace Eth
} // namespace EclipseMonitor
