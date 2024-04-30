// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <type_traits>

#include <SimpleObjects/StaticDict.hpp>
#include <SimpleObjects/Utils.hpp>

#include "ParserBase.hpp"

#ifndef SIMPLERLP_CUSTOMIZED_NAMESPACE
namespace SimpleRlp
#else
namespace SIMPLERLP_CUSTOMIZED_NAMESPACE
#endif
{

template<
	typename _ContainerType,
	typename _ByteValType,
	typename _ParserTp,
	typename _FallbackValParse,
	bool _AllowMissingItem,
	bool _AllowExtraItem,
	typename _RetType>
class StaticDictParserImpl :
	public ParserBase<_ContainerType, _ByteValType, _RetType>
{
public: // static members:

	using Self =
		StaticDictParserImpl<
			_ContainerType,
			_ByteValType,
			_ParserTp,
			_FallbackValParse,
			_AllowMissingItem,
			_AllowExtraItem,
			_RetType>;
	using Base = ParserBase<_ContainerType, _ByteValType, _RetType>;

	using InputByteType   = _ByteValType;
	using RetType         = _RetType;
	using ParserTuple     = _ParserTp;

	using FallbackValParse = _FallbackValParse;
	using TupleCore        = typename RetType::TupleCore;

public:

	StaticDictParserImpl() :
		m_parserTp()
	{}

	// LCOV_EXCL_START
	virtual ~StaticDictParserImpl() = default;
	// LCOV_EXCL_STOP

	using Base::Parse;

	virtual RetType Parse(
		InputStateMachineIf<InputByteType>& ism,
		RlpEncodeType rlpType,
		InputByteType rlpVal,
		size_t& byteLeft) const override
	{
		size_t size = Base::ProcRlpListHeader(ism, rlpType, rlpVal, byteLeft);

		Base::CheckByteLeft(byteLeft, size, ism.GetBytesCount());

		return ProcDictItems(ism, size);
	}

protected:

	RetType ProcDictItems(
		InputStateMachineIf<InputByteType>& ism,
		size_t size) const
	{
		TupleCore resTp;

		size_t numOfParsers = std::tuple_size<ParserTuple>::value;
		size_t numOfParsed = 0;
		while (size > 0)
		{
			InputByteType nextByte = ism.GetByteAndAdv();
			--size;

			RlpEncodeType nextType;
			InputByteType nextVal;
			std::tie(nextType, nextVal) =
				DecodeRlpLeadingByte(nextByte, ism.GetBytesCount());

			if (numOfParsed >= numOfParsers)
			{
				// extra items
				if (_AllowExtraItem)
				{
					// allowing extra item, consume it
					FallbackValParse().Parse(ism, nextType, nextVal, size);
				}
				else
				{
					// extra item is not allowed, throw err
					throw ParseError("The static dict parser encounters more "
						"items than expected", ism.GetBytesCount());
				}
			}
			else
			{
				// item index within range
				ParseValueCallBack cb(
					numOfParsed, ism, nextType, nextVal, size);

				Internal::Obj::Internal::TupleOperation::
					BinOp(resTp, m_parserTp, cb);
			}

			++numOfParsed;
		}

		if ((numOfParsed < numOfParsers) && !_AllowMissingItem)
		{
			// missing items
			throw ParseError("The static dict parser is expecting more items"
				" to parse", ism.GetBytesCount());
		}

		return RetType(std::move(resTp));
	}

private: // static members:

	struct ParseValueCallBack
	{

		ParseValueCallBack(
			size_t idx,
			InputStateMachineIf<InputByteType>& ism,
			RlpEncodeType rlpType,
			InputByteType rlpVal,
			size_t& byteLeft):
			m_idx(idx),
			m_ismPtr(&ism),
			m_rlpType(rlpType),
			m_rlpVal(rlpVal),
			m_byteLeftPtr(&byteLeft),
			m_parsed(0)
		{}

		~ParseValueCallBack() = default;

		template<typename _KeyType, typename _ValType, typename _ValParserType>
		void operator()(
			size_t i,
			std::pair<_KeyType, _ValType>& core,
			const std::pair<_KeyType, _ValParserType>& parser)
		{
			if (i == m_idx)
			{
				// A match is found
				core.second = parser.second.Parse(
					*m_ismPtr, m_rlpType, m_rlpVal, *m_byteLeftPtr);
				++m_parsed;
			}
		}

		size_t m_idx;
		InputStateMachineIf<InputByteType>* m_ismPtr;
		RlpEncodeType m_rlpType;
		InputByteType m_rlpVal;
		size_t* m_byteLeftPtr;
		size_t m_parsed;

	}; // struct ParseValueCallBack

private:

	ParserTuple m_parserTp;

}; // class StaticDictParserImpl


namespace Internal
{

template<typename _Key, typename _ValParser>
struct DPairParser2Val
{
	using type = std::pair<_Key, typename _ValParser::RetType>;
}; // struct DPairParser2Val

template<typename _ParserPair>
using DParserPair2PairCore =
	Internal::Obj::Internal::DPairTransform<DPairParser2Val, _ParserPair>;

template<typename _ParserTp>
using DParserTuple2TupleCore =
	Internal::Obj::Internal::TupleTransform<DParserPair2PairCore, _ParserTp>;

} // namespace Internal


} // namespace SimpleRlp
