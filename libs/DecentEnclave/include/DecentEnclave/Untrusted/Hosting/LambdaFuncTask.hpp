// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <memory>

#include <SimpleConcurrency/Threading/Task.hpp>
#include <SimpleSysIO/StreamSocketBase.hpp>

#include "../../Common/Internal/SimpleConcurrency.hpp"
#include "../../Common/Internal/SimpleSysIO.hpp"
#include "DecentLambdaFunc.hpp"


namespace DecentEnclave
{
namespace Untrusted
{
namespace Hosting
{


class LambdaFuncTask : public Common::Internal::Concurrent::Threading::Task
{
public: // static members:

	using Base = Common::Internal::Concurrent::Threading::Task;

public:

	LambdaFuncTask(
		std::shared_ptr<DecentLambdaFunc> func,
		std::unique_ptr<Common::Internal::SysIO::StreamSocketBase> socket
	) :
		Base(),
		m_func(std::move(func)),
		m_socket(std::move(socket))
	{}

	// LCOV_EXCL_START
	virtual ~LambdaFuncTask() = default;
	// LCOV_EXCL_STOP


	LambdaFuncTask(LambdaFuncTask&& other) :
		m_func(std::move(other.m_func)),
		m_socket(std::move(other.m_socket))
	{}


	LambdaFuncTask(const LambdaFuncTask& other) = delete;
	LambdaFuncTask& operator=(const LambdaFuncTask& other) = delete;
	LambdaFuncTask& operator=(LambdaFuncTask&& other) = delete;


	virtual void Run() override
	{
		m_func->HandleCall(std::move(m_socket));
	}


	virtual void Terminate() override
	{
		// The function is expected to be non-blocking
		// so we don't need (and also don't have general way) to terminate it
	}


private:

	std::shared_ptr<DecentLambdaFunc> m_func;
	std::unique_ptr<Common::Internal::SysIO::StreamSocketBase> m_socket;

}; // class LambdaFuncTask


} // namespace Hosting
} // namespace Untrusted
} // namespace DecentEnclave
