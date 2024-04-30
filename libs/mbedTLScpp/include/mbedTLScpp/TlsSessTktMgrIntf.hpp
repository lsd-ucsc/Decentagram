#pragma once

#include <cstddef>

#include <mbedtls/ssl.h>

#include "Exceptions.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	/**
	 * @brief The interface class for TLS Session Ticket Manager.
	 *
	 */
	class TlsSessTktMgrIntf
	{
	public: // Static members:

		/**
		 * @brief Parses callback function, used for giving callback functions to the mbedTLS library
		 *
		 * @param p_ticket The pointer to the SessionTicketMgr object. Must not null.
		 * @param session  The pointer to the mbedTls SSL session object.
		 * @param buf      Start of the buffer containing the ticket.
		 * @param len      Length of the ticket.
		 *
		 * @return mbedTLS errorcode.
		 */
		static int Parse(void *p_ticket, mbedtls_ssl_session *session, unsigned char *buf, size_t len) noexcept
		{
			if (p_ticket == nullptr || session == nullptr ||
				(len > 0 && buf == nullptr))
			{
				return MBEDTLS_ERR_SSL_BAD_INPUT_DATA;
			}

			try
			{
				static_cast<TlsSessTktMgrIntf*>(p_ticket)->Parse(*session, static_cast<uint8_t*>(buf), len);

				return MBEDTLS_EXIT_SUCCESS;
			}
			catch (const mbedTLSRuntimeError& e)
			{
				return e.GetErrorCode();
			}
			catch (...)
			{
				return MBEDTLS_ERR_ERROR_GENERIC_ERROR;
			}
		}

		/**
		 * @brief Writes callback function, used for giving callback functions to the mbedTLS library
		 *
		 * @param p_ticket The pointer to the SessionTicketMgr object. Must not null.
		 * @param session  The pointer to the mbedTls SSL session object.
		 * @param start    Start of the output buffer.
		 * @param end      End of the output buffer.
		 * @param tlen     On exit, holds the length written.
		 * @param lifetime On exit, holds the lifetime of the ticket in seconds.
		 *
		 * @return mbedTLS errorcode.
		 */
		static int Write(void *p_ticket, const mbedtls_ssl_session *session, unsigned char *start, const unsigned char *end, size_t *tlen, uint32_t *lifetime) noexcept
		{
			if (p_ticket == nullptr || session == nullptr ||
				start == nullptr || end == nullptr || end < start ||
				tlen == nullptr || lifetime == nullptr)
			{
				return MBEDTLS_ERR_SSL_BAD_INPUT_DATA;
			}

			try
			{
				static_cast<TlsSessTktMgrIntf*>(p_ticket)->Write(
					*session,
					start,
					end,
					*tlen,
					*lifetime
				);

				return MBEDTLS_EXIT_SUCCESS;
			}
			catch (const mbedTLSRuntimeError& e)
			{
				return e.GetErrorCode();
			}
			catch (...)
			{
				return MBEDTLS_ERR_ERROR_GENERIC_ERROR;
			}
		}

	public:

		TlsSessTktMgrIntf() = default;

		// LCOV_EXCL_START
		virtual ~TlsSessTktMgrIntf() = default;
		// LCOV_EXCL_STOP

		virtual void Parse(mbedtls_ssl_session& session, uint8_t* buf, size_t len) = 0;

		virtual void Write(const mbedtls_ssl_session& session, void* start, const void* end, size_t& tlen, uint32_t& lifetime) = 0;
	};
}
