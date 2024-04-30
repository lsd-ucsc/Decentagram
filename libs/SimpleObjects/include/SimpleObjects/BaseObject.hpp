// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <functional>
#include <string>

#include "BasicDefs.hpp"
#include "Compare.hpp"
#include "Exception.hpp"
#include "Iterator.hpp"


#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{

// Forward declarations
template<typename _ToStringType>
class NullImpl;
template<typename _ToStringType>
class RealNumBaseObject;
template<typename _CharType, typename _ToStringType>
class StringBaseObject;
template<typename _ValBaseType, typename _ToStringType>
class ListBaseObject;
template<typename _KeyType, typename _ValType, typename _ToStringType>
class DictBaseObject;
template<typename _ValType, typename _ToStringType>
class BytesBaseObject;
template<
	typename _DynKeyType,
	typename _DynValType,
	template<typename> class _KeyRefWrapType,
	template<typename> class _RefWrapType,
	typename _ToStringType>
class StaticDictBaseObject;
template<typename _ToStringType>
class HashableBaseObject;
template<typename _T>
class HashableReferenceWrapper;

template<typename _ToStringType>
class BaseObject
{
public: // Static members:

	using Self = BaseObject<_ToStringType>;
	using ToStringType = _ToStringType;


	using NullBase      = NullImpl<ToStringType>;

	using RealNumBase   = RealNumBaseObject<ToStringType>;

	using StringBase    = StringBaseObject<char, ToStringType>;

	using ListBase      = ListBaseObject<BaseObject<ToStringType>, ToStringType>;

	using HashableBase  = HashableBaseObject<ToStringType>;

	using DictBase      = DictBaseObject<HashableBaseObject<ToStringType>,
										BaseObject<ToStringType>,
										ToStringType>;

	using BytesBase     = BytesBaseObject<uint8_t, ToStringType>;

	using StatDictBase  = StaticDictBaseObject<HashableBaseObject<ToStringType>,
											BaseObject<ToStringType>,
											HashableReferenceWrapper,
											std::reference_wrapper,
											ToStringType>;

	static constexpr Self* sk_null = nullptr;

public:

	BaseObject() = default;

	// LCOV_EXCL_START
	/**
	 * @brief Destroy the Base Object
	 *
	 */
	virtual ~BaseObject() = default;
	// LCOV_EXCL_STOP

	virtual ObjCategory GetCategory() const = 0;

	virtual const char* GetCategoryName() const = 0;

	// ========== Comparisons ==========

	/**
	 * @brief Compare if two objects are the same.
	 *        Objects in different categories are considered not-equal;
	 *        Objects in the same categories are further checked by child class
	 *
	 * @param rhs The other object to test with
	 * @return whether two objects are equal
	 */
	virtual bool BaseObjectIsEqual(const Self& rhs) const = 0;

	virtual ObjectOrder BaseObjectCompare(const Self& rhs) const = 0;

	bool operator==(const Self& rhs) const
	{
		return BaseObjectIsEqual(rhs);
	}

#ifdef __cpp_lib_three_way_comparison
	std::strong_ordering operator<=>(const Self& rhs) const
	{
		auto cmpRes = BaseObjectCompare(rhs);
		switch (cmpRes)
		{
		case ObjectOrder::Less:
			return std::strong_ordering::less;
		case ObjectOrder::Equal:
			return std::strong_ordering::equal;
		case ObjectOrder::Greater:
			return std::strong_ordering::greater;
		case ObjectOrder::EqualUnordered:
		case ObjectOrder::NotEqualUnordered:
		default:
			throw UnsupportedOperation("<=>",
				this->GetCategoryName(), rhs.GetCategoryName());
		}
	}
#else
	bool operator!=(const Self& rhs) const
	{
		return !((*this) == rhs);
	}

	bool operator<(const Self& rhs) const
	{
		auto cmpRes = BaseObjectCompare(rhs);
		switch (cmpRes)
		{
		case ObjectOrder::Less:
			return true;
		case ObjectOrder::EqualUnordered:
		case ObjectOrder::NotEqualUnordered:
			throw UnsupportedOperation("<",
				this->GetCategoryName(), rhs.GetCategoryName());
		default:
			return false;
		}
	}

	bool operator>=(const Self& rhs) const
	{
		return !((*this) < rhs);
	}

	bool operator>(const Self& rhs) const
	{
		auto cmpRes = BaseObjectCompare(rhs);
		switch (cmpRes)
		{
		case ObjectOrder::Greater:
			return true;
		case ObjectOrder::EqualUnordered:
		case ObjectOrder::NotEqualUnordered:
			throw UnsupportedOperation(">",
				this->GetCategoryName(), rhs.GetCategoryName());
		default:
			return false;
		}
	}

	bool operator<=(const Self& rhs) const
	{
		return !((*this) > rhs);
	}
#endif

	// TODO:
	// =
	Self& operator=(const Self& rhs) = delete;
	// +=
	// +
	// slice
	// CppStr

	// String(objs)
	// String::CppStr

	// ========== Setters ==========

	virtual void Set(const Self& other) = 0;

	virtual void Set(Self&& other) = 0;

	virtual void Set(bool)
	{
		throw TypeError(this->GetCategoryName(), "bool");
	}

	virtual void Set(uint8_t)
	{
		throw TypeError(this->GetCategoryName(), "uint8_t");
	}

	virtual void Set(int8_t)
	{
		throw TypeError(this->GetCategoryName(), "int8_t");
	}

	virtual void Set(uint32_t)
	{
		throw TypeError(this->GetCategoryName(), "uint32_t");
	}

	virtual void Set(int32_t)
	{
		throw TypeError(this->GetCategoryName(), "int32_t");
	}

	virtual void Set(uint64_t)
	{
		throw TypeError(this->GetCategoryName(), "uint64_t");
	}

	virtual void Set(int64_t)
	{
		throw TypeError(this->GetCategoryName(), "int64_t");
	}

	virtual void Set(double)
	{
		throw TypeError(this->GetCategoryName(), "double");
	}

	// ========== Getters ==========

	virtual bool IsNull() const
	{
		return false;
	}

	// operator bool() const
	// {
	// 	return this->AsCppBool();
	// }

	virtual bool IsTrue() const = 0;

	virtual uint8_t AsCppUInt8() const
	{
		throw TypeError(
			"RealNumImpl no larger than uint8_t", this->GetCategoryName());
	}

	virtual int8_t AsCppInt8() const
	{
		throw TypeError(
			"RealNumImpl no larger than int8_t", this->GetCategoryName());
	}

	virtual uint32_t AsCppUInt32() const
	{
		throw TypeError(
			"RealNumImpl no larger than uint32_t", this->GetCategoryName());
	}

	virtual int32_t AsCppInt32() const
	{
		throw TypeError(
			"RealNumImpl no larger than int32_t", this->GetCategoryName());
	}

	virtual uint64_t AsCppUInt64() const
	{
		throw TypeError(
			"RealNumImpl no larger than uint64_t", this->GetCategoryName());
	}

	virtual int64_t AsCppInt64() const
	{
		throw TypeError(
			"RealNumImpl no larger than int64_t", this->GetCategoryName());
	}

	virtual double AsCppDouble() const
	{
		throw TypeError(
			"RealNumImpl", this->GetCategoryName());
	}

	virtual NullBase& AsNull()
	{
		throw TypeError("Null", this->GetCategoryName());
	}

	virtual const NullBase& AsNull() const
	{
		throw TypeError("Null", this->GetCategoryName());
	}

	virtual RealNumBase& AsRealNum()
	{
		throw TypeError("RealNumImpl Category", this->GetCategoryName());
	}

	virtual const RealNumBase& AsRealNum() const
	{
		throw TypeError("RealNumImpl Category", this->GetCategoryName());
	}

	virtual StringBase& AsString()
	{
		throw TypeError("String", this->GetCategoryName());
	}

	virtual const StringBase& AsString() const
	{
		throw TypeError("String", this->GetCategoryName());
	}

	virtual ListBase& AsList()
	{
		throw TypeError("List", this->GetCategoryName());
	}

	virtual const ListBase& AsList() const
	{
		throw TypeError("List", this->GetCategoryName());
	}

	virtual DictBase& AsDict()
	{
		throw TypeError("Dict", this->GetCategoryName());
	}

	virtual const DictBase& AsDict() const
	{
		throw TypeError("Dict", this->GetCategoryName());
	}

	virtual StatDictBase& AsStaticDict()
	{
		throw TypeError("StaticDict", this->GetCategoryName());
	}

	virtual const StatDictBase& AsStaticDict() const
	{
		throw TypeError("StaticDict", this->GetCategoryName());
	}

	virtual BytesBase& AsBytes()
	{
		throw TypeError("Bytes", this->GetCategoryName());
	}

	virtual const BytesBase& AsBytes() const
	{
		throw TypeError("Bytes", this->GetCategoryName());
	}

	virtual HashableBase& AsHashable()
	{
		throw TypeError("Hashable", this->GetCategoryName());
	}

	virtual const HashableBase& AsHashable() const
	{
		throw TypeError("Hashable", this->GetCategoryName());
	}

	virtual std::unique_ptr<Self> Copy(const Self* /*unused*/) const = 0;

	virtual std::unique_ptr<Self> Move(const Self* /*unused*/) = 0;

	virtual std::string DebugString() const = 0;

	virtual std::string ShortDebugString() const = 0;

	virtual ToStringType ToString() const = 0;

	virtual void DumpString(OutIterator<typename ToStringType::value_type> outIt) const = 0;

}; // class BaseObject

namespace Internal
{

template<bool _Match, typename _Child, typename _RetType>
struct AsChildType;

template<typename _Child, typename _RetType>
struct AsChildType<false, _Child, _RetType>
{
	static_assert(!std::is_same<_Child, _RetType>::value,
		"Implementation Error");

	static _RetType& AsChild(_Child&,
		const std::string& expTypeName, const std::string& srcTypeName)
	{
		throw TypeError(expTypeName, srcTypeName + "-non-default");
	}

	static const _RetType& AsChild(const _Child&,
		const std::string& expTypeName, const std::string& srcTypeName)
	{
		throw TypeError(expTypeName, srcTypeName + "-non-default");
	}
}; // struct AsChildType

template<typename _Child>
struct AsChildType<true, _Child, _Child>
{
	static _Child& AsChild(_Child& c,
		const std::string&, const std::string&)
	{
		return c;
	}

	static const _Child& AsChild(const _Child& c,
		const std::string&, const std::string&)
	{
		return c;
	}
}; // struct AsChildType

} // namespace Internal

} // namespace SimpleObjects
