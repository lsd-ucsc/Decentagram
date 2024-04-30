// Copyright (c) 2023 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstddef>


#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{
namespace Codec
{


namespace Internal
{


struct Base64Sizes
{
	static constexpr size_t sk_inGroupSize = 3;
	static constexpr size_t sk_outGroupSize = 4;

	static constexpr size_t EstNumGroupsByBytes(size_t inputSize)
	{
		return (inputSize + (sk_inGroupSize - 1)) / sk_inGroupSize;
	}

	static constexpr size_t EstEncodedSize(size_t inputSize)
	{
		return EstNumGroupsByBytes(inputSize) * sk_outGroupSize;
	}

	static constexpr size_t EstNumGroupsByChar(size_t strLen)
	{
		return (strLen + (sk_outGroupSize - 1)) / sk_outGroupSize;
	}

	static constexpr size_t EstDecodedSize(size_t strLen)
	{
		return EstNumGroupsByChar(strLen) * sk_inGroupSize;
	}

}; // struct Base64Sizes


} // namespace Internal


} // namespace Codec
} // namespace SimpleObjects
