// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <SimpleObjects/Exception.hpp>

#include "Internal/SimpleObjects.hpp"

#ifndef SIMPLERLP_CUSTOMIZED_NAMESPACE
namespace SimpleRlp
#else
namespace SIMPLERLP_CUSTOMIZED_NAMESPACE
#endif
{

/**
 * @brief Parent class of all SimpleRlp exceptions
 *
 */
class Exception : public Internal::Obj::Exception
{
public:

	using Internal::Obj::Exception::Exception;

	// LCOV_EXCL_START
	virtual ~Exception() = default;
	// LCOV_EXCL_STOP

}; // class Exception


/**
 * @brief This exception is thrown when error occurred during parsing.
 */
class ParseError : public Exception
{
public:

	static std::string ConErrorMsg(
		const std::string& issue, size_t bytePos)
	{
		return ("Parse error - " + issue) +
			" (byte @ " + std::to_string(bytePos + 1) + ")";
	}

public:

	/**
	 * @brief Construct a new Parse Error exception
	 *
	 * @param issue    What was the issue?
	 * @param bytePos  Where the parse error happened in terms of byte position
	 */
	ParseError(const std::string& issue, size_t bytePos) :
		Exception(ConErrorMsg(issue, bytePos)),
		m_bytePos(bytePos)
	{}

	/**
	 * @brief Construct a new Parse Error exception
	 *
	 * @param issue    What was the issue?
	 */
	explicit ParseError(const std::string& issue) :
		Exception(issue),
		m_bytePos(0)
	{}

	// LCOV_EXCL_START
	/**
	 * @brief Destroy the ParseError object
	 *
	 */
	virtual ~ParseError() = default;
	// LCOV_EXCL_STOP

	size_t GetBytePos() const noexcept
	{
		return m_bytePos;
	}

private:

	size_t m_bytePos;
}; // class ParseError


/**
 * @brief This exception is thrown when error occurred during writing object to
 *        RLP bytes
 */
class SerializeError : public Exception
{

public:

	explicit SerializeError(const std::string& errMsg) :
		Exception(errMsg)
	{}

	// LCOV_EXCL_START
	/**
	 * @brief Destroy the SerializeError object
	 *
	 */
	virtual ~SerializeError() = default;
	// LCOV_EXCL_STOP

}; // class SerializeError


/**
 * @brief This exception is thrown when error occurred during writing object to
 *        RLP bytes
 */
class SerializeTypeError : public SerializeError
{

public:

	explicit SerializeTypeError(const std::string& typeName) :
		SerializeError("Cannot serialize type " + typeName + " into RLP")
	{}

	// LCOV_EXCL_START
	/**
	 * @brief Destroy the SerializeTypeError object
	 *
	 */
	virtual ~SerializeTypeError() = default;
	// LCOV_EXCL_STOP

}; // class SerializeTypeError


} // namespace SimpleRlp
