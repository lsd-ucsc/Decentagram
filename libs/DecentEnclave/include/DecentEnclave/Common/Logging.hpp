// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <string>

#include "Platform/Print.hpp"


namespace DecentEnclave
{
namespace Common
{

class Logger
{
public:

	Logger(const std::string& name) :
		m_name(name)
	{}

	~Logger() = default;

	void Debug(const std::string& msg) const
	{
		return Log("DEBUG", msg);
	}

	void Info(const std::string& msg) const
	{
		return Log("INFO", msg);
	}

	void Warn(const std::string& msg) const
	{
		return Log("WARN", msg);
	}

	void Error(const std::string& msg) const
	{
		return Log("ERROR", msg);
	}

private:

	void Log(const std::string& level, const std::string& msg) const
	{
		Platform::Print::Str(
			m_name + "(" + level + "): " + msg + "\n"
		);
	}

	std::string m_name;
}; // class Logger


struct LoggerFactory
{
	using LoggerType = Logger;

	static LoggerType GetLogger(const std::string& name)
	{
		return LoggerType(name);
	}
}; // struct LoggerFactory


} // namespace Common
} // namespace DecentEnclave
