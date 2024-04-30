// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED

#include <functional>

#include <sgx_dh.h>
#include <mbedTLScpp/SKey.hpp>

#include "../../Common/AesGcmSocketHandshaker.hpp"
#include "../../Common/Internal/SimpleSysIO.hpp"
#include "../../Common/Sgx/Crypto.hpp"
#include "../../Common/Sgx/Exceptions.hpp"


namespace DecentEnclave
{
namespace Trusted
{
namespace Sgx
{


class LaResponder :
	public Common::AesGcmSocketHandshaker<128>
{
public: // static members:

	using Base = Common::AesGcmSocketHandshaker<128>;
	using RetKeyType = typename Base::RetKeyType;

	enum class HSState : uint8_t
	{
		Initial,
		Msg1Sent,
		HandshakeDone,
	}; // enum class HSState

	using PeerIdVrfyCallback =
		std::function<void(const sgx_dh_session_enclave_identity_t&)>;

public:

	LaResponder(
		PeerIdVrfyCallback peerIdVrfyCallback = PeerIdVrfyCallback()
	) :
		m_session(),
		m_state(HSState::Initial),
		m_aek(),
		m_peerId(),
		m_peerIdVrfyCallback(std::move(peerIdVrfyCallback))
	{
		sgx_status_t sgxRet =
			sgx_dh_init_session(SGX_DH_SESSION_RESPONDER, &m_session);
		DECENTENCLAVE_CHECK_SGX_RUNTIME_ERROR(
			sgxRet,
			sgx_dh_init_session
		);
	}


	LaResponder(const LaResponder&) = delete;


	// LCOV_EXCL_START
	virtual ~LaResponder() = default;
	// LCOV_EXCL_STOP


	sgx_dh_msg1_t GenMsg1()
	{
		sgx_dh_msg1_t msg1;
		std::memset(&msg1, 0, sizeof(msg1));
		sgx_status_t sgxRet =
			sgx_dh_responder_gen_msg1(&msg1, &m_session);
		DECENTENCLAVE_CHECK_SGX_RUNTIME_ERROR(
			sgxRet,
			sgx_dh_responder_gen_msg1
		);

		m_state = HSState::Msg1Sent;

		return msg1;
	}


	sgx_dh_msg3_t ProcMsg2(const sgx_dh_msg2_t& msg2)
	{
		sgx_key_128bit_t aek;
		sgx_dh_msg3_t msg3;
		std::memset(&msg3, 0, sizeof(msg3));
		sgx_status_t sgxRet =
			sgx_dh_responder_proc_msg2(
				&msg2,
				&msg3,
				&m_session,
				&aek,
				&m_peerId
			);
		static_assert(
			sizeof(aek) == RetKeyType::sk_itemCount,
			"Key size mismatch."
		);
		std::memcpy(m_aek.data(), aek, sizeof(aek));
		memset_s(aek, sizeof(aek), 0, sizeof(aek));
		DECENTENCLAVE_CHECK_SGX_RUNTIME_ERROR(
			sgxRet,
			sgx_dh_responder_proc_msg2
		);

		if (m_peerIdVrfyCallback)
		{
			m_peerIdVrfyCallback(m_peerId);
		}

		m_state = HSState::HandshakeDone;

		return msg3;
	}


	virtual RetKeyType GetSecretKey() const override
	{
		static constexpr const char sk_label[] = "SK";

		return Common::Sgx::Ckdf<
			mbedTLScpp::CipherType::AES,
			128,
			mbedTLScpp::CipherMode::ECB
		>(CtnFullR(m_aek), sk_label);
	}


	virtual RetKeyType GetMaskKey() const override
	{
		static constexpr const char sk_label[] = "MK";

		return Common::Sgx::Ckdf<
			mbedTLScpp::CipherType::AES,
			128,
			mbedTLScpp::CipherMode::ECB
		>(CtnFullR(m_aek), sk_label);
	}


	virtual bool IsHandshakeDone() const override
	{
		return m_state == HSState::HandshakeDone;
	}


	virtual void HandshakeStep(
		Common::Internal::SysIO::StreamSocketBase& sock
	) override
	{
		switch (m_state)
		{
		case HSState::Initial:
		{
			sgx_dh_msg1_t msg1 = GenMsg1();

			sock.SendPrimitive(msg1);

			return;
		}
		case HSState::Msg1Sent:
		{
			sgx_dh_msg2_t peerMsg2 = sock.RecvPrimitive<sgx_dh_msg2_t>();

			sgx_dh_msg3_t msg3 = ProcMsg2(peerMsg2);

			sock.SendPrimitive(msg3);

			return;
		}
		default:
			throw Common::Exception(
				"LaResponder::HandshakeStep - "
				"Invalid handshake state"
			);
		}
	}


private:

	sgx_dh_session_t m_session;
	HSState m_state;
	RetKeyType m_aek;
	sgx_dh_session_enclave_identity_t m_peerId;
	PeerIdVrfyCallback m_peerIdVrfyCallback;

}; // class LaResponder


} // namespace Sgx
} // namespace Trusted
} // namespace DecentEnclave

#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
