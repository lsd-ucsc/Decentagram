// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <cstring>

#include "Internal/SimpleRlp.hpp"

#include "CatId.hpp"
#include "Exceptions.hpp"

#ifndef ADVANCEDRLP_CUSTOMIZED_NAMESPACE
namespace AdvancedRlp
#else
namespace ADVANCEDRLP_CUSTOMIZED_NAMESPACE
#endif
{


template<typename _Transform>
using PrimitiveParserBase = Internal::SimRlp::ListParserImpl<
	Internal::SimRlp::InputContainerType,
	Internal::SimRlp::ByteValType,
	Internal::SimRlp::ListObjType,
	_Transform,
	Internal::SimRlp::BytesParser,
	Internal::SimRlp::FailingParserList>;


template<typename _ContainerType, typename _Transform, typename _InnerParser>
using CompositeParserBase = Internal::SimRlp::ListParserImpl<
	_ContainerType,
	Internal::SimRlp::ByteValType,
	Internal::SimRlp::ListObjType,
	_Transform,
	Internal::SimRlp::BytesParser,
	_InnerParser>;


namespace Internal
{

inline void CheckRlpListSizeEq(
	const std::string& catName,
	size_t pos,
	size_t expSize,
	size_t recvSize)
{
	if (expSize != recvSize)
	{
		throw ParseError(
			catName + " requires " +
			std::to_string(expSize) +
			" items in the RLP list, while " +
			std::to_string(recvSize) +
			" items were given",
			pos
		);
	}
}

inline void CheckRlpListSizeLe(
	const std::string& catName,
	size_t pos,
	size_t expSize,
	size_t recvSize)
{
	if (expSize > recvSize) // or !(expSize <= recvSize)
	{
		throw ParseError(
			catName + " requires at least " +
			std::to_string(expSize) +
			" items in the RLP list, while " +
			std::to_string(recvSize) +
			" items were given",
			pos
		);
	}
}


template<typename _RlpListType>
inline void CheckObjCat(
	const std::string&,
	size_t,
	const _RlpListType&,
	size_t)
{
	return;
}


template<
	typename _RlpListType,
	typename _ObjCatType,
	typename... _ObjCatTypes>
inline void CheckObjCat(
	const std::string& catName,
	size_t pos,
	const _RlpListType& l,
	size_t idx,
	_ObjCatType cat0,
	_ObjCatTypes ...cats)
{
	const auto& item = l[idx];
	if (item.GetCategory() != cat0.second)
	{
		throw ParseError(
			catName +
			"'s item " +
			std::to_string(idx) +
			" should be type of " +
			cat0.first +
			", while type of " +
			item.GetCategoryName() +
			" is given",
			pos
		);
	}
	CheckObjCat(catName, pos, l, idx + 1, cats...);
}


template<
	typename _RlpListType,
	typename... _ObjCatType>
inline void CheckRlpListTypeSizeEq(
	const std::string& catName,
	size_t pos,
	const _RlpListType& l,
	_ObjCatType ...objCat)
{
	static constexpr size_t expNumItem = sizeof...(_ObjCatType);
	CheckRlpListSizeEq(catName, pos, expNumItem, l.size());

	// check object categories
	CheckObjCat(catName, pos, l, 0, objCat...);
}


template<
	typename _RlpListType,
	typename... _ObjCatType>
inline void CheckRlpListTypeSizeLe(
	const std::string& catName,
	size_t pos,
	const _RlpListType& l,
	_ObjCatType ...objCat)
{
	static constexpr size_t expNumItem = sizeof...(_ObjCatType);
	CheckRlpListSizeLe(catName, pos, expNumItem, l.size());

	// check object categories
	CheckObjCat(catName, pos, l, 0, objCat...);
}


template<
	SimRlp::Endian _InEndian,
	SimRlp::Endian _PlatformEndian = SimRlp::Endian::native>
struct RawToPrimitive; // struct RawToPrimitive


template<>
struct RawToPrimitive<
	SimRlp::Endian::little,
	SimRlp::Endian::little>
{
	template<typename _IntType>
	static _IntType ToInt(const void* src, size_t srcSize)
	{
		if (srcSize != sizeof(_IntType))
		{
			throw ParseError(
				"The given raw data size doesn't match the size of "
				"the targeting type");
		}

		_IntType retVal;
		std::memcpy(&retVal, src, sizeof(_IntType));

		return retVal;
	}
}; // struct RawToPrimitive

} // namespace Internal

} // namespace AdvancedRlp
