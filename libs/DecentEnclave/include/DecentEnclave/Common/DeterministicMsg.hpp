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

using DetMsgIdTupleCore = std::tuple<
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("MsgType")>,
		Internal::Obj::String
	>,
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("Ext")>,
		Internal::Obj::Bytes
	>
>;

using DetMsgIdParserTp = std::tuple<
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("MsgType")>,
		Internal::AdvRlp::CatStringParser
	>,
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("Ext")>,
		Internal::AdvRlp::CatBytesParser
	>
>;

} // namespace Internal


class DetMsgId :
	public Internal::Obj::StaticDict<Internal::DetMsgIdTupleCore>
{
public: // static members:

	using Self = DetMsgId;
	using Base = Internal::Obj::StaticDict<Internal::DetMsgIdTupleCore>;

	template<typename _StrSeq>
	using _StrKey = Internal::Obj::StrKey<_StrSeq>;
	template<typename _StrSeq>
	using _RetRefType = typename Base::template GetRef<_StrKey<_StrSeq> >;
	template<typename _StrSeq>
	using _RetKRefType = typename Base::template GetConstRef<_StrKey<_StrSeq> >;

public:

	using Base::Base;

	_RetRefType<SIMOBJ_KSTR("MsgType")> get_MsgType()
	{
		return Base::template get<_StrKey<SIMOBJ_KSTR("MsgType")> >();
	}

	_RetKRefType<SIMOBJ_KSTR("MsgType")> get_MsgType() const
	{
		return Base::template get<_StrKey<SIMOBJ_KSTR("MsgType")> >();
	}

	_RetRefType<SIMOBJ_KSTR("Ext")> get_Ext()
	{
		return Base::template get<_StrKey<SIMOBJ_KSTR("Ext")> >();
	}

	_RetKRefType<SIMOBJ_KSTR("Ext")> get_Ext() const
	{
		return Base::template get<_StrKey<SIMOBJ_KSTR("Ext")> >();
	}

}; // class DetMsgId


using DetMsgIdParser = Internal::AdvRlp::CatStaticDictParserT<
	Internal::DetMsgIdParserTp,
	false,
	false,
	DetMsgId
>;


namespace Internal
{

using DetMsgTupleCore = std::tuple<
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("Version")>,
		Internal::Obj::UInt32
	>,
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("MsgId")>,
		DetMsgId
	>,
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("MsgContent")>,
		Internal::Obj::Bytes
	>
>;

using DetMsgParserTp = std::tuple<
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("Version")>,
		Internal::AdvRlp::CatIntegerParserT<
			Internal::AdvRlp::SpecificIntConverter<uint32_t>
		>
	>,
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("MsgId")>,
		DetMsgIdParser
	>,
	std::pair<
		Internal::Obj::StrKey<SIMOBJ_KSTR("MsgContent")>,
		Internal::AdvRlp::CatBytesParser
	>
>;

} // namespace Internal


class DetMsg :
	public Internal::Obj::StaticDict<Internal::DetMsgTupleCore>
{
public: // static members:

	using Self = DetMsg;
	using Base = Internal::Obj::StaticDict<Internal::DetMsgTupleCore>;

	template<typename _StrSeq>
	using _StrKey = Internal::Obj::StrKey<_StrSeq>;
	template<typename _StrSeq>
	using _RetRefType = typename Base::template GetRef<_StrKey<_StrSeq> >;
	template<typename _StrSeq>
	using _RetKRefType = typename Base::template GetConstRef<_StrKey<_StrSeq> >;

public:

	using Base::Base;

	_RetRefType<SIMOBJ_KSTR("Version")> get_Version()
	{
		return Base::template get<_StrKey<SIMOBJ_KSTR("Version")> >();
	}

	_RetKRefType<SIMOBJ_KSTR("Version")> get_Version() const
	{
		return Base::template get<_StrKey<SIMOBJ_KSTR("Version")> >();
	}

	_RetRefType<SIMOBJ_KSTR("MsgId")> get_MsgId()
	{
		return Base::template get<_StrKey<SIMOBJ_KSTR("MsgId")> >();
	}

	_RetKRefType<SIMOBJ_KSTR("MsgId")> get_MsgId() const
	{
		return Base::template get<_StrKey<SIMOBJ_KSTR("MsgId")> >();
	}

	_RetRefType<SIMOBJ_KSTR("MsgContent")> get_MsgContent()
	{
		return Base::template get<_StrKey<SIMOBJ_KSTR("MsgContent")> >();
	}

	_RetKRefType<SIMOBJ_KSTR("MsgContent")> get_MsgContent() const
	{
		return Base::template get<_StrKey<SIMOBJ_KSTR("MsgContent")> >();
	}

}; // class DetMsg


using DetMsgParser = Internal::AdvRlp::CatStaticDictParserT<
	Internal::DetMsgParserTp,
	false,
	false,
	DetMsg
>;


} // namespace Common
} // namespace DecentEnclave
