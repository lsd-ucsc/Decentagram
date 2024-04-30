// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

#include "Null.hpp"
#include "RealNum.hpp"
#include "String.hpp"
#include "List.hpp"
#include "Dict.hpp"
#include "Bytes.hpp"

#include "Object.hpp"
#include "HashableObject.hpp"

#include "ConstSequence.hpp"
#include "StaticDict.hpp"

#include "ToStringImpl.hpp"

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{

// ========== Basic Type Controll ==========

using ToStringType = std::string;

template<typename _KeyType, typename _ValType>
using MapType = std::unordered_map<_KeyType, _ValType>;

template<typename _ValType>
using VecType = std::vector<_ValType>;

// ========== Convenient types of Null ==========

using Null = NullImpl<ToStringType>;

inline const Null& None()
{
	static Null n;
	return n;
}

// ========== Convenient types of RealNumImpl ==========

template<typename _ValType>
using RealNumT = RealNumImpl<_ValType, ToStringType>;

using Bool   = RealNumT<bool    >;

using Int8   = RealNumT<int8_t  >;
using Int16  = RealNumT<int16_t >;
using Int32  = RealNumT<int32_t >;
using Int64  = RealNumT<int64_t >;

using UInt8  = RealNumT<uint8_t >;
using UInt16 = RealNumT<uint16_t>;
using UInt32 = RealNumT<uint32_t>;
using UInt64 = RealNumT<uint64_t>;

using Float  = RealNumT<float   >;
using Double = RealNumT<double  >;

// ========== Convenient types of String ==========

using String = StringImpl<std::string, ToStringType>;

// ========== Convenient types of Object ==========

using Object = ObjectImpl<ToStringType>;

// ========== Convenient types of HashableObject ==========

using HashableObject = HashableObjectImpl<ToStringType>;

// ========== Convenient types of List ==========

template<typename _ValType>
using ListT = ListImpl<VecType<_ValType>, ToStringType>;

using List = ListT<Object>;

// ========== Convenient types of Dict ==========

template<typename _KeyType, typename _Valtype>
using DictT = DictImpl<_KeyType, _Valtype, MapType, ToStringType>;

using Dict = DictT<HashableObject, Object>;

// ========== Convenient types of Bytes ==========

using BytesBaseObj = BytesBaseObject<uint8_t, ToStringType>;
using Bytes = BytesImpl<std::vector<uint8_t>, ToStringType>;

// ========== Convenient types of base classes ==========

using BaseObj = BaseObject<ToStringType>;
using HashableBaseObj = HashableBaseObject<ToStringType>;

using RealNumBaseObj = RealNumBaseObject<ToStringType>;
using StringBaseObj = StringBaseObject<char, ToStringType>;
using ListBaseObj = ListBaseObject<BaseObj, ToStringType>;
using DictBaseObj = DictBaseObject<HashableBaseObj, BaseObj, ToStringType>;
using StaticDictBaseObj = StaticDictBaseObject<
	HashableBaseObj,
	BaseObj,
	HashableReferenceWrapper,
	std::reference_wrapper,
	ToStringType>;

// ========== Convenient types of static Dict ==========

template<typename _ValType, _ValType ..._data>
using FromStrSeq = Internal::FromDataSeqImpl<String, _ValType, _data...>;
template<typename _StrSeq>
using StrKey = typename _StrSeq::template ToOther<FromStrSeq>::type;
template<int64_t _Val>
using Int64Key = StaticPrimitiveValue<Int64, _Val>;

template<typename _Tp>
using StaticDict = StaticDictImpl<
	_Tp,
	HashableBaseObj,
	BaseObj,
	HashableReferenceWrapper,
	std::reference_wrapper,
	MapType,
	ToStringType>;

} // namespace SimpleObjects
