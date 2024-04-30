// Copyright (c) 2022 SimpleConcurrency
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <exception>


#ifndef SIMPLECONCURRENCY_CUSTOMIZED_NAMESPACE
namespace SimpleConcurrency
#else
namespace SIMPLECONCURRENCY_CUSTOMIZED_NAMESPACE
#endif
{
namespace Threading
{


class Task
{
public:
	Task() = default;

	// LCOV_EXCL_START
	virtual ~Task() = default;
	// LCOV_EXCL_STOP


	/**
	 * @brief The function to be executed in thread.
	 *
	 */
	virtual void Run() = 0;


	/**
	 * @brief The function to be executed in main thread,
	 *        after the `Run` function is finished.
	 *
	 */
	virtual void Finishing()
	{}


	/**
	 * @brief The function to terminate the `Run` function, if it is running.
	 *        This function is called in main thread when the main program is
	 *        about to exit.
	 *
	 */
	virtual void Terminate() = 0;


	/**
	 * @brief This function will be called when an exception is caught by the
	 *        task runner.
	 *        NOTE: the default behavior is to ignore the exception, so that
	 *        the task runner will not result in termination state.
	 *        Don't rethrow the exception, unless you want to stop the task
	 *        runner as well.
	 *
	 */
	virtual void OnException(std::exception_ptr)
	{}


}; // class Task


} // namespace Threading
} // namespace SimpleConcurrency
