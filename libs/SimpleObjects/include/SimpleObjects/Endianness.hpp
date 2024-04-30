// Copyright (c) 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#if __cplusplus >= 201907L
#	include <version>
#	ifdef __cpp_lib_endian
#		include <bit>
#	endif // __cpp_lib_endian
#endif // __cplusplus >= 201907L


#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{

enum class Endian
{
#ifdef __cpp_lib_endian
	little =
		static_cast<std::underlying_type_t<std::endian> >(std::endian::little),
	big    =
		static_cast<std::underlying_type_t<std::endian> >(std::endian::big),
	native =
		static_cast<std::underlying_type_t<std::endian> >(std::endian::native),
// src: https://en.cppreference.com/w/cpp/types/endian
#elif defined(_WIN32)
	little = 0,
	big    = 1,
	native = little,
#elif defined(__ORDER_LITTLE_ENDIAN__) && \
		defined(__ORDER_BIG_ENDIAN__) && \
		defined(__BYTE_ORDER__)
	little = __ORDER_LITTLE_ENDIAN__,
	big    = __ORDER_BIG_ENDIAN__,
	native = __BYTE_ORDER__,
#else
#	error "Cannot determine the platform endianness"
#endif
}; // enum class Endian

} // namespace SimpleObjects
