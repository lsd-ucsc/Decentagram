// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <functional>
#include <memory>
#include <type_traits>

#include "make_unique.hpp"

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{
namespace Internal
{

template<typename _ValType, typename _BasePtrType>
class DictKeyImpl
{
public: // static members

	using Self = DictKeyImpl<_ValType, _BasePtrType>;

	typedef _ValType
		value_type;
	typedef typename std::add_const<_ValType>::type
		const_value_type;
	typedef typename std::add_lvalue_reference<value_type>::type
		conference;
	typedef typename std::add_lvalue_reference<const_value_type>::type
		const_conference;

	static Self Borrow(const _ValType* ptr)
	{
		return DictKeyImpl(ptr);
	}

	static Self Borrow(const _BasePtrType* ptr)
	{
		return DictKeyImpl(ptr);
	}

	template<typename... _T>
	static Self Make(_T&&... val)
	{
		return DictKeyImpl(std::forward<_T>(val)...);
	}

public:

	DictKeyImpl(DictKeyImpl&& other) :
		m_val(std::forward<std::unique_ptr<_ValType> >(other.m_val)),
		m_valPtr(
			m_val.get() != nullptr ?
			m_val.get() :
			other.m_valPtr),
		m_basePtr(
			m_val.get() != nullptr ?
			m_val.get() :
			other.m_basePtr)
	{
		other.m_valPtr = nullptr;
		other.m_basePtr = nullptr;
	}

	DictKeyImpl(const DictKeyImpl& other) :
		m_val(
			other.m_val.get() != nullptr ?
			Internal::make_unique<_ValType>(*(other.m_val)) :
			nullptr),
		m_valPtr(
			m_val.get() != nullptr ?
			m_val.get() :
			other.m_valPtr),
		m_basePtr(
			m_val.get() != nullptr ?
			m_val.get() :
			other.m_basePtr)
	{}

	Self& operator=(const Self& rhs)
	{
		if (this != &rhs)
		{
			m_val = rhs.m_val.get() != nullptr ?
				Internal::make_unique<_ValType>(*(rhs.m_val)) :
				nullptr;
			m_valPtr = m_val.get() != nullptr ?
				m_val.get() :
				rhs.m_valPtr;
			m_basePtr = m_val.get() != nullptr ?
				m_val.get() :
				rhs.m_basePtr;
		}
		return *this;
	}

	Self& operator=(Self&& rhs)
	{
		if (this != &rhs)
		{
			m_val = std::forward<std::unique_ptr<_ValType> >(rhs.m_val);
			m_valPtr = m_val.get() != nullptr ?
				m_val.get() :
				rhs.m_valPtr;
			m_basePtr = m_val.get() != nullptr ?
				m_val.get() :
				rhs.m_basePtr;

			rhs.m_valPtr = nullptr;
			rhs.m_basePtr = nullptr;
		}
		return *this;
	}

	~DictKeyImpl() = default;

	conference GetVal()
	{
		return *m_val;
	}

	const_conference GetVal() const
	{
		return *m_val;
	}

	const _BasePtrType* GetBasePtr() const
	{
		return m_basePtr;
	}

	std::size_t Hash() const
	{
		return m_basePtr->Hash();
	}

	bool operator==(const DictKeyImpl& rhs) const
	{
		return (m_valPtr != nullptr && rhs.m_valPtr != nullptr) ?
				(*m_valPtr == *(rhs.m_valPtr)) :
				(*m_basePtr == *(rhs.m_basePtr));
	}

	bool operator<(const DictKeyImpl& rhs) const
	{
		return (m_valPtr != nullptr && rhs.m_valPtr != nullptr) ?
				(*m_valPtr < *(rhs.m_valPtr)) :
				(*m_basePtr < *(rhs.m_basePtr));
	}

private:

	template<typename... _T>
	explicit DictKeyImpl(_T&&... val) :
		m_val(Internal::make_unique<_ValType>(
			std::forward<_T>(val)...)),
		m_valPtr(m_val.get()),
		m_basePtr(m_val.get())
	{}

	explicit DictKeyImpl(const _BasePtrType* basePtr) :
		m_val(),
		m_valPtr(nullptr),
		m_basePtr(basePtr)
	{}

	explicit DictKeyImpl(const _ValType* valPtr) :
		m_val(),
		m_valPtr(valPtr),
		m_basePtr(valPtr)
	{}

	std::unique_ptr<_ValType> m_val;
	const _ValType* m_valPtr;
	const _BasePtrType* m_basePtr;

}; // class DictKeyImpl

struct ItTransformDictKey
{
	template<typename _RetType,
		typename _ItType>
	static _RetType GetRef(const _ItType& it)
	{
		return std::get<0>(*it).GetVal();
	}

	template<typename _RetType,
		typename _ItType>
	static _RetType GetPtr(const _ItType& it)
	{
		return std::get<0>(*it).GetBasePtr();
	}

}; // struct ItTransformDictKey

} // Internal
} // namespace SimpleObjects

// ========== Hash ==========
namespace std
{

	template<typename _ValType, typename _BasePtrType>
#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
	struct hash<SimpleObjects::Internal::DictKeyImpl<_ValType, _BasePtrType> >
	{
		using _ObjType = SimpleObjects::Internal::DictKeyImpl<_ValType, _BasePtrType>;
#else
	struct hash<SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE::Internal::DictKeyImpl<_ValType, _BasePtrType> >
	{
		using _ObjType = SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE::Internal::DictKeyImpl<_ValType, _BasePtrType>;
#endif

	public:

#if __cplusplus < 201703L
		typedef size_t       result_type;
		typedef _ObjType     argument_type;
#endif

		size_t operator()(const _ObjType& val) const
		{
			return val.Hash();
		}
	}; // struct hash

} // namespace std
