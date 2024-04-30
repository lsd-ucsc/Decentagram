// Copyright (c) 2022 SimpleSysIO
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstddef>
#include <cstdint>

#include <functional>
#include <type_traits>
#include <vector>

#include <SimpleObjects/RealNumCast.hpp>

#include "Endianness.hpp"


#ifndef SIMPLESYSIO_CUSTOMIZED_NAMESPACE
namespace SimpleSysIO
#else
namespace SIMPLESYSIO_CUSTOMIZED_NAMESPACE
#endif
{


struct StreamSocketRaw;


class StreamSocketBase
{
public: //static members:

	using EndianType = Internal::Obj::Endian;

	using AsyncRecvCallback = std::function<void(std::vector<uint8_t>, bool)>;

	friend struct StreamSocketRaw;
	friend struct StreamSocketAsync;

public:
	StreamSocketBase() = default;


	// LCOV_EXCL_START
	virtual ~StreamSocketBase() = default;
	// LCOV_EXCL_STOP


	/**
	 * @brief Send bytes stored in the container to the peer.
	 *        NOTE: This function will block until all data stored in the given
	 *        container is sent, or an error occurs.
	 *
	 * @tparam _ContainerType The type of the container
	 * @param data The container storing the data to be sent
	 */
	template<typename _ContainerType>
	void SendBytes(const _ContainerType& data)
	{
		using _ValueType = typename _ContainerType::value_type;
		static_assert(std::is_trivially_copyable<_ValueType>::value,
			"Container value type must be trivially copyable");
		static_assert(sizeof(_ValueType) == 1,
			"Container value type must be byte-sized");

		SendRawUntilComplete(
			data.data(),
			data.size() * sizeof(_ValueType)
		);
	}


	/**
	 * @brief Receive bytes from the peer and stores it in the container.
	 *        NOTE:  This function will block and receive data until the
	 *               container is filled to `dataSize`, or an error occurs.
	 *
	 * @tparam _ContainerType The type of the container
	 * @param dataSize The size of the data to be received
	 * @return The container storing the received data
	 */
	template<typename _ContainerType>
	_ContainerType RecvBytes(size_t dataSize)
	{
		using _ValueType = typename _ContainerType::value_type;
		static_assert(std::is_trivially_copyable<_ValueType>::value,
			"Container value type must be trivially copyable");
		static_assert(sizeof(_ValueType) == 1,
			"Container value type must be byte-sized");

		_ContainerType res;
		res.resize(dataSize);

		RecvRawUntilComplete(
			&(res[0]),
			res.size() * sizeof(_ValueType)
		);

		return res;
	}


	/**
	 * @brief Receive bytes from the peer and stores it in the container.
	 *        NOTE:  This function will block. However, it may receive no data,
	 *        some data, or data up to the `maxSize`, or an error occurs.
	 *
	 * @tparam _ContainerType The type of the container
	 * @param maxSize The maximum size of the data to be received
	 * @return The container storing the received data
	 */
	template<typename _ContainerType>
	_ContainerType RecvSomeBytes(size_t maxSize)
	{
		using _ValueType = typename _ContainerType::value_type;
		static_assert(std::is_trivially_copyable<_ValueType>::value,
			"Container value type must be trivially copyable");
		static_assert(sizeof(_ValueType) == 1,
			"Container value type must be byte-sized");

		_ContainerType res;
		res.resize(maxSize);

		size_t recvSize = RecvRaw(
			&(res[0]),
			res.size() * sizeof(_ValueType)
		);
		res.resize(recvSize);

		return res;
	}


	/**
	 * @brief Send some primitive data, such as `int`, `float`, etc. to the
	 *        peer; this function should also work for POD (plain old data)
	 *        types.
	 *        NOTE: this function will block until all data is sent,
	 *        or an error occurs
	 *
	 * @tparam _T The type of the given data
	 * @tparam _TransmitEndian The endianness used during transmission in
	 *                         the socket
	 * @param data The data to be sent
	 */
	template<
		typename _T,
		EndianType _TransmitEndian = EndianType::little
	>
	void SendPrimitive(const _T& data)
	{
		static_assert(std::is_trivially_copyable<_T>::value,
			"Primitive value type must be trivially copyable");

		_T dataToSend = Internal::EndianConvert<
			EndianType::native,
			_TransmitEndian
		>::Primitive(data);

		SendRawUntilComplete(
			&dataToSend,
			sizeof(_T)
		);
	}


	/**
	 * @brief Receive some primitive data, such as `int`, `float`, etc. from
	 *        the peer; this function should also work for POD (plain old
	 *        data) types.
	 *        NOTE: this function will block until all data is received,
	 *        or an error occurs
	 *
	 * @tparam _T The type of the data to be received
	 * @tparam _TransmitEndian The endianness used during transmission in
	 *                         the socket
	 * @return The data received
	 */
	template<
		typename _T,
		EndianType _TransmitEndian = EndianType::little
	>
	_T RecvPrimitive()
	{
		static_assert(std::is_trivially_copyable<_T>::value,
			"Primitive value type must be trivially copyable");

		_T dataRecv;
		RecvRawUntilComplete(
			&dataRecv,
			sizeof(_T)
		);

		return Internal::EndianConvert<
			_TransmitEndian,
			EndianType::native
		>::Primitive(dataRecv);
	}


	/**
	 * @brief Send the size of the container first, and then send the bytes
	 *        stored in the container to the peer.
	 *        NOTE: This function will block until all data is sent.
	 *        NOTE: This function is built ON TOP OF the stream protocol, since
	 *        it sends size of the container first followed by the data.
	 *
	 * @tparam _ContainerType The type of the container
	 * @tparam _SizeType The type of the size value to be sent; the default is
	 *                   `uint64_t` (i.e., 64-bit unsigned integer)
	 * @tparam _TransmitEndian The endianness used during transmission in
	 *                         the socket
	 * @param data The container storing the data to be sent
	 */
	template<
		typename _ContainerType,
		typename _SizeType = uint64_t,
		EndianType _TransmitEndian = EndianType::little
	>
	void SizedSendBytes(const _ContainerType& data)
	{
		_SizeType sizeToSend =
			Internal::Obj::RealNumCast<_SizeType>(data.size());

		SendPrimitive<_SizeType, _TransmitEndian>(sizeToSend);
		SendBytes<_ContainerType>(data);
	}


	/**
	 * @brief Receive the size of the container first, and then receive the
	 *        bytes with the specified size.
	 *        NOTE: This function will block until all data is received.
	 *        NOTE: This function is built ON TOP OF the stream protocol, since
	 *        it receive the size of the container first followed by the data.
	 *
	 * @tparam _ContainerType The type of the container
	 * @tparam _SizeType The type of the size value to be received;
	 *                   must be the same as the one sent by the peer;
	 *                   the default is `uint64_t`
	 *                   (i.e., 64-bit unsigned integer)
	 * @return The container storing the received data
	 */
	template<
		typename _ContainerType,
		typename _SizeType = uint64_t,
		EndianType _TransmitEndian = EndianType::little
	>
	_ContainerType SizedRecvBytes()
	{
		_SizeType sizeToRecv = RecvPrimitive<_SizeType, _TransmitEndian>();

		size_t dataSize = Internal::Obj::RealNumCast<size_t>(sizeToRecv);
		_ContainerType res = RecvBytes<_ContainerType>(dataSize);

		return res;
	}


	/**
	 * @brief The very basic interface to receive data asynchronously
	 *
	 * @param buffSize The size of the intermediate buffer used to store
	 *                 received data
	 *                 The child class implementation should allocate this
	 *                 buffer internally and pass it to the callback function
	 * @param callback The callback function to be called when the data is
	 *                 received
	 */
	virtual void AsyncRecvRaw(size_t buffSize, AsyncRecvCallback callback) = 0;


	virtual void AsyncRecvRawUntilComplete(
		size_t expSize,
		AsyncRecvCallback callback
	)
	{
		AsyncRecvRawUntilCompleteImpl implCallbackFunctor(
			this,
			expSize,
			std::move(callback),
			std::make_shared<std::vector<uint8_t> >()
		);
		AsyncRecvCallback implCallback = std::move(implCallbackFunctor);

		AsyncRecvRaw(expSize, std::move(implCallback));
	}

	template<
		typename _ContainerType,
		typename _SizeType = uint64_t,
		EndianType _TransmitEndian = EndianType::little
	>
	void AsyncSizedRecvBytes(
		std::function<void(_ContainerType, bool)> callback
	)
	{
		AsyncSizedRecvBytesDataImpl<_ContainerType> dataCallback(
			std::move(callback)
		);
		AsyncSizedRecvBytesSizeImpl<
			_ContainerType,
			_SizeType,
			_TransmitEndian
		> sizeCallback(this, std::move(dataCallback));

		AsyncRecvRawUntilComplete(sizeof(_SizeType), sizeCallback);
	}

protected:


	/**
	 * @brief The very basic interface to send data with the given pointer
	 *        to the memory buffer containing the data to be sent
	 *        and the size of the data.
	 *        NOTE: this function will block until some data is sent (or the
	 *        underlying call return), or an error occurs
	 *
	 * @param data The pointer to the memory buffer containing the data
	 *             to be sent
	 * @param size The size of the data to be sent
	 * @return The number of bytes sent
	 */
	virtual size_t SendRaw(const void* data, size_t size) = 0;


	/**
	 * @brief The very basic interface to send data with the given pointer
	 *        to the memory buffer containing the data to be sent
	 *        and the size of the data.
	 *        NOTE: this function will block until *ALL* data is sent,
	 *        or an error occurs
	 *
	 * @param data
	 * @param size
	 */
	virtual void SendRawUntilComplete(const void* data, size_t size)
	{
		size_t sent = 0;
		while (sent < size)
		{
			sent += SendRaw(
				static_cast<const uint8_t*>(data) + sent,
				size - sent
			);
		}
	}


	/**
	 * @brief The very basic interface to receive data with the given pointer
	 *        to the memory buffer to store the received data and the size of
	 *        that buffer
	 *        NOTE: this function will block until some data is received (or
	 *        underlying call return), or an error occurs
	 *
	 * @param data The pointer to the memory buffer to store the received data
	 * @param size The size of the memory buffer
	 * @return The number of bytes received
	 */
	virtual size_t RecvRaw(void* data, size_t size) = 0;


	/**
	 * @brief The very basic interface to receive data with the given pointer
	 *        to the memory buffer to store the received data and the size of
	 *        that buffer
	 *        NOTE: this function will block until the specified size of data
	 *        is received, or an error occurs
	 *
	 * @param data The pointer to the memory buffer to store the received data
	 * @param size The size of the memory buffer
	 */
	virtual void RecvRawUntilComplete(void* data, size_t size)
	{
		size_t recv = 0;
		while (recv < size)
		{
			recv += RecvRaw(
				static_cast<uint8_t*>(data) + recv,
				size - recv
			);
		}
	}

private:

	struct AsyncRecvRawUntilCompleteImpl
	{
		AsyncRecvRawUntilCompleteImpl(
			StreamSocketBase* socket,
			size_t expSize,
			AsyncRecvCallback callback,
			std::shared_ptr<std::vector<uint8_t> > cached
		) :
			m_socket(socket),
			m_expSize(expSize),
			m_callback(std::move(callback)),
			m_cached(std::move(cached))
		{}

		AsyncRecvRawUntilCompleteImpl(AsyncRecvRawUntilCompleteImpl&& other) :
			m_socket(std::move(other.m_socket)),
			m_expSize(std::move(other.m_expSize)),
			m_callback(std::move(other.m_callback)),
			m_cached(std::move(other.m_cached))
		{}

		AsyncRecvRawUntilCompleteImpl(
			const AsyncRecvRawUntilCompleteImpl& other
		) :
			m_socket(other.m_socket),
			m_expSize(other.m_expSize),
			m_callback(other.m_callback),
			m_cached(other.m_cached)
		{}

		void operator()(std::vector<uint8_t> buf, bool hasErrorOccurred)
		{
			if (!hasErrorOccurred)
			{
				m_cached->insert(
					m_cached->end(),
					std::make_move_iterator(buf.begin()),
					std::make_move_iterator(buf.end())
				);

				if (m_cached->size() < m_expSize)
				{
					// we need to receive more data

					AsyncRecvRawUntilCompleteImpl nextCallbackFunctor(
						m_socket,
						m_expSize,
						m_callback,
						m_cached
					);
					AsyncRecvCallback nextCallback =
						std::move(nextCallbackFunctor);

					m_socket->AsyncRecvRaw(
						m_expSize - m_cached->size(),
						std::move(nextCallback)
					);
				}
				else
				{
					// we have received enough data
					m_callback(std::move(*m_cached), false);
				}
			}
			else
			{
				// error occurred
				m_callback(std::vector<uint8_t>(), true);
			}
		}

		StreamSocketBase* m_socket;
		size_t m_expSize;
		AsyncRecvCallback m_callback;
		std::shared_ptr<std::vector<uint8_t> > m_cached;
	}; // struct AsyncRecvRawUntilCompleteImpl

	template<typename _ContainerType>
	struct AsyncSizedRecvBytesDataImpl
	{
		using CallbackType = std::function<void(_ContainerType, bool)>;
		using DestValType = typename _ContainerType::value_type;
		static constexpr size_t sk_destValSize = sizeof(DestValType);

		AsyncSizedRecvBytesDataImpl(
			CallbackType callback
		):
			m_callback(std::move(callback))
		{}

		AsyncSizedRecvBytesDataImpl(AsyncSizedRecvBytesDataImpl&& other) :
			m_callback(std::move(other.m_callback))
		{}

		AsyncSizedRecvBytesDataImpl(
			const AsyncSizedRecvBytesDataImpl& other
		) :
			m_callback(other.m_callback)
		{}

		void operator()(std::vector<uint8_t> buf, bool hasErrorOccurred)
		{
			if (!hasErrorOccurred)
			{
				size_t destSize =
					(buf.size() + sk_destValSize - 1) / sk_destValSize;

				_ContainerType data;
				data.resize(destSize);

				std::memcpy(&(data[0]), buf.data(), buf.size());

				m_callback(data, hasErrorOccurred);
			}
			else
			{
				// error occurred or socket has been closed
				m_callback(_ContainerType(), true);
			}
		}

		CallbackType m_callback;
	}; // struct AsyncSizedRecvBytesDataImpl

	template<
		typename _ContainerType,
		typename _SizeType = uint64_t,
		EndianType _TransmitEndian = EndianType::little
	>
	struct AsyncSizedRecvBytesSizeImpl
	{
		using DataCallbackType = AsyncSizedRecvBytesDataImpl<_ContainerType>;

		AsyncSizedRecvBytesSizeImpl(
			StreamSocketBase* socket,
			DataCallbackType dataCallback
		):
			m_socket(socket),
			m_dataCallback(std::move(dataCallback))
		{}

		AsyncSizedRecvBytesSizeImpl(AsyncSizedRecvBytesSizeImpl&& other) :
			m_socket(std::move(other.m_socket)),
			m_dataCallback(std::move(other.m_dataCallback))
		{}

		AsyncSizedRecvBytesSizeImpl(
			const AsyncSizedRecvBytesSizeImpl& other
		) :
			m_socket(other.m_socket),
			m_dataCallback(other.m_dataCallback)
		{}

		void operator()(std::vector<uint8_t> buf, bool hasErrorOccurred)
		{
			if (!hasErrorOccurred)
			{
				_SizeType size = 0;
				std::memcpy(&size, buf.data(), sizeof(_SizeType));

				// Convert endianness from transmit --> native
				size = Internal::EndianConvert<
					_TransmitEndian,
					EndianType::native
				>::Primitive(size);

				m_socket->AsyncRecvRawUntilComplete(
					size,
					m_dataCallback
				);
			}
			else
			{
				// error occurred or socket has been closed
				m_dataCallback(std::vector<uint8_t>(), true);
			}
		}

		StreamSocketBase* m_socket;
		DataCallbackType m_dataCallback;
	}; // struct AsyncSizedRecvBytesSizeImpl

}; // class StreamSocketBase


struct StreamSocketRaw
{

static size_t Send(StreamSocketBase& sock, const void* data, size_t size)
{
	return sock.SendRaw(data, size);
}

static size_t Recv(StreamSocketBase& sock, void* buf, size_t size)
{
	return sock.RecvRaw(buf, size);
}

static void AsyncRecv(
	StreamSocketBase& sock,
	size_t buffSize,
	typename StreamSocketBase::AsyncRecvCallback callback
)
{
	sock.AsyncRecvRaw(buffSize, std::move(callback));
}

}; // struct StreamSocketRaw


} // namespace SimpleSysIO
