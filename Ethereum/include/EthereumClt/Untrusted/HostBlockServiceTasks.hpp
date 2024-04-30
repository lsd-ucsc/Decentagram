// Copyright (c) 2022 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <chrono>
#include <iostream>
#include <thread>

#include <SimpleConcurrency/Threading/TickingTask.hpp>

#include "HostBlockService.hpp"


namespace EthereumClt
{

class BlockUpdatorServiceTask :
	public SimpleConcurrency::Threading::TickingTask<int64_t>
{
public: // static members:

	using Base = SimpleConcurrency::Threading::TickingTask<int64_t>;

	static constexpr int64_t sk_taskUpdIntervalMliSec = 200;

public:
	BlockUpdatorServiceTask(
		std::shared_ptr<HostBlockService> blockUpdator,
		int64_t retryIntervalMilSec
	) :
		Base(),
		m_blockUpdator(blockUpdator),
		m_retryIntervalMilSec(retryIntervalMilSec)
	{}

	virtual ~BlockUpdatorServiceTask() = default;


protected:

	virtual void Tick() override
	{
		auto blockUpdator = m_blockUpdator.lock();
		if (blockUpdator)
		{
			if (blockUpdator->TryPushNewBlock())
			{
				// Successfully pushed a new block to the enclave
				// keep pushing without delay
				if (Base::IsTickIntervalEnabled())
				{
					Base::DisableTickInterval();
				}
			}
			else
			{
				// Failed to push a new block to the enclave
				// wait for a while before retry
				if (!Base::IsTickIntervalEnabled())
				{
					Base::SetInterval(
						sk_taskUpdIntervalMliSec,
						m_retryIntervalMilSec
					);
				}
			}
		}
		else
		{
			throw std::runtime_error(
				"BlockUpdatorServiceTask - HostBlockService is not available"
			);
		}
	}


	virtual void SleepFor(int64_t mliSec) const override
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(mliSec));
	}


private:
	std::weak_ptr<HostBlockService> m_blockUpdator;
	int64_t m_retryIntervalMilSec;

}; // class BlockUpdatorServiceTask


class HostBlockStatusLogTask :
	public SimpleConcurrency::Threading::TickingTask<int64_t>
{
public: // static members:

	using Base = SimpleConcurrency::Threading::TickingTask<int64_t>;

	static constexpr int64_t sk_taskUpdIntervalMliSec = 100;

public:

	HostBlockStatusLogTask(
		std::shared_ptr<HostBlockService> blockUpdator,
		int64_t updIntervalMliSec
	) :
		Base(sk_taskUpdIntervalMliSec, updIntervalMliSec),
		m_blockUpdator(blockUpdator),
		m_lastBlockNum(0),
		m_updIntervalSec(updIntervalMliSec / 1000.0)
	{}

	virtual ~HostBlockStatusLogTask() = default;


protected:

	virtual void Tick() override
	{
		auto blockUpdator = m_blockUpdator.lock();
		if (blockUpdator)
		{
			const size_t currBlockNum = blockUpdator->GetCurrBlockNum();

			size_t diff = currBlockNum - m_lastBlockNum;
			float rate = diff / m_updIntervalSec;

			std::cout << "HostBlockServiceStatus: " <<
				"BlockNum=" << currBlockNum << ", " <<
				"Rate=" << rate << " blocks/sec" <<
				std::endl;

			m_lastBlockNum = currBlockNum;
		}
		else
		{
			throw std::runtime_error(
				"HostBlockStatusLogTask - HostBlockService is not available"
			);
		}
	}


	virtual void SleepFor(int64_t mliSec) const override
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(mliSec));
	}


private:
	std::weak_ptr<HostBlockService> m_blockUpdator;
	size_t m_lastBlockNum;
	float m_updIntervalSec;
}; // class HostBlockStatusLogTask

} // namespace EthereumClt
