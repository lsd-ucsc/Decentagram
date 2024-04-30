// Copyright (c) 2022 SimpleConcurrency
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <atomic>
#include <memory>

#include "Task.hpp"


#ifndef SIMPLECONCURRENCY_CUSTOMIZED_NAMESPACE
namespace SimpleConcurrency
#else
namespace SIMPLECONCURRENCY_CUSTOMIZED_NAMESPACE
#endif
{
namespace Threading
{


template<
	typename _ThreadLambda,
	typename _FinishingLambda,
	typename _TerminateLambda,
	typename _ExceptionLambda
>
class LambdaTask :
	public Task
{
public:
	LambdaTask(
		_ThreadLambda threadLambda,
		_FinishingLambda finishingLambda,
		_TerminateLambda terminateLambda,
		_ExceptionLambda exceptionLambda
	) :
		m_isTerminated(false),
		m_threadLambda(threadLambda),
		m_finishingLambda(finishingLambda),
		m_terminateLambda(terminateLambda),
		m_exceptionLambda(exceptionLambda)
	{}

	// LCOV_EXCL_START
	virtual ~LambdaTask() = default;
	// LCOV_EXCL_STOP


	virtual void Run() override
	{
		m_threadLambda(std::cref(m_isTerminated));
	}


	virtual void Finishing() override
	{
		m_finishingLambda();
	}


	virtual void Terminate() override
	{
		m_isTerminated = true;
		m_terminateLambda();
	}


	virtual void OnException(std::exception_ptr ePtr) override
	{
		m_exceptionLambda(ePtr);
	}


private:
	std::atomic_bool m_isTerminated;
	_ThreadLambda m_threadLambda;
	_FinishingLambda m_finishingLambda;
	_TerminateLambda m_terminateLambda;
	_ExceptionLambda m_exceptionLambda;

}; // class LambdaTask


template<
	typename _ThreadLambda,
	typename _FinishingLambda,
	typename _TerminateLambda,
	typename _ExceptionLambda
>
std::unique_ptr<Task> MakeLambdaTask(
	_ThreadLambda threadLambda,
	_FinishingLambda finishingLambda,
	_TerminateLambda terminateLambda,
	_ExceptionLambda exceptionLambda
)
{
	using _LambdaTaskType = LambdaTask<
		_ThreadLambda,
		_FinishingLambda,
		_TerminateLambda,
		_ExceptionLambda
	>;

	std::unique_ptr<_LambdaTaskType> lTask(
		new _LambdaTaskType(
			threadLambda,
			finishingLambda,
			terminateLambda,
			exceptionLambda
		)
	);

	return lTask;
}


template<
	typename _ThreadLambda,
	typename _FinishingLambda,
	typename _TerminateLambda
>
std::unique_ptr<Task> MakeLambdaTask(
	_ThreadLambda threadLambda,
	_FinishingLambda finishingLambda,
	_TerminateLambda terminateLambda
)
{
	return MakeLambdaTask(
		threadLambda,
		finishingLambda,
		terminateLambda,
		[](std::exception_ptr) {}
	);
}


template<
	typename _ThreadLambda,
	typename _FinishingLambda
>
std::unique_ptr<Task> MakeLambdaTask(
	_ThreadLambda threadLambda,
	_FinishingLambda finishingLambda
)
{
	return MakeLambdaTask(
		threadLambda,
		finishingLambda,
		[]() {}
	);
}


template<
	typename _ThreadLambda
>
std::unique_ptr<Task> MakeLambdaTask(
	_ThreadLambda threadLambda
)
{
	return MakeLambdaTask(
		threadLambda,
		[]() {},
		[]() {}
	);
}


} // namespace Threading
} // namespace SimpleConcurrency
