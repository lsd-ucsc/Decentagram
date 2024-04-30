// Copyright (c) 2022 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <DecentEnclave/Trusted/Time.hpp>
#include <EclipseMonitor/PlatformInterfaces.hpp>


namespace EthereumClt
{
namespace Trusted
{


class Timestamper : public EclipseMonitor::TimestamperBase
{
public:


	Timestamper() = default;


	virtual ~Timestamper() = default;


	virtual uint64_t NowInSec() const override
	{
		return DecentEnclave::Trusted::UntrustedTime::Timestamp();
	}

}; // class Timestamper


} // namespace Trusted
} // namespace EthereumClt
