// Copyright (c) 2023 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <DecentEnclave/Trusted/Sgx/Random.hpp>
#include <EclipseMonitor/PlatformInterfaces.hpp>


namespace EthereumClt
{
namespace Trusted
{


class RandomGenerator : public EclipseMonitor::RandomGeneratorBase
{
public:


	RandomGenerator() = default;


	virtual ~RandomGenerator() = default;


	virtual void GenerateRandomBytes(uint8_t* buf, size_t len) const override
	{
		m_randGen.Rand(buf, len);
	}

private:

	mutable DecentEnclave::Trusted::Sgx::RandGenerator m_randGen;
}; // class RandomGenerator


} // namespace Trusted
} // namespace EthereumClt
