// Copyright (c) 2022 SimpleSysIO
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include "IOStreamBase.hpp"

#include <memory>


#ifndef SIMPLESYSIO_CUSTOMIZED_NAMESPACE
namespace SimpleSysIO
#else
namespace SIMPLESYSIO_CUSTOMIZED_NAMESPACE
#endif
{

class RBinaryIOSBase: virtual public IOStreamBase
{
public:


	RBinaryIOSBase() = default;


	// LCOV_EXCL_START
	virtual ~RBinaryIOSBase() = default;
	// LCOV_EXCL_STOP


	template<typename _ContainerType>
	_ContainerType ReadBytes(size_t count)
	{
		return ReadBytes<_ContainerType>(false, count);
	}


	template<typename _ContainerType>
	_ContainerType ReadBytes()
	{
		return ReadBytes<_ContainerType>(true, 0);
	}


protected:


	virtual size_t ReadBytesRaw(void* buffer, size_t size) = 0;


private:


	template<typename _ContainerType>
	_ContainerType ReadBytes(bool tillTheEnd, size_t count)
	{
		using _ValueType = typename _ContainerType::value_type;
		static_assert(std::is_trivially_copyable<_ValueType>::value,
			"Container value type must be trivially copyable");
		static_assert(sizeof(_ValueType) == 1,
			"Container value type must be byte-sized");

		if (tillTheEnd)
		{
			auto currPos = Tell();
			Seek(0, SeekWhence::End);
			auto endPos = Tell();
			count = endPos - currPos;
			Seek(currPos);
		}

		_ContainerType res;
		res.resize(count);
		auto countRead = ReadBytesRaw(&(res[0]), count);
		res.resize(countRead);
		return res;
	}


}; // class RBinaryIOSBase



class WBinaryIOSBase: virtual public IOStreamBase
{
public:


	WBinaryIOSBase() = default;


	// LCOV_EXCL_START
	virtual ~WBinaryIOSBase() = default;
	// LCOV_EXCL_STOP


	virtual void Flush() = 0;


	template<typename _ContainerType>
	void WriteBytes(const _ContainerType& bytes)
	{
		using _ValueType = typename _ContainerType::value_type;
		static_assert(std::is_trivially_copyable<_ValueType>::value,
			"Container value type must be trivially copyable");
		static_assert(sizeof(_ValueType) == 1,
			"Container value type must be byte-sized");

		WriteBytesRaw(bytes.data(), bytes.size());
	}


protected:


	virtual void WriteBytesRaw(const void* buffer, size_t size) = 0;


}; // class WBinaryIOSBase


class RWBinaryIOSBase:
	virtual public RBinaryIOSBase,
	virtual public WBinaryIOSBase
{
public:

	RWBinaryIOSBase() = default;

	// LCOV_EXCL_START
	virtual ~RWBinaryIOSBase() = default;
	// LCOV_EXCL_STOP

}; // class RWBinaryIOSBase


template<typename _ImplType>
class RBinaryIOSWrapper:
	virtual public RBinaryIOSBase
{
public:

	RBinaryIOSWrapper(std::unique_ptr<_ImplType> impl):
		m_impl(std::move(impl))
	{}

	virtual void Seek(
		std::ptrdiff_t offset,
		SeekWhence whence = SeekWhence::Begin
	) override
	{ m_impl->Seek(offset, whence); }

	virtual size_t Tell() const override
	{ return m_impl->Tell(); }

protected:

	virtual size_t ReadBytesRaw(void* buffer, size_t size) override
	{ return m_impl->ReadBytesRaw(buffer, size); }

private:

	std::unique_ptr<_ImplType> m_impl;
}; // class RBinaryIOSWrapper


template<typename _ImplType>
class WBinaryIOSWrapper:
	virtual public WBinaryIOSBase
{
public:

	WBinaryIOSWrapper(std::unique_ptr<_ImplType> impl):
		m_impl(std::move(impl))
	{}

	virtual void Flush() override
	{ m_impl->Flush(); }

	virtual void Seek(
		std::ptrdiff_t offset,
		SeekWhence whence = SeekWhence::Begin
	) override
	{ m_impl->Seek(offset, whence); }

	virtual size_t Tell() const override
	{ return m_impl->Tell(); }

protected:

	virtual void WriteBytesRaw(const void* buffer, size_t size) override
	{ m_impl->WriteBytesRaw(buffer, size); }

private:

	std::unique_ptr<_ImplType> m_impl;
}; // class WBinaryIOSWrapper


template<typename _ImplType>
class RWBinaryIOSWrapper:
	virtual public RWBinaryIOSBase
{
public:

	RWBinaryIOSWrapper(std::unique_ptr<_ImplType> impl):
		m_impl(std::move(impl))
	{}

	virtual void Flush() override
	{ m_impl->Flush(); }

	virtual void Seek(
		std::ptrdiff_t offset,
		SeekWhence whence = SeekWhence::Begin
	) override
	{ m_impl->Seek(offset, whence); }

	virtual size_t Tell() const override
	{ return m_impl->Tell(); }

protected:

	virtual size_t ReadBytesRaw(void* buffer, size_t size) override
	{ return m_impl->ReadBytesRaw(buffer, size); }

	virtual void WriteBytesRaw(const void* buffer, size_t size) override
	{ m_impl->WriteBytesRaw(buffer, size); }

private:

	std::unique_ptr<_ImplType> m_impl;
}; // class RWBinaryIOSWrapper


} // namespace SimpleSysIO
