// Copyright (c) 2022 SimpleSysIO
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include "../Config.hpp"


#ifdef SIMPLESYSIO_ENABLE_SYSCALL_NETWORKING


#include "../StreamSocketBase.hpp"

#include <memory>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>


#ifndef SIMPLESYSIO_CUSTOMIZED_NAMESPACE
namespace SimpleSysIO
#else
namespace SIMPLESYSIO_CUSTOMIZED_NAMESPACE
#endif
{

namespace SysCall
{

class TCPSocket : virtual public StreamSocketBase
{
public: // static members:


	friend class TCPAcceptor;


	/**
	 * @brief create a TCP socket that is neither opened, connected to any remote
	 *        endpoint nor bound (accepted) to any local endpoint
	 *
	 * @return A unique pointer to the created socket
	 */
	static std::unique_ptr<TCPSocket> Create(
		std::shared_ptr<boost::asio::io_service> ioService
	)
	{
		return std::unique_ptr<TCPSocket>(new TCPSocket(std::move(ioService)));
	}

	/**
	 * @brief Create and connect a TCP socket to a remote endpoint
	 *
	 * @param endpoint The remote endpoint to connect to
	 * @param ioService The io_service to use for asynchronous operations
	 *                  NOTE: If this parameter is not specified or a nullptr,
	 *                  a new io_service will be created and used
	 * @return A unique pointer to the connected socket
	 */
	static std::unique_ptr<TCPSocket> Connect(
		boost::asio::ip::tcp::endpoint endpoint,
		std::shared_ptr<boost::asio::io_service> ioService =
			std::make_shared<boost::asio::io_service>()
	)
	{
		if (ioService == nullptr)
		{
			ioService = std::make_shared<boost::asio::io_service>();
		}
		auto socket = Create(std::move(ioService));
		socket->m_socket.connect(endpoint);
		socket->SetDefaultOptions();
		return socket;
	}


	static std::unique_ptr<TCPSocket> Connect(
		boost::asio::ip::address_v4 ip,
		uint16_t port,
		std::shared_ptr<boost::asio::io_service> ioService =
			std::make_shared<boost::asio::io_service>()
	)
	{
		return Connect(
			boost::asio::ip::tcp::endpoint(ip, port),
			std::move(ioService)
		);
	}


	static std::unique_ptr<TCPSocket> Connect(
		boost::asio::ip::address_v6 ip,
		uint16_t port,
		std::shared_ptr<boost::asio::io_service> ioService =
			std::make_shared<boost::asio::io_service>()
	)
	{
		return Connect(
			boost::asio::ip::tcp::endpoint(ip, port),
			std::move(ioService)
		);
	}


	static std::unique_ptr<TCPSocket> ConnectV4(
		const std::string& ipv4,
		uint16_t port,
		std::shared_ptr<boost::asio::io_service> ioService =
			std::make_shared<boost::asio::io_service>()
	)
	{
		return Connect(
			boost::asio::ip::address_v4::from_string(ipv4),
			port,
			std::move(ioService)
		);
	}


	static std::unique_ptr<TCPSocket> ConnectV6(
		const std::string& ipv6,
		uint16_t port,
		std::shared_ptr<boost::asio::io_service> ioService =
			std::make_shared<boost::asio::io_service>()
	)
	{
		return Connect(
			boost::asio::ip::address_v6::from_string(ipv6),
			port,
			std::move(ioService)
		);
	}


	struct AsyncRecvHandler
	{
		std::vector<uint8_t> m_buffer;
		AsyncRecvCallback m_callback;

		AsyncRecvHandler(
			size_t bufferSize,
			AsyncRecvCallback callback
		) :
			m_buffer(bufferSize, 0),
			m_callback(std::move(callback))
		{}

		~AsyncRecvHandler() = default;

		static void Handler(
			std::shared_ptr<AsyncRecvHandler> handler,
			const boost::system::error_code& error,
			size_t bytesTransferred
		)
		{
			handler->m_buffer.resize(bytesTransferred);
			if (!error)
			{
				handler->m_callback(std::move(handler->m_buffer), false);
			}
			else
			{
				handler->m_callback(std::move(handler->m_buffer), true);
			}
		}
	}; // struct AsyncRecvHandler


public:


	virtual ~TCPSocket() = default;


	/**
	 * @brief Set default options on the opened socket
	 *        NOTE: Exception will be thrown if the socket is not opened
	 *        NOTE: this function should be called automatically
	 *              by `Connect()` and `Accept()`
	 *
	 * @exception boost::wrapexcept<boost::system::system_error> Thrown when
	 *            this function is called while this socket is not opened
	 */
	virtual void SetDefaultOptions()
	{
		m_socket.set_option(boost::asio::ip::tcp::no_delay(true));
	}


protected:


	TCPSocket(std::shared_ptr<boost::asio::io_service> ioService) :
		StreamSocketBase(),
		m_ioService(std::move(ioService)),
		m_socket(*m_ioService)
	{}


	virtual size_t SendRaw(const void* data, size_t size) override
	{
		return m_socket.send(boost::asio::buffer(data, size));
	}


	virtual size_t RecvRaw(void* data, size_t size) override
	{
		return m_socket.receive(boost::asio::buffer(data, size));
	}


	virtual void AsyncRecvRaw(
		size_t buffSize,
		AsyncRecvCallback callback
	) override
	{
		std::shared_ptr<AsyncRecvHandler> handler =
			std::make_shared<AsyncRecvHandler>(
				buffSize,
				std::move(callback)
			);

		m_socket.async_receive(
			boost::asio::buffer(
				handler->m_buffer.data(),
				handler->m_buffer.size()
			),
			std::bind(
				&AsyncRecvHandler::Handler,
				handler,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
	}


private:


	std::shared_ptr<boost::asio::io_service> m_ioService;
	boost::asio::ip::tcp::socket m_socket;


}; // class TCPSocket

} // namespace SysCall
} // namespace SimpleSysIO

#endif // SIMPLESYSIO_ENABLE_SYSCALL_NETWORKING
