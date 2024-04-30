// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "HashableBaseObject.hpp"

#include <memory>

#include "Null.hpp"

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{

template<typename _ToStringType>
class HashableObjectImpl : public HashableBaseObject<_ToStringType>
{
public: // Static members

	using ToStringType = _ToStringType;
	using Self = HashableObjectImpl<_ToStringType>;
	using Base = HashableBaseObject<_ToStringType>;
	using BaseBase = typename Base::Base;

	static_assert(std::is_same<BaseBase, BaseObject<_ToStringType> >::value,
		"Expecting Base::Base to be BaseObject class");

	using BasePtr = std::unique_ptr<Base>;

	using NullBase      = typename Base::NullBase;
	using RealNumBase   = typename Base::RealNumBase;
	using StringBase    = typename Base::StringBase;
	using ListBase      = typename Base::ListBase;
	using DictBase      = typename Base::DictBase;
	using StatDictBase  = typename Base::StatDictBase;
	using BytesBase     = typename Base::BytesBase;
	using HashableBase  = typename Base::HashableBase;

public:
	HashableObjectImpl() :
		HashableObjectImpl(NullImpl<ToStringType>())
	{}

	HashableObjectImpl(const Self& other) :
		HashableObjectImpl(*other.m_ptr)
	{}

	HashableObjectImpl(Self&& other) :
		m_ptr(std::forward<BasePtr>(other.m_ptr))
	{}

	HashableObjectImpl(const Base& other) :
		m_ptr(other.Copy(Base::sk_null))
	{}

	HashableObjectImpl(Base&& other) :
		m_ptr(other.Move(Base::sk_null))
	{}

	HashableObjectImpl(BasePtr other) :
		m_ptr(std::move(other))
	{}

	virtual ~HashableObjectImpl() = default;

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

	// ========== Overrides BaseObject ==========

	virtual ObjCategory GetCategory() const override
	{
		return m_ptr->GetCategory();
	}

	virtual const char* GetCategoryName() const override
	{
		return m_ptr->GetCategoryName();
	}

	virtual void Set(const BaseBase& other) override
	{
		m_ptr->Set(other);
	}

	virtual void Set(BaseBase&& other) override
	{
		m_ptr->Set(std::forward<BaseBase>(other));
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

	virtual bool BaseObjectIsEqual(const BaseBase& rhs) const override
	{
		return m_ptr->BaseObjectIsEqual(rhs);
	}

	virtual ObjectOrder BaseObjectCompare(const BaseBase& rhs) const override
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

	// ========== Overrides HashableBaseObject ==========

	virtual std::size_t Hash() const override
	{
		return m_ptr->Hash();
	}

	// ========== Interface copy/Move ==========

	using Base::Copy;
	virtual std::unique_ptr<Base> Copy(const Base* unused) const override
	{
		return m_ptr->Copy(unused);
	}

	using Base::Move;
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

}; // class HashableObjectImpl

} // namespace SimpleObjects

// ========== Hash ==========
namespace std
{
	template<typename _ToStringType>
#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
	struct hash<SimpleObjects::HashableObjectImpl<_ToStringType> > :
		public hash<SimpleObjects::HashableBaseObject<_ToStringType> >
	{
		using _ObjType = SimpleObjects::HashableObjectImpl<_ToStringType>;
#else
	struct hash<SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE::HashableObjectImpl<_ToStringType> > :
		public hash<SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE::HashableBaseObject<_ToStringType> >
	{
		using _ObjType = SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE::HashableObjectImpl<_ToStringType>;
#endif
	}; // struct hash
} // namespace std
