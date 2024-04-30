// Copyright (c) 2022 SimpleUtf
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <string>
#include <utility>
#include <stdexcept>

#ifndef SIMPLEUTF_CUSTOMIZED_NAMESPACE
namespace SimpleUtf
#else
namespace SIMPLEUTF_CUSTOMIZED_NAMESPACE
#endif
{
	/**
	 * @brief The exception base class for all SimpleUtf classes' exceptions.
	 *        This class is inherited from std::runtime_error class.
	 *
	 */
	class Exception : public std::runtime_error
	{
	public:

		/**
		 * @brief Construct a new Exception object.
		 *        Same usage as the one in std::runtime_error.
		 *
		 * @param what_arg explanatory string
		 */
		Exception(const std::string& what_arg) :
			std::runtime_error(what_arg)
		{}

		/**
		 * @brief Construct a new Exception object.
		 *        Same usage as the one in std::runtime_error.
		 *
		 * @param what_arg explanatory string
		 */
		Exception(const char* what_arg) :
			std::runtime_error(what_arg)
		{}

		/**
		 * @brief Construct a new Exception object.
		 *        Same usage as the one in std::runtime_error.
		 *
		 * @param other another exception object to copy
		 */
		Exception(const Exception& other) noexcept :
			std::runtime_error(other)
		{}

		/**
		 * @brief Destroy the Exception object
		 *
		 */
		virtual ~Exception()
		{}
	};

	/**
	 * @brief An exception class that is thrown when the UTF conversion failed.
	 *
	 */
	class UtfConversionException : public Exception
	{
	public:

		/**
		 * @brief Construct a new UTF conversion Exception object
		 *
		 * @param what_arg explanatory string
		 */
		UtfConversionException(const char* what_arg) :
			Exception(what_arg)
		{}

		/**
		 * @brief Construct a new UTF conversion Exception object
		 *
		 * @param what_arg explanatory string
		 */
		UtfConversionException(const std::string& what_arg) :
			Exception(what_arg)
		{}

		UtfConversionException(const UtfConversionException& other) noexcept :
			Exception(other)
		{}

		/**
		 * @brief Destroy the UTF conversion Exception object
		 *
		 */
		virtual ~UtfConversionException()
		{}
	};

}
