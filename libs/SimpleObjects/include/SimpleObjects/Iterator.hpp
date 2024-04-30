// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <memory>

#include "IteratorIf.hpp"
#include "IteratorStdCpp.hpp"
#include "IteratorZip.hpp"

#include "Internal/IteratorTransform.hpp"
#include "Internal/make_unique.hpp"

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{

//========================================
//||
//||  Wrappers for iterators interfaces pointers
//||
//========================================

/**
 * @brief The wrapper of output iterator interface, so we can support
 *        different classes of iterator implementations
 *
 * @tparam _TargetType The target type; e.g., the target type for a
 *         std::vector<uint8_t> vector will be uint8_t
 */
template<typename _TargetType>
class OutIterator
{
public: // Static members:
	using WrappedIt = OutputIteratorIf<_TargetType>;
	using WrappedItPtr = typename WrappedIt::SelfPtr;

	typedef typename WrappedIt::difference_type         difference_type;
	typedef typename WrappedIt::value_type              value_type;
	typedef typename WrappedIt::pointer                 pointer;
	typedef typename WrappedIt::const_pointer           const_pointer;
	typedef typename WrappedIt::reference               reference;
	typedef typename WrappedIt::iterator_category       iterator_category;

public:
	OutIterator() = delete;

	OutIterator(typename std::add_rvalue_reference<WrappedItPtr>::type it) :
		m_it(std::forward<WrappedItPtr>(it))
	{}

	OutIterator(const OutIterator& otherIt) :
		m_it(otherIt.m_it->Copy(*(otherIt.m_it)))
	{}

	OutIterator(OutIterator&& otherIt):
		m_it(std::forward<WrappedItPtr>(otherIt.m_it))
	{}

	virtual ~OutIterator() = default;

	OutIterator& operator=(const OutIterator& rhs)
	{
		if (this != &rhs)
		{
			m_it = rhs.m_it->Copy(*(rhs.m_it));
		}
		return *this;
	}

	OutIterator& operator=(OutIterator&& rhs)
	{
		if (this != &rhs)
		{
			m_it = std::move(rhs.m_it);
		}
		return *this;
	}

	OutIterator& operator=(const value_type& rhs)
	{
		m_it->Put(rhs);
		return *this;
	}

	OutIterator& operator=(value_type&& rhs)
	{
		m_it->Put(std::forward<value_type>(rhs));
		return *this;
	}

	OutIterator& operator*()
	{
		return *this;
	}

	OutIterator& operator++()
	{
		m_it->Increment();
		return *this;
	}

	OutIterator operator++(int)
	{
		OutIterator copy(*this);
		m_it->Increment();
		return copy;
	}

	WrappedItPtr CopyPtr() const
	{
		return m_it->Copy(*m_it);
	}

private:
	WrappedItPtr m_it;

}; // class OutIterator

/**
 * @brief The wrapper of input iterator interface, so we can support
 *        different classes of iterator implementations
 *
 * @tparam _TargetType The target type; e.g., the target type for a
 *         std::vector<uint8_t> vector will be uint8_t
 */
template<typename _TargetType>
class InIterator
{
public: // Static members:
	using WrappedIt = InputIteratorIf<_TargetType, true>;
	using WrappedItPtr = typename WrappedIt::SelfPtr;

	typedef typename WrappedIt::difference_type         difference_type;
	typedef typename WrappedIt::value_type              value_type;
	typedef typename WrappedIt::pointer                 pointer;
	typedef typename WrappedIt::const_pointer           const_pointer;
	typedef typename WrappedIt::reference               reference;
	typedef typename WrappedIt::iterator_category       iterator_category;

public:
	InIterator() = delete;

	InIterator(typename std::add_rvalue_reference<WrappedItPtr>::type it) :
		m_it(std::forward<WrappedItPtr>(it))
	{}

	InIterator(const InIterator& otherIt) :
		m_it(otherIt.m_it->Copy(*(otherIt.m_it)))
	{}

	InIterator(InIterator&& otherIt):
		m_it(std::forward<WrappedItPtr>(otherIt.m_it))
	{}

	virtual ~InIterator() = default;

	InIterator& operator=(const InIterator& rhs)
	{
		if (this != &rhs)
		{
			m_it = rhs.m_it->Copy(*(rhs.m_it));
		}
		return *this;
	}

	InIterator& operator=(InIterator&& rhs)
	{
		if (this != &rhs)
		{
			m_it = std::move(rhs.m_it);
		}
		return *this;
	}

	reference operator*() const
	{
		return m_it->GetRef();
	}

	pointer operator->() const
	{
		return m_it->GetPtr();
	}

	InIterator& operator++()
	{
		m_it->Increment();
		return *this;
	}

	// NOTE: we cannot compare iterator vs const_iterator for now, because we
	// are using Downcast in the CppStdIterator implementation
	bool operator==(const InIterator& rhs) const
	{
		return m_it->IsEqual(*rhs.m_it);
	}

	bool operator!=(const InIterator& rhs) const
	{
		return !(*this == rhs);
	}

	WrappedItPtr CopyPtr() const
	{
		return m_it->Copy(*m_it);
	}

private:
	WrappedItPtr m_it;

}; // class InIterator

/**
 * @brief The wrapper of forward iterator interface, so we can support
 *        different classes of iterator implementations
 *
 * @tparam _TargetType The target type; e.g., the target type for a
 *         std::vector<uint8_t> vector will be uint8_t
 * @tparam _IsConst Is this a const iterator
 */
template<typename _TargetType, bool _IsConst>
class FrIterator
{
public: // Static members:
	using WrappedIt = ForwardIteratorIf<_TargetType, _IsConst>;
	using WrappedItPtr = typename WrappedIt::SelfPtr;

	typedef typename WrappedIt::difference_type         difference_type;
	typedef typename WrappedIt::value_type              value_type;
	typedef typename WrappedIt::pointer                 pointer;
	typedef typename WrappedIt::const_pointer           const_pointer;
	typedef typename WrappedIt::reference               reference;
	typedef typename WrappedIt::iterator_category       iterator_category;

public:
	FrIterator() = delete;

	FrIterator(typename std::add_rvalue_reference<WrappedItPtr>::type it) :
		m_it(std::forward<WrappedItPtr>(it))
	{}

	FrIterator(const FrIterator& otherIt) :
		m_it(otherIt.m_it->Copy(*(otherIt.m_it)))
	{}

	FrIterator(FrIterator&& otherIt):
		m_it(std::forward<WrappedItPtr>(otherIt.m_it))
	{}

	virtual ~FrIterator() = default;

	FrIterator& operator=(const FrIterator& rhs)
	{
		if (this != &rhs)
		{
			m_it = rhs.m_it->Copy(*(rhs.m_it));
		}
		return *this;
	}

	FrIterator& operator=(FrIterator&& rhs)
	{
		if (this != &rhs)
		{
			m_it = std::move(rhs.m_it);
		}
		return *this;
	}

	reference operator*() const
	{
		return m_it->GetRef();
	}

	pointer operator->() const
	{
		return m_it->GetPtr();
	}

	FrIterator& operator++()
	{
		m_it->Increment();
		return *this;
	}

	FrIterator operator++(int)
	{
		FrIterator copy(*this);
		m_it->Increment();
		return copy;
	}

	// NOTE: we cannot compare iterator vs const_iterator for now, because we
	// are using Downcast in the CppStdIterator implementation
	bool operator==(const FrIterator& rhs) const
	{
		return m_it->IsEqual(*rhs.m_it);
	}

	bool operator!=(const FrIterator& rhs) const
	{
		return !(*this == rhs);
	}

	WrappedItPtr CopyPtr() const
	{
		return m_it->Copy(*m_it);
	}

private:
	WrappedItPtr m_it;

}; // class FrIterator

/**
 * @brief The wrapper of bidirectional iterator interface, so we can support
 *        different classes of iterator implementations
 *
 * @tparam _TargetType The target type; e.g., the target type for a
 *         std::vector<uint8_t> vector will be uint8_t
 * @tparam _IsConst Is this a const iterator
 */
template<typename _TargetType, bool _IsConst>
class BiIterator
{
public: // Static members:
	using WrappedIt = BidirectionalIteratorIf<_TargetType, _IsConst>;
	using WrappedItPtr = typename WrappedIt::SelfPtr;

	typedef typename WrappedIt::difference_type         difference_type;
	typedef typename WrappedIt::value_type              value_type;
	typedef typename WrappedIt::pointer                 pointer;
	typedef typename WrappedIt::const_pointer           const_pointer;
	typedef typename WrappedIt::reference               reference;
	typedef typename WrappedIt::iterator_category       iterator_category;

public:
	BiIterator() = delete;

	BiIterator(typename std::add_rvalue_reference<WrappedItPtr>::type it) :
		m_it(std::forward<WrappedItPtr>(it))
	{}

	BiIterator(const BiIterator& otherIt) :
		m_it(otherIt.m_it->Copy(*(otherIt.m_it)))
	{}

	BiIterator(BiIterator&& otherIt):
		m_it(std::forward<WrappedItPtr>(otherIt.m_it))
	{}

	virtual ~BiIterator() = default;

	BiIterator& operator=(const BiIterator& rhs)
	{
		if (this != &rhs)
		{
			m_it = rhs.m_it->Copy(*(rhs.m_it));
		}
		return *this;
	}

	BiIterator& operator=(BiIterator&& rhs)
	{
		if (this != &rhs)
		{
			m_it = std::move(rhs.m_it);
		}
		return *this;
	}

	reference operator*() const
	{
		return m_it->GetRef();
	}

	pointer operator->() const
	{
		return m_it->GetPtr();
	}

	BiIterator& operator++()
	{
		m_it->Increment();
		return *this;
	}

	BiIterator operator++(int)
	{
		BiIterator copy(*this);
		m_it->Increment();
		return copy;
	}

	BiIterator& operator--()
	{
		m_it->Decrement();
		return *this;
	}

	BiIterator operator--(int)
	{
		BiIterator copy(*this);
		m_it->Decrement();
		return copy;
	}

	bool operator==(const BiIterator& rhs) const
	{
		return m_it->IsEqual(*rhs.m_it);
	}

	bool operator!=(const BiIterator& rhs) const
	{
		return !(*this == rhs);
	}

	WrappedItPtr CopyPtr() const
	{
		return m_it->Copy(*m_it);
	}

private:
	WrappedItPtr m_it;

}; // class BiIterator

/**
 * @brief The wrapper of random access iterator interface, so we can support
 *        different classes of iterator implementations
 *
 * @tparam _TargetType The target type; e.g., the target type for a
 *         std::vector<uint8_t> vector will be uint8_t
 * @tparam _IsConst Is this a const iterator
 */
template<typename _TargetType, bool _IsConst>
class RdIterator
{
public: // Static members:
	using Self = RdIterator<_TargetType, _IsConst>;

	using WrappedIt = RandomAccessIteratorIf<_TargetType, _IsConst>;
	using WrappedItPtr = typename WrappedIt::SelfPtr;

	typedef typename WrappedIt::difference_type         difference_type;
	typedef typename WrappedIt::value_type              value_type;
	typedef typename WrappedIt::pointer                 pointer;
	typedef typename WrappedIt::const_pointer           const_pointer;
	typedef typename WrappedIt::reference               reference;
	typedef typename WrappedIt::iterator_category       iterator_category;

	friend class RdIterator<_TargetType, !_IsConst>;

public:
	RdIterator() = delete;

	RdIterator(typename std::add_rvalue_reference<WrappedItPtr>::type it) :
		m_it(std::forward<WrappedItPtr>(it))
	{}

	RdIterator(const RdIterator& otherIt) :
		m_it(otherIt.m_it->Copy(*(otherIt.m_it)))
	{}

	RdIterator(RdIterator&& otherIt):
		m_it(std::forward<WrappedItPtr>(otherIt.m_it))
	{}

	virtual ~RdIterator() = default;

	RdIterator& operator=(const RdIterator& rhs)
	{
		if (this != &rhs)
		{
			m_it = rhs.m_it->Copy(*(rhs.m_it));
		}
		return *this;
	}

	RdIterator& operator=(RdIterator&& rhs)
	{
		if (this != &rhs)
		{
			m_it = std::move(rhs.m_it);
		}
		return *this;
	}

	reference operator*() const
	{
		return m_it->GetRef();
	}

	reference operator[](difference_type idx) const
	{
		return m_it->GetPtr()[idx];
	}

	pointer operator->() const
	{
		return m_it->GetPtr();
	}

	RdIterator operator+(difference_type count) const
	{
		RdIterator copy(*this);
		copy += count;
		return copy;
	}

	RdIterator operator-(difference_type count) const
	{
		RdIterator copy(*this);
		copy -= count;
		return copy;
	}

	RdIterator& operator+=(difference_type count)
	{
		m_it->Offset(count);
		return *this;
	}

	RdIterator& operator++()
	{
		m_it->Increment();
		return *this;
	}

	RdIterator operator++(int)
	{
		RdIterator copy(*this);
		m_it->Increment();
		return copy;
	}

	RdIterator& operator-=(difference_type count)
	{
		m_it->Offset(-count);
		return *this;
	}

	RdIterator& operator--()
	{
		m_it->Decrement();
		return *this;
	}

	RdIterator operator--(int)
	{
		RdIterator copy(*this);
		m_it->Decrement();
		return copy;
	}

	bool operator==(const RdIterator& rhs) const
	{
		return m_it->IsEqual(*rhs.m_it);
	}

	bool operator!=(const RdIterator& rhs) const
	{
		return !(*this == rhs);
	}

	bool operator>(const Self& rhs) const
	{
		// ret_val <  0 -> this < other
		// ret_val == 0 -> this == other
		// ret_val >  0 -> this > other
		return m_it->Diff(*(rhs.m_it)) > 0;
	}

	bool operator<=(const Self& rhs) const
	{
		return !(*this > rhs);
	}

	bool operator<(const Self& rhs) const
	{
		// ret_val <  0 -> this < other
		// ret_val == 0 -> this == other
		// ret_val >  0 -> this > other
		return m_it->Diff(*(rhs.m_it)) < 0;
	}

	bool operator>=(const Self& rhs) const
	{
		return !(*this < rhs);
	}

	difference_type operator-(const Self& rhs) const
	{
		return m_it->Diff(*(rhs.m_it));
	}

	WrappedItPtr CopyPtr() const
	{
		return m_it->Copy(*m_it);
	}

private:
	WrappedItPtr m_it;

}; // class RdIterator

template<typename _ValType,
	typename _OriItType>
inline OutIterator<_ValType> ToOutIt(_OriItType it)
{
	using ItWrap = CppStdOutIteratorWrap<_OriItType, _ValType>;
	return OutIterator<_ValType>(ItWrap::Build(it));
}

template<typename _OriItType,
	typename _ValType = typename std::iterator_traits<_OriItType>::value_type>
inline InIterator<_ValType> ToInIt(_OriItType it)
{
	using ItWrap = CppStdInIteratorWrap<
		_OriItType, _ValType, true, Internal::ItTransformDirect>;
	return InIterator<_ValType>(ItWrap::Build(it));
}

template<bool _IsConst,
	typename _OriItType,
	typename _ValType = typename std::iterator_traits<_OriItType>::value_type>
inline FrIterator<_ValType, _IsConst> ToFrIt(_OriItType it)
{
	using ItWrap = CppStdFwIteratorWrap<
		_OriItType, _ValType, _IsConst, Internal::ItTransformDirect>;
	return FrIterator<_ValType, _IsConst>(ItWrap::Build(it));
}

template<bool _IsConst,
	typename _OriItType,
	typename _ValType = typename std::iterator_traits<_OriItType>::value_type>
inline BiIterator<_ValType, _IsConst> ToBiIt(_OriItType it)
{
	using ItWrap = CppStdBiIteratorWrap<
		_OriItType, _ValType, _IsConst, Internal::ItTransformDirect>;
	return BiIterator<_ValType, _IsConst>(ItWrap::Build(it));
}

template<bool _IsConst,
	typename _OriItType,
	typename _ValType = typename std::iterator_traits<_OriItType>::value_type>
inline RdIterator<_ValType, _IsConst> ToRdIt(_OriItType it)
{
	using ItWrap = CppStdRdIteratorWrap<
		_OriItType, _ValType, _IsConst, Internal::ItTransformDirect>;
	return RdIterator<_ValType, _IsConst>(ItWrap::Build(it));
}

template<bool _IsConst, typename ..._ItTypes>
inline FrIterator<std::tuple<_ItTypes...>, _IsConst> FwItZip(_ItTypes&&... its)
{
	using ValType = std::tuple<
		typename std::remove_reference<_ItTypes>::type ...>;
	using ItWrap = FwItZipper<_IsConst,
		typename std::remove_reference<_ItTypes>::type ...>;

	auto ptr = Internal::make_unique<ItWrap>(std::forward<_ItTypes>(its)...);

	return FrIterator<ValType, _IsConst>(std::move(ptr));
}

} // namespace SimpleObjects
