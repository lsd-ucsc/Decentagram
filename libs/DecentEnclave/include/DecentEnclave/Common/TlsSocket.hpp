// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstddef>
#include <cstdint>

#include <memory>
#include <vector>

#include <mbedTLScpp/Tls.hpp>
#include <SimpleObjects/Internal/make_unique.hpp>
#include <SimpleSysIO/StreamSocketBase.hpp>

#include "Exceptions.hpp"
#include "Internal/SimpleObj.hpp"
#include "Internal/SimpleSysIO.hpp"


namespace DecentEnclave
{
namespace Common
{


namespace Internal
{

class TlsNonblockingSocket
{
public: // static members:

	using UnderlyingType = SysIO::StreamSocketBase;

public:

	TlsNonblockingSocket(
		std::unique_ptr<UnderlyingType> socket,
		bool asyncMode = false
	) :
		m_socket(std::move(socket)),
		m_asyncMode(asyncMode),
		m_recvBuf(),
		m_recvBufPos(0),
		m_recvAsyncRequested(0)
	{}

	~TlsNonblockingSocket() = default;

	int Send(const unsigned char *buf, size_t len)
	{
		return static_cast<int>(
			SysIO::StreamSocketRaw::Send(*m_socket, buf, len)
		);
	}

	int Recv(unsigned char *buf, size_t len)
	{
		if (!m_recvBuf.empty())
		{
			// The recv buffer is not empty
			// consume the buffer first
			size_t copyLen = std::min(len, m_recvBuf.size() - m_recvBufPos);
			std::copy(
				m_recvBuf.begin() + m_recvBufPos,
				m_recvBuf.begin() + m_recvBufPos + copyLen,
				buf
			);
			m_recvBufPos += copyLen;

			// if the buffer is all consumed, clear the buffer
			if (m_recvBufPos >= m_recvBuf.size())
			{
				m_recvBuf.clear();
				m_recvBufPos = 0;
			}

			// return the number of bytes consumed
			return static_cast<int>(copyLen);
		}
		else if (!m_asyncMode)
		{
			// The recv buffer is empty
			// and the socket is not in async mode (blocking mode)
			return static_cast<int>(
				SysIO::StreamSocketRaw::Recv(*m_socket, buf, len)
			);
		}
		else
		{
			// The recv buffer is empty
			// and the socket is in async mode
			m_recvAsyncRequested = len;
			return MBEDTLS_ERR_SSL_WANT_READ;
		}
	}

	void SetAsyncMode(bool asyncMode = true)
	{
		m_asyncMode = asyncMode;
	}

	void ResetRecvBuf(std::vector<uint8_t> buf)
	{
		m_recvBuf.swap(buf);
		m_recvBufPos = 0;
	}

	UnderlyingType& GetUnderlyingSocket()
	{
		return *m_socket;
	}

	size_t GetRecvAsyncRequested() const
	{
		return m_recvAsyncRequested;
	}

private:

	std::unique_ptr<UnderlyingType> m_socket;
	bool m_asyncMode;

	std::vector<uint8_t> m_recvBuf;
	size_t m_recvBufPos;
	size_t m_recvAsyncRequested;
}; // class TlsNonblockingSocket


class TlsSocketWrapper
{
public: // static members:

	using WrappedType = TlsNonblockingSocket;

public:
	TlsSocketWrapper(
		std::shared_ptr<WrappedType> socket
	) :
		m_socket(std::move(socket))
	{}

	~TlsSocketWrapper() = default;

	int Send(const unsigned char *buf, size_t len)
	{
		return m_socket->Send(buf, len);
	}

	int Recv(unsigned char *buf, size_t len)
	{
		return m_socket->Recv(buf, len);
	}

private:

	std::shared_ptr<WrappedType> m_socket;
}; // class TlsSocketWrapper


inline void TlsRecvOrAsyncRecv(
	std::shared_ptr<mbedTLScpp::Tls<TlsSocketWrapper> > tls,
	std::shared_ptr<TlsNonblockingSocket> socket,
	size_t bufSize,
	typename SysIO::StreamSocketBase::AsyncRecvCallback callback
)
{
	std::vector<uint8_t> buf(bufSize);
	int recvRet = tls->RecvData(buf.data(), buf.size());
	if (recvRet >= 0)
	{
		// Received data
		buf.resize(static_cast<size_t>(recvRet));
		callback(std::move(buf), false);
	}
	else if (recvRet == MBEDTLS_ERR_SSL_WANT_READ)
	{
		// Need to receive more data
		size_t tlsRequested = socket->GetRecvAsyncRequested();

		std::weak_ptr<mbedTLScpp::Tls<TlsSocketWrapper> > tlsWeak = tls;
		std::weak_ptr<TlsNonblockingSocket> socketWeak = socket;
		auto& underlyingSocket = socket->GetUnderlyingSocket();

		SysIO::StreamSocketRaw::AsyncRecv(
			underlyingSocket,
			tlsRequested,
			[
				socketWeak,
				tlsWeak,
				bufSize,
				callback
			](std::vector<uint8_t> buf, bool hasErrorOccurred)
			{
				auto tls = tlsWeak.lock();
				auto socket = socketWeak.lock();
				if (
					!hasErrorOccurred &&
					tls != nullptr &&
					socket != nullptr
				)
				{
					socket->ResetRecvBuf(std::move(buf));
					TlsRecvOrAsyncRecv(tls, socket, bufSize, callback);
				}
				else
				{
					callback(std::vector<uint8_t>(), true);
				}
			}
		);
	}
	else
	{
		// Error occurred
		callback(std::vector<uint8_t>(), true);
	}
}

} // namespace Internal


class TlsSocket:
	public Internal::SysIO::StreamSocketBase
{
public: //static members:

	using Base = Internal::SysIO::StreamSocketBase;
	using SharedSocketType = Internal::TlsNonblockingSocket;
	using TlsType = mbedTLScpp::Tls<Internal::TlsSocketWrapper>;

public:
	TlsSocket(
		std::shared_ptr<const mbedTLScpp::TlsConfig> tlsConfig,
		std::shared_ptr<const mbedTLScpp::TlsSession> session,
		std::unique_ptr<Internal::SysIO::StreamSocketBase> socket
	) :
		m_socket(
			std::make_shared<SharedSocketType>(std::move(socket))
		),
		m_tls(
			std::make_shared<TlsType>(
				std::move(tlsConfig),
				std::move(session),
				Internal::Obj::Internal::make_unique<
					Internal::TlsSocketWrapper
				>(m_socket)
			)
		)
	{}

	~TlsSocket() = default;


	virtual size_t SendRaw(const void* buf, size_t len) override
	{
		return static_cast<size_t>(m_tls->SendData(buf, len));
	}


	virtual size_t RecvRaw(void* buf, size_t len) override
	{
		m_socket->SetAsyncMode(false);
		int tlsRet = m_tls->RecvData(buf, len);
		return tlsRet >= 0 ?
			static_cast<size_t>(tlsRet) :
			throw Exception(
				"TlsSocket::RecvRaw - Underlying socket is in incorrect state"
			);
	}

	virtual void AsyncRecvRaw(
		size_t bufSize,
		typename Base::AsyncRecvCallback callback
	) override
	{
		m_socket->SetAsyncMode(true);
		Internal::TlsRecvOrAsyncRecv(
			m_tls,
			m_socket,
			bufSize,
			std::move(callback)
		);
	}

private:

	std::shared_ptr<SharedSocketType> m_socket;
	std::shared_ptr<TlsType> m_tls;

}; // class TlsSocket


} // namespace Common
} // namespace DecentEnclave
