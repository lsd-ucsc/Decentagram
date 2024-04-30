// Copyright (c) 2022 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstddef>

#include <atomic>
#include <memory>

#include <EclipseMonitor/Eth/DataTypes.hpp>
#include <SimpleRlp/SimpleRlp.hpp>

#include "BlockReceiver.hpp"
#include "GethRequester.hpp"


namespace EthereumClt
{


class HostBlockService :
	public std::enable_shared_from_this<HostBlockService>
{
public: // static members:

	static std::shared_ptr<HostBlockService> Create(
		const std::string& gethUrl
	)
	{
		return std::shared_ptr<HostBlockService>(
			new HostBlockService(gethUrl)
		);
	}

private: // Constructor - not allowed to be called directly

	HostBlockService(
		const std::string& gethUrl
	) :
		m_gethReq(gethUrl),
		m_blockReceiver(),
		//m_isUpdSvcStarted(false),
		m_currBlockNum(0)
	{}

public:

	~HostBlockService() = default;

	EclipseMonitor::Eth::BlockNumber GetCurrBlockNum() const
	{
		return m_currBlockNum;
	}

	void SetUpdSvcStartBlock(EclipseMonitor::Eth::BlockNumber startBlockNum)
	{
		m_currBlockNum = startBlockNum;
	}

	// bool GetIsUpdSvcStarted() const
	// {
	// 	return m_isUpdSvcStarted;
	// }

	std::shared_ptr<HostBlockService> GetSharedPtr()
	{
		return shared_from_this();
	}

	/**
	 * @brief Bind a BlockReceiver to this HostBlockService.
	 *        NOTE: it's a 1:1 binding, so the previous BlockReceiver will be
	 *        replaced by the new one, if there is any.
	 *
	 * @param blockReceiver The new BlockReceiver to bind.
	 */
	void BindReceiver(std::shared_ptr<BlockReceiver> blockReceiver)
	{
		m_blockReceiver = blockReceiver;
	}

	void PushBlock(const std::vector<uint8_t>& headerRlp) const
	{
		std::shared_ptr<BlockReceiver> blockReceiver =
			m_blockReceiver.lock();
		if (blockReceiver == nullptr)
		{
			throw std::runtime_error(
				"HostBlockService - BlockReceiver is not available"
			);
		}
		else
		{
			blockReceiver->RecvBlock(headerRlp);
		}
	}

	void PushBlock(EclipseMonitor::Eth::BlockNumber blockNum) const
	{
		auto headerRlp = m_gethReq.GetHeaderRlpByNum(blockNum);

		return PushBlock(headerRlp);
	}

	bool TryPushNewBlock()
	{
		// if (!m_isUpdSvcStarted)
		// {
		// 	throw std::runtime_error(
		// 		"HostBlockService - BlockUpdateService is not started"
		// 	);
		// }

		std::vector<uint8_t> headerRlp;
		try
		{
			headerRlp = m_gethReq.GetHeaderRlpByNum(m_currBlockNum);
		}
		catch(const std::exception& e)
		{
			return false;
		}

		PushBlock(headerRlp);
		++m_currBlockNum;
		return true;
	}

	template<typename _RetType>
	_RetType GetReceiptsRlpByNum(
		uint64_t blockNum
	) const
	{
		return m_gethReq.GetReceiptsRlpByNum<_RetType>(blockNum);
	}


	uint64_t GetLatestBlockNum() const
	{
		auto hdrRlp = m_gethReq.GetHeaderRlpByParam("latest");
		auto hdr = SimpleRlp::EthHeaderParser().Parse(hdrRlp);
		return
			EclipseMonitor::Eth::BlkNumTypeTrait::FromBytes(hdr.get_Number());
	}


private:
	GethRequester m_gethReq;
	std::weak_ptr<BlockReceiver> m_blockReceiver;
	//std::atomic_bool m_isUpdSvcStarted;
	std::atomic<EclipseMonitor::Eth::BlockNumber> m_currBlockNum;

}; // class HostBlockService


} // namespace EthereumClt
