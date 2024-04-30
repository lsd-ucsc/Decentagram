// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <cstdint>

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{

namespace Internal
{

// ========== Base Struct  ==========
template<typename TypeA, typename TypeB>
struct InferBinOpRetType {};

// ========== Same type ==========
template<typename SameType>
struct InferBinOpRetType<SameType, SameType>
{
	using RetType = SameType;
};

// ========== bool and different types ==========
// ===== signed =====
template<>
struct InferBinOpRetType<bool, int8_t>
{
	using RetType = int8_t;
};
template<>
struct InferBinOpRetType<int8_t, bool> : InferBinOpRetType<bool, int8_t> {};
template<>
struct InferBinOpRetType<bool, int16_t>
{
	using RetType = int16_t;
};
template<>
struct InferBinOpRetType<int16_t, bool> : InferBinOpRetType<bool, int16_t> {};
template<>
struct InferBinOpRetType<bool, int32_t>
{
	using RetType = int32_t;
};
template<>
struct InferBinOpRetType<int32_t, bool> : InferBinOpRetType<bool, int32_t> {};
template<>
struct InferBinOpRetType<bool, int64_t>
{
	using RetType = int64_t;
};
template<>
struct InferBinOpRetType<int64_t, bool> : InferBinOpRetType<bool, int64_t> {};
template<>
struct InferBinOpRetType<bool, uint8_t>
{
	using RetType = uint8_t;
};
// ===== unsigned =====
template<>
struct InferBinOpRetType<uint8_t, bool> : InferBinOpRetType<bool, uint8_t> {};
template<>
struct InferBinOpRetType<bool, uint16_t>
{
	using RetType = uint16_t;
};
template<>
struct InferBinOpRetType<uint16_t, bool> : InferBinOpRetType<bool, uint16_t> {};
template<>
struct InferBinOpRetType<bool, uint32_t>
{
	using RetType = uint32_t;
};
template<>
struct InferBinOpRetType<uint32_t, bool> : InferBinOpRetType<bool, uint32_t> {};
template<>
struct InferBinOpRetType<bool, uint64_t>
{
	using RetType = uint64_t;
};
template<>
struct InferBinOpRetType<uint64_t, bool> : InferBinOpRetType<bool, uint64_t> {};
// ===== floating =====
template<>
struct InferBinOpRetType<bool, float>
{
	using RetType = float;
};
template<>
struct InferBinOpRetType<float, bool> : InferBinOpRetType<bool, float> {};
template<>
struct InferBinOpRetType<bool, double>
{
	using RetType = double;
};
template<>
struct InferBinOpRetType<double, bool> : InferBinOpRetType<bool, double> {};

// ========== int8_t and different types ==========
// ===== signed =====
template<>
struct InferBinOpRetType<int8_t, int16_t>
{
	using RetType = int16_t;
};
template<>
struct InferBinOpRetType<int16_t, int8_t> : InferBinOpRetType<int8_t, int16_t> {};
template<>
struct InferBinOpRetType<int8_t, int32_t>
{
	using RetType = int32_t;
};
template<>
struct InferBinOpRetType<int32_t, int8_t> : InferBinOpRetType<int8_t, int32_t> {};
template<>
struct InferBinOpRetType<int8_t, int64_t>
{
	using RetType = int64_t;
};
template<>
struct InferBinOpRetType<int64_t, int8_t> : InferBinOpRetType<int8_t, int64_t> {};
// ===== unsigned =====
template<>
struct InferBinOpRetType<int8_t, uint8_t>
{
	using RetType = int16_t;
};
template<>
struct InferBinOpRetType<uint8_t, int8_t> : InferBinOpRetType<int8_t, uint8_t> {};
template<>
struct InferBinOpRetType<int8_t, uint16_t>
{
	using RetType = int32_t;
};
template<>
struct InferBinOpRetType<uint16_t, int8_t> : InferBinOpRetType<int8_t, uint16_t> {};
template<>
struct InferBinOpRetType<int8_t, uint32_t>
{
	using RetType = int64_t;
};
template<>
struct InferBinOpRetType<uint32_t, int8_t> : InferBinOpRetType<int8_t, uint32_t> {};
template<>
struct InferBinOpRetType<int8_t, uint64_t>
{
	using RetType = int64_t;
};
template<>
struct InferBinOpRetType<uint64_t, int8_t> : InferBinOpRetType<int8_t, uint64_t> {};
// ===== floating =====
template<>
struct InferBinOpRetType<int8_t, float> : InferBinOpRetType<bool, float> {};
template<>
struct InferBinOpRetType<float, int8_t> : InferBinOpRetType<int8_t, float> {};
template<>
struct InferBinOpRetType<int8_t, double> : InferBinOpRetType<bool, double> {};
template<>
struct InferBinOpRetType<double, int8_t> : InferBinOpRetType<int8_t, double> {};

// ========== int16_t and different types ==========
// ===== signed =====
template<>
struct InferBinOpRetType<int16_t, int32_t>
{
	using RetType = int32_t;
};
template<>
struct InferBinOpRetType<int32_t, int16_t> : InferBinOpRetType<int16_t, int32_t> {};
template<>
struct InferBinOpRetType<int16_t, int64_t>
{
	using RetType = int64_t;
};
template<>
struct InferBinOpRetType<int64_t, int16_t> : InferBinOpRetType<int16_t, int64_t> {};
// ===== unsigned =====
template<>
struct InferBinOpRetType<int16_t, uint8_t>
{
	using RetType = int16_t;
};
template<>
struct InferBinOpRetType<uint8_t, int16_t> : InferBinOpRetType<int16_t, uint8_t> {};
template<>
struct InferBinOpRetType<int16_t, uint16_t>
{
	using RetType = int32_t;
};
template<>
struct InferBinOpRetType<uint16_t, int16_t> : InferBinOpRetType<int16_t, uint16_t> {};
template<>
struct InferBinOpRetType<int16_t, uint32_t>
{
	using RetType = int64_t;
};
template<>
struct InferBinOpRetType<uint32_t, int16_t> : InferBinOpRetType<int16_t, uint32_t> {};
template<>
struct InferBinOpRetType<int16_t, uint64_t>
{
	using RetType = int64_t;
};
template<>
struct InferBinOpRetType<uint64_t, int16_t> : InferBinOpRetType<int16_t, uint64_t> {};
// ===== floating =====
template<>
struct InferBinOpRetType<int16_t, float> : InferBinOpRetType<int8_t, float> {};
template<>
struct InferBinOpRetType<float, int16_t> : InferBinOpRetType<int16_t, float> {};
template<>
struct InferBinOpRetType<int16_t, double> : InferBinOpRetType<int8_t, double> {};
template<>
struct InferBinOpRetType<double, int16_t> : InferBinOpRetType<int16_t, double> {};

// ========== int32_t and different types ==========
// ===== signed =====
template<>
struct InferBinOpRetType<int32_t, int64_t>
{
	using RetType = int64_t;
};
template<>
struct InferBinOpRetType<int64_t, int32_t> : InferBinOpRetType<int32_t, int64_t> {};
// ===== unsigned =====
template<>
struct InferBinOpRetType<int32_t, uint8_t>
{
	using RetType = int32_t;
};
template<>
struct InferBinOpRetType<uint8_t, int32_t> : InferBinOpRetType<int32_t, uint8_t> {};
template<>
struct InferBinOpRetType<int32_t, uint16_t>
{
	using RetType = int32_t;
};
template<>
struct InferBinOpRetType<uint16_t, int32_t> : InferBinOpRetType<int32_t, uint16_t> {};
template<>
struct InferBinOpRetType<int32_t, uint32_t>
{
	using RetType = int64_t;
};
template<>
struct InferBinOpRetType<uint32_t, int32_t> : InferBinOpRetType<int32_t, uint32_t> {};
template<>
struct InferBinOpRetType<int32_t, uint64_t>
{
	using RetType = int64_t;
};
template<>
struct InferBinOpRetType<uint64_t, int32_t> : InferBinOpRetType<int32_t, uint64_t> {};
// ===== floating =====
template<>
struct InferBinOpRetType<int32_t, float> : InferBinOpRetType<int16_t, float> {};
template<>
struct InferBinOpRetType<float, int32_t> : InferBinOpRetType<int32_t, float> {};
template<>
struct InferBinOpRetType<int32_t, double> : InferBinOpRetType<int16_t, double> {};
template<>
struct InferBinOpRetType<double, int32_t> : InferBinOpRetType<int32_t, double> {};

// ========== int64_t and different types ==========
// ===== unsigned =====
template<>
struct InferBinOpRetType<int64_t, uint8_t>
{
	using RetType = int64_t;
};
template<>
struct InferBinOpRetType<uint8_t, int64_t> : InferBinOpRetType<int64_t, uint8_t> {};
template<>
struct InferBinOpRetType<int64_t, uint16_t>
{
	using RetType = int64_t;
};
template<>
struct InferBinOpRetType<uint16_t, int64_t> : InferBinOpRetType<int64_t, uint16_t> {};
template<>
struct InferBinOpRetType<int64_t, uint32_t>
{
	using RetType = int64_t;
};
template<>
struct InferBinOpRetType<uint32_t, int64_t> : InferBinOpRetType<int64_t, uint32_t> {};
template<>
struct InferBinOpRetType<int64_t, uint64_t>
{
	using RetType = int64_t;
};
template<>
struct InferBinOpRetType<uint64_t, int64_t> : InferBinOpRetType<int64_t, uint64_t> {};
// ===== floating =====
template<>
struct InferBinOpRetType<int64_t, float> : InferBinOpRetType<int32_t, float> {};
template<>
struct InferBinOpRetType<float, int64_t> : InferBinOpRetType<int64_t, float> {};
template<>
struct InferBinOpRetType<int64_t, double> : InferBinOpRetType<int32_t, double> {};
template<>
struct InferBinOpRetType<double, int64_t> : InferBinOpRetType<int64_t, double> {};

// ========== uint8_t and different types ==========
// ===== unsigned =====
template<>
struct InferBinOpRetType<uint8_t, uint16_t>
{
	using RetType = uint16_t;
};
template<>
struct InferBinOpRetType<uint16_t, uint8_t> : InferBinOpRetType<uint8_t, uint16_t> {};
template<>
struct InferBinOpRetType<uint8_t, uint32_t>
{
	using RetType = uint32_t;
};
template<>
struct InferBinOpRetType<uint32_t, uint8_t> : InferBinOpRetType<uint8_t, uint32_t> {};
template<>
struct InferBinOpRetType<uint8_t, uint64_t>
{
	using RetType = uint64_t;
};
template<>
struct InferBinOpRetType<uint64_t, uint8_t> : InferBinOpRetType<uint8_t, uint64_t> {};
// ===== floating =====
template<>
struct InferBinOpRetType<uint8_t, float> : InferBinOpRetType<int8_t, float> {};
template<>
struct InferBinOpRetType<float, uint8_t> : InferBinOpRetType<uint8_t, float> {};
template<>
struct InferBinOpRetType<uint8_t, double> : InferBinOpRetType<int8_t, double> {};
template<>
struct InferBinOpRetType<double, uint8_t> : InferBinOpRetType<uint8_t, double> {};

// ========== uint16_t and different types ==========
// ===== unsigned =====
template<>
struct InferBinOpRetType<uint16_t, uint32_t>
{
	using RetType = uint32_t;
};
template<>
struct InferBinOpRetType<uint32_t, uint16_t> : InferBinOpRetType<uint16_t, uint32_t> {};
template<>
struct InferBinOpRetType<uint16_t, uint64_t>
{
	using RetType = uint64_t;
};
template<>
struct InferBinOpRetType<uint64_t, uint16_t> : InferBinOpRetType<uint16_t, uint64_t> {};
// ===== floating =====
template<>
struct InferBinOpRetType<uint16_t, float> : InferBinOpRetType<uint8_t, float> {};
template<>
struct InferBinOpRetType<float, uint16_t> : InferBinOpRetType<uint16_t, float> {};
template<>
struct InferBinOpRetType<uint16_t, double> : InferBinOpRetType<uint8_t, double> {};
template<>
struct InferBinOpRetType<double, uint16_t> : InferBinOpRetType<uint16_t, double> {};

// ========== uint32_t and different types ==========
// ===== unsigned =====
template<>
struct InferBinOpRetType<uint32_t, uint64_t>
{
	using RetType = uint64_t;
};
template<>
struct InferBinOpRetType<uint64_t, uint32_t> : InferBinOpRetType<uint32_t, uint64_t> {};
// ===== floating =====
template<>
struct InferBinOpRetType<uint32_t, float> : InferBinOpRetType<uint16_t, float> {};
template<>
struct InferBinOpRetType<float, uint32_t> : InferBinOpRetType<uint32_t, float> {};
template<>
struct InferBinOpRetType<uint32_t, double> : InferBinOpRetType<uint16_t, double> {};
template<>
struct InferBinOpRetType<double, uint32_t> : InferBinOpRetType<uint32_t, double> {};

// ========== uint64_t and different types ==========
// ===== floating =====
template<>
struct InferBinOpRetType<uint64_t, float> : InferBinOpRetType<uint32_t, float> {};
template<>
struct InferBinOpRetType<float, uint64_t> : InferBinOpRetType<uint64_t, float> {};
template<>
struct InferBinOpRetType<uint64_t, double> : InferBinOpRetType<uint32_t, double> {};
template<>
struct InferBinOpRetType<double, uint64_t> : InferBinOpRetType<uint64_t, double> {};

// ========== float and different types ==========
// ===== floating =====
template<>
struct InferBinOpRetType<float, double> : InferBinOpRetType<bool, double> {};
template<>
struct InferBinOpRetType<double, float> : InferBinOpRetType<float, double> {};

} // namespace Internal

} // namespace SimpleObjects
