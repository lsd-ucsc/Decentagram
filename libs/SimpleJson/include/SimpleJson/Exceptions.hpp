// Copyright (c) 2022 SimpleJson
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "Internal/SimpleObjects.hpp"

#ifndef SIMPLEJSON_CUSTOMIZED_NAMESPACE
namespace SimpleJson
#else
namespace SIMPLEJSON_CUSTOMIZED_NAMESPACE
#endif
{

/**
 * @brief Parent class of all SimpleJson exceptions
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
		const std::string& issue, size_t lineNum, size_t colNum)
	{
		return ("Parse error - " + issue) +
			" (line " + std::to_string(lineNum + 1) +
			" column " + std::to_string(colNum + 1) + ")";
	}

public:

	/**
	 * @brief Construct a new Parse Error exception
	 *
	 * @param issue    What was the issue?
	 * @param lineNum  Where the parse error happened in terms of line number
	 * @param colNum   Where the parse error happened in terms of colum number
	 */
	ParseError(const std::string& issue, size_t lineNum, size_t colNum) :
		Exception(ConErrorMsg(issue, lineNum, colNum)),
		m_lineNum(lineNum),
		m_colNum(colNum)
	{}

	/**
	 * @brief Construct a new Parse Error exception
	 *
	 * @param issue    What was the issue?
	 */
	explicit ParseError(const std::string& issue) :
		Exception(issue),
		m_lineNum(0),
		m_colNum(0)
	{}

	// LCOV_EXCL_START
	/**
	 * @brief Destroy the ParseError object
	 *
	 */
	virtual ~ParseError() = default;
	// LCOV_EXCL_STOP

	size_t GetLineNum() const noexcept
	{
		return m_lineNum;
	}

	size_t GetColNum() const noexcept
	{
		return m_colNum;
	}

private:

	size_t m_lineNum;
	size_t m_colNum;
}; // class ParseError

/**
 * @brief This exception is thrown when error occurred during writing object to
 *        JSON string.
 */
class SerializeTypeError : public Exception
{

public:

	explicit SerializeTypeError(const std::string& typeName) :
		Exception("Cannot serialize type " + typeName + " into JSON string")
	{}

	// LCOV_EXCL_START
	/**
	 * @brief Destroy the SerializeTypeError object
	 *
	 */
	virtual ~SerializeTypeError() = default;
	// LCOV_EXCL_STOP

}; // class SerializeTypeError

} // namespace SimpleJson
