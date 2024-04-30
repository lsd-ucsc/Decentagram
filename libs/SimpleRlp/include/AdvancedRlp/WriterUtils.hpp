// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <cstring>

#include "Internal/SimpleRlp.hpp"

#include "Exceptions.hpp"

#ifndef ADVANCEDRLP_CUSTOMIZED_NAMESPACE
namespace AdvancedRlp
#else
namespace ADVANCEDRLP_CUSTOMIZED_NAMESPACE
#endif
{

namespace Internal
{

template<
	SimRlp::Endian _OutEndian,
	SimRlp::Endian _PlatformEndian = SimRlp::Endian::native>
struct PrimitiveToRaw; // struct PrimitiveToRaw


template<>
struct PrimitiveToRaw<
	SimRlp::Endian::little,
	SimRlp::Endian::little>
{
	template<typename _IntType>
	static void FromInt(void* dest, size_t destSize, _IntType val)
	{
		if (destSize != sizeof(_IntType))
		{
			throw SerializeError(
				"The given raw data space doesn't match the size of "
				"the source type");
		}

		std::memcpy(dest, &val, sizeof(_IntType));
	}
}; // struct PrimitiveToRaw

} // namespace Internal

} // namespace AdvancedRlp
