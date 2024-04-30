// Copyright (c) 2022 SimpleSysIO
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstddef>
#include <cstdint>


#ifndef SIMPLESYSIO_CUSTOMIZED_NAMESPACE
namespace SimpleSysIO
#else
namespace SIMPLESYSIO_CUSTOMIZED_NAMESPACE
#endif
{

enum class SeekWhence : uint8_t
{
	Begin   = 0,
	Current = 1,
	End     = 2,
}; // enum class SeekWhence

class IOStreamBase
{
public:
	IOStreamBase() = default;

	// LCOV_EXCL_START
	virtual ~IOStreamBase() = default;
	// LCOV_EXCL_STOP


	virtual void Seek(
		std::ptrdiff_t offset,
		SeekWhence whence = SeekWhence::Begin
	) = 0;

	virtual size_t Tell() const = 0;

	virtual size_t GetFileSize()
	{
		const auto curPos = Tell();
		Seek(0, SeekWhence::End);
		const auto fileSize = Tell();
		Seek(curPos, SeekWhence::Begin);
		return fileSize;
	}

}; // class IOStreamBase

} // namespace SimpleSysIO
