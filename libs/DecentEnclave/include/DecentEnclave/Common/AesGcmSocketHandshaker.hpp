// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <vector>

#include <mbedTLScpp/SKey.hpp>
#include <SimpleSysIO/StreamSocketBase.hpp>

#include "Internal/SimpleSysIO.hpp"


namespace DecentEnclave
{
namespace Common
{


template<size_t _keyBitSize>
class AesGcmSocketHandshaker
{
public: // static members:

	static constexpr size_t sk_keyBitSize = _keyBitSize;
	static constexpr size_t sk_keyByteSize = sk_keyBitSize / 8;

	using RetKeyType = mbedTLScpp::SKey<sk_keyBitSize>;

public:

	AesGcmSocketHandshaker() = default;
	// LCOV_EXCL_START
	virtual ~AesGcmSocketHandshaker() = default;
	// LCOV_EXCL_STOP


	virtual RetKeyType GetSecretKey() const = 0;
	virtual RetKeyType GetMaskKey() const = 0;


	virtual bool IsHandshakeDone() const = 0;


	virtual void HandshakeStep(Internal::SysIO::StreamSocketBase& sock) = 0;
	virtual void Handshake(Internal::SysIO::StreamSocketBase& sock)
	{
		while (!IsHandshakeDone())
		{
			HandshakeStep(sock);
		}
	}

}; // class AesGcmSocketHandshaker


} // namespace Common
} // namespace DecentEnclave
