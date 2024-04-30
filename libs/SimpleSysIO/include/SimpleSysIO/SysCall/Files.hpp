// Copyright (c) 2022 SimpleSysIO
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include "../Config.hpp"


#ifdef SIMPLESYSIO_ENABLE_SYSCALL_FILESYSTEM


#include <cstdio>

#include <memory>
#include <string>

#include <SimpleObjects/RealNumCast.hpp>

#include "../Exceptions.hpp"
#include "../Internal/SimpleObjects.hpp"
#include "../BinaryIOStreamBase.hpp"


#ifndef SIMPLESYSIO_CUSTOMIZED_NAMESPACE
namespace SimpleSysIO
#else
namespace SIMPLESYSIO_CUSTOMIZED_NAMESPACE
#endif
{

namespace SysCall
{

namespace SysCallInternal
{

class COpenImpl
{
public: // static members:

	static std::FILE* COpenS(
		const std::string& path,
		const std::string& mode
	)
	{
		std::FILE* file = nullptr;
#if defined(_WIN32)
		auto err = fopen_s(&file, path.c_str(), mode.c_str());
		if (err != 0)
		{
			throw Exception("I/O error while opening the file at " + path);
		}
#else
		file = std::fopen(path.c_str(), mode.c_str());
		if (file == nullptr)
		{
			throw Exception("I/O error while opening the file at " + path);
		}
#endif // defined(_WIN32)
		return file;
	}

public:

	COpenImpl(const std::string& path, const std::string& mode) :
		COpenImpl(COpenS(path, mode))
	{}


	~COpenImpl()
	{
		if (m_filePtr != nullptr)
		{
			std::fclose(m_filePtr);
		}
	}


	void Seek(
		std::ptrdiff_t offset,
		SeekWhence whence = SeekWhence::Begin
	)
	{
		ThrowIfFilePtrIsNull();

		using _COffsetType = long;
		_COffsetType cOffset =
			Internal::Obj::RealNumCast<_COffsetType>(offset);

		switch (whence)
		{
		case SeekWhence::Begin:
			std::fseek(m_filePtr, cOffset, SEEK_SET);
			break;

		case SeekWhence::Current:
			std::fseek(m_filePtr, cOffset, SEEK_CUR);
			break;

		case SeekWhence::End:
			std::fseek(m_filePtr, cOffset, SEEK_END);
			break;

		default:
			throw Exception("Invalid SeekWhence value");
		}
	}


	size_t Tell() const
	{
		ThrowIfFilePtrIsNull();

		auto cRes = std::ftell(m_filePtr);

		return Internal::Obj::RealNumCast<size_t>(cRes);
	}


	void Flush()
	{
		ThrowIfFilePtrIsNull();

		std::fflush(m_filePtr);
	}


	size_t ReadBytesRaw(void* buffer, size_t size)
	{
		ThrowIfFilePtrIsNull();

		auto cReadSize = std::fread(buffer, 1, size, m_filePtr);

		size_t readSize = Internal::Obj::RealNumCast<size_t>(cReadSize);
		if ((readSize < size) &&
			(std::ferror(m_filePtr) != 0)
			)
		{
			throw Exception("I/O error while reading the file");
		}

		return readSize;
	}


	size_t WriteBytesRaw(const void* buffer, size_t size)
	{
		ThrowIfFilePtrIsNull();

		auto cWriteSize = std::fwrite(buffer, 1, size, m_filePtr);

		size_t writeSize = Internal::Obj::RealNumCast<size_t>(cWriteSize);
		if ((writeSize < size) &&
			(std::ferror(m_filePtr) != 0)
			)
		{
			throw Exception("I/O error while writing the file");
		}

		return writeSize;
	}


private:


	COpenImpl(std::FILE* filePtr) noexcept :
		m_filePtr(filePtr)
	{}


	void ThrowIfFilePtrIsNull() const
	{
		if (m_filePtr == nullptr)
		{
			throw Exception("File is not opened");
		}
	}


	std::FILE* m_filePtr;

}; // class COpenImpl

template<
	template<typename> class _WrapperType,
	typename _BaseType
>
struct COpenerImpl
{

	using ImplType = COpenImpl;
	using WrapperType = _WrapperType<ImplType>;
	using RetType = std::unique_ptr<_BaseType>;

protected:

	static RetType OpenImpl(
		const std::string& path,
		const std::string& mode
	)
	{
		auto impl =
			Internal::Obj::Internal::make_unique<ImplType>(path, mode);

		return
			Internal::Obj::Internal::make_unique<WrapperType>(
				std::move(impl)
			);
	}

}; // struct COpenerImpl

} // namespace SysCallInternal


struct RBinaryFile :
	SysCallInternal::COpenerImpl<RBinaryIOSWrapper, RBinaryIOSBase>
{
	static RetType Open(const std::string& path)
	{
		return OpenImpl(path, "rb");
	}
}; // struct RBinaryFile


struct WBinaryFile :
	SysCallInternal::COpenerImpl<WBinaryIOSWrapper, WBinaryIOSBase>
{
	static RetType Create(const std::string& path)
	{
		return OpenImpl(path, "wb");
	}

	static RetType Append(const std::string& path)
	{
		return OpenImpl(path, "ab");
	}
}; // struct WBinaryFile


struct RWBinaryFile :
	SysCallInternal::COpenerImpl<RWBinaryIOSWrapper, RWBinaryIOSBase>
{
	using ImplType = SysCallInternal::COpenImpl;
	using WrapperType = RWBinaryIOSWrapper<ImplType>;
	using RetType = std::unique_ptr<RWBinaryIOSBase>;


	static RetType Create(const std::string& path)
	{
		return OpenImpl(path, "wb+");
	}

	static RetType Append(const std::string& path)
	{
		return OpenImpl(path, "ab+");
	}
}; // struct RWBinaryFile


} // namespace SysCall
} // namespace SimpleSysIO

#endif // SIMPLESYSIO_ENABLE_SYSCALL_FILESYSTEM
