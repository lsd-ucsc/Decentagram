// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once



#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED


#include "../../Common/Sgx/Exceptions.hpp"
#include "../../Common/Sgx/UntrustedBuffer.hpp"
#include "../../SgxEdgeSources/sys_io_t.h"


namespace DecentEnclave
{
namespace Trusted
{
namespace Sgx
{

template<typename _DataType>
struct UntrustedBuffer
{
public: // static members:

	static constexpr Common::Sgx::UBufferDataType sk_dataType =
		Common::Sgx::GetUBufferDataType<_DataType>();

public:

	UntrustedBuffer() :
		m_data(nullptr),
		m_size(0)
	{}

	UntrustedBuffer(_DataType* ptr, size_t size) :
		m_data(ptr),
		m_size(size)
	{}

	UntrustedBuffer(const UntrustedBuffer&) = delete;

	UntrustedBuffer(UntrustedBuffer&& rhs) :
		m_data(rhs.m_data),
		m_size(rhs.m_size)
	{
		rhs.m_data = nullptr;
		rhs.m_size = 0;
	}

	~UntrustedBuffer()
	{
		if (m_data != nullptr)
		{
			ocall_decent_untrusted_buffer_delete(
				static_cast<uint8_t>(sk_dataType),
				m_data
			);
			m_data = nullptr;
			m_size = 0;
		}
	}

	template<typename _OutContainerType>
	_OutContainerType CopyToContainer() const
	{
		using _OutValueType = typename _OutContainerType::value_type;
		static_assert(
			std::is_same<_OutValueType, _DataType>::value,
			"The output container is having a different value type"
		);

		_OutContainerType ret(m_size, _OutValueType());
		std::copy(m_data, m_data + m_size, &(ret[0]));
		return ret;
	}

	_DataType* m_data;
	size_t m_size;

}; // struct UntrustedBuffer

} // namespace Sgx
} // namespace Trusted
} // namespace DecentEnclave

#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
