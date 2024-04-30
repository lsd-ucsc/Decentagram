// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <atomic>
#include <functional>
#include <mutex>
#include <vector>

#include <SimpleSysIO/StreamSocketBase.hpp>

#include "../Common/Internal/SimpleSysIO.hpp"
#include "Time.hpp"


namespace DecentEnclave
{
namespace Trusted
{


enum class HeartbeatStatus : uint8_t
{
	Normal    = 0, // Everything is OK
	Suspended = 1, // The enclave is suspended, until the receiver recovered
	Damaged   = 2, // The enclave is damaged, and cannot be recovered
}; // enum class HeartbeatStatus


template<typename _TimestampType>
class HeartbeatConstraint
{
public: // static members:

	using TimestampType = _TimestampType;

public:

	HeartbeatConstraint() :
		m_lastUpdate(0)
	{}

	// LCOV_EXCL_START
	virtual ~HeartbeatConstraint() = default;
	// LCOV_EXCL_STOP

	void InitTime(
		const TimestampType& currTime
	)
	{
		m_lastUpdate = currTime;
	}

	virtual HeartbeatStatus CheckStatus(
		const TimestampType& currTime
	) const = 0;

	virtual void OnHeartbeatRecv(
		const TimestampType& currTime
	)
	{
		m_lastUpdate = currTime;
	}

protected:

	std::atomic<TimestampType> m_lastUpdate;

}; // class HeartbeatConstraint

template<typename _TimestampType>
class HeartbeatTimeConstraint :
	public HeartbeatConstraint<_TimestampType>
{
public: // static members:

	using Base = HeartbeatConstraint<_TimestampType>;
	using TimestampType = typename Base::TimestampType;

public:

	HeartbeatTimeConstraint(
		TimestampType timeout,
		HeartbeatStatus timeoutStatus = HeartbeatStatus::Damaged
	) :
		Base(),
		m_timeout(timeout),
		m_timeoutStatus(timeoutStatus),
		m_isDamaged(false)
	{}

	// LCOV_EXCL_START
	virtual ~HeartbeatTimeConstraint() = default;
	// LCOV_EXCL_STOP

	virtual HeartbeatStatus CheckStatus(
		const TimestampType& currTime
	) const override
	{
		if (m_isDamaged)
		{
			return HeartbeatStatus::Damaged;
		}

		TimestampType elapsed = currTime - Base::m_lastUpdate;
		if (elapsed > m_timeout)
		{
			// The heartbeat has timed out
			if (m_timeoutStatus == HeartbeatStatus::Damaged)
			{
				// if it should become damaged,
				// then it should be damaged forever
				m_isDamaged = true;
			}
			return m_timeoutStatus;
		}

		return HeartbeatStatus::Normal;
	}

protected:

	TimestampType m_timeout;
	HeartbeatStatus m_timeoutStatus;

	mutable std::atomic_bool m_isDamaged;

}; // class HeartbeatTimeConstraint


class HeartbeatRecvMgr
{
public: // static members:

	using TimestampType = uint64_t;

	using SocketType = Common::Internal::SysIO::StreamSocketBase;
	using SocketPtrType = std::shared_ptr<SocketType>;
	using SocketIdType = std::uintptr_t;

	using ConstraintType = HeartbeatConstraint<TimestampType>;
	using ConstraintPtrType = std::shared_ptr<ConstraintType>;
	using ConstraintIdType = std::uintptr_t;

	using RecvFunc = std::function<void(std::vector<uint8_t>)>;

	using SocketMapType =
		std::unordered_map<SocketIdType, SocketPtrType>;
	using ConstraintMapType =
		std::unordered_map<ConstraintIdType, ConstraintPtrType>;

	static SocketIdType GetSocketId(const SocketPtrType& socket)
	{
		return reinterpret_cast<SocketIdType>(socket.get());
	}

	static ConstraintIdType GetConstraintId(const ConstraintPtrType& constraint)
	{
		return reinterpret_cast<ConstraintIdType>(constraint.get());
	}

	static HeartbeatRecvMgr& GetInstance()
	{
		static HeartbeatRecvMgr s_inst;
		return s_inst;
	}

public:


	~HeartbeatRecvMgr() = default;


	void AddRecv(
		ConstraintPtrType constraint,
		SocketPtrType socket,
		RecvFunc recvFunc,
		bool initConstraint
	)
	{
		// Add the constraint to the constraint map
		AddConstraint(constraint);

		// Add the socket to the socket map
		AddSocket(socket);

		// After this point, we can start waiting for the heartbeat
		// so we can init the timestamp in the constraint
		if (initConstraint)
		{
			constraint->InitTime(GetCurrTimestamp());
		}

		StartWaiting(
			std::move(constraint),
			std::move(socket),
			std::move(recvFunc)
		);
	}


	void RemoveRecv(
		ConstraintIdType constraintId,
		SocketIdType socketId
	)
	{
		RemoveConstraint(constraintId);
		RemoveSocket(socketId);
	}


	void RemoveRecv(
		ConstraintPtrType constraint,
		SocketPtrType socket
	)
	{
		RemoveRecv(GetConstraintId(constraint), GetSocketId(socket));
	}


	HeartbeatStatus GetStatus() const
	{
		if (m_status == HeartbeatStatus::Damaged)
		{
			return HeartbeatStatus::Damaged;
		}

		// retrieve the list of constraints
		std::vector<ConstraintPtrType> constraintList;
		{
			std::lock_guard<std::mutex> lock(m_constraintMapMutex);
			constraintList.reserve(m_constraintMap.size());
			for (const auto& constraintPair : m_constraintMap)
			{
				constraintList.push_back(constraintPair.second);
			}
		}

		// get current timestamp
		const TimestampType currTimestamp = GetCurrTimestamp();

		// original status
		HeartbeatStatus cleanStatus = HeartbeatStatus::Normal;

		// check status
		for (const auto& constraint : constraintList)
		{
			HeartbeatStatus status = constraint->CheckStatus(currTimestamp);
			if (status == HeartbeatStatus::Damaged)
			{
				m_status = HeartbeatStatus::Damaged;
				return status;
			}
			else if (status == HeartbeatStatus::Suspended)
			{
				cleanStatus = HeartbeatStatus::Suspended;
			}
		}

		m_status = cleanStatus;
		return m_status;
	}


private:

	// StartWaiting uses the singleton to remove invalid sockets
	// so we need to make this class singleton only
	HeartbeatRecvMgr() :
		m_constraintMapMutex(),
		m_constraintMap(),
		m_socketMapMutex(),
		m_socketMap(),
		m_status(HeartbeatStatus::Normal)
	{}


	static void StartWaiting(
		ConstraintPtrType constraint,
		SocketPtrType socket,
		RecvFunc recvFunc
	)
	{
		std::weak_ptr<ConstraintType> weakConstraint = constraint;
		std::weak_ptr<SocketType> weakSocket = socket;

		SocketIdType socketId = GetSocketId(socket);

		auto wrappedRecv = [
			weakConstraint,
			weakSocket,
			recvFunc,
			socketId
		](std::vector<uint8_t> msg, bool hasErrorOccurred)
		{
			auto constraint = weakConstraint.lock();
			auto socket = weakSocket.lock();

			if (!hasErrorOccurred &&
				(constraint != nullptr) &&
				(socket != nullptr)
			)
			{
				// record the time when the heartbeat is received
				TimestampType currTimestamp = GetCurrTimestamp();

				// check the heartbeat constraint first
				HeartbeatStatus hStatus = constraint->CheckStatus(currTimestamp);

				if (hStatus != HeartbeatStatus::Damaged)
				{
					// it's not damaged, so we can keep updating
					constraint->OnHeartbeatRecv(GetCurrTimestamp());

					// call the heartbeat handling function
					recvFunc(std::move(msg));

					// start waiting for the next heartbeat
					StartWaiting(
						std::move(constraint),
						std::move(socket),
						std::move(recvFunc)
					);
				}
				else
				{
					HeartbeatRecvMgr::GetInstance().RemoveSocket(socketId);
				}
			}
			else
			{
				HeartbeatRecvMgr::GetInstance().RemoveSocket(socketId);
			}
		};

		using namespace Common::Internal::SysIO;
		socket->AsyncSizedRecvBytes<std::vector<uint8_t> >(
			std::move(wrappedRecv)
		);
	}


	static TimestampType GetCurrTimestamp()
	{
		return UntrustedTime::Timestamp();
	}


	void AddConstraint(
		ConstraintPtrType constraint
	)
	{
		ConstraintIdType constraintId = GetConstraintId(constraint);

		std::lock_guard<std::mutex> lock(m_constraintMapMutex);
		auto it = m_constraintMap.find(constraintId);
		if (it == m_constraintMap.end())
		{
			// the constraint is not in the map
			// add it to the map
			m_constraintMap.emplace(constraintId, constraint);
		}
	}


	void RemoveConstraint(
		ConstraintIdType constraintId
	)
	{
		Common::Platform::Print::StrDebug("Removing constraint: " + std::to_string(constraintId));
		std::lock_guard<std::mutex> lock(m_constraintMapMutex);
		auto it = m_constraintMap.find(constraintId);
		if (it != m_constraintMap.end())
		{
			m_constraintMap.erase(it);
		}
	}


	void AddSocket(
		SocketPtrType socket
	)
	{
		SocketIdType socketId = GetSocketId(socket);

		std::lock_guard<std::mutex> lock(m_socketMapMutex);
		auto it = m_socketMap.find(socketId);
		if (it == m_socketMap.end())
		{
			// the socket is not in the map
			// add it to the map
			m_socketMap.emplace(socketId, socket);
		}
		else
		{
			// the socket is already in the map
			// we don't accept duplicated receiver on the same socket
			throw Common::Exception("The given socket is already in the map");
		}
	}


	void RemoveSocket(
		SocketIdType socketId
	)
	{
		Common::Platform::Print::StrDebug("Removing socket: " + std::to_string(socketId));
		std::lock_guard<std::mutex> lock(m_socketMapMutex);
		auto it = m_socketMap.find(socketId);
		if (it != m_socketMap.end())
		{
			m_socketMap.erase(it);
		}
	}


	mutable std::mutex m_constraintMapMutex;
	ConstraintMapType m_constraintMap;

	mutable std::mutex m_socketMapMutex;
	SocketMapType m_socketMap;

	mutable std::atomic<HeartbeatStatus> m_status;
}; // class HeartbeatRecvMgr


} // namespace Trusted
} // namespace DecentEnclave
