// Copyright 2005-2014 Daniel James.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  Based on Peter Dimov's proposal
//  http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2005/n1756.pdf
//  issue 6.18.
//
//  This also contains public domain code from MurmurHash. From the
//  MurmurHash header:

// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

// -- SimpleObjects --
// NOTE: this source code is imported from:
// https://github.com/boostorg/container_hash/blob/boost-1.79.0/include/boost/container_hash/hash.hpp
// The only thing we want is to calculate the hash of bytes string
// Instead of adding dependency to the entire boost::container_hash module,
// We only pick out the code we need

// For Future Reference:
// In addition to boost::container_hash, another potential option is
// https://github.com/llvm/llvm-project/blob/llvmorg-14.0.1/libcxx/include/__functional/hash.h

#include <cstddef>
#include <cstdint>

#include <functional>
#include <iterator>
#include <limits>

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{
namespace Internal
{

template<typename _TypeX, typename _TypeR>
inline constexpr _TypeX hash_rotl32(_TypeX x, _TypeR r)
{
	return (x << r) | (x >> (32 - r));
}

template<size_t Bits>
struct hash_combine_impl
{
	template <typename SizeT>
	inline static SizeT fn(SizeT seed, SizeT value)
	{
		seed ^= value + 0x9e3779b9 + (seed<<6) + (seed>>2);
		return seed;
	}
}; // struct hash_combine_impl

template<>
struct hash_combine_impl<32>
{
	inline static uint32_t fn(uint32_t h1, uint32_t k1)
	{
		const uint32_t c1 = 0xcc9e2d51;
		const uint32_t c2 = 0x1b873593;

		k1 *= c1;
		k1 = hash_rotl32(k1, 15);
		k1 *= c2;

		h1 ^= k1;
		h1 = hash_rotl32(h1, 13);
		h1 = h1*5+0xe6546b64;

		return h1;
	}
}; // struct hash_combine_impl

template<>
struct hash_combine_impl<64>
{
	inline static uint64_t fn(uint64_t h, uint64_t k)
	{
		const uint64_t m = (uint64_t(0xc6a4a793) << 32) + 0x5bd1e995;
		const int r = 47;

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;

		// Completely arbitrary number, to prevent 0's
		// from hashing to 0.
		h += 0xe6546b64;

		return h;
	}
}; // struct hash_combine_impl

struct SizeTBitSize
{
	static constexpr size_t charBit = 8;
	static constexpr size_t value = sizeof(size_t) * charBit;
};

template<typename T>
inline void hash_combine(size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed = hash_combine_impl<SizeTBitSize::value>::fn(seed, hasher(v));
}

template<typename It>
inline size_t hash_range(It first, It last)
{
	size_t seed = 0;

	for(; first != last; ++first)
	{
		hash_combine<typename std::iterator_traits<It>::value_type>(
			seed, *first);
	}

	return seed;
}

} //namespace Internal
} // namespace SimpleObjects
