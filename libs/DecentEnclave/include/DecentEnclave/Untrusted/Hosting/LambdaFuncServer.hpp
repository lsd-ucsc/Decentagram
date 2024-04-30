// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <memory>
#include <unordered_map>

#include <SimpleConcurrency/Threading/ThreadPool.hpp>
#include <SimpleObjects/Internal/make_unique.hpp>
#include <SimpleSysIO/StreamSocketBase.hpp>

#include "../../Common/Exceptions.hpp"
#include "../../Common/Internal/SimpleConcurrency.hpp"
#include "../../Common/Internal/SimpleObj.hpp"
#include "../../Common/Internal/SimpleSysIO.hpp"
#include "../../Common/Platform/Print.hpp"
#include "../Config/EndpointsMgr.hpp"
#include "DecentLambdaFunc.hpp"
#include "LambdaFuncTask.hpp"


namespace DecentEnclave
{
namespace Untrusted
{
namespace Hosting
{


class LambdaFuncServer
{
public: // static members:

	using SocketType = Common::Internal::SysIO::StreamSocketBase;
	using AcceptorType = Common::Internal::SysIO::StreamAcceptorBase;
	using ThreadPoolType = Common::Internal::Concurrent::Threading::ThreadPool;

	using ServerBinding = std::pair<
		std::shared_ptr<DecentLambdaFunc>,
		std::shared_ptr<AcceptorType>
	>;

public:

	LambdaFuncServer(
		std::shared_ptr<Config::EndpointsMgr> endpointsMgr,
		std::shared_ptr<ThreadPoolType> threadPool
	) :
		m_endpointsMgr(std::move(endpointsMgr)),
		m_threadPool(std::move(threadPool)),
		m_funcMap()
	{}


	~LambdaFuncServer() = default;


	void AddFunction(
		const std::string& name,
		std::shared_ptr<DecentLambdaFunc> func
	)
	{
		if (m_funcMap.find(name) != m_funcMap.end())
		{
			throw Common::Exception("Function name already exists.");
		}

		std::shared_ptr<AcceptorType> acceptor =
			m_endpointsMgr->GetStreamAcceptor(name);

		auto res = m_funcMap.emplace(
			name,
			std::make_pair(std::move(func), std::move(acceptor))
		);

		StartAccepting(
			res.first->second.first,
			res.first->second.second,
			m_threadPool
		);
	}


private: // static members:

	static void StartAccepting(
		std::weak_ptr<DecentLambdaFunc> func,    // m_funcMap owns this object
		std::weak_ptr<AcceptorType> acceptor,    // m_funcMap owns this object
		std::weak_ptr<ThreadPoolType> threadPool // m_threadPool owns this object
	)
	{
		auto callback =
			[func, acceptor, threadPool](
				std::unique_ptr<SocketType> sock,
				bool hasErrorOccurred
			)
			{
				auto funcPtr = func.lock();
				auto acceptorPtr = acceptor.lock();
				auto threadPoolPtr = threadPool.lock();

				if (
					!hasErrorOccurred &&
					(threadPoolPtr != nullptr) &&
					(funcPtr != nullptr)
				)
				{
					// no error occurred
					// and thread pool is still alive
					// lambda function is still alive

					// log new connection
					Common::Platform::Print::StrInfo(
						"LambdaFuncServer - New connection accepted"
					);

					if (acceptorPtr != nullptr)
					{
						// Repeat to accept new connection
						StartAccepting(func, acceptor, threadPool);
					}

					// proceed to handle the call
					threadPoolPtr->AddTask(
						Common::Internal::Obj::Internal::make_unique<
							LambdaFuncTask
						>(
							funcPtr,
							std::move(sock)
						)
					);
				}
			};

		auto acceptorPtr = acceptor.lock();
		if (acceptorPtr != nullptr)
		{
			Common::Platform::Print::StrDebug(
				"LambdaFuncServer - Listening for incoming connection..."
			);
			acceptorPtr->AsyncAccept(std::move(callback));
		}
	}

private:

	std::shared_ptr<Config::EndpointsMgr> m_endpointsMgr;
	std::shared_ptr<ThreadPoolType> m_threadPool;

	std::unordered_map<std::string, ServerBinding> m_funcMap;

}; // class LambdaFuncServer


} // namespace Hosting
} // namespace Untrusted
} // namespace DecentEnclave
