// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "DictBaseObject.hpp"

#include <memory>

#include "Internal/DictKey.hpp"
#include "Internal/make_unique.hpp"

#include "ToString.hpp"

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{

template<
	typename _KeyType,
	typename _ValType,
	template<typename, typename> class _ContainerType,
	typename _ToStringType>
class DictImpl :
	public DictBaseObject<
		HashableBaseObject<_ToStringType>,
		BaseObject<_ToStringType>,
		_ToStringType>
{
public: // Static member:

	using ToStringType = _ToStringType;
	using Self = DictImpl<_KeyType, _ValType, _ContainerType, _ToStringType>;
	using Base = DictBaseObject<
		HashableBaseObject<_ToStringType>,
		BaseObject<_ToStringType>,
		_ToStringType>;
	using BaseBase = typename Base::Base;

	using DictKey = Internal::DictKeyImpl<
		_KeyType,
		typename Base::key_type>;
	using ContainerType = _ContainerType<DictKey, _ValType>;

	static_assert(std::is_same<BaseBase, BaseObject<_ToStringType> >::value,
		"Expecting Base::Base to be BaseObject class");

	typedef _KeyType                               key_type;
	typedef _ValType                               mapped_type;
	typedef std::pair<key_type, mapped_type>       value_type;
	typedef typename ContainerType::value_type     wrapped_value_type;
	typedef FrIterator<wrapped_value_type, false>  iterator;
	typedef FrIterator<wrapped_value_type, true>   const_iterator;

	typedef typename Base::key_type                  base_key_type;
	typedef typename Base::mapped_type               base_mapped_type;
	typedef typename Base::key_iterator              base_key_iterator;
	typedef typename Base::mapped_iterator           base_mapped_iterator;
	typedef typename Base::const_mapped_iterator     base_const_mapped_iterator;

	using _CtnConstIterator = typename ContainerType::const_iterator;
	using _CtnIterator = typename ContainerType::iterator;
	using _KeyIteratorWrap = CppStdFwIteratorWrap<
			_CtnConstIterator,
			base_key_type,
			true,
			Internal::ItTransformDictKey >;
	using _ValKIteratorWrap = CppStdFwIteratorWrap<
			_CtnConstIterator,
			base_mapped_type,
			true,
			Internal::ItTransformTupleGet<1> >;
	using _ValIteratorWrap = CppStdFwIteratorWrap<
			_CtnIterator,
			base_mapped_type,
			false,
			Internal::ItTransformTupleGet<1> >;

	static constexpr ObjCategory sk_cat()
	{
		return ObjCategory::Dict;
	}

public:

	DictImpl() :
		m_data()
	{}

	DictImpl(std::initializer_list<value_type> l) :
		m_data()
	{
		for (auto it = l.begin(); it != l.end(); ++it)
		{
			m_data.insert(std::make_pair(DictKey::Make(it->first), it->second));
		}
	}

	DictImpl(const Self& other) :
		m_data(other.m_data)
	{}

	DictImpl(Self&& other) :
		m_data(std::forward<ContainerType>(other.m_data))
	{}

	virtual ~DictImpl() = default;

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

	virtual bool operator==(const Self& rhs) const
	{
		return m_data == rhs.m_data;
	}
#ifndef __cpp_lib_three_way_comparison
	virtual bool operator!=(const Self& rhs) const
	{
		return !(m_data == rhs.m_data);
	}
#endif

	bool operator<(const Self& rhs) = delete;
	bool operator>(const Self& rhs) = delete;
	bool operator<=(const Self& rhs) = delete;
	bool operator>=(const Self& rhs) = delete;

	// ===== DictBase class

	virtual bool DictBaseIsEqual(const Base& rhs) const override
	{
		// reference: https://github.com/llvm/llvm-project/blob/main/libcxx/include/unordered_map#L1877

		if (m_data.size() != rhs.size())
		{
			return false;
		}
		auto xi = m_data.cbegin();
		auto xe = m_data.cend();
		auto ye = rhs.ValsCEnd();
		for (; xi != xe; ++xi)
		{
			auto yj = rhs.FindVal(xi->first.GetVal());
			if (yj == ye || !(xi->second == *yj))
			{
				return false;
			}
		}
		return true;
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

	// ========== Functions provided by this class ==========

	// ========== iterators ==========

	iterator begin()
	{
		return ToFrIt<false>(m_data.begin());
	}

	iterator end()
	{
		return ToFrIt<false>(m_data.end());
	}

	const_iterator cbegin() const
	{
		return ToFrIt<true>(m_data.cbegin());
	}

	const_iterator cend() const
	{
		return ToFrIt<true>(m_data.cend());
	}

	const_iterator begin() const
	{
		return cbegin();
	}

	const_iterator end() const
	{
		return cend();
	}

	// ========== value access ==========

	mapped_type& operator[](const key_type& key)
	{
		auto wrappedKey = DictKey::Borrow(&key);
		auto it = m_data.find(wrappedKey);
		if (it == m_data.end())
		{
			// Key is not present, we need to copy the key
			return m_data[DictKey::Make(key)];
		}
		else
		{
			// key is present, we can just utilize the existing pair
			return it->second;
		}
	}

	const mapped_type& operator[](const key_type& key) const
	{
		try
		{
			auto wrappedKey = DictKey::Borrow(&key);
			return m_data.at(wrappedKey);
		}
		catch (const std::out_of_range&)
		{
			throw KeyError(key.ShortDebugString(), KeyError::sk_keyName);
		}
	}

	// ========== item searching ==========

	const_iterator find(const key_type& key) const
	{
		auto wrappedKey = DictKey::Borrow(&key);
		return ToFrIt<true>(m_data.find(wrappedKey));
	}

	iterator find(const key_type& key)
	{
		auto wrappedKey = DictKey::Borrow(&key);
		return ToFrIt<false>(m_data.find(wrappedKey));
	}

	bool HasKey(const key_type& key) const
	{
		auto wrappedKey = DictKey::Borrow(&key);
		return m_data.find(wrappedKey) != m_data.cend();
	}

	// ========== adding/removing values ==========

	std::pair<iterator, bool> InsertOnly(
		const key_type& key, const mapped_type& other)
	{
		auto wrappedKey = DictKey::Borrow(&key);
		auto it = m_data.find(wrappedKey);
		if (it == m_data.end())
		{
			// insert
			it = m_data.emplace(DictKey::Make(key), other).first;
			return {ToFrIt<false>(it), true};
		}
		return {ToFrIt<false>(it), false};
	}

	std::pair<iterator, bool> InsertOnly(
		key_type&& key, mapped_type&& other)
	{
		auto wrappedKey = DictKey::Borrow(&key);
		auto it = m_data.find(wrappedKey);
		if (it == m_data.end())
		{
			// insert
			it = m_data.emplace(
				DictKey::Make(std::forward<key_type>(key)),
				std::forward<mapped_type>(other)).first;
			return {ToFrIt<false>(it), true};
		}
		return {ToFrIt<false>(it), false};
	}

	std::pair<iterator, bool> InsertOrAssign(
		const key_type& key, const mapped_type& other)
	{
		auto wrappedKey = DictKey::Borrow(&key);
		auto it = m_data.find(wrappedKey);
		if (it == m_data.end())
		{
			// insert
			it = m_data.emplace(DictKey::Make(key), other).first;
			return {ToFrIt<false>(it), true};
		}
		else
		{
			// assign
			(*it).second = other;
			return {ToFrIt<false>(it), false};
		}
	}

	std::pair<iterator, bool> InsertOrAssign(
		key_type&& key, mapped_type&& other)
	{
		auto wrappedKey = DictKey::Borrow(&key);
		auto it = m_data.find(wrappedKey);
		if (it == m_data.end())
		{
			// insert
			it = m_data.emplace(
				DictKey::Make(std::forward<key_type>(key)),
				std::forward<mapped_type>(other)).first;
			return {ToFrIt<false>(it), true};
		}
		else
		{
			// assign
			(*it).second = std::forward<mapped_type>(other);
			return {ToFrIt<false>(it), false};
		}
	}

	void Remove(const key_type& key)
	{
		auto wrappedKey = DictKey::Borrow(&key);
		m_data.erase(wrappedKey);
	}

	// ========== Overrides BaseObject ==========

	virtual ObjCategory GetCategory() const override
	{
		return sk_cat();
	}

	using BaseBase::Set;

	virtual void Set(const BaseBase& other) override
	{
		try
		{
			const Self& casted = dynamic_cast<const Self&>(other);
			*this = casted;
		}
		catch(const std::bad_cast&)
		{
			throw TypeError("Dict", this->GetCategoryName());
		}
	}

	virtual void Set(BaseBase&& other) override
	{
		try
		{
			Self&& casted = dynamic_cast<Self&&>(other);
			*this = std::forward<Self>(casted);
		}
		catch(const std::bad_cast&)
		{
			throw TypeError("Dict", this->GetCategoryName());
		}
	}

	virtual bool IsTrue() const override
	{
		return m_data.size() > 0;
	}

	// ========== Overrides DictBaseObject ==========

	// ========== capacity ==========

	virtual size_t size() const override
	{
		return m_data.size();
	}

	// ========== removing values ==========

	virtual void clear() override
	{
		m_data.clear();
	}

	// ========== iterators ==========

	virtual base_key_iterator KeysBegin() const override
	{
		return base_key_iterator(_KeyIteratorWrap::Build(m_data.cbegin()));
	}

	virtual base_key_iterator KeysEnd() const override
	{
		return base_key_iterator(_KeyIteratorWrap::Build(m_data.cend()));
	}

	virtual base_const_mapped_iterator ValsCBegin() const override
	{
		return base_const_mapped_iterator(
			_ValKIteratorWrap::Build(m_data.cbegin()));
	}

	virtual base_const_mapped_iterator ValsCEnd() const override
	{
		return base_const_mapped_iterator(
			_ValKIteratorWrap::Build(m_data.cend()));
	}

	virtual base_mapped_iterator ValsBegin() override
	{
		return base_mapped_iterator(_ValIteratorWrap::Build(m_data.begin()));
	}

	virtual base_mapped_iterator ValsEnd() override
	{
		return base_mapped_iterator(_ValIteratorWrap::Build(m_data.end()));
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
		std::string res;
		res += '{';
		res += ' ';
		size_t i = 0;
		for (const auto& item : m_data)
		{
			res += item.first.GetVal().DebugString();
			res += " : ";
			res += item.second.DebugString();
			if (i < m_data.size() - 1)
			{
				res += ',';
				res += ' ';
			}
			++i;
		}
		res += ' ';
		res += '}';
		return res;
	}

	virtual std::string ShortDebugString() const override
	{
		std::string res;
		res += '{';
		size_t i = 0;
		for (const auto& item : m_data)
		{
			res += item.first.GetVal().ShortDebugString();
			res += ":";
			res += item.second.ShortDebugString();
			if (i < m_data.size() - 1)
			{
				res += ',';
			}
			++i;
		}
		res += '}';
		return res;
	}

	virtual ToStringType ToString() const override
	{
		auto res = Internal::ToString<ToStringType>("{ ");
		size_t i = 0;
		for (const auto& item : m_data)
		{
			res += item.first.GetVal().ToString();
			res += Internal::ToString<ToStringType>(" : ");
			res += item.second.ToString();
			if (i < m_data.size() - 1)
			{
				res += Internal::ToString<ToStringType>(", ");
			}
			++i;
		}
		res += Internal::ToString<ToStringType>(" }");
		return res;
	}

	virtual void DumpString(OutIterator<typename ToStringType::value_type> outIt) const override
	{
		*outIt++ = '{';
		*outIt++ = ' ';
		size_t i = 0;
		for (const auto& item : m_data)
		{
			item.first.GetVal().DumpString(outIt);
			*outIt++ = ' ';
			*outIt++ = ':';
			*outIt++ = ' ';
			item.second.DumpString(outIt);
			if (i < m_data.size() - 1)
			{
				*outIt++ = ',';
				*outIt++ = ' ';
			}
			++i;
		}
		*outIt++ = ' ';
		*outIt++ = '}';
	}

protected:

	// ========== Overrides DictBaseObject ==========

	virtual base_const_mapped_iterator DictBaseFindVal(
		const base_key_type& key) const override
	{
		auto wrappedKey = DictKey::Borrow(&key);
		return base_const_mapped_iterator(
			_ValKIteratorWrap::Build(m_data.find(wrappedKey)));
	}

	virtual base_mapped_iterator DictBaseFindVal(
		const base_key_type& key) override
	{
		auto wrappedKey = DictKey::Borrow(&key);
		return base_mapped_iterator(
			_ValIteratorWrap::Build(m_data.find(wrappedKey)));
	}

	virtual base_mapped_iterator DictBaseFindValOrAddDefault(
		const base_key_type& key) override
	{
		auto wrappedKey = DictKey::Borrow(&key);
		auto it = m_data.find(wrappedKey);
		if (it == m_data.end())
		{
			// Key is not present, we need to copy the key
			return base_mapped_iterator(
				_ValIteratorWrap::Build(
					m_data.emplace(
						DictKey::Make(key),
						mapped_type()
					).first
				)
			);
		}
		else
		{
			// key is present, we can just utilize the existing pair
			return base_mapped_iterator(_ValIteratorWrap::Build(it));
		}
	}

	virtual bool DictBaseInsertOnly(
		const base_key_type& key, const base_mapped_type& val) override
	{
		return InsertOnly(DynCastKey(key), DynCastVal(val)).second;
	}

	virtual bool DictBaseInsertOnly(
		base_key_type&& key, base_mapped_type&& val) override
	{
		return InsertOnly(
			DynCastKey(std::forward<base_key_type>(key)),
			DynCastVal(std::forward<base_mapped_type>(val))).second;
	}

	virtual bool DictBaseInsertOrAssign(
		const base_key_type& key, const base_mapped_type& val) override
	{
		return InsertOrAssign(DynCastKey(key), DynCastVal(val)).second;
	}

	virtual bool DictBaseInsertOrAssign(
		base_key_type&& key, base_mapped_type&& val) override
	{
		return InsertOrAssign(
			DynCastKey(std::forward<base_key_type>(key)),
			DynCastVal(std::forward<base_mapped_type>(val))).second;
	}

	virtual void DictBaseRemove(const base_key_type& key) override
	{
		auto wrappedKey = DictKey::Borrow(&key);
		m_data.erase(wrappedKey);
	}

private:

	static const key_type& DynCastKey(const base_key_type& key)
	{
		try
		{
			return dynamic_cast<const key_type&>(key);
		}
		catch(const std::bad_cast&)
		{
			throw TypeError("key type of the Dict", key.GetCategoryName());
		}
	}

	static key_type&& DynCastKey(base_key_type&& key)
	{
		try
		{
			return dynamic_cast<key_type&&>(key);
		}
		catch(const std::bad_cast&)
		{
			throw TypeError("key type of the Dict", key.GetCategoryName());
		}
	}

	static const mapped_type& DynCastVal(const base_mapped_type& val)
	{
		try
		{
			return dynamic_cast<const mapped_type&>(val);
		}
		catch(const std::bad_cast&)
		{
			throw TypeError("value type of the Dict", val.GetCategoryName());
		}
	}

	static mapped_type&& DynCastVal(base_mapped_type&& val)
	{
		try
		{
			return dynamic_cast<mapped_type&&>(val);
		}
		catch(const std::bad_cast&)
		{
			throw TypeError("value type of the Dict", val.GetCategoryName());
		}
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

}; // class DictImpl

} // namespace SimpleObjects
