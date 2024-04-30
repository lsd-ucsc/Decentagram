// Copyright (c) 2023 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <string>

#include "Config.hpp"


#ifndef ECLIPSEMONITOR_LOGGING_HEADER
	// Logging is disabled
namespace EclipseMonitor
{
namespace Internal
{


class DummyLogger
{
public:

	DummyLogger(const std::string&)
	{}

	~DummyLogger() = default;

	void Debug(const std::string&) const
	{}

	void Info(const std::string&) const
	{}

	void Warn(const std::string&) const
	{}

	void Error(const std::string&) const
	{}
}; // class DummyLogger


struct DummyLoggerFactory
{
	using LoggerType = DummyLogger;

	static LoggerType GetLogger(const std::string& name)
	{
		return LoggerType(name);
	}
}; // struct DummyLoggerFactory


} // namespace Internal


using LoggerFactory = Internal::DummyLoggerFactory;


} // namespace EclipseMonitor

#else // !ECLIPSEMONITOR_LOGGING_HEADER
	// Logging is enabled
#	include ECLIPSEMONITOR_LOGGING_HEADER
#endif // !ECLIPSEMONITOR_LOGGING_HEADER


namespace EclipseMonitor
{

using Logger = typename LoggerFactory::LoggerType;

} // namespace EclipseMonitor
