// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <memory>
#include <thread>

#include <SimpleConcurrency/Threading/TickingTask.hpp>

#include "../../Common/Internal/SimpleConcurrency.hpp"
#include "HeartbeatEmitter.hpp"


namespace DecentEnclave
{
namespace Untrusted
{
namespace Hosting
{


class HeartbeatEmitterService :
	public Common::Internal::Concurrent::Threading::TickingTask<uint64_t>
{
public: // static members:

	using Base = Common::Internal::Concurrent::Threading::TickingTask<uint64_t>;

public:

	HeartbeatEmitterService(
		std::shared_ptr<HeartbeatEmitter> heartbeatEmitter,
		TimeType updInterval
	) :
		Base(updInterval, 200),
		m_heartbeatEmitter(std::move(heartbeatEmitter))
	{}

	// LCOV_EXCL_START
	virtual ~HeartbeatEmitterService() = default;
	// LCOV_EXCL_STOP


	HeartbeatEmitterService(HeartbeatEmitterService&& other) :
		m_heartbeatEmitter(std::move(other.m_heartbeatEmitter))
	{}


	HeartbeatEmitterService(const HeartbeatEmitterService& other) = delete;
	HeartbeatEmitterService& operator=(const HeartbeatEmitterService& other) = delete;
	HeartbeatEmitterService& operator=(HeartbeatEmitterService&& other) = delete;


protected:

	virtual void Tick() override
	{
		m_heartbeatEmitter->Heartbeat();
	}


	virtual void SleepFor(TimeType time) const override
	{
		std::this_thread::sleep_for(
			std::chrono::milliseconds(time)
		);
	}


private:

	std::shared_ptr<HeartbeatEmitter> m_heartbeatEmitter;

}; // class HeartbeatEmitterService


} // namespace Hosting
} // namespace Untrusted
} // namespace DecentEnclave
