// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <SimpleObjects/Endianness.hpp>

#include "Internal/SimpleObjects.hpp"


#ifndef SIMPLERLP_CUSTOMIZED_NAMESPACE
namespace SimpleRlp
#else
namespace SIMPLERLP_CUSTOMIZED_NAMESPACE
#endif
{

/**
 * @brief The categories of RLP encoding types
 *
 */
enum class RlpEncTypeCat
{
	Bytes,
	List
}; // enum class RlpEncTypeCat

/**
 * @brief The RLP encoding types
 *
 */
enum class RlpEncodeType
{
	Byte,
	BytesShort,
	BytesLong,
	ListShort,
	ListLong,
}; // enum class RlpEncodeType

namespace Internal
{

template<RlpEncTypeCat _Cat>
struct RlpEncTypeCatName;

template<>
struct RlpEncTypeCatName<RlpEncTypeCat::Bytes>
{
	static const char* value()
	{
		return "Bytes";
	}
}; // struct RlpEncTypeCatName<RlpEncTypeCat::Bytes>

template<>
struct RlpEncTypeCatName<RlpEncTypeCat::List>
{
	static const char* value()
	{
		return "List";
	}
}; // struct RlpEncTypeCatName<RlpEncTypeCat::List>

} // namespace Internal

using Endian = Internal::Obj::Endian;

} // namespace SimpleRlp
