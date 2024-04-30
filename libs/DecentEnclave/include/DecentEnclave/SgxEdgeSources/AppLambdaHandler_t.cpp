// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#include <memory>

#include <sgx_error.h>
#include <SimpleObjects/Internal/make_unique.hpp>

#include "../Common/DecentTlsConfig.hpp"
#include "../Common/Internal/SimpleSysIO.hpp"
#include "../Common/Internal/SimpleObj.hpp"
#include "../Common/TlsSocket.hpp"
#include "../Common/Platform/Print.hpp"

#include "../Trusted/DecentLambdaSvr.hpp"
#include "../Trusted/HeartbeatEmitterMgr.hpp"
#include "../Trusted/Sgx/ComponentConnection.hpp"


extern "C" sgx_status_t ecall_decent_lambda_handler(
	void* sock_ptr
)
{
	using namespace DecentEnclave::Common;
	using namespace DecentEnclave::Common::Internal;
	using namespace DecentEnclave::Trusted;
	using namespace DecentEnclave::Trusted::Sgx;
	using namespace DecentEnclave::Common::Internal::SysIO;

	StreamSocketBase* realSockPtr = static_cast<StreamSocketBase*>(sock_ptr);

	std::unique_ptr<StreamSocket> sock =
		Obj::Internal::make_unique<StreamSocket>(realSockPtr);

	try
	{
		const auto& svrConfig = LambdaServerConfig::GetInstance();

		auto tlsCfg = DecentTlsConfig::MakeTlsConfig(
			true,
			svrConfig.m_keyName,
			svrConfig.m_certName
		);
		std::unique_ptr<TlsSocket> tlsSock =
			Obj::Internal::make_unique<TlsSocket>(
				tlsCfg,
				nullptr,
				std::move(sock)
			);

		auto detMsgAdvRlp = tlsSock->SizedRecvBytes<std::vector<uint8_t> >();

		LambdaHandlerMgr::GetInstance().HandleCall(
			std::move(tlsSock),
			detMsgAdvRlp
		);
	}
	catch(const std::exception& e)
	{
		Platform::Print::StrErr(
			std::string("Failed to handle a Decent Lambda call: ") +
			e.what()
		);
	}

	return SGX_SUCCESS;
}


extern "C" sgx_status_t ecall_decent_heartbeat()
{
	using namespace DecentEnclave::Common;
	using namespace DecentEnclave::Trusted;

	try
	{
		HeartbeatEmitterMgr::GetInstance().EmitAll();
	}
	catch(const std::exception& e)
	{
		Platform::Print::StrErr(
			std::string("Failed to emit heartbeat: ") +
			e.what()
		);
	}

	return SGX_SUCCESS;
}
