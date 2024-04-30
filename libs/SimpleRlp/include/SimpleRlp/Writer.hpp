// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <SimpleObjects/BasicDefs.hpp>

#include "RlpEncoding.hpp"

#ifndef SIMPLERLP_CUSTOMIZED_NAMESPACE
namespace SimpleRlp
#else
namespace SIMPLERLP_CUSTOMIZED_NAMESPACE
#endif
{

template<typename _OutCtnType>
struct OutContainerConcat
{
	void operator()(_OutCtnType& dest, const _OutCtnType& src)
	{
		dest.insert(dest.end(), src.begin(), src.end());
	}
}; // struct OutContainerConcat

template<typename _OutCtnType>
struct WriterBytesImpl
{
	using Concatenator = OutContainerConcat<_OutCtnType>;

	template<typename _BytesObjType>
	inline static _OutCtnType Write(const _BytesObjType& inBytes)
	{
		using _OutCtnValType = typename _OutCtnType::value_type;
		_OutCtnType outBytes;

		const size_t inSize = inBytes.size();

		auto begin = inBytes.data();
		auto end = begin + inSize;

		outBytes.reserve(inSize);
		for (auto it = begin; it != end; ++it)
		{
			outBytes.push_back(static_cast<_OutCtnValType>(*it));
		}

		return SerializeBytes<RlpEncTypeCat::Bytes>(outBytes, Concatenator());
	}

	template<typename _BytesObjType>
	inline static size_t CalcSize(const _BytesObjType& inBytes)
	{
		return SerializedSize<RlpEncTypeCat::Bytes>::Calc(
			inBytes.size(), inBytes.data()
		);
	}

}; // struct WriterBytesImpl

template<typename _OutCtnType, typename _GenericWriter>
struct WriterListImpl
{
	using Self = WriterListImpl<_OutCtnType, _GenericWriter>;

	using GenericWriter = _GenericWriter;
	using Concatenator = OutContainerConcat<_OutCtnType>;

	template<typename _ListObjType>
	inline static _OutCtnType Write(const _ListObjType& inList)
	{
		_OutCtnType outBytes;

		for (const auto& item : inList)
		{
			auto subBytes = GenericWriter::Write(item);
			Concatenator()(outBytes, subBytes);
		}

		return SerializeBytes<RlpEncTypeCat::List>(outBytes, Concatenator());
	}

	template<typename _ListObjType>
	inline static size_t CalcSize(const _ListObjType& inList)
	{
		using _OutValType = typename _OutCtnType::value_type;

		size_t innerSize = 0;
		for (const auto& item : inList)
		{
			innerSize += GenericWriter::CalcSize(item);
		}
		return SerializedSize<RlpEncTypeCat::List>::Calc<_OutValType>(innerSize);
	}

}; // struct WriterListImpl

template<typename _OutCtnType, typename _GenericWriter>
struct WriterStaticDictImpl
{
	using Self = WriterStaticDictImpl<_OutCtnType, _GenericWriter>;

	using GenericWriter = _GenericWriter;
	using Concatenator = OutContainerConcat<_OutCtnType>;

	template<typename _StaticDictObjType>
	inline static _OutCtnType Write(
		const _StaticDictObjType& inDict,
		size_t skipLast = 0
	)
	{
		_OutCtnType outBytes;

		size_t itemLeft = inDict.size();
		for (const auto& item : inDict)
		{
			if (itemLeft <= skipLast)
			{
				break;
			}
			auto subBytes = GenericWriter::Write(item.second.get());
			Concatenator()(outBytes, subBytes);
			--itemLeft;
		}

		return SerializeBytes<RlpEncTypeCat::List>(outBytes, Concatenator());
	}

	template<typename _StaticDictObjType>
	inline static size_t CalcSize(
		const _StaticDictObjType& inDict,
		size_t skipLast = 0
	)
	{
		using _OutValType = typename _OutCtnType::value_type;

		size_t innerSize = 0;
		size_t itemLeft = inDict.size();
		for (const auto& item : inDict)
		{
			if (itemLeft <= skipLast)
			{
				break;
			}
			innerSize += GenericWriter::CalcSize(item.second.get());
			--itemLeft;
		}
		return SerializedSize<RlpEncTypeCat::List>::Calc<_OutValType>(innerSize);
	}

}; // struct WriterStaticDictImpl

template<
	typename _OutCtnType,
	template<typename> class _BytesWriterT,
	template<typename, typename> class _ListWriterT,
	template<typename, typename> class _StaticDictWriterT>
struct WriterGenericImpl
{
	using Self = WriterGenericImpl<
		_OutCtnType,
		_BytesWriterT,
		_ListWriterT,
		_StaticDictWriterT>;

	using BytesWriter      = _BytesWriterT<_OutCtnType>;
	using ListWriter       = _ListWriterT<_OutCtnType, Self>;
	using StaticDictWriter = _StaticDictWriterT<_OutCtnType, Self>;

	template<typename _GenericObjType>
	inline static _OutCtnType Write(const _GenericObjType& obj)
	{
		switch (obj.GetCategory())
		{
		case Internal::Obj::ObjCategory::Bytes:
			return BytesWriter::Write(obj.AsBytes());

		case Internal::Obj::ObjCategory::List:
			return ListWriter::Write(obj.AsList());

		case Internal::Obj::ObjCategory::StaticDict:
			return StaticDictWriter::Write(obj.AsStaticDict());

		default:
			throw SerializeTypeError(obj.GetCategoryName());
		}
	}

	template<typename _GenericObjType>
	inline static size_t CalcSize(const _GenericObjType& obj)
	{
		switch (obj.GetCategory())
		{
		case Internal::Obj::ObjCategory::Bytes:
			return BytesWriter::CalcSize(obj.AsBytes());

		case Internal::Obj::ObjCategory::List:
			return ListWriter::CalcSize(obj.AsList());

		case Internal::Obj::ObjCategory::StaticDict:
			return StaticDictWriter::CalcSize(obj.AsStaticDict());

		default:
			throw SerializeTypeError(obj.GetCategoryName());
		}
	}

}; // struct WriterGenericImpl

} // namespace SimpleRlp
