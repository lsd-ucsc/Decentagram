// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>

#include <SimpleObjects/SimpleObjects.hpp>
#include <SimpleSysIO/StreamSocketBase.hpp>

#include "../Common/DeterministicMsg.hpp"
#include "../Common/Exceptions.hpp"
#include "../Common/Internal/SimpleObj.hpp"
#include "../Common/Internal/SimpleSysIO.hpp"


namespace DecentEnclave
{
namespace Trusted
{


struct LambdaServerConfig
{
	static const LambdaServerConfig& GetInstance(
		const LambdaServerConfig* initVal = nullptr
	)
	{
		static const LambdaServerConfig sk_config = (initVal != nullptr) ?
			*initVal :
			throw Common::Exception(
				"LambdaServerConfig is not initialized yet."
			);

		return sk_config;
	}

	LambdaServerConfig(
		const std::string& keyName,
		const std::string& certName
	) :
		m_keyName(keyName),
		m_certName(certName)
	{}

	~LambdaServerConfig() = default;

	std::string m_keyName;
	std::string m_certName;
}; // struct LambdaServerConfig


class LambdaHandlerMgr
{
public: // static members:

	using SocketType     = Common::Internal::SysIO::StreamSocketBase;
	using SocketPtrType  = std::unique_ptr<SocketType>;

	using MsgTypeType    = std::string;
	using MsgIdExtType   = Common::Internal::Obj::Bytes;
	using MsgContentType = Common::Internal::Obj::Bytes;

	using HandlerFunc = std::function<
		void(
			SocketPtrType&,
			const MsgIdExtType&,
			const MsgContentType&
		)
	>;


	static LambdaHandlerMgr& GetInstance()
	{
		static LambdaHandlerMgr s_inst;
		return s_inst;
	}


public:

	LambdaHandlerMgr() :
		m_handlerMapMutex(),
		m_handlerMap()
	{}

	~LambdaHandlerMgr() = default;

	void RegisterHandler(
		const MsgTypeType& msgType,
		HandlerFunc handler
	)
	{
		std::lock_guard<std::mutex> lock(m_handlerMapMutex);
		m_handlerMap[msgType].emplace_back(std::move(handler));
	}

	void HandleCall(
		SocketPtrType socket,
		const std::vector<uint8_t>& msgAdvRlp
	) const
	{
		auto detMsg = Common::DetMsgParser().Parse(msgAdvRlp);
		MsgTypeType msgType = MsgTypeType(
			detMsg.get_MsgId().get_MsgType().data(),
			detMsg.get_MsgId().get_MsgType().data() +
				detMsg.get_MsgId().get_MsgType().size()
		);

		// Retrieve handlers
		std::vector<std::reference_wrapper<const HandlerFunc> > handlers;
		{
			std::lock_guard<std::mutex> lock(m_handlerMapMutex);

			auto it = m_handlerMap.find(msgType);
			if (it == m_handlerMap.end() || it->second.empty())
			{
				throw Common::Exception("The given message type has no handler");
			}

			for (const auto& handler : it->second)
			{
				handlers.emplace_back(handler);
			}
		}

		// Call handlers
		for (const HandlerFunc& handler : handlers)
		{
			(handler)(
				socket,
				detMsg.get_MsgId().get_Ext(),
				detMsg.get_MsgContent()
			);
		}
	}

private:

	mutable std::mutex m_handlerMapMutex;
	std::unordered_map<MsgTypeType, std::vector<HandlerFunc> > m_handlerMap;
}; // class LambdaHandlerMgr


} // namespace Trusted
} // namespace DecentEnclave
