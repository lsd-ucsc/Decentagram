// Copyright (c) 2022 SimpleSysIO
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <functional>
#include <memory>

#include "StreamSocketBase.hpp"


#ifndef SIMPLESYSIO_CUSTOMIZED_NAMESPACE
namespace SimpleSysIO
#else
namespace SIMPLESYSIO_CUSTOMIZED_NAMESPACE
#endif
{

class StreamAcceptorBase
{
public: // static members:

	using AsyncAcceptCallback =
		std::function<void(std::unique_ptr<StreamSocketBase>, bool)>;

public:


	StreamAcceptorBase() = default;

	// LCOV_EXCL_START
	virtual ~StreamAcceptorBase() = default;
	// LCOV_EXCL_STOP


	/**
	 * @brief Accept a new connection
	 *        NOTE: this function will block until a new connection is accepted,
	 *        or an error occurs
	 *
	 * @return a unique pointer to the newly accepted socket
	 */
	virtual std::unique_ptr<StreamSocketBase> Accept() = 0;


	/**
	 * @brief Accept a new connection asynchronously
	 *
	 * @param callback the callback function to be called when a new connection
	 *                 is accepted, or an error occurs
	 */
	virtual void AsyncAccept(AsyncAcceptCallback callback) = 0;


}; // class StreamSocketServerBase

} // namespace SimpleSysIO
