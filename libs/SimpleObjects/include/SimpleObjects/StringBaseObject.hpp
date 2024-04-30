// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "HashableBaseObject.hpp"

#include "Iterator.hpp"

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{

/**
 * @brief Defining a interface class for objects that are string-like
 *
 * @tparam _CharType type of each charactor value.
 */
template<typename _CharType, typename _ToStringType>
class StringBaseObject : public HashableBaseObject<_ToStringType>
{
public: // Static members

	using ToStringType = _ToStringType;
	using Self = StringBaseObject<_CharType, _ToStringType>;
	using Base = HashableBaseObject<_ToStringType>;
	using BaseBase = typename Base::Base;

	static_assert(std::is_same<BaseBase, BaseObject<_ToStringType> >::value,
		"Expecting Base::Base to be BaseObject class");

	using StringBase = typename BaseBase::StringBase;

	typedef _CharType                           value_type;
	typedef value_type&                         reference;
	typedef const value_type&                   const_reference;
	typedef value_type*                         pointer;
	typedef const value_type*                   const_pointer;
	typedef RdIterator<value_type, false>       iterator;
	typedef RdIterator<value_type, true>        const_iterator;

	static constexpr Self* sk_null = nullptr;

public:
	StringBaseObject() = default;

	// LCOV_EXCL_START
	/**
	 * @brief Destroy the String Base Object
	 *
	 */
	virtual ~StringBaseObject() = default;
	// LCOV_EXCL_STOP

	virtual const char* GetCategoryName() const override
	{
		return "String";
	}

	virtual StringBase& AsString() override
	{
		return Internal::AsChildType<
				std::is_same<Self, StringBase>::value, Self, StringBase
			>::AsChild(*this, "String", this->GetCategoryName());
	}

	virtual const StringBase& AsString() const override
	{
		return Internal::AsChildType<
				std::is_same<Self, StringBase>::value, Self, StringBase
			>::AsChild(*this, "String", this->GetCategoryName());
	}

	// ========== Comparisons ==========

	// ===== This class

	/**
	 * @brief lexicographical compare this string with the other
	 *
	 * @param pos1   the begin position of this string
	 * @param count1 the length since the begin position to be compared
	 * @param begin  the begin of the other string
	 * @param end    the end of the other string
	 * @return compare result
	 */
	virtual bool StringBaseEqual(size_t pos1, size_t count1,
		const_pointer begin, const_pointer end) const = 0;

	virtual int StringBaseCompare(size_t pos1, size_t count1,
		const_pointer begin, const_pointer end) const = 0;

	bool operator==(const Self& rhs) const
	{
		return StringBaseEqual(
			0, size(),
			rhs.data(), rhs.data() + rhs.size());
	}

#ifdef __cpp_lib_three_way_comparison
	std::strong_ordering operator<=>(const Self& rhs) const
	{
		auto cmpRes = StringBaseCompare(
			0, size(),
			rhs.data(), rhs.data() + rhs.size());
		return cmpRes == 0 ? std::strong_ordering::equal :
				(cmpRes < 0 ? std::strong_ordering::less :
				(std::strong_ordering::greater));
	}
#else
	bool operator!=(const Self& rhs) const
	{
		return !(*this == rhs);
	}

	bool operator<(const Self& rhs) const
	{
		return StringBaseCompare(
			0, size(),
			rhs.data(), rhs.data() + rhs.size()) < 0;
	}

	bool operator>(const Self& rhs) const
	{
		return StringBaseCompare(
			0, size(),
			rhs.data(), rhs.data() + rhs.size()) > 0;
	}

	bool operator<=(const Self& rhs) const
	{
		return !(*this > rhs);
	}

	bool operator>=(const Self& rhs) const
	{
		return !(*this < rhs);
	}
#endif

	// ===== ObjectBase class

	virtual bool BaseObjectIsEqual(const BaseBase& rhs) const override
	{
		if(rhs.GetCategory() == ObjCategory::String)
		{
			const auto& rhsStr = rhs.AsString();
			auto rhsStrData = rhsStr.data();
			return StringBaseEqual(0, size(),
				rhsStrData, rhsStrData + rhsStr.size());
		}
		else
		{
			return false;
		}
	}

	virtual ObjectOrder BaseObjectCompare(const BaseBase& rhs) const override
	{
		switch (rhs.GetCategory())
		{
		case ObjCategory::String:
		{
			const auto& rhsStr = rhs.AsString();
			auto rhsStrData = rhsStr.data();
			auto cmpRes = StringBaseCompare(0, size(),
				rhsStrData, rhsStrData + rhsStr.size());

			return cmpRes == 0 ? ObjectOrder::Equal :
					(cmpRes < 0 ? ObjectOrder::Less :
					(ObjectOrder::Greater));
		}
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

	// ========== capacity ==========

	virtual size_t size() const = 0;

	virtual void resize(size_t len) = 0;

	virtual void reserve(size_t len) = 0;

	// ========== value access ==========

	virtual reference operator[](size_t idx) = 0;

	virtual const_reference operator[](size_t idx) const = 0;

	virtual const_pointer c_str() const = 0;

	virtual const_pointer data() const = 0;

	//virtual std::unique_ptr<Self> Copy() const = 0;

	//virtual std::unique_ptr<Self> Slice(size_t begin, size_t end) const = 0;

	// ========== adding/removing values ==========

	virtual void push_back(const_reference ch) = 0;

	virtual void pop_back() = 0;

	virtual void Append(const_iterator begin,
		const_iterator end) = 0;

	virtual void Append(const Self& other)
	{
		return this->Append(other.cbegin(), other.cend());
	}

	virtual Self& operator+=(const Self& rhs)
	{
		this->Append(rhs);
		return *this;
	}

	// ========== item searching ==========

	virtual bool StartsWith(const_iterator begin,
		const_iterator end) const = 0;

	virtual bool StartsWith(const Self& other) const
	{
		if (this->size() < other.size())
		{
			return false;
		}
		return this->StartsWith(other.cbegin(), other.end());
	}

	virtual bool EndsWith(
		const_iterator begin, const_iterator end) const = 0;

	virtual bool EndsWith(const Self& other) const
	{
		if (this->size() < other.size())
		{
			return false;
		}
		return this->EndsWith(other.cbegin(), other.end());
	}

	virtual const_iterator Contains(
		const_iterator begin, const_iterator end) const = 0;

	virtual const_iterator Contains(const Self& other) const
	{
		if (this->size() < other.size())
		{
			return cend();
		}
		return this->Contains(other.cbegin(), other.end());
	}

	// ========== iterators ==========

	virtual iterator begin() = 0;
	virtual iterator end() = 0;

	virtual const_iterator cbegin() const = 0;
	virtual const_iterator cend() const = 0;

	virtual iterator rbegin() = 0;
	virtual iterator rend() = 0;

	virtual const_iterator crbegin() const = 0;
	virtual const_iterator crend() const = 0;

	virtual const_iterator begin() const
	{
		return this->cbegin();
	}

	virtual const_iterator end() const
	{
		return this->cend();
	}

	// ========== Interface copy/Move ==========

	virtual std::unique_ptr<Self> Copy(const Self* /*unused*/) const = 0;

	virtual std::unique_ptr<Self> Move(const Self* /*unused*/) = 0;

	using Base::Copy;
	virtual std::unique_ptr<Base> Copy(const Base* /*unused*/) const override
	{
		return Copy(sk_null);
	}

	using Base::Move;
	virtual std::unique_ptr<Base> Move(const Base* /*unused*/) override
	{
		return Move(sk_null);
	}

}; // class StringBaseObject

} // namespace SimpleObjects
