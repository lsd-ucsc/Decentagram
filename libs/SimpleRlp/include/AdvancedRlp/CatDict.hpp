// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <algorithm>

#include "ParserUtils.hpp"

#ifndef ADVANCEDRLP_CUSTOMIZED_NAMESPACE
namespace AdvancedRlp
#else
namespace ADVANCEDRLP_CUSTOMIZED_NAMESPACE
#endif
{


// ====================
// Parsers
// ====================


template<typename _ListObjType>
inline void PreCheckCatDict(size_t pos, const _ListObjType& l)
{
	using namespace Internal::SimRlp::Internal::Obj;
	Internal::CheckRlpListTypeSizeLe("CAT Dict", pos, l,
		std::pair<std::string, ObjCategory>("Bytes", ObjCategory::Bytes)
	);

	const auto& specs = l[0].AsBytes();
	if (specs.size() != 1)
	{
		throw ParseError(
			"CAT Dict's specs bytes should be exactly 1 byte",
			pos);
	}
	if (GetCatIdFromByte(specs[0]) != CatId::Dict)
	{
		throw ParseError(
			"The given RLP list is not in CAT Dict",
			pos);
	}
}


namespace Internal
{

template<bool _Validate>
struct CatDictInsertRest;

template<>
struct CatDictInsertRest<true>
{
	template<typename _DictObjType, typename _ListObjType>
	void operator()(_DictObjType& d, _ListObjType&& l, size_t pos)
	{
		using DictKeyType = typename _DictObjType::key_type;
		using HashableBase =
			typename Internal::SimRlp::Internal::Obj::BaseObj::HashableBase;

		for (size_t i = 3; i < l.size(); i += 2)
		{
			const auto& prevKey = l[i - 2];
			const auto& keyObj = l[i];
			DictKeyType keyHashable = keyObj.AsHashable().Copy(HashableBase::sk_null);
			auto& val = l[i + 1];

			// 1. check key existence
			if (d.HasKey(keyHashable))
			{
				throw ParseError("The given dictionary has duplicated key of " +
					keyHashable.DebugString(), pos);
			}

			// 2. check key order
			if (keyObj < prevKey)
			{
				throw ParseError("The key " + keyObj.DebugString() +
					" in given dictionary is out of order", pos);
			}

			d.InsertOnly(std::move(keyHashable), std::move(val));
		}
	}
}; // struct CatDictInsertRest<true>

} // namespace Internal


template<
	bool _Validate,
	typename _ListObjType,
	typename _DictObjType>
struct TransformCatDictImpl
{
	using RetType        = _DictObjType;
	using DictObjType   = _DictObjType;

	static constexpr bool sk_validate = _Validate;


	RetType operator()(size_t pos, _ListObjType&& l)
	{
		using DictKeyType = typename DictObjType::key_type;
		using HashableBase =
			typename Internal::SimRlp::Internal::Obj::BaseObj::HashableBase;

		PreCheckCatDict(pos, l);

		DictObjType res;

		// check num of items is even
		size_t numOfItems = l.size() - 1;
		if (numOfItems % 2 != 0)
		{
			throw ParseError("The number of given items cannot form "
				"key-value pairs for a dictionary",
				pos);
		}

		// insert first pair
		if (l.size() > 2)
		{
			DictKeyType key = l[1].AsHashable().Copy(HashableBase::sk_null);
			auto& val = l[2];
			res.InsertOnly(std::move(key), std::move(val));
		}

		// insert the rest
		Internal::CatDictInsertRest<sk_validate>()(
			res, std::forward<_ListObjType>(l), pos);

		return res;
	}
}; // struct TransformCatString


using TransformCatDict = TransformCatDictImpl<
	true,
	Internal::SimRlp::ListObjType,
	Internal::SimRlp::Internal::Obj::Dict>;


// ====================
// Writers
// ====================


template<
	typename _InDictObjType,
	typename _GenericWriter,
	typename _OutCtnType>
struct CatDictWriterImpl
{
	using Self = CatArrayWriterImpl<
		_InDictObjType,
		_GenericWriter,
		_OutCtnType>;

	using GenericWriter = _GenericWriter;

	using Concatenator = Internal::SimRlp::OutContainerConcat<_OutCtnType>;


	inline static _OutCtnType Write(const _InDictObjType& val)
	{
		Concatenator ccntr;
		_OutCtnType outBytes;

		// 1.specs
		auto catId = SerializeCatId(CatId::Dict);
		outBytes.push_back(catId);

		// 2.items
		auto itEnd = val.end();
		using DictItType = decltype(itEnd);
		// put iterators into a vector
		std::vector<DictItType> itVec;
		for (auto it = val.begin(); it != itEnd; ++it)
		{
			itVec.push_back(it);
		}
		// sort by key
		std::sort(itVec.begin(), itVec.end(),
			[](const DictItType& a, const DictItType& b)
			{
				return *std::get<0>(*a) < *std::get<0>(*b);
			});
		// generate bytes for items
		for (auto it = itVec.begin(); it != itVec.end(); ++it)
		{
			ccntr(outBytes, GenericWriter::Write(*std::get<0>(**it)));
			ccntr(outBytes, GenericWriter::Write(*std::get<1>(**it)));
		}

		// 3.build RLP list
		return Internal::SimRlp::
			SerializeBytes<Internal::SimRlp::RlpEncTypeCat::List>(
				outBytes,
				ccntr
			);
	}

}; // struct CatDictWriterImpl


template<typename _GenericWriter>
using CatDictWriterT =
	CatDictWriterImpl<
		Internal::SimRlp::Internal::Obj::DictBaseObj,
		_GenericWriter,
		Internal::SimRlp::OutputContainerType>;


} // namespace AdvancedRlp
