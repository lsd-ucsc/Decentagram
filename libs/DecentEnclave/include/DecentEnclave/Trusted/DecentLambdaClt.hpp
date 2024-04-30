// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <AdvancedRlp/AdvancedRlp.hpp>
#include <SimpleObjects/Internal/make_unique.hpp>

#include "../Common/DecentTlsConfig.hpp"
#include "../Common/DeterministicMsg.hpp"
#include "../Common/Internal/SimpleObj.hpp"
#include "../Common/Internal/SimpleRlp.hpp"
#include "../Common/TlsSocket.hpp"

#include "ComponentConnection.hpp"


namespace DecentEnclave
{
namespace Trusted
{


inline std::unique_ptr<DecentEnclave::Common::TlsSocket> MakeLambdaCall(
	const std::string& componentName,
	std::shared_ptr<Common::DecentTlsConfig> tlsConfig,
	Common::DetMsg& msg
)
{
	using namespace DecentEnclave::Common;

	static constexpr uint32_t sk_detMsgVer = 1;

	auto socket = ComponentConnection::Connect(componentName);

	std::unique_ptr<TlsSocket> tlsSock =
		Internal::Obj::Internal::make_unique<TlsSocket>(
			tlsConfig,
			nullptr,
			std::move(socket)
		);

	msg.get_Version() = Internal::Obj::UInt32(sk_detMsgVer);
	auto msgAdvRlp = Internal::AdvRlp::GenericWriter::Write(msg);

	tlsSock->SizedSendBytes(msgAdvRlp);

	return tlsSock;
}


} // namespace Trusted
} // namespace DecentEnclave
