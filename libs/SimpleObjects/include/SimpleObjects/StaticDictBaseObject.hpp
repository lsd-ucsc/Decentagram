// Copyright (c) 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.



#pragma once

#include "BaseObject.hpp"

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{

template<
	typename _DynKeyType,
	typename _DynValType,
	template<typename> class _KeyRefWrapType,
	template<typename> class _RefWrapType,
	typename _ToStringType>
class StaticDictBaseObject : public BaseObject<_ToStringType>
{
public: // Static members

	using Self = StaticDictBaseObject<
		_DynKeyType,
		_DynValType,
		_KeyRefWrapType,
		_RefWrapType,
		_ToStringType>;
	using Base = BaseObject<_ToStringType>;
	using ToStringType = _ToStringType;

	using StatDictBase = typename Base::StatDictBase;

	typedef _DynKeyType                               key_type;
	typedef _DynValType                               mapped_type;
	typedef _KeyRefWrapType<const key_type>           key_const_ref_type;
	typedef _RefWrapType<mapped_type>                 mapped_ref_type;
	typedef _RefWrapType<const mapped_type>           mapped_const_ref_type;

	typedef std::pair<const key_const_ref_type, const mapped_ref_type>
		iterator_value_type;
	typedef std::pair<const key_const_ref_type, const mapped_const_ref_type>
		const_iterator_value_type;
	typedef RdIterator<iterator_value_type,       true>  iterator;
	typedef RdIterator<const_iterator_value_type, true>  const_iterator;

	static constexpr Self* sk_null = nullptr;

public:
	StaticDictBaseObject() = default;

	// LCOV_EXCL_START
	/**
	 * @brief Destroy the Dict Base Object
	 *
	 */
	virtual ~StaticDictBaseObject() = default;
	// LCOV_EXCL_STOP

	virtual const char* GetCategoryName() const override
	{
		return "StaticDict";
	}

	virtual StatDictBase& AsStaticDict() override
	{
		return Internal::AsChildType<
				std::is_same<Self, StatDictBase>::value, Self, StatDictBase
			>::AsChild(*this, "StaticDict", this->GetCategoryName());
	}

	virtual const StatDictBase& AsStaticDict() const override
	{
		return Internal::AsChildType<
				std::is_same<Self, StatDictBase>::value, Self, StatDictBase
			>::AsChild(*this, "StaticDict", this->GetCategoryName());
	}

	// ========== Comparisons ==========

	// ===== This class

	virtual bool StaticDictBaseEqual(const Self& rhs) const = 0;

	bool operator==(const Self& rhs) const
	{
		return StaticDictBaseEqual(rhs);
	}

#ifndef __cpp_lib_three_way_comparison
	bool operator!=(const Self& rhs) const
	{
		return !(*this == rhs);
	}
#endif

	bool operator<(const Self& rhs) = delete;
	bool operator>(const Self& rhs) = delete;
	bool operator<=(const Self& rhs) = delete;
	bool operator>=(const Self& rhs) = delete;

	// ===== ObjectBase class

	virtual bool BaseObjectIsEqual(const Base& rhs) const override
	{
		return (rhs.GetCategory() == ObjCategory::StaticDict) &&
				StaticDictBaseEqual(rhs.AsStaticDict());
	}

	virtual ObjectOrder BaseObjectCompare(const Base& rhs) const override
	{
		switch (rhs.GetCategory())
		{
		case ObjCategory::StaticDict:
			return StaticDictBaseEqual(rhs.AsStaticDict()) ?
					ObjectOrder::EqualUnordered :
					ObjectOrder::NotEqualUnordered;
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

	// ========== member testing ==========

	virtual bool HasKey(const key_type& key) const = 0;

	// ========== value access ==========

	virtual mapped_type& at(const key_type& key) = 0;

	virtual const mapped_type& at(const key_type& key) const = 0;

	virtual mapped_type& operator[](const key_type& key) = 0;

	virtual const mapped_type& operator[](const key_type& key) const = 0;

	virtual mapped_type& at(size_t idx) = 0;

	virtual const mapped_type& at(size_t idx) const = 0;

	virtual mapped_type& operator[](size_t idx) = 0;

	virtual const mapped_type& operator[](size_t idx) const = 0;

	// ========== iterators ==========

	virtual iterator begin() = 0;
	virtual iterator end() = 0;

	virtual const_iterator cbegin() const = 0;
	virtual const_iterator cend() const = 0;

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

	virtual std::unique_ptr<Base> Copy(const Base* /*unused*/) const override
	{
		return Copy(sk_null);
	}

	virtual std::unique_ptr<Base> Move(const Base* /*unused*/) override
	{
		return Move(sk_null);
	}

}; // class DictBaseObject

} // namespace SimpleObjects
