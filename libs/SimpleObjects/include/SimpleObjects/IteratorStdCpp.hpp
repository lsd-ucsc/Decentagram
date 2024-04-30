// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <memory>
#include <type_traits>

#include "Internal/make_unique.hpp"

#include "IteratorIf.hpp"
#include "Utils.hpp"

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{

//========================================
//||
//||  Wrappers for C++ standard iterators
//||
//========================================

template<typename _OriItType, typename _TargetType>
class CppStdOutIteratorWrap : public OutputIteratorIf<_TargetType>
{
public: // Static members:
	using _BaseIf = OutputIteratorIf<_TargetType>;
	using _BaseIfPtr = std::unique_ptr<_BaseIf>;

	typedef typename _BaseIf::difference_type         difference_type;
	typedef typename _BaseIf::value_type              value_type;
	typedef typename _BaseIf::pointer                 pointer;
	typedef typename _BaseIf::const_pointer           const_pointer;
	typedef typename _BaseIf::reference               reference;
	typedef typename _BaseIf::iterator_category       iterator_category;

	static _BaseIfPtr Build(_OriItType oriIt)
	{
		return Internal::make_unique<CppStdOutIteratorWrap>(oriIt);
	}

	static_assert(
		std::is_base_of<
			iterator_category,
			typename std::iterator_traits<_OriItType>::iterator_category>::value,
		"The given C++ standard iterator must be a category based on output iterator");

public:

	CppStdOutIteratorWrap(_OriItType oriIt) :
		_BaseIf::OutputIteratorIf(),
		m_it(oriIt)
	{}

	CppStdOutIteratorWrap(const CppStdOutIteratorWrap& other) :
		_BaseIf::OutputIteratorIf(other),
		m_it(other.m_it)
	{}

	CppStdOutIteratorWrap(CppStdOutIteratorWrap&& other) :
		_BaseIf::OutputIteratorIf(std::forward<_BaseIf>(other)),
		m_it(std::forward<_OriItType>(other.m_it))
	{}

	virtual ~CppStdOutIteratorWrap() = default;

	virtual void Increment() override
	{
		++m_it;
	}

	virtual void Put(const value_type& val) override
	{
		*m_it = val;
	}

	virtual void Put(value_type&& val) override
	{
		*m_it = std::forward<value_type>(val);
	}

	virtual _BaseIfPtr Copy(const _BaseIf&) const override
	{
		return Internal::make_unique<CppStdOutIteratorWrap>(*this);
	}

public:

	_OriItType m_it;
}; // class CppStdOutIteratorWrap

template<
	typename _OriItType,
	typename _TargetType,
	bool _IsConst,
	typename _Transform>
class CppStdInIteratorWrap : public InputIteratorIf<_TargetType, _IsConst>
{
public: // Static members:
	using _BaseIf = InputIteratorIf<_TargetType, _IsConst>;
	using _BaseIfPtr = std::unique_ptr<_BaseIf>;

	using Self = CppStdInIteratorWrap<
		_OriItType, _TargetType, _IsConst, _Transform>;

	typedef typename _BaseIf::difference_type         difference_type;
	typedef typename _BaseIf::value_type              value_type;
	typedef typename _BaseIf::pointer                 pointer;
	typedef typename _BaseIf::const_pointer           const_pointer;
	typedef typename _BaseIf::reference               reference;
	typedef typename _BaseIf::iterator_category       iterator_category;

	static _BaseIfPtr Build(_OriItType oriIt)
	{
		return Internal::make_unique<CppStdInIteratorWrap>(oriIt);
	}

	static_assert(
		std::is_base_of<
			iterator_category,
			typename std::iterator_traits<_OriItType>::iterator_category>::value,
		"The given C++ standard iterator must be a category based on input iterator");

public:

	CppStdInIteratorWrap(_OriItType oriIt) :
		_BaseIf::InputIteratorIf(),
		m_it(oriIt)
	{}

	CppStdInIteratorWrap(const CppStdInIteratorWrap& other) :
		_BaseIf::InputIteratorIf(),
		m_it(other.m_it)
	{}

	CppStdInIteratorWrap(CppStdInIteratorWrap&& other) :
		_BaseIf::InputIteratorIf(),
		m_it(std::forward<_OriItType>(other.m_it))
	{}

	virtual ~CppStdInIteratorWrap() = default;

	virtual void Increment() override
	{
		++m_it;
	}

	virtual reference GetRef() override
	{
		return _Transform::template GetRef<reference>(m_it);
	}

	virtual pointer GetPtr() const override
	{
		return _Transform::template GetPtr<pointer>(m_it);
	}

	virtual bool IsEqual(const _BaseIf& rhs) const override
	{
		const auto& otherWrap = Internal::DownCast<Self>(rhs);
		return (m_it == otherWrap.m_it);
	}

	virtual _BaseIfPtr Copy(const _BaseIf&) const override
	{
		return Internal::make_unique<CppStdInIteratorWrap>(*this);
	}

public:

	_OriItType m_it;

}; // class CppStdInIteratorWrap


template<
	typename _OriItType,
	typename _TargetType,
	bool _IsConst,
	typename _Transform>
class CppStdFwIteratorWrap :
	public ForwardIteratorIf<_TargetType, _IsConst>,
	public CppStdInIteratorWrap<_OriItType, _TargetType, _IsConst, _Transform>
{
public: // Static members:

	using Self = CppStdFwIteratorWrap<
		_OriItType, _TargetType, _IsConst, _Transform>;

	using _BaseInIf = InputIteratorIf<_TargetType, _IsConst>;
	using _BaseInIfPtr = std::unique_ptr<_BaseInIf>;

	using _BaseIf = ForwardIteratorIf<_TargetType, _IsConst>;
	using _BaseIfPtr = std::unique_ptr<_BaseIf>;

	using _Base = CppStdInIteratorWrap<
		_OriItType, _TargetType, _IsConst, _Transform>;

	typedef typename _BaseIf::difference_type         difference_type;
	typedef typename _BaseIf::value_type              value_type;
	typedef typename _BaseIf::pointer                 pointer;
	typedef typename _BaseIf::const_pointer           const_pointer;
	typedef typename _BaseIf::reference               reference;
	typedef typename _BaseIf::iterator_category       iterator_category;

	static _BaseIfPtr Build(_OriItType oriIt)
	{
		return Internal::make_unique<CppStdFwIteratorWrap>(oriIt);
	}

	static_assert(
		std::is_base_of<
			iterator_category,
			typename std::iterator_traits<_OriItType>::iterator_category>::value,
		"The given C++ standard iterator must be a category based on forward iterator");

public:

	CppStdFwIteratorWrap(_OriItType oriIt) :
		_BaseIf::ForwardIteratorIf(),
		_Base::CppStdInIteratorWrap(oriIt)
	{}

	CppStdFwIteratorWrap(const CppStdFwIteratorWrap& other) :
		_BaseIf::ForwardIteratorIf(),
		_Base::CppStdInIteratorWrap(other)
	{}

	CppStdFwIteratorWrap(CppStdFwIteratorWrap&& other) :
		_BaseIf::ForwardIteratorIf(),
		_Base::CppStdInIteratorWrap(std::forward<_Base>(other))
	{}

	virtual ~CppStdFwIteratorWrap() = default;

	virtual void Increment() override
	{ return _Base::Increment(); }

	virtual reference GetRef() override
	{ return _Base::GetRef(); }

	virtual pointer GetPtr() const override
	{ return _Base::GetPtr(); }

	virtual bool IsEqual(const _BaseInIf& rhs) const override
	{ return _Base::IsEqual(rhs); }

	virtual _BaseInIfPtr Copy(const _BaseInIf&) const override
	{ return CopyImpl(); }
	virtual _BaseIfPtr Copy(const _BaseIf&) const override
	{ return CopyImpl(); }

private:

	_BaseIfPtr CopyImpl() const
	{
		return Internal::make_unique<CppStdFwIteratorWrap>(*this);
	}

}; // class CppStdFwIteratorWrap


template<
	typename _OriItType,
	typename _TargetType,
	bool _IsConst,
	typename _Transform>
class CppStdBiIteratorWrap :
	public BidirectionalIteratorIf<_TargetType, _IsConst>,
	public CppStdFwIteratorWrap<_OriItType, _TargetType, _IsConst, _Transform>
{
public: // Static members:

	using _BaseInIf = InputIteratorIf<_TargetType, _IsConst>;
	using _BaseInIfPtr = std::unique_ptr<_BaseInIf>;

	using _BaseFwIf = ForwardIteratorIf<_TargetType, _IsConst>;
	using _BaseFwIfPtr = std::unique_ptr<_BaseFwIf>;

	using _BaseIf = BidirectionalIteratorIf<_TargetType, _IsConst>;
	using _BaseIfPtr = std::unique_ptr<_BaseIf>;

	using _Base = CppStdFwIteratorWrap<
		_OriItType, _TargetType, _IsConst, _Transform>;

	typedef typename _BaseIf::difference_type         difference_type;
	typedef typename _BaseIf::value_type              value_type;
	typedef typename _BaseIf::pointer                 pointer;
	typedef typename _BaseIf::const_pointer           const_pointer;
	typedef typename _BaseIf::reference               reference;
	typedef typename _BaseIf::iterator_category       iterator_category;

	static _BaseIfPtr Build(_OriItType oriIt)
	{
		return Internal::make_unique<CppStdBiIteratorWrap>(oriIt);
	}

	static_assert(
		std::is_base_of<
			iterator_category,
			typename std::iterator_traits<_OriItType>::iterator_category>::value,
		"The given C++ standard iterator must be a category based on bidirectional iterator");

public:

	CppStdBiIteratorWrap(_OriItType oriIt) :
		_BaseIf::BidirectionalIteratorIf(),
		_Base::CppStdFwIteratorWrap(oriIt)
	{}

	CppStdBiIteratorWrap(const CppStdBiIteratorWrap& other) :
		_BaseIf::BidirectionalIteratorIf(),
		_Base::CppStdFwIteratorWrap(other)
	{}

	CppStdBiIteratorWrap(CppStdBiIteratorWrap&& other) :
		_BaseIf::BidirectionalIteratorIf(),
		_Base::CppStdFwIteratorWrap(std::forward<_Base>(other))
	{}

	virtual ~CppStdBiIteratorWrap() = default;

	virtual void Increment() override { return _Base::Increment(); }
	virtual reference GetRef() override { return _Base::GetRef(); }
	virtual pointer GetPtr() const override { return _Base::GetPtr(); }
	virtual bool IsEqual(const _BaseInIf& rhs) const override
	{ return _Base::IsEqual(rhs); }

	virtual _BaseInIfPtr Copy(const _BaseInIf&) const override
	{ return CopyImpl(); }
	virtual _BaseFwIfPtr Copy(const _BaseFwIf&) const override
	{ return CopyImpl(); }
	virtual _BaseIfPtr Copy(const _BaseIf&) const override
	{ return CopyImpl(); }

	virtual void Decrement() override
	{
		--(_Base::m_it);
	}

private:

	_BaseIfPtr CopyImpl() const
	{
		return Internal::make_unique<CppStdBiIteratorWrap>(*this);
	}

}; // class CppStdBiIteratorWrap

template<
	typename _OriItType,
	typename _TargetType,
	bool _IsConst,
	typename _Transform>
class CppStdRdIteratorWrap :
	public RandomAccessIteratorIf<_TargetType, _IsConst>,
	public CppStdBiIteratorWrap<_OriItType, _TargetType, _IsConst, _Transform>
{
public: // Static members:
	using Self = CppStdRdIteratorWrap<
		_OriItType, _TargetType, _IsConst, _Transform>;

	using _BaseInIf = InputIteratorIf<_TargetType, _IsConst>;
	using _BaseInIfPtr = std::unique_ptr<_BaseInIf>;

	using _BaseFwIf = ForwardIteratorIf<_TargetType, _IsConst>;
	using _BaseFwIfPtr = std::unique_ptr<_BaseFwIf>;

	using _BaseBiIf = BidirectionalIteratorIf<_TargetType, _IsConst>;
	using _BaseBiIfPtr = std::unique_ptr<_BaseBiIf>;

	using _BaseIf = RandomAccessIteratorIf<_TargetType, _IsConst>;
	using _BaseIfPtr = std::unique_ptr<_BaseIf>;

	using _Base = CppStdBiIteratorWrap<
		_OriItType, _TargetType, _IsConst, _Transform>;

	typedef typename _BaseIf::difference_type         difference_type;
	typedef typename _BaseIf::value_type              value_type;
	typedef typename _BaseIf::pointer                 pointer;
	typedef typename _BaseIf::const_pointer           const_pointer;
	typedef typename _BaseIf::reference               reference;
	typedef typename _BaseIf::iterator_category       iterator_category;

	static _BaseIfPtr Build(_OriItType oriIt)
	{
		return Internal::make_unique<CppStdRdIteratorWrap>(oriIt);
	}

	static_assert(
		std::is_base_of<
			iterator_category,
			typename std::iterator_traits<_OriItType>::iterator_category>::value,
		"The given C++ standard iterator must be a category based on random access iterator");

public:

	CppStdRdIteratorWrap(_OriItType oriIt) :
		_BaseIf::RandomAccessIteratorIf(),
		_Base::CppStdBiIteratorWrap(oriIt)
	{}

	CppStdRdIteratorWrap(const CppStdRdIteratorWrap& other) :
		_BaseIf::RandomAccessIteratorIf(),
		_Base::CppStdBiIteratorWrap(other)
	{}

	CppStdRdIteratorWrap(CppStdRdIteratorWrap&& other) :
		_BaseIf::RandomAccessIteratorIf(),
		_Base::CppStdBiIteratorWrap(std::forward<_BaseIf>(other))
	{}

	virtual ~CppStdRdIteratorWrap() = default;

	virtual void Increment() override { return _Base::Increment(); }
	virtual void Decrement() override { return _Base::Decrement(); }
	virtual reference GetRef() override { return _Base::GetRef(); }
	virtual pointer GetPtr() const override { return _Base::GetPtr(); }
	virtual bool IsEqual(const _BaseInIf& rhs) const override
	{ return _Base::IsEqual(rhs); }

	virtual _BaseInIfPtr Copy(const _BaseInIf&) const override
	{ return CopyImpl(); }
	virtual _BaseFwIfPtr Copy(const _BaseFwIf&) const override
	{ return CopyImpl(); }
	virtual _BaseBiIfPtr Copy(const _BaseBiIf&) const override
	{ return CopyImpl(); }
	virtual _BaseIfPtr Copy(const _BaseIf&) const override
	{ return CopyImpl(); }

	virtual void Offset(difference_type offset) override
	{
		_Base::m_it += offset;
	}

	/**
	 * @brief NOTE: We assume here we are comparing two iterator with the same
	 *              child type, since in most cases, it does not make sense to
	 *              compare two different iterator types.
	 *
	 * @param other
	 * @return difference_type
	 */
	virtual difference_type Diff(const _BaseIf& other) const override
	{
		const auto& otherWrap = Internal::DownCast<Self>(other);
		return _Base::m_it - otherWrap.m_it;
	}


private:
	_BaseIfPtr CopyImpl() const
	{
		return Internal::make_unique<CppStdRdIteratorWrap>(*this);
	}

}; // class CppStdRdIteratorWrap

} // namespace SimpleObjects
