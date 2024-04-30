// Copyright (c) 2022 SimpleSysIO
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <SimpleObjects/Endianness.hpp>

#include "Internal/SimpleObjects.hpp"


#ifndef SIMPLESYSIO_CUSTOMIZED_NAMESPACE
namespace SimpleSysIO
#else
namespace SIMPLESYSIO_CUSTOMIZED_NAMESPACE
#endif
{
namespace Internal
{

/**
 * @brief Convert the endianness of some value or object
 *
 * @tparam _FromEndian From this endian
 * @tparam _ToEndian Convert to this endian
 */
template<Obj::Endian _FromEndian, Obj::Endian _ToEndian>
struct EndianConvert;


template<Obj::Endian _SameEndian>
struct EndianConvert<_SameEndian, _SameEndian>
{
	template<typename _T>
	static constexpr _T Primitive(_T value) noexcept
	{
		return value;
	}
}; // struct EndianConvert<_SameEndian, _SameEndian>

} // namespace Internal
} // namespace SimpleSysIO
