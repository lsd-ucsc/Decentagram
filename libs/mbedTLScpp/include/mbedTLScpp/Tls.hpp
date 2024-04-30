// Copyright (c) 2022 mbedTLScpp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#pragma once


#include "ObjectBase.hpp"

#include <mbedtls/ssl.h>

#include "Common.hpp"
#include "Exceptions.hpp"
#include "TlsConfig.hpp"
#include "TlsSession.hpp"


#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{


/**
 * @brief TLS object allocator.
 *
 */
struct TlsObjAllocator : DefaultAllocBase
{
	typedef mbedtls_ssl_context      CObjType;

	using DefaultAllocBase::NewObject;
	using DefaultAllocBase::DelObject;

	static void Init(CObjType* ptr)
	{
		return mbedtls_ssl_init(ptr);
	}

	static void Free(CObjType* ptr) noexcept
	{
		return mbedtls_ssl_free(ptr);
	}
}; // struct TlsObjAllocator


/**
 * @brief TLS object trait.
 *
 */
using DefaultTlsObjTrait = ObjTraitBase<TlsObjAllocator,
											false,
											false>;


template<typename _ConnType>
class Tls : public ObjectBase<DefaultTlsObjTrait>
{
public: // Static members:

	using ConnType    = _ConnType;
	using TlsObjTrait = DefaultTlsObjTrait;
	using _Base       = ObjectBase<TlsObjTrait>;

	static int SendCallBack(
		void* ctx,
		const unsigned char* buf,
		size_t len
	) noexcept
	{
		if (ctx == nullptr ||
			(len > 0 && buf == nullptr))
		{
			return MBEDTLS_ERR_SSL_BAD_INPUT_DATA;
		}

		try
		{
			Tls* ctxTls = static_cast<Tls*>(ctx);
			ctxTls->NullCheck();
			if(ctxTls->m_conn == nullptr)
			{
				return MBEDTLS_ERR_SSL_BAD_INPUT_DATA;
			}
			else
			{
				return ctxTls->m_conn->Send(buf, len);
			}
		}
		catch (const mbedTLSRuntimeError& e)
		{
			return e.GetErrorCode();
		}
		catch (...)
		{
			return MBEDTLS_ERR_X509_FATAL_ERROR;
		}
	}

	static int RecvCallBack(
		void* ctx,
		unsigned char* buf,
		size_t len
	) noexcept
	{
		if (ctx == nullptr ||
			(len > 0 && buf == nullptr))
		{
			return MBEDTLS_ERR_SSL_BAD_INPUT_DATA;
		}

		try
		{
			Tls* ctxTls = static_cast<Tls*>(ctx);
			ctxTls->NullCheck();
			if(ctxTls->m_conn == nullptr)
			{
				return MBEDTLS_ERR_SSL_BAD_INPUT_DATA;
			}
			else
			{
				return ctxTls->m_conn->Recv(buf, len);
			}
		}
		catch (const mbedTLSRuntimeError& e)
		{
			return e.GetErrorCode();
		}
		catch (...)
		{
			return MBEDTLS_ERR_X509_FATAL_ERROR;
		}
	}

	static int RecvTimeoutCallBack(
		void* ctx,
		unsigned char* buf,
		size_t len,
		uint32_t t
	) noexcept
	{
		if (ctx == nullptr ||
			(len > 0 && buf == nullptr))
		{
			return MBEDTLS_ERR_SSL_BAD_INPUT_DATA;
		}

		try
		{
			Tls* ctxTls = static_cast<Tls*>(ctx);
			ctxTls->NullCheck();
			if(ctxTls->m_conn == nullptr)
			{
				return MBEDTLS_ERR_SSL_BAD_INPUT_DATA;
			}
			else
			{
				return ctxTls->m_conn->RecvTimeout(buf, len, t);
			}
		}
		catch (const mbedTLSRuntimeError& e)
		{
			return e.GetErrorCode();
		}
		catch (...)
		{
			return MBEDTLS_ERR_X509_FATAL_ERROR;
		}
	}

public:

	Tls(
		std::shared_ptr<const TlsConfig> tlsConfig,
		std::shared_ptr<const TlsSession> session,
		std::unique_ptr<ConnType> connForHandshake
	) :
		_Base::ObjectBase(),
		m_tlsConfig(tlsConfig),
		m_conn(std::move(connForHandshake))
	{
		if (m_tlsConfig == nullptr)
		{
			throw InvalidArgumentException(
				"Tls::Tls - TLS config is required for TLS construction."
			);
		}

		m_tlsConfig->NullCheck();

		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			Tls::Tls,
			mbedtls_ssl_setup,
			NonVirtualGet(),
			m_tlsConfig->Get()
		);

		mbedtls_ssl_set_bio(
			NonVirtualGet(),
			this,
			&Tls::SendCallBack,
			&Tls::RecvCallBack,
			nullptr
		);

		if (session)
		{
			session->NullCheck();

			MBEDTLSCPP_MAKE_C_FUNC_CALL(
				Tls::Tls,
				mbedtls_ssl_session_reset,
				NonVirtualGet()
			);

			MBEDTLSCPP_MAKE_C_FUNC_CALL(
				Tls::Tls,
				mbedtls_ssl_set_session,
				NonVirtualGet(),
				session->Get()
			);
		}

		if (m_conn != nullptr)
		{
			MBEDTLSCPP_MAKE_C_FUNC_CALL(
				Tls::Tls,
				mbedtls_ssl_handshake,
				NonVirtualGet()
			);
		}
	}

	/**
	 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
	 *
	 * @exception None No exception thrown
	 * @param rhs The other Tls instance.
	 */
	Tls(Tls&& rhs) noexcept :
		_Base::ObjectBase(std::forward<_Base>(rhs)), //noexcept
		m_tlsConfig(std::move(rhs.m_tlsConfig)),
		m_conn(std::move(rhs.m_conn))
	{
		RecoverBioPtrs(NonVirtualGet());
	}

	Tls(const Tls& rhs) = delete;

	// LCOV_EXCL_START
	virtual ~Tls() = default;
	// LCOV_EXCL_STOP

	/**
	 * @brief Move assignment. The `rhs` will be empty/null afterwards.
	 *
	 * @exception None No exception thrown
	 * @param rhs The other Tls instance.
	 * @return Tls& A reference to this instance.
	 */
	Tls& operator=(Tls&& rhs) noexcept
	{
		_Base::operator=(std::forward<_Base>(rhs)); //noexcept

		if (this != &rhs)
		{
			m_tlsConfig = std::move(rhs.m_tlsConfig);
			m_conn = std::move(rhs.m_conn);

			RecoverBioPtrs(Get());
		}

		return *this;
	}

	Tls& operator=(const Tls& other) = delete;

	/**
	 * @brief Check if the current instance is holding a null pointer for
	 *        the mbedTLS object. If so, exception will be thrown. Helper
	 *        function to be called before accessing the mbedTLS object.
	 *
	 * @exception InvalidObjectException Thrown when the current instance is
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 */
	virtual void NullCheck() const
	{
		_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(Tls));
	}

	virtual bool IsNull() const noexcept override
	{
		return _Base::IsNull() || (m_tlsConfig == nullptr);
	}

	using _Base::NullCheck;
	using _Base::Get;
	using _Base::NonVirtualGet;
	using _Base::Swap;

	void EnableRecvTimeout()
	{
		NullCheck();

		mbedtls_ssl_set_bio(
			Get(),
			this,
			&Tls::SendCallBack,
			&Tls::RecvCallBack,
			&Tls::RecvTimeoutCallBack
		);
	}

	void DisableRecvTimeout()
	{
		NullCheck();

		mbedtls_ssl_set_bio(
			Get(),
			this,
			&Tls::SendCallBack,
			&Tls::RecvCallBack,
			nullptr
		);
	}

	void OverrideAuthmode(bool vrfyPeer)
	{
		NullCheck();

		mbedtls_ssl_set_hs_authmode(
			NonVirtualGet(),
			vrfyPeer ? MBEDTLS_SSL_VERIFY_REQUIRED : MBEDTLS_SSL_VERIFY_NONE
		);
	}

	void UnsetAuthmode()
	{
		NullCheck();

		mbedtls_ssl_set_hs_authmode(
			NonVirtualGet(),
			MBEDTLS_SSL_VERIFY_UNSET
		);
	}

	void Handshake()
	{
		NullCheck();

		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			Tls::Handshake,
			mbedtls_ssl_handshake,
			Get()
		);
	}

	void HandshakeStep()
	{
		NullCheck();

		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			Tls::Handshake,
			mbedtls_ssl_handshake_step,
			Get()
		);
	}

	bool HasHandshakeOver() const
	{
		NullCheck();

		return mbedtls_ssl_is_handshake_over(MutableGet()) != 0;
	}

	int SendData(const void* buf, size_t len)
	{
		NullCheck();
		if(len > 0 && buf == nullptr)
		{
			throw InvalidArgumentException(
				"Tls::SendData - The given buffer address is nullptr."
			);
		}

		int retVal = mbedtls_ssl_write(
			Get(),
			static_cast<const unsigned char*>(buf),
			len
		);

		if (retVal < 0)
		{
			MBEDTLSCPP_THROW_IF_ERROR_CODE_NON_SUCCESS(
				retVal, Tls::Handshake, mbedtls_ssl_write
			);
		}

		return retVal;
	}

	int RecvData(void* buf, size_t len)
	{
		NullCheck();
		if(len > 0 && buf == nullptr)
		{
			throw InvalidArgumentException(
				"Tls::SendData - The given buffer address is nullptr."
			);
		}

		int retVal = mbedtls_ssl_read(
			Get(),
			static_cast<unsigned char*>(buf),
			len
		);

		if (
			(retVal < 0) &&
			(retVal != MBEDTLS_ERR_SSL_WANT_READ)
		)
		{
			MBEDTLSCPP_THROW_IF_ERROR_CODE_NON_SUCCESS(
				retVal, Tls::Handshake, mbedtls_ssl_read
			);
		}

		return retVal;
	}

	TlsSession GetSession() const
	{
		NullCheck();

		TlsSession sess;

		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			Tls::GetSession,
			mbedtls_ssl_get_session,
			Get(), sess.Get()
		);

		return sess;
	}

	template<
		typename _CertType = X509CertBase<BorrowedX509CertTrait>,
		enable_if_t<
			IsCppObjOfCtype<_CertType, mbedtls_x509_crt>::value,
			int
		> = 0
	>
	const _CertType BorrowPeerCert() const
	{
		NullCheck();

		const mbedtls_x509_crt* ptr = mbedtls_ssl_get_peer_cert(Get());
		if (ptr == nullptr)
		{
			throw InvalidArgumentException(
				"Tls::BorrowPeerCert"
				" - Can't get peer's certificate in this TLS context"
				" (Hint: Was peer cert required? Has TLS handshake done?)"
			);
		}

		return _CertType(
			const_cast<mbedtls_x509_crt*>(ptr)
		);
	}

	template<
		typename _CertType = X509Cert,
		enable_if_t<
			IsCppObjOfCtype<_CertType, mbedtls_x509_crt>::value,
			int
		> = 0
	>
	_CertType GetPeerCert() const
	{
		std::vector<uint8_t> borrowedDer = BorrowPeerCert().GetDer();

		return _CertType::FromDER(CtnFullR(borrowedDer));
	}

protected:

	const std::unique_ptr<ConnType>& GetConnPtr() const
	{
		return m_conn;
	}

	std::unique_ptr<ConnType>& GetConnPtr()
	{
		return m_conn;
	}

	void RecoverBioPtrs(mbedtls_ssl_context* ctx)
	{
		if (ctx->MBEDTLS_PRIVATE(f_recv_timeout) != nullptr)
		{
			mbedtls_ssl_set_bio(
				ctx,
				this,
				&Tls::SendCallBack,
				&Tls::RecvCallBack,
				&Tls::RecvTimeoutCallBack
			);
		}
		else
		{
			mbedtls_ssl_set_bio(
				ctx,
				this,
				&Tls::SendCallBack,
				&Tls::RecvCallBack,
				nullptr
			);
		}
	}

private:

	std::shared_ptr<const TlsConfig> m_tlsConfig;
	std::unique_ptr<ConnType> m_conn;

}; // class Tls


} // namespace mbedTLScpp
