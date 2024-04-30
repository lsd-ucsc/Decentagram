// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#if defined(DECENT_ENCLAVE_PLATFORM_SGX_UNTRUSTED)


#include <cstddef>
#include <cstdint>


namespace DecentEnclave
{
namespace Untrusted
{
namespace Sgx
{

template<typename _DataType>
struct UntrustedBuffer
{
	static UntrustedBuffer Allocate(size_t size)
	{
		if (size == 0)
		{
			return UntrustedBuffer(nullptr, 0);
		}
		else
		{
			return UntrustedBuffer(new _DataType[size], size);
		}
	}

	UntrustedBuffer(
		_DataType* data,
		size_t size
	) noexcept :
		m_data(data),
		m_size(size)
	{}

	~UntrustedBuffer()
	{}

	_DataType* m_data;
	size_t m_size;
}; // struct UntrustedBuffer

} // namespace Sgx
} // namespace Untrusted
} // namespace DecentEnclave

#endif // DECENT_ENCLAVE_PLATFORM_SGX_UNTRUSTED
