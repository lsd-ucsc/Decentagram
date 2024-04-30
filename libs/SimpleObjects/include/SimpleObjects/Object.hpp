// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "BaseObject.hpp"

#include <memory>

#include "Null.hpp"

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{

template<typename _ToStringType>
class ObjectImpl : public BaseObject<_ToStringType>
{
public: // Static members

	using ToStringType = _ToStringType;
	using Self = ObjectImpl<_ToStringType>;
	using Base = BaseObject<_ToStringType>;

	using NullBase      = typename Base::NullBase;
	using RealNumBase   = typename Base::RealNumBase;
	using StringBase    = typename Base::StringBase;
	using ListBase      = typename Base::ListBase;
	using DictBase      = typename Base::DictBase;
	using StatDictBase  = typename Base::StatDictBase;
	using BytesBase     = typename Base::BytesBase;
	using HashableBase  = typename Base::HashableBase;

	using BasePtr = std::unique_ptr<Base>;

public:
	ObjectImpl() :
		ObjectImpl(NullImpl<ToStringType>())
	{}

	ObjectImpl(const Self& other) :
		ObjectImpl(*other.m_ptr)
	{}

	ObjectImpl(Self&& other) :
		m_ptr(std::forward<BasePtr>(other.m_ptr))
	{}

	ObjectImpl(const Base& other) :
		m_ptr(other.Copy(Base::sk_null))
	{}

	ObjectImpl(Base&& other) :
		m_ptr(other.Move(Base::sk_null))
	{}

	ObjectImpl(BasePtr other) :
		m_ptr(std::move(other))
	{}

	virtual ~ObjectImpl() = default;

	Self& operator=(const Self& rhs)
	{
		*this = *rhs.m_ptr;
		return *this;
	}

	Self& operator=(Self&& rhs)
	{
		m_ptr = std::forward<BasePtr>(rhs.m_ptr);
		return *this;
	}

	Self& operator=(const Base& rhs)
	{
		m_ptr = rhs.Copy(Base::sk_null);
		return *this;
	}

	Self& operator=(Base&& rhs)
	{
		m_ptr = rhs.Move(Base::sk_null);
		return *this;
	}

	// ========== operators ==========

	// ===== This class

	bool operator==(const Self& rhs) const
	{
		return *m_ptr == *(rhs.m_ptr);
	}

#ifdef __cpp_lib_three_way_comparison
	auto operator<=>(const Self& rhs) const
	{
		return *m_ptr <=> *(rhs.m_ptr);
	}
#else
	bool operator!=(const Self& rhs) const
	{
		return *m_ptr != *(rhs.m_ptr);
	}

	bool operator<(const Self& rhs) const
	{
		return *m_ptr < *(rhs.m_ptr);
	}

	bool operator>(const Self& rhs) const
	{
		return *m_ptr > *(rhs.m_ptr);
	}

	bool operator<=(const Self& rhs) const
	{
		return *m_ptr <= *(rhs.m_ptr);
	}

	bool operator>=(const Self& rhs) const
	{
		return *m_ptr >= *(rhs.m_ptr);
	}
#endif

	// ===== BaseObject class

	virtual bool BaseObjectIsEqual(const Base& rhs) const override
	{
		return m_ptr->BaseObjectIsEqual(rhs);
	}

	virtual ObjectOrder BaseObjectCompare(const Base& rhs) const override
	{
		return m_ptr->BaseObjectCompare(rhs);
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

	// ========== Overrides BaseObject ==========

	virtual ObjCategory GetCategory() const override
	{
		return m_ptr->GetCategory();
	}

	virtual const char* GetCategoryName() const override
	{
		return m_ptr->GetCategoryName();
	}

	virtual void Set(const Base& other) override
	{
		m_ptr->Set(other);
	}

	virtual void Set(Base&& other) override
	{
		m_ptr->Set(std::forward<Base>(other));
	}

	virtual void Set(bool val) override
	{
		m_ptr->Set(val);
	}

	virtual void Set(uint8_t val) override
	{
		m_ptr->Set(val);
	}

	virtual void Set(int8_t val) override
	{
		m_ptr->Set(val);
	}

	virtual void Set(uint32_t val) override
	{
		m_ptr->Set(val);
	}

	virtual void Set(int32_t val) override
	{
		m_ptr->Set(val);
	}

	virtual void Set(uint64_t val) override
	{
		m_ptr->Set(val);
	}

	virtual void Set(int64_t val) override
	{
		m_ptr->Set(val);
	}

	virtual void Set(double val) override
	{
		m_ptr->Set(val);
	}

	virtual bool IsNull() const override
	{
		return m_ptr->IsNull();
	}

	virtual bool IsTrue() const override
	{
		return m_ptr->IsTrue();
	}

	virtual uint8_t AsCppUInt8() const override
	{
		return m_ptr->AsCppUInt8();
	}

	virtual int8_t AsCppInt8() const override
	{
		return m_ptr->AsCppInt8();
	}

	virtual uint32_t AsCppUInt32() const override
	{
		return m_ptr->AsCppUInt32();
	}

	virtual int32_t AsCppInt32() const override
	{
		return m_ptr->AsCppInt32();
	}

	virtual uint64_t AsCppUInt64() const override
	{
		return m_ptr->AsCppUInt64();
	}

	virtual int64_t AsCppInt64() const override
	{
		return m_ptr->AsCppInt64();
	}

	virtual double AsCppDouble() const override
	{
		return m_ptr->AsCppDouble();
	}

	virtual NullBase& AsNull() override
	{
		return m_ptr->AsNull();
	}

	virtual const NullBase& AsNull() const override
	{
		return m_ptr->AsNull();
	}

	virtual RealNumBase& AsRealNum() override
	{
		return m_ptr->AsRealNum();
	}

	virtual const RealNumBase& AsRealNum() const override
	{
		return m_ptr->AsRealNum();
	}

	virtual StringBase& AsString() override
	{
		return m_ptr->AsString();
	}

	virtual const StringBase& AsString() const override
	{
		return m_ptr->AsString();
	}

	virtual ListBase& AsList() override
	{
		return m_ptr->AsList();
	}

	virtual const ListBase& AsList() const override
	{
		return m_ptr->AsList();
	}

	virtual DictBase& AsDict() override
	{
		return m_ptr->AsDict();
	}

	virtual const DictBase& AsDict() const override
	{
		return m_ptr->AsDict();
	}

	virtual StatDictBase& AsStaticDict() override
	{
		return m_ptr->AsStaticDict();
	}

	virtual const StatDictBase& AsStaticDict() const override
	{
		return m_ptr->AsStaticDict();
	}

	virtual BytesBase& AsBytes() override
	{
		return m_ptr->AsBytes();
	}

	virtual const BytesBase& AsBytes() const override
	{
		return m_ptr->AsBytes();
	}

	virtual HashableBase& AsHashable() override
	{
		return m_ptr->AsHashable();
	}

	virtual const HashableBase& AsHashable() const override
	{
		return m_ptr->AsHashable();
	}

	// ========== Interface copy/Move ==========

	virtual std::unique_ptr<Base> Copy(const Base* unused) const override
	{
		return m_ptr->Copy(unused);
	}

	virtual std::unique_ptr<Base> Move(const Base* unused) override
	{
		return m_ptr->Move(unused);
	}

	// ========== To string ==========

	virtual std::string DebugString() const override
	{
		return m_ptr->DebugString();
	}

	virtual std::string ShortDebugString() const override
	{
		return m_ptr->ShortDebugString();
	}

	virtual ToStringType ToString() const override
	{
		return m_ptr->ToString();
	}

	virtual void DumpString(OutIterator<typename ToStringType::value_type> outIt) const override
	{
		return m_ptr->DumpString(outIt);
	}

private:

	BasePtr m_ptr;

}; // class ObjectImpl

} // namespace SimpleObjects
