// Copyright (c) 2022 mbedTLScpp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include "ObjectBase.hpp"

#include <mbedtls/ssl_ticket.h>

#include "CipherBase.hpp"
#include "Common.hpp"
#include "Exceptions.hpp"
#include "RandInterfaces.hpp"
#include "TlsSessTktMgrIntf.hpp"


#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{

/**
 * @brief TLS Session Ticket Manager object allocator.
 *
 */
struct TlsSessTktMgrObjAllocator : DefaultAllocBase
{
	typedef mbedtls_ssl_ticket_context      CObjType;

	using DefaultAllocBase::NewObject;
	using DefaultAllocBase::DelObject;

	static void Init(CObjType* ptr)
	{
		return mbedtls_ssl_ticket_init(ptr);
	}

	static void Free(CObjType* ptr) noexcept
	{
		return mbedtls_ssl_ticket_free(ptr);
	}
}; // struct TlsSessTktMgrObjAllocator


/**
 * @brief TLS Session Ticket Manager object trait.
 *
 */
using DefaultTlsSessTktMgrObjTrait = ObjTraitBase<TlsSessTktMgrObjAllocator,
											false,
											false>;

template<
	CipherType _cipherType,
	size_t _bitSize,
	CipherMode _cipherMode,
	uint32_t _tktLifetime
>
class TlsSessTktMgr :
	public ObjectBase<DefaultTlsSessTktMgrObjTrait>,
	public TlsSessTktMgrIntf
{
public: // Static members:

	using TlsSessTktMgrObjTrait = DefaultTlsSessTktMgrObjTrait;
	using _Base                 = ObjectBase<TlsSessTktMgrObjTrait>;

	static constexpr mbedtls_cipher_type_t sk_mbedCipherType =
		GetMbedTlsCipherType(
			GetCipherSizedType(_cipherType, _bitSize),
			_cipherMode
		);

	static constexpr uint32_t              sk_tktLifetime    =
		_tktLifetime;

public:

	TlsSessTktMgr(
		std::unique_ptr<RbgInterface> rand
	) :
		_Base::ObjectBase(),
		m_rand(std::move(rand))
	{
		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			TlsSessTktMgr::TlsSessTktMgr,
			mbedtls_ssl_ticket_setup,
			NonVirtualGet(),
			&RbgInterface::CallBack, m_rand.get(),
			sk_mbedCipherType,
			sk_tktLifetime
		);
	}

	/**
	 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
	 *
	 * @exception None No exception thrown
	 * @param rhs The other TlsSessTktMgr instance.
	 */
	TlsSessTktMgr(TlsSessTktMgr&& rhs) noexcept :
		_Base::ObjectBase(std::forward<_Base>(rhs)), //noexcept
		m_rand(std::move(rhs.m_rand)) //noexcept
	{}

	TlsSessTktMgr(const TlsSessTktMgr& rhs) = delete;

	// LCOV_EXCL_START
	virtual ~TlsSessTktMgr() = default;
	// LCOV_EXCL_STOP

	/**
	 * @brief Move assignment. The `rhs` will be empty/null afterwards.
	 *
	 * @exception None No exception thrown
	 * @param rhs The other TlsSessTktMgr instance.
	 * @return TlsSessTktMgr& A reference to this instance.
	 */
	TlsSessTktMgr& operator=(TlsSessTktMgr&& rhs) noexcept
	{
		_Base::operator=(std::forward<_Base>(rhs)); //noexcept

		if (this != &rhs)
		{
			m_rand = std::move(rhs.m_rand); //noexcept
		}

		return *this;
	}

	TlsSessTktMgr& operator=(const TlsSessTktMgr& other) = delete;

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
		_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(TlsSessTktMgr));
	}

	virtual bool IsNull() const noexcept override
	{
		return _Base::IsNull() || (m_rand.get() == nullptr);
	}

	using _Base::NullCheck;
	using _Base::Get;
	using _Base::NonVirtualGet;
	using _Base::Swap;

	/**
	 * @brief Parses the binary data into TLS session.
	 *
	 * @param session  The reference to the mbedTls SSL session object to be written.
	 * @param buf      Start of the binary buffer containing the ticket.
	 * @param len      Length of the ticket.
	 *
	 */
	virtual void Parse(
		mbedtls_ssl_session& session,
		uint8_t* buf,
		size_t len
	) override
	{
		NullCheck();

		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			TlsSessTktMgr::Parse,
			mbedtls_ssl_ticket_parse,
			Get(),
			&session,
			buf,
			len
		);
	}

	/**
	 * @brief Writes TLS session into TLS session ticket.
	 *
	 * @param session  The reference to the mbedTls SSL session object.
	 * @param start    Start of the output buffer.
	 * @param end      End of the output buffer.
	 * @param tlen     On exit, holds the length written.
	 * @param lifetime On exit, holds the lifetime of the ticket in seconds.
	 *
	 */
	virtual void Write(
		const mbedtls_ssl_session& session,
		void* start,
		const void* end,
		size_t& tlen,
		uint32_t& lifetime
	) override
	{
		NullCheck();

		MBEDTLSCPP_MAKE_C_FUNC_CALL(
			TlsSessTktMgr::Write,
			mbedtls_ssl_ticket_write,
			Get(), &session,
			static_cast<unsigned char*>(start),
			static_cast<const unsigned char*>(end),
			&tlen, &lifetime
		);
	}

private:
	std::unique_ptr<RbgInterface> m_rand;
}; // class TlsSessTktMgr

} // namespace mbedTLScpp
