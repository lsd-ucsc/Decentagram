// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#if defined(DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED) || \
	defined(DECENT_ENCLAVE_PLATFORM_SGX_UNTRUSTED)


#include <cstddef>
#include <cstdint>


namespace DecentEnclave
{
namespace Common
{
namespace Sgx
{

enum class UBufferDataType : uint8_t
{
	Bytes  = 0,
	String = 1,
}; // enum class UBufferDataType

template<typename _DataType>
inline constexpr UBufferDataType GetUBufferDataType();

template<>
inline constexpr UBufferDataType GetUBufferDataType<uint8_t>()
{
	return UBufferDataType::Bytes;
}

template<>
inline constexpr UBufferDataType GetUBufferDataType<char>()
{
	return UBufferDataType::String;
}

} // namespace Sgx
} // namespace Common
} // namespace DecentEnclave

#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED || _UNTRUSTED
