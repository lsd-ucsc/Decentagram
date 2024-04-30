// Copyright (c) 2022 SimpleSysIO
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include "../Config.hpp"


#ifdef SIMPLESYSIO_ENABLE_SYSCALL_NETWORKING


#include "../StreamAcceptorBase.hpp"

#include <memory>
#include <mutex>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "../Exceptions.hpp"
#include "TCPSocket.hpp"


#ifndef SIMPLESYSIO_CUSTOMIZED_NAMESPACE
namespace SimpleSysIO
#else
namespace SIMPLESYSIO_CUSTOMIZED_NAMESPACE
#endif
{


namespace SysCall
{


class TCPAcceptor : virtual public StreamAcceptorBase
{
public: // static members:


	/**
	 * @brief Create a TCP acceptor that is neither opened nor bound to
	 *        any local endpoint
	 *
	 * @return A unique pointer to the created acceptor
	 */
	static std::unique_ptr<TCPAcceptor> Create(
		std::shared_ptr<boost::asio::io_service> ioService
	)
	{
		return std::unique_ptr<TCPAcceptor>(
			new TCPAcceptor(std::move(ioService))
		);
	}


	/**
	 * @brief Create and bind a TCP acceptor to a local endpoint
	 *
	 * @param endpoint The local endpoint to bind to
	 * @return A unique pointer to the bound acceptor
	 */
	static std::unique_ptr<TCPAcceptor> Bind(
		boost::asio::ip::tcp::endpoint endpoint,
		std::shared_ptr<boost::asio::io_service> ioService =
			std::make_shared<boost::asio::io_service>()
	)
	{
		auto acceptor = Create(std::move(ioService));
		acceptor->m_acceptor.open(endpoint.protocol());
		acceptor->m_acceptor.bind(endpoint);
		acceptor->m_acceptor.listen();
		return acceptor;
	}


	static std::unique_ptr<TCPAcceptor> Bind(
		boost::asio::ip::address_v4 ip,
		uint16_t port,
		std::shared_ptr<boost::asio::io_service> ioService =
			std::make_shared<boost::asio::io_service>()
	)
	{
		return Bind(
			boost::asio::ip::tcp::endpoint(ip, port),
			std::move(ioService)
		);
	}


	static std::unique_ptr<TCPAcceptor> Bind(
		boost::asio::ip::address_v6 ip,
		uint16_t port,
		std::shared_ptr<boost::asio::io_service> ioService =
			std::make_shared<boost::asio::io_service>()
	)
	{
		return Bind(
			boost::asio::ip::tcp::endpoint(ip, port),
			std::move(ioService)
		);
	}


	static std::unique_ptr<TCPAcceptor> BindV4(
		const std::string& ipv4,
		uint16_t port,
		std::shared_ptr<boost::asio::io_service> ioService =
			std::make_shared<boost::asio::io_service>()
	)
	{
		return Bind(
			boost::asio::ip::address_v4::from_string(ipv4),
			port,
			std::move(ioService)
		);
	}


	static std::unique_ptr<TCPAcceptor> BindV6(
		const std::string& ipv6,
		uint16_t port,
		std::shared_ptr<boost::asio::io_service> ioService =
			std::make_shared<boost::asio::io_service>()
	)
	{
		return Bind(
			boost::asio::ip::address_v6::from_string(ipv6),
			port,
			std::move(ioService)
		);
	}


	struct AsyncAcceptHandler
	{
		std::unique_ptr<TCPSocket> m_socket;
		AsyncAcceptCallback m_callback;

		AsyncAcceptHandler(
			std::unique_ptr<TCPSocket> socket,
			AsyncAcceptCallback callback
		) :
			m_socket(std::move(socket)),
			m_callback(std::move(callback))
		{}

		~AsyncAcceptHandler() = default;

		static void Handler(
			std::shared_ptr<AsyncAcceptHandler> handler,
			const boost::system::error_code& error
		)
		{
			if (!error)
			{
				handler->m_socket->SetDefaultOptions();
				handler->m_callback(std::move(handler->m_socket), false);
			}
			else
			{
				handler->m_callback(std::move(handler->m_socket), true);
			}
		}
	}; // struct AsyncAcceptHandler


public:


	// LCOV_EXCL_START
	virtual ~TCPAcceptor() = default;
	// LCOV_EXCL_STOP


	virtual std::unique_ptr<TCPSocket> TCPAccept()
	{
		auto socket = TCPSocket::Create(m_ioService);
		m_acceptor.accept(socket->m_socket);
		socket->SetDefaultOptions();
		return socket;
	}


	virtual std::unique_ptr<StreamSocketBase> Accept() override
	{
		return TCPAccept();
	}


	virtual uint16_t GetLocalPort() const
	{
		return m_acceptor.local_endpoint().port();
	}


	virtual void AsyncAccept(AsyncAcceptCallback callback) override
	{
		auto asyncSocket = TCPSocket::Create(m_ioService);
		std::shared_ptr<AsyncAcceptHandler> handler =
			std::make_shared<AsyncAcceptHandler>(
				std::move(asyncSocket),
				std::move(callback)
			);

		m_acceptor.async_accept(
			handler->m_socket->m_socket,
			std::bind(
				&AsyncAcceptHandler::Handler,
				handler,
				std::placeholders::_1
			)
		);
	}


	virtual void AsyncCancel() //override
	{
		m_acceptor.cancel();
	}


protected:


	TCPAcceptor(std::shared_ptr<boost::asio::io_service> ioService) :
		StreamAcceptorBase(),
		m_ioService(std::move(ioService)),
		m_acceptor(*m_ioService)
	{}


private:


	std::shared_ptr<boost::asio::io_service> m_ioService;
	boost::asio::ip::tcp::acceptor m_acceptor;


}; // class TCPAcceptor


} // namespace SysCall
} // namespace SimpleSysIO

#endif // SIMPLESYSIO_ENABLE_SYSCALL_NETWORKING
