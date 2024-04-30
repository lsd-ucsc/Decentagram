// Copyright (c) 2022 SimpleConcurrency
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

#include "Task.hpp"


#ifndef SIMPLECONCURRENCY_CUSTOMIZED_NAMESPACE
namespace SimpleConcurrency
#else
namespace SIMPLECONCURRENCY_CUSTOMIZED_NAMESPACE
#endif
{
namespace Threading
{


class TaskRunner
{
public:
	TaskRunner() :
		m_taskROMutex(),
		m_taskNRMutex(),
		m_taskCV(),
		m_task(),
		m_isTerminated(false),
		m_isTerminating(false),
		m_isThreadTaskFinished(false)
	{}

	// LCOV_EXCL_START
	virtual ~TaskRunner()
	{
		// terminate the task
		TerminateTask();
	}
	// LCOV_EXCL_STOP


	template<typename _FinishedCallback>
	void ThreadRunner(_FinishedCallback finishCallback)
	{
		while(!m_isTerminating)
		{
			// wait until there is a task to run
			std::unique_lock<std::mutex> roLock(m_taskROMutex);
			// roLock ensure other threads has read only access to m_task
			// wait release the lock, so other threads can write to m_task
			m_taskCV.wait(
				roLock,
				[this]() {
					return
						(m_task != nullptr) ||
						m_isTerminating;
				}
			);

			// mutex is locked again here
			// roLock ensure other threads has read only access to m_task

			if (!m_isTerminating)
			{
				try
				{
					// It's not terminating, so it must be a task to run
					RunThreadTask(); // m_task is accessed here
				}
				catch(...)
				{
					// `RunThreadTask` should handle the exception already,
					// so we don't handle exception here

					// it is going to throw, so it must be terminating
					m_isTerminated = true;
					throw;
				}

				// ======
				// this task is finished, notify the caller,
				// and try to get a new task
				// =====-

				std::unique_ptr<Task> tmpTask;
				{
					std::lock_guard<std::mutex> nrLock(m_taskNRMutex);
					// nrLock ensure other threads has no read access
					tmpTask = std::move(m_task);
				}

				try
				{
					tmpTask = finishCallback(
						this, std::move(tmpTask)
					);
				}
				catch(...)
				{
					m_isTerminated = true;
					throw;
				}

				{
					std::lock_guard<std::mutex> nrLock(m_taskNRMutex);
					// nrLock ensure other threads has no read access
					m_task = std::move(tmpTask);
				}

				// new task is assigned
				m_isThreadTaskFinished = false;

				// m_task is updated above
				// if the task is nullptr, we will try to wait for a new task
				// in the next loop
				// otherwise, instead of waiting, we will run the new task
				// in the next loop
			}

			// back to waiting for new task
		}

		// exit the while loop, so it must be terminating
		m_isTerminated = true;
	}


	void TerminateTask()
	{
		// first let other thread know that it's terminating
		m_isTerminating = true;
		// in case the other thread is waiting for a task, notify it
		m_taskCV.notify_all();
		// in case the thread is already running the task, terminate it
		if (m_task)
		{
			// a peek at the task shows there might be a task running
			std::lock_guard<std::mutex> lock(m_taskNRMutex);
			// nrLock ensure this thread gets read only access to m_task
			// have to lock the mutex to make sure the task is still there
			// before terminating it
			if (m_task)
			{
				m_task->Terminate();
			}
		}
	}


	void AssignTask(std::unique_ptr<Task> task)
	{
		{
			std::lock_guard<std::mutex> lock(m_taskROMutex);
			// roLock ensure other threads has read only access to m_task
			// the mutex is locked by this thread
			// (i.e., not locked by other thread)
			// so the other thread must be waiting for a task
			std::lock_guard<std::mutex> nrLock(m_taskNRMutex);
			// nrLock ensure other threads has no read access

			// assign the task
			m_task = std::move(task);
		}

		// notify the other thread that there is a task to run
		m_taskCV.notify_all();
	}


	bool IsTerminated() const
	{
		return m_isTerminated;
	}


protected:


	void RunThreadTask()
	{
		if (m_task)
		{
			try
			{
				m_task->Run();
			}
			catch(...)
			{
				m_isThreadTaskFinished = true;

				m_task->OnException(std::current_exception());
			}
		}
		m_isThreadTaskFinished = true;
	}


private:

	mutable std::mutex m_taskROMutex;
	mutable std::mutex m_taskNRMutex;
	mutable std::condition_variable m_taskCV;
	std::unique_ptr<Task> m_task;
	std::atomic_bool m_isTerminated;
	std::atomic_bool m_isTerminating;
	std::atomic_bool m_isThreadTaskFinished;

}; // class TaskRunner


} // namespace Threading
} // namespace SimpleConcurrency
