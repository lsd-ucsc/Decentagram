// Copyright (c) 2022 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <cstddef>
#include <cstdint>

#include "DataTypes.hpp"


namespace EclipseMonitor
{


class TimestamperBase
{
public:

	TimestamperBase() = default;

	// LCOV_EXCL_START
	virtual ~TimestamperBase() = default;
	// LCOV_EXCL_STOP

	virtual TrustedTimestamp NowInSec() const = 0;

}; // class TimestamperBase



class RandomGeneratorBase
{
public:

	RandomGeneratorBase() = default;

	// LCOV_EXCL_START
	virtual ~RandomGeneratorBase() = default;
	// LCOV_EXCL_STOP

	virtual void GenerateRandomBytes(uint8_t* buf, size_t len) const = 0;

}; // class RandomGeneratorBase


} // namespace EclipseMonitor
