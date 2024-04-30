// Copyright (c) 2022 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "DataTypes.hpp"

namespace EclipseMonitor
{
namespace Eth
{


struct Params
{
	// Reference: https://github.com/ethereum/go-ethereum/blob/master/consensus/ethash/difficulty.go
	// Reference: https://github.com/ethereum/go-ethereum/blob/master/params/protocol_params.go

	static constexpr int GetDifficultyBoundDivisorBitShift()
	{
		return 11;
	}

	static constexpr uint64_t GetMinimumDifficulty()
	{
		return 131072;
	}

	static constexpr uint64_t GetDurationLimit()
	{
		return 13;
	}

}; // struct Params


template<typename _ConfigDetails>
struct NetworkConfigImpl
{
	using Details = _ConfigDetails;
	using BlkNumType = typename Details::BlkNumType;

	static bool IsBlockOf(
		const BlkNumType* forkBlkNum,
		const BlkNumType* blkNum
	)
	{
		if (forkBlkNum == nullptr || blkNum == nullptr)
		{
			return false;
		}
		return (*blkNum) >= (*forkBlkNum);
	}

	static bool IsBlockOfParis(const BlkNumType& blkNum)
	{
		return IsBlockOf(Details::ParisBlkNum(), &blkNum);
	}

	static bool IsBlockOfGrayGlacier(const BlkNumType& blkNum)
	{
		return IsBlockOf(Details::GrayGlacierBlkNum(), &blkNum);
	}

	static bool IsBlockOfArrowGlacier(const BlkNumType& blkNum)
	{
		return IsBlockOf(Details::ArrowGlacierBlkNum(), &blkNum);
	}

	static bool IsBlockOfLondon(const BlkNumType& blkNum)
	{
		return IsBlockOf(Details::LondonBlkNum(), &blkNum);
	}

	static bool IsBlockOfMuirGlacier(const BlkNumType& blkNum)
	{
		return IsBlockOf(Details::MuirGlacierBlkNum(), &blkNum);
	}

	static bool IsBlockOfConstantinople(const BlkNumType& blkNum)
	{
		return IsBlockOf(Details::ConstantinopleBlkNum(), &blkNum);
	}

	static bool IsBlockOfByzantium(const BlkNumType& blkNum)
	{
		return IsBlockOf(Details::ByzantiumBlkNum(), &blkNum);
	}

	static bool IsBlockOfHomestead(const BlkNumType& blkNum)
	{
		return IsBlockOf(Details::HomesteadBlkNum(), &blkNum);
	}
}; // struct NetworkConfigImpl


struct MainnetConfigDetails
{
	// numbers are generally retrieved from:
	// https://github.com/ethereum/go-ethereum/blob/5ccc99b258461457955fc523839fd373b33186af/params/config.go#L59

	using BlkNumType = BlockNumber;

	static const BlkNumType* ParisBlkNum()
	{
		// https://github.com/ethereum/execution-specs/blob/master/network-upgrades/mainnet-upgrades/paris.md
		static const BlkNumType blkNum(15537394UL);
		return &blkNum;
	}

	static const BlkNumType* GrayGlacierBlkNum()
	{
		static const BlkNumType blkNum(15050000UL);
		return &blkNum;
	}

	static const BlkNumType* ArrowGlacierBlkNum()
	{
		static const BlkNumType blkNum(13773000UL);
		return &blkNum;
	}

	static const BlkNumType* LondonBlkNum()
	{
		static const BlkNumType blkNum(12965000UL);
		return &blkNum;
	}

	static const BlkNumType* MuirGlacierBlkNum()
	{
		static const BlkNumType blkNum(9200000UL);
		return &blkNum;
	}

	static const BlkNumType* ConstantinopleBlkNum()
	{
		static const BlkNumType blkNum(7280000UL);
		return &blkNum;
	}

	static const BlkNumType* ByzantiumBlkNum()
	{
		static const BlkNumType blkNum(4370000UL);
		return &blkNum;
	}

	static const BlkNumType* HomesteadBlkNum()
	{
		static const BlkNumType blkNum(1150000UL);
		return &blkNum;
	}
}; // struct MainnetConfigDetails


struct GoerliConfigDetails
{
	// numbers are generally retrieved from:
	// https://github.com/ethereum/go-ethereum/blob/5ccc99b258461457955fc523839fd373b33186af/params/config.go#L179

	using BlkNumType = BlockNumber;

	static const BlkNumType* ParisBlkNum()
	{
		//
		static const BlkNumType blkNum(7382819UL);
		return &blkNum;
	}

	static const BlkNumType* GrayGlacierBlkNum()
	{
		return nullptr;
	}

	static const BlkNumType* ArrowGlacierBlkNum()
	{
		return nullptr;
	}

	static const BlkNumType* LondonBlkNum()
	{
		static const BlkNumType blkNum(5062605UL);
		return &blkNum;
	}

	static const BlkNumType* MuirGlacierBlkNum()
	{
		return nullptr;
	}

	static const BlkNumType* ConstantinopleBlkNum()
	{
		static const BlkNumType blkNum(0UL);
		return &blkNum;
	}

	static const BlkNumType* ByzantiumBlkNum()
	{
		static const BlkNumType blkNum(0UL);
		return &blkNum;
	}

	static const BlkNumType* HomesteadBlkNum()
	{
		static const BlkNumType blkNum(0UL);
		return &blkNum;
	}
}; // struct GoerliConfigDetails


using MainnetConfig = NetworkConfigImpl<MainnetConfigDetails>;

using GoerliConfig = NetworkConfigImpl<GoerliConfigDetails>;


} // namespace Eth
} // namespace EclipseMonitor
