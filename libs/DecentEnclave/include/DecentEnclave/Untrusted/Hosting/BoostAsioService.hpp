// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <memory>

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_service.hpp>
#include <SimpleConcurrency/Threading/Task.hpp>

#include "../../Common/Internal/SimpleConcurrency.hpp"


namespace DecentEnclave
{
namespace Untrusted
{
namespace Hosting
{


class BoostAsioService : public Common::Internal::Concurrent::Threading::Task
{
public: // static members:

	using Base = Common::Internal::Concurrent::Threading::Task;

public:

	BoostAsioService(
		std::shared_ptr<boost::asio::io_service> ioService =
			std::make_shared<boost::asio::io_service>()
	) :
		Base(),
		m_ioService(std::move(ioService)),
		m_workGuard(boost::asio::make_work_guard(*m_ioService))
	{}

	// LCOV_EXCL_START
	virtual ~BoostAsioService() = default;
	// LCOV_EXCL_STOP


	BoostAsioService(BoostAsioService&& other) :
		m_ioService(std::move(other.m_ioService)),
		m_workGuard(std::move(other.m_workGuard))
	{}


	BoostAsioService(const BoostAsioService& other) = delete;
	BoostAsioService& operator=(const BoostAsioService& other) = delete;
	BoostAsioService& operator=(BoostAsioService&& other) = delete;


	std::shared_ptr<boost::asio::io_service> GetIoService() const
	{
		return m_ioService;
	}


	virtual void Run() override
	{
		m_ioService->run();
	}


	virtual void Terminate() override
	{
		m_ioService->stop();
	}


private:

	std::shared_ptr<boost::asio::io_service> m_ioService;
	boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
		m_workGuard;

}; // class BoostAsioService


} // namespace Hosting
} // namespace Untrusted
} // namespace DecentEnclave
