// Copyright (c) 2022 EclipseMonitor, EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <tuple>
#include <type_traits>
#include <vector>

#include <SimpleObjects/BasicDefs.hpp>
#include <SimpleRlp/RlpDecoding.hpp>

#include "../Internal/SimpleObj.hpp"
#include "../Internal/SimpleRlp.hpp"
#include "../Exceptions.hpp"


namespace EclipseMonitor
{
namespace Eth
{


// ==========
// Constant values
// ==========


struct AbiParserConst
{
	static constexpr size_t sk_chunkSize() noexcept
	{
		return 32;
	}
}; // struct AbiParserConst


// ==========
// Utilities
// ==========


namespace EthInternal
{

template<
	Internal::Obj::RealNumType _RealNumType,
	typename _PrimitiveType
>
struct RealNumTypeTraitsPrimitiveImpl
{
	using Primitive = _PrimitiveType;

	static constexpr size_t sk_consumedSize() noexcept
	{
		return sizeof(Primitive);
	}
}; // struct RealNumTypeTraitsPrimitiveImpl


template<typename _T>
inline constexpr _T AbiCeilingDiv(_T a, _T b) noexcept
{
	return (a + (b - 1)) / b;
}


template<Internal::Obj::RealNumType _RealNumType>
struct RealNumTypeTraits;


template<>
struct RealNumTypeTraits<Internal::Obj::RealNumType::UInt8> :
	public RealNumTypeTraitsPrimitiveImpl<
		Internal::Obj::RealNumType::UInt8,
		uint8_t
	>
{}; // struct RealNumTypeTraits<Internal::Obj::RealNumType::UInt8>


template<>
struct RealNumTypeTraits<Internal::Obj::RealNumType::UInt16> :
	public RealNumTypeTraitsPrimitiveImpl<
		Internal::Obj::RealNumType::UInt16,
		uint16_t
	>
{}; // struct RealNumTypeTraits<Internal::Obj::RealNumType::UInt16>


template<>
struct RealNumTypeTraits<Internal::Obj::RealNumType::UInt32> :
	public RealNumTypeTraitsPrimitiveImpl<
		Internal::Obj::RealNumType::UInt32,
		uint32_t
	>
{}; // struct RealNumTypeTraits<Internal::Obj::RealNumType::UInt32>


template<>
struct RealNumTypeTraits<Internal::Obj::RealNumType::UInt64> :
	public RealNumTypeTraitsPrimitiveImpl<
		Internal::Obj::RealNumType::UInt64,
		uint64_t
	>
{}; // struct RealNumTypeTraits<Internal::Obj::RealNumType::UInt64>


template<bool _CheckVal, typename _It>
inline _It AbiParserSkipPadding(size_t skipLen, _It begin, _It end)
{
	for (size_t i = 0; i < skipLen; ++i)
	{
		if (begin == end)
		{
			throw Exception("ABI parser - unexpected end of input");
		}
		if (_CheckVal && (*begin != 0))
		{
			throw Exception(
				"ABI parser - there are non-zero bytes in skipped bytes"
			);
		}
		++begin;
	}
	return begin;
}


template<typename _SrcIt, typename _DestIt>
inline _SrcIt AbiParserCopyBytes(
	size_t len,
	_SrcIt begin,
	_SrcIt end,
	_DestIt dest
)
{
	for (size_t i = 0; i < len; ++i)
	{
		if (begin == end)
		{
			throw Exception("ABI parser - unexpected end of input");
		}
		*(dest++) = *(begin++);
	}
	return begin;
}


template<bool _CheckVal, typename _SrcIt, typename _DestIt>
inline _SrcIt AbiParserCopyBytesThenSkip(
	size_t copyLen,
	size_t skipLen,
	_SrcIt begin,
	_SrcIt end,
	_DestIt dest
)
{
	begin = AbiParserCopyBytes(copyLen, begin, end, dest);
	return AbiParserSkipPadding<_CheckVal>(skipLen, begin, end);
}


inline constexpr size_t AbiWithinChunkSize(size_t size)
{
	return size <= AbiParserConst::sk_chunkSize() ?
		size :
		throw Exception("ABI parser - bytes type is too large");
}


} // namespace EthInternal


// ==========
// Essential parser implementations
// ==========


namespace EthInternal
{


template<
	Internal::Obj::ObjCategory _DataType,
	typename... _Args
>
struct AbiCodecImpl;


// ==========
// AbiCodecImpl for integer types
// ==========


template<Internal::Obj::RealNumType _RealNumType>
struct AbiCodecImpl<
	Internal::Obj::ObjCategory::Integer,
	std::integral_constant<Internal::Obj::RealNumType, _RealNumType>
>
{
	using RealNumTraits = RealNumTypeTraits<_RealNumType>;
	static constexpr size_t sk_consumedSize = RealNumTraits::sk_consumedSize();
	static_assert(
		sk_consumedSize <= AbiParserConst::sk_chunkSize(),
		"ABI parser - integer type is too large"
	);
	static constexpr size_t sk_skipLeadSize =
		AbiParserConst::sk_chunkSize() - sk_consumedSize;
	using Primitive = typename RealNumTraits::Primitive;


	AbiCodecImpl() = default;
	~AbiCodecImpl() = default;


	template<typename _ItType>
	std::tuple<
		Primitive, /* Parsed value */
		_ItType,   /* Iterator of where the parsing stopped */
		size_t     /* Number of chunks consumed */
	>
	ToPrimitive(_ItType begin, _ItType end) const
	{
		// Skip the leading zero bytes that are larger than the target type
		begin = AbiParserSkipPadding<true>(
			sk_skipLeadSize,
			begin,
			end
		);

		// Parse the integer to target type
		auto inFunc = [&begin, &end]()
		{
			return begin != end ?
				*(begin++) :
				throw Exception("ABI parser - unexpected end of input");
		};
		Primitive res = Internal::Rlp::ParsePrimitiveIntValue<
			Primitive,
			Internal::Rlp::Endian::native
		>::Parse(sk_consumedSize, inFunc);

		return std::make_tuple(res, begin, 1);
	}
}; // struct AbiCodecImpl<Internal::Obj::ObjCategory::Integer, ...>


// ==========
// AbiCodecImpl for bool types
// ==========


template<>
struct AbiCodecImpl<
	Internal::Obj::ObjCategory::Bool
>
{
	using IntParser = AbiCodecImpl<
		Internal::Obj::ObjCategory::Integer,
		std::integral_constant<
			Internal::Obj::RealNumType,
			Internal::Obj::RealNumType::UInt8
		>
	>;
	using Primitive = bool;


	AbiCodecImpl() = default;
	~AbiCodecImpl() = default;


	template<typename _ItType>
	std::tuple<
		Primitive,  /* Parsed value */
		_ItType,    /* Iterator of where the parsing stopped */
		size_t      /* Number of chunks consumed */
	>
	ToPrimitive(_ItType begin, _ItType end) const
	{
		uint8_t valInt = 0;
		size_t chunkConsumed = 0;
		std::tie(valInt, begin, chunkConsumed) =
			IntParser().ToPrimitive(begin, end);

		bool valBool =
			(valInt == 1 ? true  :
			(valInt == 0 ? false :
				throw Exception("ABI parser - invalid bool value")));

		return std::make_tuple(valBool, begin, chunkConsumed);
	}
}; // struct AbiCodecImpl<Internal::Obj::ObjCategory::Bool>


// ==========
// AbiCodecImpl for bytes<M> types
// ==========


template<>
struct AbiCodecImpl<
	Internal::Obj::ObjCategory::Bytes,
	std::false_type /* IsDynamic? - false */
>
{
	using Primitive = std::vector<uint8_t>;


	// NOTE: This constructor does not check the size value, since this is an
	// internal type and we assume the size is checked before calling this
	AbiCodecImpl(size_t size) :
		m_size(size)
	{}
	~AbiCodecImpl() = default;


	template<typename _ItType>
	std::tuple<
		Primitive, /* Parsed value */
		_ItType,       /* Iterator of where the parsing stopped */
		size_t         /* Number of chunks consumed */
	>
	ToPrimitive(_ItType begin, _ItType end) const
	{
		std::vector<uint8_t> res;
		res.reserve(m_size);

		begin = EthInternal::AbiParserCopyBytesThenSkip<true>(
			m_size,
			AbiParserConst::sk_chunkSize() - m_size,
			begin,
			end,
			std::back_inserter(res)
		);

		return std::make_tuple(res, begin, 1);
	}

private:

	size_t m_size = 0;

}; // struct AbiCodecImpl<Internal::Obj::ObjCategory::Bytes, false>


// ==========
// AbiCodecImpl for bytes types
// ==========


template<>
struct AbiCodecImpl<
	Internal::Obj::ObjCategory::Bytes,
	std::true_type /* IsDynamic? - true */
>
{
	using Primitive = std::vector<uint8_t>;
	using DynLenParser = AbiCodecImpl<
		Internal::Obj::ObjCategory::Integer,
		std::integral_constant<
			Internal::Obj::RealNumType,
			Internal::Obj::RealNumType::UInt64
		>
	>;

	AbiCodecImpl() = default;
	~AbiCodecImpl() = default;


	template<typename _ItType>
	std::tuple<
		Primitive, /* Parsed value */
		_ItType,       /* Iterator of where the parsing stopped */
		size_t         /* Number of chunks consumed */
	>
	ToPrimitive(_ItType begin, _ItType end) const
	{
		// first, parse the length of the bytes
		uint64_t len = 0;
		size_t chunkConsumed = 0;
		std::tie(len, begin, chunkConsumed) =
			DynLenParser().ToPrimitive(begin, end);

		size_t numChunk = EthInternal::AbiCeilingDiv(
			static_cast<size_t>(len),
			AbiParserConst::sk_chunkSize()
		);
		size_t paddingSize =
			(numChunk * AbiParserConst::sk_chunkSize()) - len;

		std::vector<uint8_t> res;
		res.reserve(len);
		begin = EthInternal::AbiParserCopyBytesThenSkip<true>(
			len,
			paddingSize,
			begin,
			end,
			std::back_inserter(res)
		);

		// Calc the number of chunks consumed
		chunkConsumed += numChunk;

		return std::make_tuple(res, begin, chunkConsumed);
	}

}; // struct AbiCodecImpl<Internal::Obj::ObjCategory::Bytes, true>


// ==========
// AbiCodecImpl for T[k] types, where T is static type
// ==========


template<typename _ItemParser>
struct AbiCodecImpl<
	Internal::Obj::ObjCategory::List,
	_ItemParser,
	std::false_type, /* IsLenDynamic? - false */
	std::false_type  /* IsItemDynamic? - false */
>
{
	static_assert(
		!_ItemParser::sk_hasTail,
		"ABI parser - static item must have no tail"
	);


	using ItemParser = _ItemParser;
	using ItemPrimitive = typename ItemParser::Primitive;
	using Primitive = std::vector<ItemPrimitive>;


	AbiCodecImpl(ItemParser itemParser, size_t size) :
		m_itemParser(std::move(itemParser)),
		m_size(size)
	{}
	~AbiCodecImpl() = default;


	template<typename _ItType>
	std::tuple<
		Primitive, /* Parsed value */
		_ItType,       /* Iterator of where the parsing stopped */
		size_t         /* Number of chunks consumed */
	>
	ToPrimitive(size_t size, _ItType begin, _ItType end) const
	{
		// parse through heads
		Primitive res;
		res.reserve(size);
		size_t totalChunkConsumed = 0;
		for (size_t i = 0; i < size; ++i)
		{
			ItemPrimitive item;
			size_t chunkConsumed = 0;
			std::tie(item, begin, chunkConsumed) =
				m_itemParser.HeadToPrimitive(begin, end);
			res.push_back(item);
			totalChunkConsumed += chunkConsumed;
		}

		return std::make_tuple(res, begin, totalChunkConsumed);
	}


	template<typename _ItType>
	std::tuple<
		Primitive, /* Parsed value */
		_ItType,       /* Iterator of where the parsing stopped */
		size_t         /* Number of chunks consumed */
	>
	ToPrimitive(_ItType begin, _ItType end) const
	{
		return ToPrimitive(m_size, begin, end);
	}

private:

	ItemParser m_itemParser;
	size_t m_size;

}; // struct AbiCodecImpl<Internal::Obj::ObjCategory::List, _Item, false, false>


// ==========
// AbiCodecImpl for T[k] types, where T is dynamic type
// ==========


template<typename _ItemParser>
struct AbiCodecImpl<
	Internal::Obj::ObjCategory::List,
	_ItemParser,
	std::false_type, /* IsLenDynamic? - false */
	std::true_type  /* IsItemDynamic? - true */
>
{
	static_assert(
		_ItemParser::sk_hasTail,
		"ABI parser - dynamic item must have tail"
	);


	using ItemParser = _ItemParser;
	using ItemPrimitive = typename ItemParser::Primitive;
	using Primitive = std::vector<ItemPrimitive>;


	AbiCodecImpl(ItemParser itemParser, size_t size) :
		m_itemParser(std::move(itemParser)),
		m_size(size)
	{}
	~AbiCodecImpl() = default;


	template<typename _ItType>
	std::tuple<
		Primitive, /* Parsed value */
		_ItType,       /* Iterator of where the parsing stopped */
		size_t         /* Number of chunks consumed */
	>
	ToPrimitive(size_t size, _ItType begin, _ItType end) const
	{
		using HeadPrimitive = typename ItemParser::HeadPrimitive;

		// parse through heads
		std::vector<HeadPrimitive> heads;
		heads.reserve(size);
		size_t totalChunkConsumed = 0;
		for (size_t i = 0; i < size; ++i)
		{
			HeadPrimitive head = 0;
			size_t chunkConsumed = 0;
			std::tie(head, begin, chunkConsumed) =
				m_itemParser.HeadToPrimitive(begin, end);
			heads.push_back(head);
			totalChunkConsumed += chunkConsumed;
		}

		// parse through tails
		Primitive res;
		res.reserve(size);
		for (const auto& head : heads)
		{
			// check the offset is correct
			size_t bytesConsumed =
				totalChunkConsumed * AbiParserConst::sk_chunkSize();
			if (head != bytesConsumed)
			{
				throw Exception("ABI parser - invalid offset");
			}
			ItemPrimitive item;
			size_t chunkConsumed = 0;
			std::tie(item, begin, chunkConsumed) =
				m_itemParser.TailToPrimitive(begin, end);
			res.push_back(item);
			totalChunkConsumed += chunkConsumed;
		}

		return std::make_tuple(res, begin, totalChunkConsumed);
	}


	template<typename _ItType>
	std::tuple<
		Primitive, /* Parsed value */
		_ItType,       /* Iterator of where the parsing stopped */
		size_t         /* Number of chunks consumed */
	>
	ToPrimitive(_ItType begin, _ItType end) const
	{
		return ToPrimitive(m_size, begin, end);
	}

private:

	ItemParser m_itemParser;
	size_t m_size;

}; // struct AbiCodecImpl<Internal::Obj::ObjCategory::List, _Item, false, true>


// ==========
// AbiCodecImpl for T[] types, where T is static type
// ==========


template<typename _ItemParser>
struct AbiCodecImpl<
	Internal::Obj::ObjCategory::List,
	_ItemParser,
	std::true_type,  /* IsLenDynamic? - true */
	std::false_type  /* IsItemDynamic? - false */
>
{
	static_assert(
		!_ItemParser::sk_hasTail,
		"ABI parser - static item must have no tail"
	);


	using ItemParser = _ItemParser;
	using ItemPrimitive = typename ItemParser::Primitive;
	using Primitive = std::vector<ItemPrimitive>;

	using DynLenParser = AbiCodecImpl<
		Internal::Obj::ObjCategory::Integer,
		std::integral_constant<
			Internal::Obj::RealNumType,
			Internal::Obj::RealNumType::UInt64
		>
	>;
	using DataParser = AbiCodecImpl<
		Internal::Obj::ObjCategory::List,
		ItemParser,
		std::false_type, /* IsLenDynamic? - false */
		std::false_type  /* IsItemDynamic? - false */
	>;


	AbiCodecImpl(ItemParser itemParser) :
		m_dataParser(std::move(itemParser), 0)
	{}
	~AbiCodecImpl() = default;


	template<typename _ItType>
	std::tuple<
		Primitive, /* Parsed value */
		_ItType,       /* Iterator of where the parsing stopped */
		size_t         /* Number of chunks consumed */
	>
	ToPrimitive(_ItType begin, _ItType end) const
	{
		// first, parse the length of the bytes
		size_t totalChunkConsumed = 0;

		uint64_t len = 0;
		size_t chunkConsumed = 0;
		std::tie(len, begin, chunkConsumed) =
			DynLenParser().ToPrimitive(begin, end);
		totalChunkConsumed += chunkConsumed;

		size_t lenSize = static_cast<size_t>(len);

		// then, parse the data / list items
		Primitive res;
		std::tie(res, begin, chunkConsumed) =
			m_dataParser.ToPrimitive(lenSize, begin, end);
		totalChunkConsumed += chunkConsumed;

		return std::make_tuple(res, begin, totalChunkConsumed);
	}

private:

	DataParser m_dataParser;

}; // struct AbiCodecImpl<Internal::Obj::ObjCategory::List, _Item, true, false>


// ==========
// AbiCodecImpl for T[] types, where T is dynamic type
// ==========


template<typename _ItemParser>
struct AbiCodecImpl<
	Internal::Obj::ObjCategory::List,
	_ItemParser,
	std::true_type, /* IsLenDynamic? - true */
	std::true_type  /* IsItemDynamic? - true */
>
{
	static_assert(
		_ItemParser::sk_hasTail,
		"ABI parser - dynamic item must have tail"
	);


	using ItemParser = _ItemParser;
	using ItemPrimitive = typename ItemParser::Primitive;
	using Primitive = std::vector<ItemPrimitive>;

	using DynLenParser = AbiCodecImpl<
		Internal::Obj::ObjCategory::Integer,
		std::integral_constant<
			Internal::Obj::RealNumType,
			Internal::Obj::RealNumType::UInt64
		>
	>;
	using DataParser = AbiCodecImpl<
		Internal::Obj::ObjCategory::List,
		ItemParser,
		std::false_type, /* IsLenDynamic? - false */
		std::true_type   /* IsItemDynamic? - true */
	>;

	AbiCodecImpl(ItemParser itemParser) :
		m_dataParser(std::move(itemParser), 0)
	{}
	~AbiCodecImpl() = default;


	template<typename _ItType>
	std::tuple<
		Primitive, /* Parsed value */
		_ItType,       /* Iterator of where the parsing stopped */
		size_t         /* Number of chunks consumed */
	>
	ToPrimitive(_ItType begin, _ItType end) const
	{
		// first, parse the length of the bytes
		size_t totalChunkConsumed = 0;

		uint64_t len = 0;
		size_t chunkConsumed = 0;
		std::tie(len, begin, chunkConsumed) =
			DynLenParser().ToPrimitive(begin, end);
		totalChunkConsumed += chunkConsumed;

		size_t lenSize = static_cast<size_t>(len);

		// then, parse the data / list items
		Primitive res;
		std::tie(res, begin, chunkConsumed) =
			m_dataParser.ToPrimitive(lenSize, begin, end);
		totalChunkConsumed += chunkConsumed;

		return std::make_tuple(res, begin, totalChunkConsumed);
	}

private:

	DataParser m_dataParser;

}; // struct AbiCodecImpl<Internal::Obj::ObjCategory::List, _Item, true, true>


// ==========
// Wrappers for ABI parser
// ==========


template<
	typename _CodecImpl
>
struct AbiParserHeadOnlyTypes
{
	using Codec = _CodecImpl;
	using HeadCodec = Codec;

	static constexpr bool sk_hasTail = false;
	using HeadPrimitive = typename HeadCodec::Primitive;
	using Primitive = HeadPrimitive;

	// constructors
	AbiParserHeadOnlyTypes(HeadCodec headCodec) :
		m_headCodec(std::move(headCodec))
	{}
	// destructor
	// LCOV_EXCL_START
	virtual ~AbiParserHeadOnlyTypes() = default;
	// LCOV_EXCL_STOP


	template<typename _ItType>
	std::tuple<
		HeadPrimitive, /* Parsed value */
		_ItType,       /* Iterator of where the parsing stopped */
		size_t         /* Number of chunks consumed */
	>
	HeadToPrimitive(_ItType begin, _ItType end) const
	{
		return m_headCodec.ToPrimitive(begin, end);
	}


	template<typename _ItType>
	std::tuple<
		Primitive, /* Parsed value */
		_ItType    /* Iterator of where the *head* parsing stopped */
	>
	ToPrimitive(_ItType begin, _ItType end, _ItType) const
	{
		auto res = HeadToPrimitive(begin, end);
		return std::make_tuple(
			std::get<0>(res),
			std::get<1>(res)
		);
	}

protected:

	HeadCodec m_headCodec;

}; // struct AbiParserHeadOnlyTypes


template<
	typename _CodecImpl
>
struct AbiParserHeadTailTypes
{
	using Codec = _CodecImpl;
	using HeadCodec = AbiCodecImpl<
		Internal::Obj::ObjCategory::Integer,
		std::integral_constant<
			Internal::Obj::RealNumType,
			Internal::Obj::RealNumType::UInt64
		>
	>; // head is a uint256 offset
	using TailCodec = Codec;

	static constexpr bool sk_hasTail = true;
	using HeadPrimitive = typename HeadCodec::Primitive;
	using TailPrimitive = typename TailCodec::Primitive;
	using Primitive = TailPrimitive;

	// constructors
	AbiParserHeadTailTypes(TailCodec tailCodec) :
		m_headCodec(),
		m_tailCodec(std::move(tailCodec))
	{}
	// destructor
	// LCOV_EXCL_START
	virtual ~AbiParserHeadTailTypes() = default;
	// LCOV_EXCL_STOP


	template<typename _ItType>
	std::tuple<
		HeadPrimitive, /* Parsed value */
		_ItType,       /* Iterator of where the parsing stopped */
		size_t         /* Number of chunks consumed */
	>
	HeadToPrimitive(_ItType begin, _ItType end) const
	{
		return m_headCodec.ToPrimitive(begin, end);
	}


	template<typename _ItType>
	std::tuple<
		TailPrimitive, /* Parsed value */
		_ItType,       /* Iterator of where the parsing stopped */
		size_t         /* Number of chunks consumed */
	>
	TailToPrimitive(_ItType begin, _ItType end) const
	{
		return m_tailCodec.ToPrimitive(begin, end);
	}


	/**
	 * @brief Parse the ABI-encoded data
	 *        NOTE: this function will parse the offset and then jump to the
	 *        tail part of the data. So if the given iterator is type of
	 *        `input_iterator` or `forward_iterator`, the data in the range
	 *        that is being jumped over will be lost.
	 *
	 * @tparam _ItType
	 * @param begin
	 * @param end
	 * @param blockBegin
	 * @return
	 */
	template<typename _ItType>
	std::tuple<
		Primitive, /* Parsed value */
		_ItType    /* Iterator of where the *head* parsing stopped */
	>
	ToPrimitive(_ItType begin, _ItType end, _ItType blockBegin) const
	{
		using _ItDiffType =
			typename std::iterator_traits<_ItType>::difference_type;

		HeadPrimitive offset = 0;
		_ItType headEnd = begin;
		std::tie(offset, headEnd, std::ignore) = HeadToPrimitive(begin, end);

		// make sure we can jump to the offset
		auto totalSizeDiff = std::distance(blockBegin, end);
		if (totalSizeDiff < 0)
		{
			throw Exception("ABI parser - invalid block begin iterator");
		}
		size_t totalSize = static_cast<size_t>(totalSizeDiff);
		if (offset > totalSize)
		{
			throw Exception("ABI parser - the input is too short");
		}
		begin = std::next(blockBegin, static_cast<_ItDiffType>(offset));

		// parse the tail
		Primitive res;
		std::tie(res, std::ignore, std::ignore) =
			TailToPrimitive(begin, end);

		return std::make_tuple(res, headEnd);
	}

protected:

	HeadCodec m_headCodec;
	TailCodec m_tailCodec;

}; // struct AbiParserHeadTailTypes


template<bool _HasTail, typename _CodecImpl>
using AbiHeadTailTypesSelector = typename std::conditional<
	_HasTail,
	AbiParserHeadTailTypes<_CodecImpl>,
	AbiParserHeadOnlyTypes<_CodecImpl>
>::type;


} // namespace EthInternal


// ==========
// Alias types for convenience
// ==========


using AbiUInt8 =
	std::integral_constant<
		Internal::Obj::RealNumType,
		Internal::Obj::RealNumType::UInt8
	>;
using AbiUInt64 =
	std::integral_constant<
		Internal::Obj::RealNumType,
		Internal::Obj::RealNumType::UInt64
	>;

template<size_t _Size>
using AbiSize = std::integral_constant<size_t, _Size>;


// ==========
// AbiParser general template
// ==========

template<
	Internal::Obj::ObjCategory _DataType,
	typename... _Args
>
struct AbiParser;


// ==========
// AbiParser for integer types
// ==========


template<Internal::Obj::RealNumType _RealNumType>
struct AbiParser<
	Internal::Obj::ObjCategory::Integer,
	std::integral_constant<Internal::Obj::RealNumType, _RealNumType>
> :
	EthInternal::AbiParserHeadOnlyTypes<
		EthInternal::AbiCodecImpl<
			Internal::Obj::ObjCategory::Integer,
			std::integral_constant<Internal::Obj::RealNumType, _RealNumType>
		>
	>
{
	using Base = EthInternal::AbiParserHeadOnlyTypes<
		EthInternal::AbiCodecImpl<
			Internal::Obj::ObjCategory::Integer,
			std::integral_constant<Internal::Obj::RealNumType, _RealNumType>
		>
	>;
	using Codec = typename Base::Codec;

	AbiParser() :
		Base(Codec())
	{}
	// LCOV_EXCL_START
	virtual ~AbiParser() = default;
	// LCOV_EXCL_STOP
}; // struct AbiParser<Internal::Obj::ObjCategory::Integer, ...>


// ==========
// AbiParser for bool type
// ==========


template<>
struct AbiParser<
	Internal::Obj::ObjCategory::Bool
> :
	EthInternal::AbiParserHeadOnlyTypes<
		EthInternal::AbiCodecImpl<
			Internal::Obj::ObjCategory::Bool
		>
	>
{
	using Base = EthInternal::AbiParserHeadOnlyTypes<
		EthInternal::AbiCodecImpl<
			Internal::Obj::ObjCategory::Bool
		>
	>;
	using Codec = typename Base::Codec;

	AbiParser() :
		Base(Codec())
	{}
	// LCOV_EXCL_START
	virtual ~AbiParser() = default;
	// LCOV_EXCL_STOP
}; // struct AbiParser<Internal::Obj::ObjCategory::Bool>


// ==========
// AbiParser for bytes<M> types
// ==========


template<>
struct AbiParser<
	Internal::Obj::ObjCategory::Bytes,
	std::false_type /* IsDynamic? - false */
> :
	EthInternal::AbiParserHeadOnlyTypes<
		EthInternal::AbiCodecImpl<
			Internal::Obj::ObjCategory::Bytes,
			std::false_type
		>
	>
{
	using Base = EthInternal::AbiParserHeadOnlyTypes<
		EthInternal::AbiCodecImpl<
			Internal::Obj::ObjCategory::Bytes,
			std::false_type
		>
	>;
	using Codec = typename Base::Codec;


	AbiParser(size_t size) :
		Base(Codec(EthInternal::AbiWithinChunkSize(size)))
	{}
	// LCOV_EXCL_START
	virtual ~AbiParser() = default;
	// LCOV_EXCL_STOP
}; // struct AbiParser<Internal::Obj::ObjCategory::Bool, false>


template<size_t _Size>
struct AbiParser<
	Internal::Obj::ObjCategory::Bytes,
	std::integral_constant<size_t, _Size>
> :
	EthInternal::AbiParserHeadOnlyTypes<
		EthInternal::AbiCodecImpl<
			Internal::Obj::ObjCategory::Bytes,
			std::false_type
		>
	>
{
	static constexpr size_t sk_size = _Size;
	static_assert(
		sk_size <= AbiParserConst::sk_chunkSize(),
		"ABI parser - bytes type is too large"
	);

	using Base = EthInternal::AbiParserHeadOnlyTypes<
		EthInternal::AbiCodecImpl<
			Internal::Obj::ObjCategory::Bytes,
			std::false_type
		>
	>;
	using Codec = typename Base::Codec;

	AbiParser() :
		Base(Codec(sk_size))
	{}
	// LCOV_EXCL_START
	virtual ~AbiParser() = default;
	// LCOV_EXCL_STOP
}; // struct AbiParser<Internal::Obj::ObjCategory::Bool, size_t>


// ==========
// AbiParser for bytes types
// ==========


template<>
struct AbiParser<
	Internal::Obj::ObjCategory::Bytes,
	std::true_type /* IsDynamic? - true */
> :
	EthInternal::AbiParserHeadTailTypes<
		EthInternal::AbiCodecImpl<
			Internal::Obj::ObjCategory::Bytes,
			std::true_type
		>
	>
{
	using Base = EthInternal::AbiParserHeadTailTypes<
		EthInternal::AbiCodecImpl<
			Internal::Obj::ObjCategory::Bytes,
			std::true_type
		>
	>;
	using Codec = typename Base::Codec;


	AbiParser() :
		Base(Codec())
	{}
	// LCOV_EXCL_START
	virtual ~AbiParser() = default;
	// LCOV_EXCL_STOP
}; // struct AbiParser<Internal::Obj::ObjCategory::Bool, true>


// ==========
// AbiParser for list types (T[k])
// ==========


template<typename _ItemParser>
struct AbiParser<
	Internal::Obj::ObjCategory::List,
	_ItemParser,
	std::false_type /* IsLenDynamic? - false */
> :
	EthInternal::AbiHeadTailTypesSelector<
		_ItemParser::sk_hasTail,
		EthInternal::AbiCodecImpl<
			Internal::Obj::ObjCategory::List,
			_ItemParser,
			std::false_type,
			std::integral_constant<
				bool,
				_ItemParser::sk_hasTail
			>
		>
	>
{
	using Base = EthInternal::AbiHeadTailTypesSelector<
		_ItemParser::sk_hasTail,
		EthInternal::AbiCodecImpl<
			Internal::Obj::ObjCategory::List,
			_ItemParser,
			std::false_type,
			std::integral_constant<
				bool,
				_ItemParser::sk_hasTail
			>
		>
	>;
	using Codec = typename Base::Codec;


	AbiParser(_ItemParser itemParser, size_t len) :
		Base(Codec(std::move(itemParser), len))
	{}
	// LCOV_EXCL_START
	virtual ~AbiParser() = default;
	// LCOV_EXCL_STOP
}; // struct AbiParser<Internal::Obj::ObjCategory::List, _Item, false>


template<typename _ItemParser, size_t _Size>
struct AbiParser<
	Internal::Obj::ObjCategory::List,
	_ItemParser,
	std::integral_constant<size_t, _Size>
> :
	EthInternal::AbiHeadTailTypesSelector<
		_ItemParser::sk_hasTail,
		EthInternal::AbiCodecImpl<
			Internal::Obj::ObjCategory::List,
			_ItemParser,
			std::false_type,
			std::integral_constant<
				bool,
				_ItemParser::sk_hasTail
			>
		>
	>
{
	using Base = EthInternal::AbiHeadTailTypesSelector<
		_ItemParser::sk_hasTail,
		EthInternal::AbiCodecImpl<
			Internal::Obj::ObjCategory::List,
			_ItemParser,
			std::false_type,
			std::integral_constant<
				bool,
				_ItemParser::sk_hasTail
			>
		>
	>;
	using Codec = typename Base::Codec;

	static constexpr size_t sk_size = _Size;

	AbiParser(_ItemParser itemParser) :
		Base(Codec(std::move(itemParser), sk_size))
	{}
	// LCOV_EXCL_START
	virtual ~AbiParser() = default;
	// LCOV_EXCL_STOP
}; // struct AbiParser<Internal::Obj::ObjCategory::List, _Item, Len>


// ==========
// AbiParser for list types (T[])
// ==========


template<typename _ItemParser>
struct AbiParser<
	Internal::Obj::ObjCategory::List,
	_ItemParser,
	std::true_type /* IsLenDynamic? - true */
> :
	EthInternal::AbiParserHeadTailTypes<
		// dynamic length make this type always has tail
		EthInternal::AbiCodecImpl<
			Internal::Obj::ObjCategory::List,
			_ItemParser,
			std::true_type,
			std::integral_constant<
				bool,
				_ItemParser::sk_hasTail
			>
		>
	>
{
	using Base =
		EthInternal::AbiParserHeadTailTypes<
			EthInternal::AbiCodecImpl<
				Internal::Obj::ObjCategory::List,
				_ItemParser,
				std::true_type,
				std::integral_constant<
					bool,
					_ItemParser::sk_hasTail
				>
			>
		>;
	using Codec = typename Base::Codec;


	explicit AbiParser(_ItemParser itemParser) :
		Base(Codec(std::move(itemParser)))
	{}
	// LCOV_EXCL_START
	virtual ~AbiParser() = default;
	// LCOV_EXCL_STOP
}; // struct AbiParser<Internal::Obj::ObjCategory::List, _Item, true>


} // namespace Eth
} // namespace EclipseMonitor
