// Copyright (c) 2022 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <DecentEnclave/Common/CertStore.hpp>

#include <mbedTLScpp/X509Cert.hpp>


namespace End2EndLatency
{

DECENTENCLAVE_CERTSTORE_CERT(Secp256r1, ::mbedTLScpp::X509Cert);


DECENTENCLAVE_CERTSTORE_CERT(Secp256k1, ::mbedTLScpp::X509Cert);

} // namespace End2EndLatency
