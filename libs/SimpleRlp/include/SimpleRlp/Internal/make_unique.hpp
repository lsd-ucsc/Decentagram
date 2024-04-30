#pragma once
// -*- C++ -*-
//===-------------------------- memory ------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//


// Additional notes for SimpleRlp:
// C++11 does not support make_unique, thus, we need to include it manually.
// The definition of make_unique is from LLVM.
// To avoid conflicting the definition in std namespace,
// we encapsulated it in our own namespace.

#include <memory>

#ifndef SIMPLERLP_CUSTOMIZED_NAMESPACE
namespace SimpleRlp
#else
namespace SIMPLERLP_CUSTOMIZED_NAMESPACE
#endif
{
namespace Internal
{

template<class _Tp>
struct __unique_if
{
	typedef std::unique_ptr<_Tp> __unique_single;
};

template<class _Tp>
struct __unique_if<_Tp[]>
{
	typedef std::unique_ptr<_Tp[]> __unique_array_unknown_bound;
};

template<class _Tp, size_t _Np>
struct __unique_if<_Tp[_Np]>
{
	typedef void __unique_array_known_bound;
};

template<class _Tp, class... _Args>
inline typename __unique_if<_Tp>::__unique_single
	make_unique(_Args&&... __args)
{
	return std::unique_ptr<_Tp>(new _Tp(std::forward<_Args>(__args)...));
}

template<class _Tp>
inline typename __unique_if<_Tp>::__unique_array_unknown_bound
	make_unique(size_t __n)
{
	typedef typename std::remove_extent<_Tp>::type _Up;
	return std::unique_ptr<_Tp>(new _Up[__n]());
}

template<class _Tp, class... _Args>
typename __unique_if<_Tp>::__unique_array_known_bound
	make_unique(_Args&&...) = delete;

} // namespace Internal
} // namespace SimpleRlp
