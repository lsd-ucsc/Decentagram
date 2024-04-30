// Copyright (c) 2022 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <array>

#include "../DataTypes.hpp"
#include "../Internal/SimpleRlp.hpp"

namespace EclipseMonitor
{
namespace Eth
{

template<typename _PrimitiveType>
struct PrimitiveTypeTrait
{
	using value_type = _PrimitiveType;

	static value_type FromBytes(const Internal::Rlp::BytesObjType& b)
	{
		size_t i = 0;
		auto res = Internal::Rlp::ParsePrimitiveIntValue<
			value_type,
			Internal::Rlp::Endian::native>::Parse(
				b.size(),
				[&i, &b](){
					return b[i++];
				}
			);
		return res;
	}

	static Internal::Rlp::BytesObjType ToBytes(const value_type& v)
	{
		auto res = Internal::Rlp::BytesObjType();
		Internal::Rlp::EncodePrimitiveIntValue<
			value_type,
			Internal::Rlp::Endian::native,
			false>::
				Encode(res, v);
		return res;
	}
}; // struct PrimitiveTypeTrait

struct BlkNumTypeTrait : PrimitiveTypeTrait<uint64_t>
{}; // struct BlkNumTypeTrait

struct TimeTypeTrait : PrimitiveTypeTrait<uint64_t>
{}; // struct TimeTypeTrait

struct DiffTypeTrait : PrimitiveTypeTrait<uint64_t>
{}; // struct DiffTypeTrait

using BlockNumber = typename BlkNumTypeTrait::value_type;

using Timestamp = typename TimeTypeTrait::value_type;

using Difficulty = typename DiffTypeTrait::value_type;



using ContractAddr = std::array<uint8_t, 20>;

using EventTopic = std::array<uint8_t, 32>;

using EventCallbackId = std::uintptr_t;


} // namespace Eth
} // namespace EclipseMonitor
