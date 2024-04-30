// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <iterator>
#include <type_traits>

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{

//========================================
//||
//||          Interfaces
//||
//========================================

/**
 * @brief The interface of a basic iterator
 *
 * @tparam _TargetType The target type; e.g., the target type for a
 *         std::vector<uint8_t> vector will be uint8_t
 */
template<typename _TargetType>
class OutputIteratorIf
{
public: // Static members:

	/**
	 * @brief The target type but with const, volatile, and reference specifiers
	 *        removed.
	 *
	 */
	using RawTargetType = typename std::remove_cv<
		typename std::remove_reference<_TargetType>::type>::type;

	using Self = OutputIteratorIf<_TargetType>;

	using SelfPtr = std::unique_ptr<Self>;

	// some standard typedefs

	typedef std::ptrdiff_t                                            difference_type;
	typedef RawTargetType                                             value_type;
	typedef typename std::add_pointer<RawTargetType>::type            pointer;
	typedef typename std::add_pointer<
		typename std::add_const<RawTargetType>::type>::type           const_pointer;
	typedef typename std::add_lvalue_reference<RawTargetType>::type   reference;
	typedef std::output_iterator_tag                                  iterator_category;

public:
	OutputIteratorIf() = default;

	virtual ~OutputIteratorIf() = default;

	virtual void Increment() = 0;

	virtual void Put(const value_type&) = 0;

	virtual void Put(value_type&&) = 0;

	virtual SelfPtr Copy(const Self& /*unused*/) const = 0;

}; // class OutputIteratorIf

/**
 * @brief The interface of a input iterator
 *
 * @tparam _TargetType The target type; e.g., the target type for a
 *         std::vector<uint8_t> vector will be uint8_t
 * @tparam _IsConst Is this a const iterator
 */
template<typename _TargetType, bool _IsConst>
class InputIteratorIf
{
public: // Static members:

	/**
	 * @brief The target type but with const, volatile, and reference specifiers
	 *        removed.
	 *
	 */
	using RawTargetType = typename std::remove_cv<
		typename std::remove_reference<_TargetType>::type>::type;

	static constexpr bool sk_isConst = _IsConst;

	/**
	 * @brief The actual target type. Based on _IsConst, it is either the raw
	 *        target type with const specifier (when _IsConst == true),
	 *        or the raw target type (when _IsConst == false).
	 *
	 */
	using ActTargetType = typename std::conditional<sk_isConst,
		typename std::add_const<RawTargetType>::type,
		RawTargetType>::type;

	using Self = InputIteratorIf<_TargetType, _IsConst>;

	using SelfPtr = std::unique_ptr<Self>;

	// some standard typedefs

	typedef std::ptrdiff_t                                    difference_type;
	typedef RawTargetType                                     value_type;
	typedef typename std::add_pointer<ActTargetType>::type    pointer;
	typedef typename std::add_pointer<
		typename std::add_const<RawTargetType>::type>::type   const_pointer;
	typedef typename std::add_lvalue_reference<
		ActTargetType>::type                                  reference;
	typedef std::input_iterator_tag                           iterator_category;

public:
	InputIteratorIf() = default;

	virtual ~InputIteratorIf() = default;

	virtual void Increment() = 0;

	virtual reference GetRef() = 0;

	virtual pointer GetPtr() const = 0;

	virtual bool IsEqual(const Self& rhs) const = 0;

	virtual SelfPtr Copy(const Self& /*unused*/) const = 0;

}; //class InputIteratorIf

/**
 * @brief The interface of a basic iterator
 *
 * @tparam _TargetType The target type; e.g., the target type for a
 *         std::vector<uint8_t> vector will be uint8_t
 * @tparam _IsConst Is this a const iterator
 */
template<typename _TargetType, bool _IsConst>
class ForwardIteratorIf : public InputIteratorIf<_TargetType, _IsConst>
{
public: // Static members:

	using _Base = InputIteratorIf<_TargetType, _IsConst>;

	using Self = ForwardIteratorIf<_TargetType, _IsConst>;

	using SelfPtr = std::unique_ptr<Self>;

	typedef typename _Base::difference_type         difference_type;
	typedef typename _Base::value_type              value_type;
	typedef typename _Base::pointer                 pointer;
	typedef typename _Base::const_pointer           const_pointer;
	typedef typename _Base::reference               reference;

public:
	ForwardIteratorIf() = default;

	virtual ~ForwardIteratorIf() = default;

	using _Base::Copy;
	virtual SelfPtr Copy(const Self& /*unused*/) const = 0;

}; //class ForwardIteratorIf

template<typename _TargetType, bool _IsConst>
class BidirectionalIteratorIf : public ForwardIteratorIf<_TargetType, _IsConst>
{
public: // Static members:

	using _Base = ForwardIteratorIf<_TargetType, _IsConst>;

	using Self = BidirectionalIteratorIf<_TargetType, _IsConst>;

	using SelfPtr = std::unique_ptr<Self>;

	typedef typename _Base::difference_type         difference_type;
	typedef typename _Base::value_type              value_type;
	typedef typename _Base::pointer                 pointer;
	typedef typename _Base::const_pointer           const_pointer;
	typedef typename _Base::reference               reference;
	typedef std::bidirectional_iterator_tag         iterator_category;

public:
	BidirectionalIteratorIf() = default;

	virtual ~BidirectionalIteratorIf() = default;

	virtual void Decrement() = 0;

	using _Base::Copy;
	virtual SelfPtr Copy(const Self& /*unused*/) const = 0;

}; //class BidirectionalIteratorIf

template<typename _TargetType, bool _IsConst>
class RandomAccessIteratorIf : public BidirectionalIteratorIf<_TargetType, _IsConst>
{
public: // Static members:

	using _Base = BidirectionalIteratorIf<_TargetType, _IsConst>;

	using Self = RandomAccessIteratorIf<_TargetType, _IsConst>;

	using SelfPtr = std::unique_ptr<Self>;

	typedef typename _Base::difference_type         difference_type;
	typedef typename _Base::value_type              value_type;
	typedef typename _Base::pointer                 pointer;
	typedef typename _Base::const_pointer           const_pointer;
	typedef typename _Base::reference               reference;
	typedef std::random_access_iterator_tag         iterator_category;

public:
	RandomAccessIteratorIf() = default;

	virtual ~RandomAccessIteratorIf() = default;

	virtual void Offset(difference_type offset) = 0;

	virtual difference_type Diff(const Self& other) const = 0;

	using _Base::Copy;
	virtual SelfPtr Copy(const Self& /*unused*/) const = 0;

}; //class RandomAccessIteratorIf

} // namespace SimpleObjects
