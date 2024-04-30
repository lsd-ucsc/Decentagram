// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <functional>
#include <mutex>
#include <vector>

#include "../Common/Platform/Print.hpp"


namespace DecentEnclave
{
namespace Trusted
{


class HeartbeatEmitterMgr
{
public: // static members:

	using EmitterFunc = std::function<void()>;
	using EmitterListType = std::vector<EmitterFunc>;

	static HeartbeatEmitterMgr& GetInstance()
	{
		static HeartbeatEmitterMgr inst;
		return inst;
	}

public:

	HeartbeatEmitterMgr() :
		m_emitterListMutex(),
		m_emitterList()
	{}

	~HeartbeatEmitterMgr() = default;


	void AddEmitter(EmitterFunc emitter)
	{
		std::lock_guard<std::mutex> lock(m_emitterListMutex);
		m_emitterList.emplace_back(std::move(emitter));
	}

	void EmitAll()
	{
		// Obtain the list of emitters by swapping the list with an empty list
		// So that other threads can still add new emitters meanwhile
		EmitterListType tmpList;
		{
			std::lock_guard<std::mutex> lock(m_emitterListMutex);
			tmpList.swap(m_emitterList);
		}

		for (auto it = tmpList.begin(); it != tmpList.end();)
		{
			try
			{
				(*it)();
				// If no exception is thrown, then the emitter is still valid
				++it;
			}
			catch (const std::exception& e)
			{
				// If an exception is thrown, then the emitter is no longer valid
				Common::Platform::Print::StrDebug(
					std::string("Exception thrown when emitting heartbeat: ") +
					e.what() +
					"; The emitter will be removed"
				);
				it = tmpList.erase(it);
			}
		}

		// Put the list of valid emitters back to the list
		// and merge with newly added emitters
		{
			std::lock_guard<std::mutex> lock(m_emitterListMutex);
			m_emitterList.swap(tmpList);
			m_emitterList.insert(
				m_emitterList.end(),
				std::make_move_iterator(tmpList.begin()),
				std::make_move_iterator(tmpList.end())
			);
		}
	}

private:

	mutable std::mutex m_emitterListMutex;
	EmitterListType m_emitterList;

}; // class HeartbeatEmitterMgr


} // namespace Trusted
} // namespace DecentEnclave
