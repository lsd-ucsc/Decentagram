// Copyright (c) 2023 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>


#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{
namespace Codec
{


enum class HexPad : uint8_t
{
	Disabled,
	Front,
}; // enum class HexPad


enum class HexZero : uint8_t
{
	Keep,
	AtLeastOne,
	SkipAll,
}; // enum class HexZero


} // namespace Codec
} // namespace SimpleObjects
