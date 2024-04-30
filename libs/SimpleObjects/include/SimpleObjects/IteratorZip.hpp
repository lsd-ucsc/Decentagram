// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <tuple>

#include "IteratorIf.hpp"
#include "Utils.hpp"

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{

namespace Internal
{

struct TupleItemIncrement
{
	template<typename _ItemType>
	void operator()(size_t, _ItemType& i)
	{
		++i;
	}
}; // struct TupleItemIncrement

struct IsIteratorTupleEqual
{
	IsIteratorTupleEqual() :
		m_isEqual(false)
	{}

	/**
	 * @brief The result will be true as long as there is at least one
	 *        called pair is equal
	 *
	 */
	template<typename _ItemType1, typename _ItemType2>
	void operator()(size_t, const _ItemType1& a, const _ItemType2& b)
	{
		m_isEqual = m_isEqual || (a == b);
	}

	bool m_isEqual;
}; // struct IsIteratorTupleEqual

} // namespace Internal

/**
 * @brief An iterator zipper (that provides forward iterator capabilities) for
 *        forward iterators
 *
 * @tparam _Iterators The list of types of the given iterators
 */
template<bool _IsConst, typename ..._Iterators>
class FwItZipper :
	public ForwardIteratorIf<std::tuple<_Iterators...>, _IsConst>
{
public: // Static members:

	using TupleType = std::tuple<_Iterators...>;

	using Self = FwItZipper<_IsConst, _Iterators...>;

	using BaseInIf = InputIteratorIf<TupleType, _IsConst>;
	using BaseInIfPtr = std::unique_ptr<BaseInIf>;

	using BaseIf = ForwardIteratorIf<TupleType, _IsConst>;
	using BaseIfPtr = std::unique_ptr<BaseIf>;

	typedef typename BaseIf::difference_type         difference_type;
	typedef typename BaseIf::value_type              value_type;
	typedef typename BaseIf::pointer                 pointer;
	typedef typename BaseIf::const_pointer           const_pointer;
	typedef typename BaseIf::reference               reference;
	typedef typename BaseIf::iterator_category       iterator_category;

public:

	template<typename... T>
	explicit FwItZipper(T&&... args) :
		m_its(std::forward<T>(args)...)
	{}

	FwItZipper(const Self& other):
		m_its(other.m_its)
	{}

	FwItZipper(Self&& other):
		m_its(std::forward<TupleType>(other.m_its))
	{}

	virtual void Increment() override
	{
		Internal::TupleOperation::UnaOp(m_its, Internal::TupleItemIncrement());
	}

	virtual reference GetRef() override
	{
		return m_its;
	}

	virtual pointer GetPtr() const override
	{
		return &m_its;
	}

	virtual bool IsEqual(const BaseInIf& rhs) const override
	{
		const auto& otherWrap = Internal::DownCast<Self>(rhs);

		Internal::IsIteratorTupleEqual res;
		Internal::TupleOperation::BinOp(m_its, otherWrap.m_its, res);

		return res.m_isEqual;
	}

	virtual BaseInIfPtr Copy(const BaseInIf&) const override
	{ return CopyImpl(); }
	virtual BaseIfPtr Copy(const BaseIf&) const override
	{ return CopyImpl(); }

private:

	BaseIfPtr CopyImpl() const
	{
		return Internal::make_unique<FwItZipper>(*this);
	}

	mutable TupleType m_its;

}; // class FwItZipper

} // namespace SimpleObjects
