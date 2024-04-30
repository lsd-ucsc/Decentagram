// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <mutex>
#include <limits>
#include <unordered_map>

#include <SimpleObjects/Internal/make_unique.hpp>

#include "../Common/Exceptions.hpp"
#include "../Common/Internal/SimpleObj.hpp"


namespace DecentEnclave
{
namespace Trusted
{


template<typename _CallbackFuncType>
class UntrustedAsyncEventHandler
{
public: // Static members:

	using CallbackFuncType = _CallbackFuncType;
	using IDType = uint64_t;

public:
	UntrustedAsyncEventHandler() :
		m_counterMutex(),
		m_counter(0),
		m_callbackMap()
	{}


	~UntrustedAsyncEventHandler() = default;


	IDType RegisterCallback(CallbackFuncType callback)
	{
		std::lock_guard<std::mutex> lock(m_counterMutex);

		IDType id = AfterLockGetAvailableID();
		m_callbackMap.emplace(id, std::move(callback));

		return id;
	}


	template<typename ... _Args>
	void DispatchCallback(IDType id, bool dispose, _Args&& ... args)
	{
		std::unique_ptr<CallbackFuncType> callback;

		{
			std::lock_guard<std::mutex> lock(m_counterMutex);

			// retrieve callback function
			auto it = m_callbackMap.find(id);
			if (it == m_callbackMap.end())
			{
				throw Common::Exception("Callback ID is not registered.");
			}

			using namespace Common::Internal::Obj::Internal;

			if (dispose)
			{
				// move the callback function out of the map
				// and dispose the callback entry
				callback = make_unique<CallbackFuncType>(
					std::move(it->second)
				);
				m_callbackMap.erase(it);
			}
			else
			{
				// copy the callback function out of the map
				// and keep the callback entry
				callback = make_unique<CallbackFuncType>(
					it->second
				);
			}
		}
		// release the lock

		// call the callback function
		(*callback)(std::forward<_Args>(args)...);
	}


private:

	IDType AfterLockGetAvailableID()
	{
		if (m_callbackMap.size() >= std::numeric_limits<IDType>::max())
		{
			throw Common::Exception("Too many callbacks are registered.");
		}

		auto it = m_callbackMap.find(m_counter);
		while(it != m_callbackMap.end())
		{
			// this ID is already taken, try next one.
			++m_counter;
			it = m_callbackMap.find(m_counter);
		}

		// return the available ID
		// and increment to the next possibly available ID.
		return m_counter++;
	}

	std::mutex m_counterMutex;
	IDType m_counter;
	std::unordered_map<IDType, CallbackFuncType> m_callbackMap;

}; // class UntrustedAsyncEventHandler


} // namespace Trusted
} // namespace DecentEnclave
