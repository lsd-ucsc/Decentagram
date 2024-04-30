// Copyright (c) 2022 SimpleConcurrency
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <atomic>

#include "Task.hpp"


#ifndef SIMPLECONCURRENCY_CUSTOMIZED_NAMESPACE
namespace SimpleConcurrency
#else
namespace SIMPLECONCURRENCY_CUSTOMIZED_NAMESPACE
#endif
{
namespace Threading
{


template<typename _TimeType>
class TickingTask :
	public Task
{
public: // static members:

	using TimeType = _TimeType;

public:

	TickingTask() :
		m_isTerminating(false),
		m_enableTickInterval(false),
		m_updInterval(0),
		m_tickInterval(0),
		m_sinceLastUpd(0)
	{}


	TickingTask(TimeType updInterval, TimeType tickInterval) :
		m_isTerminating(false),
		m_enableTickInterval(true),
		m_updInterval(updInterval),
		m_tickInterval(tickInterval),
		m_sinceLastUpd(tickInterval) // tick immediately for the first time
	{}


	// LCOV_EXCL_START
	virtual ~TickingTask() = default;
	// LCOV_EXCL_STOP


	virtual void Run() override
	{
		while (!m_isTerminating)
		{
			if (!m_enableTickInterval)
			{
				// Tick without delay
				Tick();
			}
			else
			{
				// Tick after every `m_tickInterval` time interval
				if (m_sinceLastUpd >= m_tickInterval)
				{
					// update time has accumulated up to the tick interval
					Tick();
					ResetSinceLastUpd(); // m_sinceLastUpd = 0;
				}

				SleepFor(m_updInterval);
				UpdateSinceLastUpd(); // m_sinceLastUpd += m_updInterval;
			}
		}
	}


	virtual void Terminate() override
	{
		m_isTerminating = true;
	}


protected:

	virtual void Tick() = 0;


	virtual void SleepFor(TimeType time) const = 0;


	virtual void ResetSinceLastUpd()
	{
		m_sinceLastUpd = 0;
	}


	virtual void UpdateSinceLastUpd()
	{
		m_sinceLastUpd += m_updInterval;
	}


	void SetInterval(TimeType updInterval, TimeType tickInterval)
	{
		m_enableTickInterval = true;
		m_updInterval = updInterval;
		m_tickInterval = tickInterval;

		m_sinceLastUpd = tickInterval; // tick immediately for the first time
	}


	void DisableTickInterval()
	{
		m_enableTickInterval = false;
	}


	bool IsTickIntervalEnabled() const
	{
		return m_enableTickInterval;
	}


private:

	std::atomic_bool m_isTerminating;
	bool m_enableTickInterval;
	TimeType m_updInterval;
	TimeType m_tickInterval;
	TimeType m_sinceLastUpd;

}; // class TickingTask


} // namespace Threading
} // namespace SimpleConcurrency
