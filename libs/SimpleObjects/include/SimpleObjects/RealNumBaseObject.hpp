// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "HashableBaseObject.hpp"

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{

/**
 * @brief Defining a interface class for numeric objects
 *
 */
template<typename _ToStringType>
class RealNumBaseObject : public HashableBaseObject<_ToStringType>
{
public: // Static Member

	using ToStringType = _ToStringType;
	using Self = RealNumBaseObject<_ToStringType>;
	using Base = HashableBaseObject<_ToStringType>;
	using BaseBase = typename Base::Base;

	static_assert(std::is_same<BaseBase, BaseObject<_ToStringType> >::value,
		"Expecting Base::Base to be BaseObject class");

	using RealNumBase = typename BaseBase::RealNumBase;

	static constexpr Self* sk_null = nullptr;

public:
	RealNumBaseObject() = default;

	// LCOV_EXCL_START
	/**
	 * @brief Destroy the RealNumImpl Base Object
	 *
	 */
	virtual ~RealNumBaseObject() = default;
	// LCOV_EXCL_STOP

	virtual RealNumType GetNumType() const = 0;

	virtual const char* GetNumTypeName() const = 0;

	virtual RealNumBase& AsRealNum() override
	{
		return Internal::AsChildType<
				std::is_same<Self, RealNumBase>::value, Self, RealNumBase
			>::AsChild(*this, "RealNumImpl Category", this->GetCategoryName());
	}

	virtual const RealNumBase& AsRealNum() const override
	{
		return Internal::AsChildType<
				std::is_same<Self, RealNumBase>::value, Self, RealNumBase
			>::AsChild(*this, "RealNumImpl Category", this->GetCategoryName());
	}

	// ========== Comparisons ==========

	// ===== This class

	virtual bool RealNumBaseEqual(const Self& rhs) const = 0;

	virtual int RealNumBaseCmp(const Self& rhs) const = 0;

	bool operator==(const Self& rhs) const
	{
		return RealNumBaseEqual(rhs);
	}

#ifdef __cpp_lib_three_way_comparison
	std::strong_ordering operator<=>(const Self& rhs) const
	{
		auto cmpRes = RealNumBaseCmp(rhs);
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
		return RealNumBaseCmp(rhs) < 0;
	}

	bool operator>(const Self& rhs) const
	{
		return RealNumBaseCmp(rhs) > 0;
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

	// ===== BaseObject class

	virtual bool BaseObjectIsEqual(const BaseBase& rhs) const override
	{
		switch (rhs.GetCategory())
		{
		case ObjCategory::Bool:
		case ObjCategory::Integer:
		case ObjCategory::Real:
			return RealNumBaseEqual(rhs.AsRealNum());
		default:
			return false;
		}
	}

	virtual ObjectOrder BaseObjectCompare(const BaseBase& rhs) const override
	{
		switch (rhs.GetCategory())
		{
		case ObjCategory::Bool:
		case ObjCategory::Integer:
		case ObjCategory::Real:
		{
			auto cmpRes = RealNumBaseCmp(rhs.AsRealNum());
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

}; // class RealNumBaseObject

} // namespace SimpleObjects
