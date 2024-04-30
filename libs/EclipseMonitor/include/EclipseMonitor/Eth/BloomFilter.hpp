// Copyright (c) 2023 EclipseMonitor, EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <functional>
#include <initializer_list>
#include <type_traits>
#include <vector>

#include "../Exceptions.hpp"
#include "../Internal/SimpleObj.hpp"
#include "Keccak256.hpp"


namespace EclipseMonitor
{


namespace Internal
{

// From: https://en.cppreference.com/w/cpp/types/conjunction
template<bool _C, typename _TType, typename _FType>
using conditional_t =
	typename std::conditional<_C, _TType, _FType>::type;

template<class...>
struct conjunction :
	std::true_type
{};

template<class B1>
struct conjunction<B1> :
	B1
{};

template<class B1, class... Bn>
struct conjunction<B1, Bn...> :
	conditional_t<bool(B1::value), conjunction<Bn...>, B1>
{};

} // namespace Internal


namespace Eth
{

class BloomFilter
{
public: // static members:

	using EventHash = std::array<uint8_t, 32>;
	using EventHashKRef = std::reference_wrapper<const EventHash>;

	static constexpr size_t sk_bloomBitSize = 2048;
	static constexpr size_t sk_bloomByteSize = sk_bloomBitSize / 8;

public:

	BloomFilter(const Internal::Obj::Bytes& bloomBytes) :
		m_bloomBeginPtr(CheckBloomBytes(bloomBytes))
	{}


	~BloomFilter() = default;


	bool CheckBloomBits(
		const std::array<uint8_t, 32>& hashedData
	) const
	{
		// Adapted from: https://github.com/noxx3xxon/evm-by-example
		uint8_t v1 = (1 << (hashedData[1] & 0x7));
		uint8_t v2 = (1 << (hashedData[3] & 0x7));
		uint8_t v3 = (1 << (hashedData[5] & 0x7));

		uint16_t i1 = hashedData[0] << 8 | hashedData[1];
		i1 = (i1 & 0x7FF) >> 3;
		i1 = (256 - i1 - 1);

		uint16_t i2 = hashedData[2] << 8 | hashedData[3];
		i2 = (i2 & 0x7FF) >> 3;
		i2 = (256 - i2 - 1);

		uint16_t i3 = hashedData[4] << 8 | hashedData[5];
		i3 = (i3 & 0x7FF) >> 3;
		i3 = (256 - i3 - 1);

		bool inBloom = (
			(m_bloomBeginPtr[i1] & v1) &&
			(m_bloomBeginPtr[i2] & v2) &&
			(m_bloomBeginPtr[i3] & v3)
		);

		return inBloom;
	}


	template<typename _It>
	bool AreHashesInBloom(_It begin, _It end) const
	{
		for (auto it = begin; it != end; ++it)
		{
			if (!CheckBloomBits(*it))
			{
				return false;
			}
		}
		return true;
	}


	bool AreHashesInBloom(
		std::initializer_list<EventHashKRef> hashes
	) const
	{
		return AreHashesInBloom(hashes.begin(), hashes.end());
	}


	template< typename... _ArgTs >
	bool IsHashInBloom(const _ArgTs&... hashes) const
	{
		static_assert(
			Internal::conjunction<std::is_same<_ArgTs, EventHash>...>::value,
			"Invalid hash type"
		);

		return AreHashesInBloom({ EventHashKRef(hashes)... });
	}


	template< typename... _ArgTs >
	bool IsEventInBloom(const _ArgTs&... eventData) const
	{
		return IsHashInBloom(
			Keccak256(eventData)...
		);
	}


	size_t Count1Bits() const
	{
		size_t count = 0;
		for (size_t i = 0; i < sk_bloomByteSize; ++i)
		{
			count += Count1BitsInByte(m_bloomBeginPtr[i]);
		}
		return count;
	}


	size_t Count0Bits() const
	{
		return sk_bloomBitSize - Count1Bits();
	}


private:

	static size_t Count1BitsInByte(uint8_t byte)
	{
		size_t count = 0;
		count += (byte & 0x01);
		count += (byte & 0x02) >> 1;
		count += (byte & 0x04) >> 2;
		count += (byte & 0x08) >> 3;
		count += (byte & 0x10) >> 4;
		count += (byte & 0x20) >> 5;
		count += (byte & 0x40) >> 6;
		count += (byte & 0x80) >> 7;
		return count;
	}

	static const uint8_t* CheckBloomBytes(
		const Internal::Obj::Bytes& bloomBytes
	)
	{
		if (bloomBytes.size() != sk_bloomByteSize)
		{
			throw Exception("Invalid bloom bytes size");
		}

		return bloomBytes.data();
	}


	const uint8_t* m_bloomBeginPtr;

}; // class BloomFilter

} // namespace Eth
} // namespace EclipseMonitor
