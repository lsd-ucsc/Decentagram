// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <SimpleObjects/Exception.hpp>

#include "../SimpleRlp/Internal/SimpleObjects.hpp"

#include "Internal/SimpleRlp.hpp"

#ifndef ADVANCEDRLP_CUSTOMIZED_NAMESPACE
namespace AdvancedRlp
#else
namespace ADVANCEDRLP_CUSTOMIZED_NAMESPACE
#endif
{

/**
 * @brief Parent class of all AdvancedRlp exceptions
 *
 */
class Exception : public Internal::SimRlp::Internal::Obj::Exception
{
public: // static members:

	using Base = Internal::SimRlp::Internal::Obj::Exception;

public:

	using Base::Base;

	// LCOV_EXCL_START
	virtual ~Exception() = default;
	// LCOV_EXCL_STOP

}; // class Exception


/**
 * @brief This exception is thrown when a unknown CAT ID is given.
 */
class UnknownCatId : public Exception
{

public:

	/**
	 * @brief Construct a new UnknownCatId exception
	 *
	 * @param issue    What was the issue?
	 * @param bytePos  Where the parse error happened in terms of byte position
	 */
	explicit UnknownCatId(uint8_t catId) :
		Exception(ConErrorMsg(catId))
	{}

	// LCOV_EXCL_START
	/**
	 * @brief Destroy the UnknownCatId object
	 *
	 */
	virtual ~UnknownCatId() = default;
	// LCOV_EXCL_STOP

private: // static members:

	static std::string ConErrorMsg(uint8_t catId)
	{
		std::string catIdHex;
		Internal::SimRlp::Internal::Obj::Internal::
			ByteToHEX<true, std::string::value_type>(
				std::back_inserter(catIdHex), catId);

		return "Unkown CAT ID - " + catIdHex;
	}

}; // class UnknownCatId


/**
 * @brief This exception is thrown when error occurred during parsing.
 */
class ParseError : public Exception
{

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

private: // static members:

	static std::string ConErrorMsg(
		const std::string& issue, size_t bytePos)
	{
		return ("Parse error - " + issue) +
			" (byte @ " + std::to_string(bytePos + 1) + ")";
	}

private:

	size_t m_bytePos;
}; // class ParseError


/**
 * @brief This exception is thrown when error occurred during writing object to
 *        AdvRLP bytes
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
 *        AdvRLP bytes
 */
class SerializeTypeError : public SerializeError
{

public:

	SerializeTypeError(
		const std::string& typeName,
		const std::string& writerName) :
		SerializeError(ConErrorMsg(typeName, writerName))
	{}

	// LCOV_EXCL_START
	/**
	 * @brief Destroy the SerializeTypeError object
	 *
	 */
	virtual ~SerializeTypeError() = default;
	// LCOV_EXCL_STOP

private: // static members:

	static std::string ConErrorMsg(
		const std::string& typeName,
		const std::string& writerName)
	{
		return (
			"Cannot serialize type " +
			typeName +
			" into AdvRLP with " +
			writerName);
	}

}; // class SerializeTypeError


} // namespace AdvancedRlp
