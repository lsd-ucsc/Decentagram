// Copyright (c) 2022 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <algorithm>

#include "../Internal/SimpleObj.hpp"
#include "../Internal/SimpleRlp.hpp"

#include "BloomFilter.hpp"
#include "DataTypes.hpp"
#include "Keccak256.hpp"


namespace EclipseMonitor
{
namespace Eth
{


class HeaderMgr
{
public: // static member

	using RawHeaderType = Internal::Rlp::EthHeader;
	using RawHeaderParser = Internal::Rlp::EthHeaderParser;

	using BytesObjType = Internal::Rlp::BytesObjType;

	static const BytesObjType& GetEmptyUncleHash()
	{
		static const BytesObjType inst({
			0X1DU, 0XCCU, 0X4DU, 0XE8U, 0XDEU, 0XC7U, 0X5DU, 0X7AU, 0XABU, 0X85U,
			0XB5U, 0X67U, 0XB6U, 0XCCU, 0XD4U, 0X1AU, 0XD3U, 0X12U, 0X45U, 0X1BU,
			0X94U, 0X8AU, 0X74U, 0X13U, 0XF0U, 0XA1U, 0X42U, 0XFDU, 0X40U, 0XD4U,
			0X93U, 0X47U,
		});
		return inst;
	}

public:

	HeaderMgr() :
		m_rawHeader(),
		m_trustedTime(0),
		m_bloomFilter(
			m_rawHeader.get_LogsBloom() =
				BytesObjType(std::vector<uint8_t>(BloomFilter::sk_bloomByteSize, 0))
		),
		m_hash(),
		m_hashObj(m_hash.begin(), m_hash.end()),
		m_blkNum(0),
		m_time(0),
		m_diff(0),
		m_hasUncle(false)
	{}

	HeaderMgr(const std::vector<uint8_t>& rawBinary, uint64_t trustedTime) :
		m_rawHeader(RawHeaderParser().Parse(rawBinary)),
		m_trustedTime(trustedTime),
		m_bloomFilter(m_rawHeader.get_LogsBloom()),
		m_hash(Keccak256(rawBinary)),
		m_hashObj(m_hash.begin(), m_hash.end()),
		m_blkNum(BlkNumTypeTrait::FromBytes(m_rawHeader.get_Number())),
		m_time(TimeTypeTrait::FromBytes(m_rawHeader.get_Timestamp())),
		m_diff(DiffTypeTrait::FromBytes(m_rawHeader.get_Difficulty())),
		m_hasUncle(m_rawHeader.get_Sha3Uncles() != GetEmptyUncleHash())
	{}

	// LCOV_EXCL_START
	~HeaderMgr() = default;
	// LCOV_EXCL_STOP

	void SetNumber(const BlockNumber& blkNum)
	{
		m_rawHeader.get_Number() = BlkNumTypeTrait::ToBytes(blkNum);
		m_blkNum = blkNum;
	}

	void SetTime(const Timestamp& time)
	{
		m_rawHeader.get_Timestamp() = TimeTypeTrait::ToBytes(time);
		m_time = time;
	}

	void SetDiff(const Difficulty& diff)
	{
		m_rawHeader.get_Difficulty() = DiffTypeTrait::ToBytes(diff);
		m_diff = diff;
	}

	void SetUncleHash(const BytesObjType& uncleHash)
	{
		m_rawHeader.get_Sha3Uncles() = uncleHash;
		m_hasUncle = (uncleHash != GetEmptyUncleHash());
	}

	const RawHeaderType& GetRawHeader() const
	{
		return m_rawHeader;
	}

	uint64_t GetTrustedTime() const
	{
		return m_trustedTime;
	}

	const std::array<uint8_t, 32>& GetHash() const
	{
		return m_hash;
	}

	const Internal::Obj::Bytes& GetHashObj() const
	{
		return m_hashObj;
	}

	const BlockNumber& GetNumber() const
	{
		return m_blkNum;
	}

	const Timestamp& GetTime() const
	{
		return m_time;
	}

	const Difficulty& GetDiff() const
	{
		return m_diff;
	}

	bool HasUncle() const
	{
		return m_hasUncle;
	}

	const BloomFilter& GetBloomFilter() const
	{
		return m_bloomFilter;
	}

private:

	RawHeaderType m_rawHeader;
	uint64_t m_trustedTime;
	BloomFilter m_bloomFilter;
	std::array<uint8_t, 32> m_hash;
	Internal::Obj::Bytes m_hashObj;
	BlockNumber m_blkNum;
	Timestamp m_time;
	Difficulty m_diff;
	bool m_hasUncle;
}; // class HeaderMgr


} // namespace Eth
} // namespace EclipseMonitor
