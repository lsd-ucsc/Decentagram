// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include "BasicDefs.hpp"


#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{


template<typename _T>
struct PrimitiveTypeName;

template<>
struct PrimitiveTypeName<bool>
{
	static constexpr const char* sk_name() { return "bool"; }
}; // struct PrimitiveTypeName<bool>

template<>
struct PrimitiveTypeName<char>
{
	static constexpr const char* sk_name() { return "char"; }
}; // struct PrimitiveTypeName<char>

template<>
struct PrimitiveTypeName<signed char>
{
	static constexpr const char* sk_name() { return "signed char"; }
}; // struct PrimitiveTypeName<signed char>

template<>
struct PrimitiveTypeName<unsigned char>
{
	static constexpr const char* sk_name() { return "unsigned char"; }
}; // struct PrimitiveTypeName<unsigned char>

template<>
struct PrimitiveTypeName<wchar_t>
{
	static constexpr const char* sk_name() { return "wchar_t"; }
}; // struct PrimitiveTypeName<wchar_t>

template<>
struct PrimitiveTypeName<char16_t>
{
	static constexpr const char* sk_name() { return "char16_t"; }
}; // struct PrimitiveTypeName<char16_t>

template<>
struct PrimitiveTypeName<char32_t>
{
	static constexpr const char* sk_name() { return "char32_t"; }
}; // struct PrimitiveTypeName<char32_t>

template<>
struct PrimitiveTypeName<short>
{
	static constexpr const char* sk_name() { return "short"; }
}; // struct PrimitiveTypeName<short>

template<>
struct PrimitiveTypeName<unsigned short>
{
	static constexpr const char* sk_name() { return "unsigned short"; }
}; // struct PrimitiveTypeName<unsigned short>

template<>
struct PrimitiveTypeName<int>
{
	static constexpr const char* sk_name() { return "int"; }
}; // struct PrimitiveTypeName<int>

template<>
struct PrimitiveTypeName<unsigned int>
{
	static constexpr const char* sk_name() { return "unsigned int"; }
}; // struct PrimitiveTypeName<unsigned int>

template<>
struct PrimitiveTypeName<long>
{
	static constexpr const char* sk_name() { return "long"; }
}; // struct PrimitiveTypeName<long>

template<>
struct PrimitiveTypeName<unsigned long>
{
	static constexpr const char* sk_name() { return "unsigned long"; }
}; // struct PrimitiveTypeName<unsigned long>

template<>
struct PrimitiveTypeName<long long>
{
	static constexpr const char* sk_name() { return "long long"; }
}; // struct PrimitiveTypeName<long long>

template<>
struct PrimitiveTypeName<unsigned long long>
{
	static constexpr const char* sk_name() { return "unsigned long long"; }
}; // struct PrimitiveTypeName<unsigned long long>

template<>
struct PrimitiveTypeName<float>
{
	static constexpr const char* sk_name() { return "float"; }
}; // struct PrimitiveTypeName<float>

template<>
struct PrimitiveTypeName<double>
{
	static constexpr const char* sk_name() { return "double"; }
}; // struct PrimitiveTypeName<double>

template<>
struct PrimitiveTypeName<long double>
{
	static constexpr const char* sk_name() { return "long double"; }
}; // struct PrimitiveTypeName<long double>





template<typename _ValType>
struct RealNumTraits; // struct RealNumTraits

template<>
struct RealNumTraits<bool> :
	public Internal::CategoryTraits<ObjCategory::Bool>
{
	static constexpr bool sk_isRealNumUnderlyingType = true;

	static constexpr RealNumType sk_numType()
	{ return RealNumType::Bool;  }

	static constexpr const char* sk_numTypeName()
	{ return "Bool";  }

	static constexpr const char* sk_primitiveName()
	{ return PrimitiveTypeName<bool>::sk_name();  }
}; // struct RealNumTraits<bool>

template<>
struct RealNumTraits<uint8_t> :
	public Internal::CategoryTraits<ObjCategory::Integer>
{
	static constexpr bool sk_isRealNumUnderlyingType = true;

	static constexpr RealNumType sk_numType()
	{ return RealNumType::UInt8;  }

	static constexpr const char* sk_numTypeName()
	{ return "UInt8";  }

	static constexpr const char* sk_primitiveName()
	{ return PrimitiveTypeName<uint8_t>::sk_name();  }
}; // struct RealNumTraits<uint8_t>

template<>
struct RealNumTraits<int8_t> :
	public Internal::CategoryTraits<ObjCategory::Integer>
{
	static constexpr bool sk_isRealNumUnderlyingType = true;

	static constexpr RealNumType sk_numType()
	{ return RealNumType::Int8;  }

	static constexpr const char* sk_numTypeName()
	{ return "Int8";  }

	static constexpr const char* sk_primitiveName()
	{ return PrimitiveTypeName<int8_t>::sk_name();  }
}; // struct RealNumTraits<int8_t>

template<>
struct RealNumTraits<uint16_t> :
	public Internal::CategoryTraits<ObjCategory::Integer>
{
	static constexpr bool sk_isRealNumUnderlyingType = true;

	static constexpr RealNumType sk_numType()
	{ return RealNumType::UInt16;  }

	static constexpr const char* sk_numTypeName()
	{ return "UInt16";  }

	static constexpr const char* sk_primitiveName()
	{ return PrimitiveTypeName<uint16_t>::sk_name();  }
}; // struct RealNumTraits<uint16_t>

template<>
struct RealNumTraits<int16_t> :
	public Internal::CategoryTraits<ObjCategory::Integer>
{
	static constexpr bool sk_isRealNumUnderlyingType = true;

	static constexpr RealNumType sk_numType()
	{ return RealNumType::Int16;  }

	static constexpr const char* sk_numTypeName()
	{ return "Int16";  }

	static constexpr const char* sk_primitiveName()
	{ return PrimitiveTypeName<int16_t>::sk_name();  }
}; // struct RealNumTraits<int16_t>

template<>
struct RealNumTraits<uint32_t> :
	public Internal::CategoryTraits<ObjCategory::Integer>
{
	static constexpr bool sk_isRealNumUnderlyingType = true;

	static constexpr RealNumType sk_numType()
	{ return RealNumType::UInt32;  }

	static constexpr const char* sk_numTypeName()
	{ return "UInt32";  }

	static constexpr const char* sk_primitiveName()
	{ return PrimitiveTypeName<uint32_t>::sk_name();  }
}; // struct RealNumTraits<uint32_t>

template<>
struct RealNumTraits<int32_t> :
	public Internal::CategoryTraits<ObjCategory::Integer>
{
	static constexpr bool sk_isRealNumUnderlyingType = true;

	static constexpr RealNumType sk_numType()
	{ return RealNumType::Int32;  }

	static constexpr const char* sk_numTypeName()
	{ return "Int32";  }

	static constexpr const char* sk_primitiveName()
	{ return PrimitiveTypeName<int32_t>::sk_name();  }
}; // struct RealNumTraits<int32_t>

template<>
struct RealNumTraits<uint64_t> :
	public Internal::CategoryTraits<ObjCategory::Integer>
{
	static constexpr bool sk_isRealNumUnderlyingType = true;

	static constexpr RealNumType sk_numType()
	{ return RealNumType::UInt64;  }

	static constexpr const char* sk_numTypeName()
	{ return "UInt64";  }

	static constexpr const char* sk_primitiveName()
	{ return PrimitiveTypeName<uint64_t>::sk_name();  }
}; // struct RealNumTraits<uint64_t>

template<>
struct RealNumTraits<int64_t> :
	public Internal::CategoryTraits<ObjCategory::Integer>
{
	static constexpr bool sk_isRealNumUnderlyingType = true;

	static constexpr RealNumType sk_numType()
	{ return RealNumType::Int64;  }

	static constexpr const char* sk_numTypeName()
	{ return "Int64";  }

	static constexpr const char* sk_primitiveName()
	{ return PrimitiveTypeName<int64_t>::sk_name();  }
}; // struct RealNumTraits<int64_t>

template<>
struct RealNumTraits<float> :
	public Internal::CategoryTraits<ObjCategory::Real>
{
	static constexpr bool sk_isRealNumUnderlyingType = true;

	static constexpr RealNumType sk_numType()
	{ return RealNumType::Float;  }

	static constexpr const char* sk_numTypeName()
	{ return "Float";  }

	static constexpr const char* sk_primitiveName()
	{ return PrimitiveTypeName<float>::sk_name();  }
}; // struct RealNumTraits<float>

template<>
struct RealNumTraits<double> :
	public Internal::CategoryTraits<ObjCategory::Real>
{
	static constexpr bool sk_isRealNumUnderlyingType = true;

	static constexpr RealNumType sk_numType()
	{ return RealNumType::Double;  }

	static constexpr const char* sk_numTypeName()
	{ return "Double";  }

	static constexpr const char* sk_primitiveName()
	{ return PrimitiveTypeName<double>::sk_name();  }
}; // struct RealNumTraits<double>

template<typename _T>
struct RealNumTraits
{
	static constexpr bool sk_isRealNumUnderlyingType = false;

	static constexpr const char* sk_numTypeName()
	{ return sk_primitiveName();  }

	static constexpr const char* sk_primitiveName()
	{ return PrimitiveTypeName<_T>::sk_name();  }
}; // struct RealNumTraits

} // namespace SimpleObjects
