// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <string>

#include <SimpleObjects/Codec/Hex.hpp>

#include "../Common/Internal/SimpleObj.hpp"


#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
#include "Sgx/SealKey.hpp"

namespace DecentEnclave
{
namespace Trusted
{

using PlatformIdImpl = Sgx::PlatformId;

} // namespace Trusted
} // namespace DecentEnclave

#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED


namespace DecentEnclave
{
namespace Trusted
{

struct PlatformId : public PlatformIdImpl
{

static std::string GetIdHex()
{
	return Common::Internal::Obj::Codec::Hex::Encode<std::string>(
			GetId()
		);
}

}; // struct PlatformId

} // namespace Trusted
} // namespace DecentEnclave
