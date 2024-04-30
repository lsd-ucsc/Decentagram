// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "BaseObject.hpp"

#include "Iterator.hpp"

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{

template<typename _ValBaseType, typename _ToStringType>
class ListBaseObject : public BaseObject<_ToStringType>
{
public: // Static Objects

	using ToStringType = _ToStringType;
	using Self = ListBaseObject<_ValBaseType, _ToStringType>;
	using Base = BaseObject<_ToStringType>;

	using ListBase = typename Base::ListBase;

	typedef _ValBaseType                        value_type;
	typedef value_type&                         reference;
	typedef const value_type&                   const_reference;
	typedef value_type*                         pointer;
	typedef const value_type*                   const_pointer;
	typedef RdIterator<value_type, false>       iterator;
	typedef RdIterator<value_type, true>        const_iterator;

	static constexpr Self* sk_null = nullptr;

public:
	ListBaseObject() = default;

	// LCOV_EXCL_START
	/**
	 * @brief Destroy the List Base Object
	 *
	 */
	virtual ~ListBaseObject() = default;
	// LCOV_EXCL_STOP

	virtual const char* GetCategoryName() const override
	{
		return "List";
	}

	virtual ListBase& AsList() override
	{
		return Internal::AsChildType<
				std::is_same<Self, ListBase>::value, Self, ListBase
			>::AsChild(*this, "List", this->GetCategoryName());
	}

	virtual const ListBase& AsList() const override
	{
		return Internal::AsChildType<
				std::is_same<Self, ListBase>::value, Self, ListBase
			>::AsChild(*this, "List", this->GetCategoryName());
	}

	// ========== operators ==========

	// ===== This class

	virtual bool ListBaseIsEqual(const Self& rhs) const = 0;

	virtual ObjectOrder ListBaseCompare(const Self& rhs) const = 0;

	bool operator==(const Self& rhs) const
	{
		return ListBaseIsEqual(rhs);
	}

	bool operator!=(const Self& rhs) const
	{
		return !(*this == rhs);
	}

	bool operator<(const Self& rhs) const
	{
		auto cmpRes = ListBaseCompare(rhs);
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

	bool operator>(const Self& rhs) const
	{
		auto cmpRes = ListBaseCompare(rhs);
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
		return !(*this > rhs);
	}

	bool operator>=(const Self& rhs) const
	{
		return !(*this < rhs);
	}

	// ===== ObjectBase class

	virtual bool BaseObjectIsEqual(const Base& rhs) const override
	{
		return (rhs.GetCategory() == ObjCategory::List) &&
				ListBaseIsEqual(rhs.AsList());
	}

	virtual ObjectOrder BaseObjectCompare(const Base& rhs) const override
	{
		switch (rhs.GetCategory())
		{
		case ObjCategory::List:
			return ListBaseCompare(rhs.AsList());
		default:
			return ObjectOrder::NotEqualUnordered;
		}
	}

	using Base::operator==;
#ifdef __cpp_lib_three_way_comparison
	using Base::operator<=>;
#else
	using Base::operator!=;
	using Base::operator<;
	using Base::operator>;
	using Base::operator<=;
	using Base::operator>=;
#endif

	// ========== Functions that doesn't have value_type in prototype ==========

	virtual size_t size() const = 0;

	virtual void pop_back() = 0;

	virtual void resize(size_t len) = 0;

	virtual void reserve(size_t len) = 0;

	virtual void Remove(size_t idx) = 0;

	virtual void clear() = 0;

	// ========== Functions that involves value_type in prototype ==========

	// ========== value access ==========

	reference operator[](size_t idx)
	{
		return ListBaseAt(idx);
	}

	const_reference operator[](size_t idx) const
	{
		return ListBaseAt(idx);
	}

	// ========== adding/removing values ==========

	void push_back(value_type&& val)
	{
		return ListBasePushBack(std::forward<value_type>(val));
	}

	void push_back(const_reference val)
	{
		return ListBasePushBack(val);
	}

	void Append(const_iterator begin, const_iterator end)
	{
		while (begin != end)
		{
			push_back(*begin);
			++begin;
		}
	}

	void Append(const Self& other)
	{
		Append(other.cbegin(), other.cend());
	}

	// ========== item searching ==========

	bool Contains(const_reference val) const
	{
		auto e = cend();
		return std::find(cbegin(), e, val) != e;
	}

	// ========== iterators ==========

	RdIterator<value_type, false> begin()
	{
		return ListBaseBegin();
	}

	RdIterator<value_type, false> end()
	{
		return ListBaseEnd();
	}

	RdIterator<value_type, true> cbegin() const
	{
		return ListBaseBegin();
	}

	RdIterator<value_type, true> cend() const
	{
		return ListBaseEnd();
	}

	RdIterator<value_type, true> begin() const
	{
		return cbegin();
	}

	RdIterator<value_type, true> end() const
	{
		return cend();
	}

	// ========== Copy and Move ==========

	virtual std::unique_ptr<Self> Copy(const Self* /*unused*/) const = 0;

	virtual std::unique_ptr<Self> Move(const Self* /*unused*/) = 0;

	virtual std::unique_ptr<Base> Copy(const Base* /*unused*/) const override
	{
		return Copy(sk_null);
	}

	virtual std::unique_ptr<Base> Move(const Base* /*unused*/) override
	{
		return Move(sk_null);
	}

protected:

	virtual RdIterator<value_type, false> ListBaseBegin() = 0;

	virtual RdIterator<value_type, false> ListBaseEnd() = 0;

	virtual RdIterator<value_type, true> ListBaseBegin() const = 0;

	virtual RdIterator<value_type, true> ListBaseEnd() const = 0;

	virtual reference ListBaseAt(size_t idx) = 0;

	virtual const_reference ListBaseAt(size_t idx) const = 0;

	virtual void ListBasePushBack(value_type&& val) = 0;

	virtual void ListBasePushBack(const_reference val) = 0;

}; // class ListBaseObject

} // namespace SimpleObjects
