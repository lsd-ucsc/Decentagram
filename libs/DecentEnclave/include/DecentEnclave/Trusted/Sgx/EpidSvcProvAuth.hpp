// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED


#include <sgx_tcrypto.h>


namespace DecentEnclave
{
namespace Trusted
{
namespace Sgx
{


/**
 * @brief An interface for authenticating service provider's public key.
 *
 */
class EpidSvcProvAuth
{
public:

	EpidSvcProvAuth() = default;


	virtual ~EpidSvcProvAuth() = default;


	virtual bool Authenticate(const sgx_ec256_public_t& svcProvKey) = 0;

}; // class EpidSvcProvAuth


/**
 * @brief An implementation of EpidSvcProvAuth that accepts all
 *        service provider's public key.
 *        This is useful for Decent protocol, since the identity of
 *        service provider is not important.
 *
 */
class EpidSvcProvAuthAcceptAll :
	public EpidSvcProvAuth
{
public:

	EpidSvcProvAuthAcceptAll() = default;

	virtual ~EpidSvcProvAuthAcceptAll() = default;

	virtual bool Authenticate(const sgx_ec256_public_t&) override
	{
		return true;
	}

}; // class EpidSvcProvAuthAcceptAll


} // namespace Sgx
} // namespace Trusted
} // namespace DecentEnclave


#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
