// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "StringBaseObject.hpp"

#include <algorithm>

#include "Compare.hpp"
#include "ToString.hpp"
#include "Utils.hpp"

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{

template<typename _CtnType, typename _ToStringType>
class StringImpl :
	public StringBaseObject<
		typename _CtnType::value_type,
		_ToStringType>
{
public: // Static member:

	using ContainerType = _CtnType;
	using ToStringType = _ToStringType;
	using Self = StringImpl<ContainerType, ToStringType>;
	using Base = StringBaseObject<
		typename ContainerType::value_type, ToStringType>;
	using BaseBase = typename Base::Base;
	using BaseBaseBase = typename BaseBase::Base;

	static_assert(std::is_same<BaseBase, HashableBaseObject<_ToStringType> >::value,
		"Expecting Base::Base to be HashableBaseObject class");
	static_assert(std::is_same<BaseBaseBase, BaseObject<_ToStringType> >::value,
		"Expecting Base::Base::Base to be BaseObject class");

	typedef typename ContainerType::traits_type          traits_type;
	typedef typename ContainerType::allocator_type       allocator_type;
	typedef typename ContainerType::value_type           value_type;
	typedef typename ContainerType::size_type            size_type;
	typedef typename ContainerType::difference_type      difference_type;
	typedef typename ContainerType::reference            reference;
	typedef typename ContainerType::const_reference      const_reference;
	typedef typename ContainerType::pointer              pointer;
	typedef typename ContainerType::const_pointer        const_pointer;
	typedef typename Base::iterator                      iterator;
	typedef typename Base::const_iterator                const_iterator;
	typedef typename Base::iterator                      reverse_iterator;
	typedef typename Base::const_iterator                const_reverse_iterator;

	static constexpr size_type npos = ContainerType::npos;

	static constexpr ObjCategory sk_cat()
	{
		return ObjCategory::String;
	}

	// static_assert(std::is_same<Base::value_type, ContainerType::value_type>::value,
	// 	"value_type of base class does not match the internal type.");

	static_assert(std::is_same<value_type, char>::value,
		"Current implementation only supports char strings.");

public:

	StringImpl() :
		m_data()
	{}

	StringImpl(const ContainerType& str):
		m_data(str)
	{}

	StringImpl(ContainerType&& str):
		m_data(std::forward<ContainerType>(str))
	{}

	StringImpl(const_pointer str) :
		m_data(str)
	{}

	StringImpl(const Self& other) :
		m_data(other.m_data)
	{}

	StringImpl(Self&& other) :
		m_data(std::forward<ContainerType>(other.m_data))
	{}

	virtual ~StringImpl() = default;

	Self& operator=(const Self& rhs)
	{
		if (this != &rhs)
		{
			m_data = rhs.m_data;
		}
		return *this;
	}

	Self& operator=(Self&& rhs)
	{
		if (this != &rhs)
		{
			m_data = std::forward<ContainerType>(rhs.m_data);
		}
		return *this;
	}

	const ContainerType& GetVal() const
	{
		return m_data;
	}

	// ========== operators ==========

	// ===== This class

	bool operator==(const Self& rhs) const
	{
		return m_data == rhs.m_data;
	}
#ifdef __cpp_lib_three_way_comparison
	auto operator<=>(const Self& rhs) const
	{
		return m_data <=> rhs.m_data;
	}
#else
	bool operator!=(const Self& rhs) const
	{
		return m_data != rhs.m_data;
	}
	bool operator<(const Self& rhs) const
	{
		return m_data < rhs.m_data;
	}
	bool operator>(const Self& rhs) const
	{
		return m_data > rhs.m_data;
	}
	bool operator<=(const Self& rhs) const
	{
		return m_data <= rhs.m_data;
	}
	bool operator>=(const Self& rhs) const
	{
		return m_data >= rhs.m_data;
	}
#endif

	// ===== StringBase class

	virtual bool StringBaseEqual(size_t pos1, size_t count1,
		const_pointer begin, const_pointer end) const override
	{
		auto ptrDiff = end - begin;
		return Internal::RealNumCompare<decltype(ptrDiff), size_t>::Equal(
				ptrDiff, count1) ?
			std::equal(
				m_data.data() + pos1, m_data.data() + pos1 + count1,
				begin) :
			false;
	}

	virtual int StringBaseCompare(size_t pos1, size_t count1,
		const_pointer begin, const_pointer end) const override
	{
		return Internal::LexicographicalCompareThreeWay(
			m_data.data() + pos1, m_data.data() + pos1 + count1,
			begin, end);
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

	// ========== Overrides StringBaseObject ==========

	// ========== capacity ==========

	virtual size_t size() const override
	{
		return m_data.size();
	}

	virtual void resize(size_t len) override
	{
		m_data.resize(len);
	}

	virtual void reserve(size_t len) override
	{
		m_data.reserve(len);
	}

	// ========== value access ==========

	virtual reference operator[](size_t idx) override
	{
		try
		{
			return m_data.at(idx);
		}
		catch(const std::out_of_range&)
		{
			throw IndexError(idx);
		}
	}

	virtual const_reference operator[](size_t idx) const override
	{
		try
		{
			return m_data.at(idx);
		}
		catch(const std::out_of_range&)
		{
			throw IndexError(idx);
		}
	}

	const_pointer data() const override
	{
		return m_data.data();
	}

	const_pointer c_str() const override
	{
		return m_data.c_str();
	}

	// ========== adding/removing values ==========

	virtual void push_back(const value_type& ch) override
	{
		m_data.push_back(ch);
	}

	virtual void pop_back() override
	{
		m_data.pop_back();
	}

	using Base::Append;
	virtual void Append(const_iterator begin, const_iterator end) override
	{
		std::copy(begin, end, std::back_inserter(m_data));
	}

	// ========== item searching ==========

	using Base::StartsWith;
	virtual bool StartsWith(
		const_iterator begin, const_iterator end) const override
	{
		return Internal::FindAt(cbegin(), cend(), begin, end);
	}

	using Base::EndsWith;
	virtual bool EndsWith(
		const_iterator begin, const_iterator end) const override
	{
		return Internal::FindAt(crbegin(), crend(),
			std::reverse_iterator<const_iterator >(end),
			std::reverse_iterator<const_iterator >(begin));
	}

	using Base::Contains;
	virtual const_iterator Contains(
		const_iterator begin, const_iterator end) const override
	{
		auto res = cbegin();
		for(; res != cend(); ++res)
		{
			if (Internal::FindAt(res, cend(), begin, end))
			{
				return res;
			}
		}
		return res;
	}

	// ========== iterators ==========

	using Base::begin;
	using Base::end;

	virtual iterator begin() override
	{
		return ToRdIt<false>(m_data.begin());
	}

	virtual iterator end() override
	{
		return ToRdIt<false>(m_data.end());
	}

	virtual const_iterator cbegin() const override
	{
		return ToRdIt<true>(m_data.cbegin());
	}

	virtual const_iterator cend() const override
	{
		return ToRdIt<true>(m_data.cend());
	}

	virtual reverse_iterator rbegin() override
	{
		return ToRdIt<false>(m_data.rbegin());
	}

	virtual reverse_iterator rend() override
	{
		return ToRdIt<false>(m_data.rend());
	}

	virtual const_reverse_iterator crbegin() const override
	{
		return ToRdIt<true>(m_data.crbegin());
	}

	virtual const_reverse_iterator crend() const override
	{
		return ToRdIt<true>(m_data.crend());
	}

	// ========== Overrides HashableBaseObject ==========

	virtual std::size_t Hash() const override
	{
		return std::hash<ContainerType>()(m_data);
	}

	// ========== Overrides BaseObject ==========

	virtual ObjCategory GetCategory() const override
	{
		return sk_cat();
	}

	using BaseBaseBase::Set;

	virtual void Set(const BaseBaseBase& other) override
	{
		try
		{
			const Self& casted = dynamic_cast<const Self&>(other);
			*this = casted;
		}
		catch(const std::bad_cast&)
		{
			throw TypeError("String", this->GetCategoryName());
		}
	}

	virtual void Set(BaseBaseBase&& other) override
	{
		try
		{
			Self&& casted = dynamic_cast<Self&&>(other);
			*this = std::forward<Self>(casted);
		}
		catch(const std::bad_cast&)
		{
			throw TypeError("String", this->GetCategoryName());
		}
	}

	virtual bool IsTrue() const override
	{
		return m_data.size() > 0;
	}

	// ========== Interface copy/Move ==========

	using Base::Copy;
	virtual std::unique_ptr<Base> Copy(const Base* /*unused*/) const override
	{
		return CopyImpl();
	}

	using Base::Move;
	virtual std::unique_ptr<Base> Move(const Base* /*unused*/) override
	{
		return MoveImpl();
	}

	// ========== To string ==========

	virtual std::string DebugString() const override
	{
		return "\"" +
			Internal::ToString<std::string>(m_data.begin(), m_data.end()) +
			"\"";
	}

	virtual std::string ShortDebugString() const override
	{
		return DebugString();
	}

	virtual ToStringType ToString() const override
	{
		return Internal::ToString<ToStringType>("\"") +
			Internal::ToString<ToStringType>(m_data.begin(), m_data.end()) +
			Internal::ToString<ToStringType>("\"");
	}

	virtual void DumpString(OutIterator<typename ToStringType::value_type> outIt) const override
	{
		*outIt++='\"';
		std::copy(m_data.begin(), m_data.end(), outIt);
		*outIt++='\"';
	}

private:

	std::unique_ptr<Self> CopyImpl() const
	{
		return Internal::make_unique<Self>(*this);
	}

	std::unique_ptr<Self> MoveImpl()
	{
		return Internal::make_unique<Self>(std::move(*this));
	}

	ContainerType m_data;

}; // class StringImpl

} // namespace SimpleObjects

// ========== Hash ==========
namespace std
{

	template<typename _CtnType, typename _ToStringType>
#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
	struct hash<SimpleObjects::StringImpl<_CtnType, _ToStringType> >
	{
		using _ObjType = SimpleObjects::StringImpl<_CtnType, _ToStringType>;
#else
	struct hash<SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE::StringImpl<_CtnType, _ToStringType> >
	{
		using _ObjType = SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE::StringImpl<_CtnType, _ToStringType>;
#endif

	public:

#if __cplusplus < 201703L
		typedef size_t       result_type;
		typedef _ObjType     argument_type;
#endif

		size_t operator()(const _ObjType& cnt) const
		{
			return cnt.Hash();
		}
	}; // struct hash

} // namespace std
