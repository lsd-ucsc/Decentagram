#pragma once

#include "ObjectBase.hpp"

#include <mbedtls/ssl.h>

#include "Common.hpp"
#include "Exceptions.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	/**
	 * @brief TLS Session object allocator.
	 *
	 */
	struct TlsSessionObjAllocator : DefaultAllocBase
	{
		typedef mbedtls_ssl_session      CObjType;

		using DefaultAllocBase::NewObject;
		using DefaultAllocBase::DelObject;

		static void Init(CObjType* ptr)
		{
			return mbedtls_ssl_session_init(ptr);
		}

		static void Free(CObjType* ptr) noexcept
		{
			return mbedtls_ssl_session_free(ptr);
		}
	};

	/**
	 * @brief TLS Session object trait.
	 *
	 */
	using DefaultTlsSessionObjTrait = ObjTraitBase<TlsSessionObjAllocator,
											 false,
											 false>;

	class TlsSession : public ObjectBase<DefaultTlsSessionObjTrait>
	{
	public: // Static members:

		using TlsSessionObjTrait = DefaultTlsSessionObjTrait;
		using _Base              = ObjectBase<TlsSessionObjTrait>;

	public:

		TlsSession() :
			_Base::ObjectBase()
		{}

		/**
		 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
		 *
		 * @exception None No exception thrown
		 * @param rhs The other TlsSession instance.
		 */
		TlsSession(TlsSession&& rhs) noexcept :
			_Base::ObjectBase(std::forward<_Base>(rhs)) //noexcept
		{}

		TlsSession(const TlsSession& rhs) = delete;

		// LCOV_EXCL_START
		virtual ~TlsSession() = default;
		// LCOV_EXCL_STOP

		/**
		 * @brief Move assignment. The `rhs` will be empty/null afterwards.
		 *
		 * @exception None No exception thrown
		 * @param rhs The other TlsSession instance.
		 * @return TlsSession& A reference to this instance.
		 */
		TlsSession& operator=(TlsSession&& rhs) noexcept
		{
			_Base::operator=(std::forward<_Base>(rhs)); //noexcept

			return *this;
		}

		TlsSession& operator=(const TlsSession& other) = delete;

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
			_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(TlsSession));
		}

		using _Base::NullCheck;
		using _Base::Get;
		using _Base::NonVirtualGet;
		using _Base::Swap;
	};
}
