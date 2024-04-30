// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <AdvancedRlp/AdvancedRlp.hpp>
#include <SimpleObjects/SimpleObjects.hpp>

#include "Internal/SimpleObj.hpp"
#include "Internal/SimpleRlp.hpp"


namespace DecentEnclave
{
namespace Common
{

namespace Internal
{

using AuthListTupleCore = std::tuple<
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("SVN")>,
		Internal::Obj::UInt32
	>,
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("HashToName")>,
		Internal::Obj::Dict
	>
>;


using AuthListParserTp = std::tuple<
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("SVN")>,
		Internal::AdvRlp::CatIntegerParserT<
			Internal::AdvRlp::SpecificIntConverter<uint32_t>
		>
	>,
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("HashToName")>,
		Internal::AdvRlp::CatDictParser
	>
>;


} // namespace Internal


inline constexpr uint32_t sk_authListSVN()
{
	return 1;
}


/**
 * @brief Static dictionary for AuthList encoding and decoding
 *
 */
class AuthList :
	public Internal::Obj::StaticDict<Internal::AuthListTupleCore>
{
public: // static members:

	using Self = AuthList;
	using Base = Internal::Obj::StaticDict<Internal::AuthListTupleCore>;

	template<typename _StrSeq>
	using _StrKey = Internal::Obj::StrKey<_StrSeq>;
	template<typename _StrSeq>
	using _RetRefType = typename Base::template GetRef<_StrKey<_StrSeq> >;
	template<typename _StrSeq>
	using _RetKRefType = typename Base::template GetConstRef<_StrKey<_StrSeq> >;

public:

	using Base::Base;

	/**
	 * @brief The secure version number of the AuthList structure
	 *
	 */
	_RetRefType<SIMOBJ_KSTR("SVN")> get_SVN()
	{
		return Base::template get<_StrKey<SIMOBJ_KSTR("SVN")> >();
	}

	_RetKRefType<SIMOBJ_KSTR("SVN")> get_SVN() const
	{
		return Base::template get<_StrKey<SIMOBJ_KSTR("SVN")> >();
	}

	/**
	 * @brief The hash to name mapping
	 *
	 */
	_RetRefType<SIMOBJ_KSTR("HashToName")> get_HashToName()
	{
		return Base::template get<_StrKey<SIMOBJ_KSTR("HashToName")> >();
	}

	_RetKRefType<SIMOBJ_KSTR("HashToName")> get_HashToName() const
	{
		return Base::template get<_StrKey<SIMOBJ_KSTR("HashToName")> >();
	}

}; // class AuthList


using AuthListParser = Internal::AdvRlp::CatStaticDictParserT<
	Internal::AuthListParserTp,
	false,
	false,
	AuthList
>;


} // namespace Common
} // namespace DecentEnclave
