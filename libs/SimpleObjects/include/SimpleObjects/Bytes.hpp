// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "BytesBaseObject.hpp"

#include "Internal/hash.hpp"
#include "Internal/make_unique.hpp"

#include "Compare.hpp"
#include "ToString.hpp"

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{

template<typename _CtnType, typename _ToStringType>
class BytesImpl :
	public BytesBaseObject<
		typename _CtnType::value_type,
		_ToStringType>
{
public: // Static member:

	using ContainerType = _CtnType;
	using ToStringType = _ToStringType;
	using Self = BytesImpl<ContainerType, ToStringType>;
	using Base = BytesBaseObject<
		typename ContainerType::value_type, ToStringType>;
	using BaseBase = typename Base::Base;
	using BaseBaseBase = typename BaseBase::Base;

	static_assert(std::is_same<BaseBase, HashableBaseObject<_ToStringType> >::value,
		"Expecting Base::Base to be HashableBaseObject class");
	static_assert(std::is_same<BaseBaseBase, BaseObject<_ToStringType> >::value,
		"Expecting Base::Base::Base to be BaseObject class");

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

	static constexpr ObjCategory sk_cat()
	{
		return ObjCategory::Bytes;
	}

	static_assert(std::is_same<value_type, uint8_t>::value,
		"Current implementation only supports uint8_t bytes.");

public:

	BytesImpl() :
		m_data()
	{}

	explicit BytesImpl(const ContainerType& data):
		m_data(data)
	{}

	explicit BytesImpl(ContainerType&& data):
		m_data(std::forward<ContainerType>(data))
	{}

	BytesImpl(std::initializer_list<value_type> l):
		m_data(l)
	{}

	template<typename _ItType>
	BytesImpl(_ItType begin, _ItType end) :
		m_data(begin, end)
	{}

	BytesImpl(const Self& other) :
		m_data(other.m_data)
	{}

	BytesImpl(Self&& other) :
		m_data(std::forward<ContainerType>(other.m_data))
	{}

	virtual ~BytesImpl() = default;

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

	// ========== Comparisons ==========

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

	// ===== BytesBase class

	virtual bool BytesBaseEqual(size_t pos1, size_t count1,
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

	virtual int BytesBaseCompare(size_t pos1, size_t count1,
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
			throw TypeError("Bytes", this->GetCategoryName());
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
			throw TypeError("Bytes", this->GetCategoryName());
		}
	}

	virtual bool IsTrue() const override
	{
		return m_data.size() > 0;
	}

	// ========== Overrides HashableBaseObject ==========

	virtual std::size_t Hash() const override
	{
		return Internal::hash_range(m_data.cbegin(), m_data.cend());
	}

	// ========== Overrides BytesBaseObject ==========

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

	virtual const_pointer data() const override
	{
		return m_data.data();
	}

	// ========== adding/removing values ==========

	virtual void push_back(const_reference b) override
	{
		m_data.push_back(b);
	}

	virtual void pop_back() override
	{
		m_data.pop_back();
	}

	virtual void Append(const_iterator begin, const_iterator end) override
	{
		m_data.insert(m_data.end(), begin, end);
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

	virtual iterator rbegin() override
	{
		return ToRdIt<false>(m_data.rbegin());
	}

	virtual iterator rend() override
	{
		return ToRdIt<false>(m_data.rend());
	}

	virtual const_iterator crbegin() const override
	{
		return ToRdIt<true>(m_data.crbegin());
	}

	virtual const_iterator crend() const override
	{
		return ToRdIt<true>(m_data.crend());
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
		return InternalToString<std::string>();
	}

	virtual std::string ShortDebugString() const override
	{
		return DebugString();
	}

	virtual ToStringType ToString() const override
	{
		return InternalToString<ToStringType>();
	}

	virtual void DumpString(
		OutIterator<typename ToStringType::value_type> outIt) const override
	{
		InternalDumpString<typename ToStringType::value_type>(outIt);
	}

private:

	template<typename _StrType>
	_StrType InternalToString() const
	{
		_StrType res;

		InternalDumpString<typename _StrType::value_type>(
			std::back_inserter(res));

		return res;
	}

	template<typename _CharType, typename _ItType>
	void InternalDumpString(_ItType outit) const
	{
		*outit++ = '\"';

		for (const auto& b : m_data)
		{
			Internal::ByteToHEX<true, _CharType>(
				outit,
				static_cast<uint8_t>(b)
			);
		}

		*outit++ = '\"';
	}

	std::unique_ptr<Self> CopyImpl() const
	{
		return Internal::make_unique<Self>(*this);
	}

	std::unique_ptr<Self> MoveImpl()
	{
		return Internal::make_unique<Self>(std::move(*this));
	}

	ContainerType m_data;

}; // class BytesImpl

} // namespace SimpleObjects

// ========== Hash ==========
namespace std
{

	template<typename _CtnType, typename _ToStringType>
#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
	struct hash<SimpleObjects::BytesImpl<_CtnType, _ToStringType> >
	{
		using _ObjType = SimpleObjects::BytesImpl<_CtnType, _ToStringType>;
#else
	struct hash<SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE::BytesImpl<_CtnType, _ToStringType> >
	{
		using _ObjType = SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE::BytesImpl<_CtnType, _ToStringType>;
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
