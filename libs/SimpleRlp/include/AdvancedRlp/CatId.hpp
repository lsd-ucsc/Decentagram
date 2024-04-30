// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <cstdint>

#include "Exceptions.hpp"

#ifndef ADVANCEDRLP_CUSTOMIZED_NAMESPACE
namespace AdvancedRlp
#else
namespace ADVANCEDRLP_CUSTOMIZED_NAMESPACE
#endif
{

enum class CatId : uint8_t
{
	// Primitive data types
	Bytes                         = 0x00U,
	Null                          = 0x01U,
	False                         = 0x02U,
	True                          = 0x03U,
	Integer                       = 0x04U,
	Float                         = 0x05U,
	String                        = 0x08U,

	// Composite data types
	Array                         = 0x20U,
	Dict                          = 0x22U,
	StaticDict                    = 0x23U,
}; // enum class CatId


inline CatId GetCatIdFromByte(uint8_t idByte)
{
	switch (idByte)
	{
	case static_cast<uint8_t>(CatId::Bytes):
		return CatId::Bytes;
	case static_cast<uint8_t>(CatId::Null):
		return CatId::Null;
	case static_cast<uint8_t>(CatId::False):
		return CatId::False;
	case static_cast<uint8_t>(CatId::True):
		return CatId::True;
	case static_cast<uint8_t>(CatId::Integer):
		return CatId::Integer;
	case static_cast<uint8_t>(CatId::Float):
		return CatId::Float;
	case static_cast<uint8_t>(CatId::String):
		return CatId::String;
	case static_cast<uint8_t>(CatId::Array):
		return CatId::Array;
	case static_cast<uint8_t>(CatId::Dict):
		return CatId::Dict;
	case static_cast<uint8_t>(CatId::StaticDict):
		return CatId::StaticDict;

	default:
		throw UnknownCatId(idByte);
	}
}


inline constexpr uint8_t SerializeCatId(CatId catId)
{
	static_assert(
		std::is_same<
			std::underlying_type<CatId>::type,
			uint8_t>::value,
		"The underlying type of CAT ID should be uint8_t");

	return static_cast<uint8_t>(catId);
}

} // namespace AdvancedRlp
