// Copyright (c) 2022 SimpleConcurrency
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstddef>

#include <atomic>
#include <condition_variable>
#include <list>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "TaskRunner.hpp"


#ifndef SIMPLECONCURRENCY_CUSTOMIZED_NAMESPACE
namespace SimpleConcurrency
#else
namespace SIMPLECONCURRENCY_CUSTOMIZED_NAMESPACE
#endif
{
namespace Threading
{


class ThreadPool
{
public:
	ThreadPool(size_t poolSize) :
		m_poolSize(poolSize),

		m_terminated(false),

		m_threadsMutex(),
		m_threads(),
		m_threadsSize(0),
		m_busyTaskRunners(),
		//m_idleTaskRunners(),

		m_pendingTasksMutex(),
		m_pendingTasksCV(),
		m_pendingTasksSize(0),
		m_pendingTasks(),

		m_finishTasksQueueMutex(),
		m_finishTasksQueue(),
		m_finishTasksQueueSize(0)
	{}


	// LCOV_EXCL_START
	~ThreadPool()
	{
		// terminate all threads
		Terminate();
	}
	// LCOV_EXCL_STOP


	void Update()
	{
		// check if there are any finished tasks
		while (m_finishTasksQueueSize > 0)
		{
			std::unique_ptr<Task> task;

			// Fetch a finished task
			{
				std::lock_guard<std::mutex> lock(m_finishTasksQueueMutex);

				task = std::move(m_finishTasksQueue.front());
				m_finishTasksQueue.pop();
				--m_finishTasksQueueSize;
			}

			// call finishing function
			task->Finishing();
		}
	}


	void AddTask(std::unique_ptr<Task> task)
	{
		// add task to pending tasks
		{
			std::lock_guard<std::mutex> lock(m_pendingTasksMutex);
			++m_pendingTasksSize;
			m_pendingTasks.push_back(std::move(task));
		}

		// notify a task runner
		m_pendingTasksCV.notify_one();

		if (
			m_pendingTasksSize > 0 &&
			m_threadsSize < m_poolSize
		)
		{
			bool needNotify = false;

			{
				// Task is still pending, so probably there is no idle runner
				// And there is still room for a new thread
				std::lock_guard<std::mutex> lock(m_pendingTasksMutex);
				if (m_pendingTasks.size() > 0)
				{
					std::unique_ptr<Task>& firstTask = m_pendingTasks.front();
					CreateNewThread(firstTask);

					if (firstTask == nullptr)
					{
						// task is moved to the new thread
						--m_pendingTasksSize;
						m_pendingTasks.pop_front();
					}
					else
					{
						// no thread was created; task is still pending
						// we may need to notify again
						needNotify = true;
					}
				}
			}

			if (needNotify)
			{
				m_pendingTasksCV.notify_one();
			}
		}
	}


	void Terminate()
	{
		m_terminated = true;

		m_pendingTasksCV.notify_all();

		std::lock_guard<std::mutex> lock(m_threadsMutex);

		// terminate all task runners
		// for (auto& taskRunner : m_idleTaskRunners)
		// {
		// 	taskRunner->TerminateTask();
		// }
		for (auto& taskRunner : m_busyTaskRunners)
		{
			// repeat function call to help the task runner to terminate
			while(!taskRunner->IsTerminated())
			{
				m_pendingTasksCV.notify_all();
				taskRunner->TerminateTask();
			}
		}

		// join all threads
		for (auto& thread : m_threads)
		{
			thread.join();
		}

		// clear all threads first
		m_threads.clear();

		// now it's safe to clear all task runners
		//m_idleTaskRunners.clear();
		m_busyTaskRunners.clear();
	}


private: // private functions:


	void PushTaskToFinishQueue(std::unique_ptr<Task> task)
	{
		std::lock_guard<std::mutex> lock(m_finishTasksQueueMutex);
		m_finishTasksQueue.push(std::move(task));
		++m_finishTasksQueueSize;
	}


	std::unique_ptr<Task> BlockingFetchPendingTask()
	{
		std::unique_lock<std::mutex> lock(m_pendingTasksMutex);

		// wait for pending tasks
		m_pendingTasksCV.wait(
			lock,
			[this]()
			{
				return (!m_pendingTasks.empty() || m_terminated);
			}
		);

		if (m_terminated)
		{
			return nullptr;
		}
		else
		{
			// fetch a pending task
			--m_pendingTasksSize;
			std::unique_ptr<Task> task = std::move(m_pendingTasks.front());
			m_pendingTasks.pop_front();

			return task;
		}
	}


	std::unique_ptr<Task> OnTaskFinished(
		TaskRunner*,
		std::unique_ptr<Task> task
	)
	{
		// push task to finish queue
		PushTaskToFinishQueue(std::move(task));

		// check / wait for pending tasks
		return BlockingFetchPendingTask();
	}


	void CreateNewThread(std::unique_ptr<Task>& task)
	{
		std::lock_guard<std::mutex> lock(m_threadsMutex);
		// lock threads mutex before doing management job

		if (m_threads.size() >= m_poolSize)
		{
			// pool is full, do nothing
			return;
		}

		// pool is not full, create a new thread
		++m_threadsSize;

		// Create a new task runner, and assign an initial task to it
		std::unique_ptr<TaskRunner> taskRunner(new TaskRunner());
		TaskRunner* taskRunnerPtr = taskRunner.get();
		m_busyTaskRunners.emplace_back(std::move(taskRunner));
		taskRunnerPtr->AssignTask(std::move(task));

		// create a thread and start the task runner
		m_threads.emplace_back(
			[this, taskRunnerPtr]() {
				taskRunnerPtr->ThreadRunner(
					// callback for finished tasks:
					[this](TaskRunner* tr, std::unique_ptr<Task> task)
					{
						return OnTaskFinished(tr, std::move(task));
					}
				);
			}
		);
	}

private:
	size_t m_poolSize;

	std::atomic_bool m_terminated;

	mutable std::mutex m_threadsMutex;
	std::vector<std::thread> m_threads;
	std::atomic_uint64_t m_threadsSize;
	std::vector<std::unique_ptr<TaskRunner> > m_busyTaskRunners;
	//std::vector<std::unique_ptr<TaskRunner> > m_idleTaskRunners;

	mutable std::mutex m_pendingTasksMutex;
	mutable std::condition_variable m_pendingTasksCV;
	std::atomic_uint64_t m_pendingTasksSize;
	std::list<std::unique_ptr<Task> > m_pendingTasks;

	mutable std::mutex m_finishTasksQueueMutex;
	std::queue<std::unique_ptr<Task> > m_finishTasksQueue;
	std::atomic_uint64_t m_finishTasksQueueSize;

}; // class ThreadPool


} // namespace Threading
} // namespace SimpleConcurrency
