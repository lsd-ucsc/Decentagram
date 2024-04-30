// Copyright (c) 2022 SimpleJson
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <iterator>

#include "Exceptions.hpp"
#include "Utils.hpp"

#ifndef SIMPLEJSON_CUSTOMIZED_NAMESPACE
namespace SimpleJson
#else
namespace SIMPLEJSON_CUSTOMIZED_NAMESPACE
#endif
{

/**
 * @brief The interface of a input state machine
 *
 * @tparam _ValType The type of each character value
 */
template<typename _ValType>
class InputStateMachineIf
{
public: // Static members:

	using value_type = _ValType;

public:

	InputStateMachineIf() = default;

	// LCOV_EXCL_START
	virtual ~InputStateMachineIf() = default;
	// LCOV_EXCL_STOP

	/**
	 * @brief Get current position in terms of number of lines, where
	 *        each line is ended by `\\n`, `\\r\\n`, or `\\r`
	 *
	 * @return The number of lines
	 */
	virtual size_t GetLineCount() const = 0;

	/**
	 * @brief Get current position in terms of column in a line
	 *
	 * @return The number of lines
	 */
	virtual size_t GetColCount() const = 0;

	/**
	 * @brief Does the input has an end
	 *
	 * @return true if it has an end, otherwise, false
	 */
	virtual bool HasEnd() const = 0;

	/**
	 * @brief Does the input reaches to the end
	 *
	 * @return true if it is reached to the end, otherwise, false
	 */
	virtual bool IsEnd() const = 0;

	/**
	 * @brief Skip white space charaters
	 *
	 */
	virtual void SkipWhiteSpace() = 0;

	/**
	 * @brief 1) skip all white spaces
	 *        2) return the current non-space charater
	 *        3) increment to next position
	 *
	 * @exception ParseError May be* thrown when reach to the end of input
	 *                       without finding the non-space character
	 *                       (*depending on if the actual implementation is
	 *                       aware of the end of input)
	 *
	 * @return the current non-space charater
	 */
	virtual value_type SkipSpaceAndGetCharAndAdv() = 0;

	/**
	 * @brief 1) skip all white spaces
	 *        2) return the current non-space charater
	 *
	 * @exception ParseError May be* thrown when reach to the end of input
	 *                       without finding the non-space character
	 *                       (*depending on if the actual implementation is
	 *                       aware of the end of input)
	 *
	 * @return the current non-space charater
	 */
	virtual value_type SkipSpaceAndGetChar() = 0;

	/**
	 * @brief 1) return the current charater (no matter if it's space or not)
	 *        2) increment to next position
	 *
	 * @exception ParseError May be* thrown when reach to the end of input
	 *                       (*depending on if the actual implementation is
	 *                       aware of the end of input)
	 *
	 * @return the current charater
	 */
	virtual value_type GetCharAndAdv() = 0;

	/**
	 * @brief 1) return the current charater (no matter if it's space or not)
	 *
	 * @exception ParseError May be* thrown when reach to the end of input
	 *                       (*depending on if the actual implementation is
	 *                       aware of the end of input)
	 *
	 * @return the current charater
	 */
	virtual value_type GetChar() = 0;

	/**
	 * @brief Expecting a delimiter.
	 *        1) skip whitespaces until there is a non-whitespace char
	 *        2) check if the non-whitespace char is the given delimiter, if not
	 *           an exception will be thrown
	 *
	 * @param d The delimiter character
	 */
	virtual void ExpDelimiter(const value_type& d)
	{
		auto ch = this->SkipSpaceAndGetCharAndAdv();
		if (ch != d)
		{
			throw ParseError(
				std::string("Expecting '") + d + "' delimiter",
				this->GetLineCount(), this->GetColCount());
		}
	}

}; // class InputStateMachineIf



/**
 * @brief An implementation of InputStateMachine interface that accepts an
 *        input iterator
 *        NOTE: this implementation will advance to the next position without
 *        checking for the end of input; so it is expected to either have a
 *        input string that is unlikely to end (e.g. console), or to stop
 *        getting next charactor right when the string ends
 *
 * @tparam _InputItType The type of the input iterator
 */
template<typename _InputItType>
class InputIteratorStateMachine :
	public InputStateMachineIf<
		typename std::iterator_traits<_InputItType>::value_type>
{
public: // static members:

	using value_type = typename std::iterator_traits<_InputItType>::value_type;
	using Base = InputStateMachineIf<value_type>;
	using Self = InputIteratorStateMachine<_InputItType>;

public:

	InputIteratorStateMachine(_InputItType it):
		Base::InputStateMachineIf(),
		m_it(it),
		m_lineNum(0),
		m_colNum(0),
		m_current(*m_it)
	{}

	virtual ~InputIteratorStateMachine() = default;

	virtual size_t GetLineCount() const override
	{
		return m_lineNum;
	}

	virtual size_t GetColCount() const override
	{
		return m_colNum;
	}

	virtual bool HasEnd() const override
	{
		return false;
	}

	virtual bool IsEnd() const override
	{
		return false;
	}

	void SkipWhiteSpace() override
	{
		while(Internal::IsSpaceCh(m_current))
		{
			// line ends with \r or \r\n
			if (m_current == '\r')
			{
				Advance(); // Skip \r
				if (m_current == '\n')
				{
					Advance(); // Skip \n
				}
				NewLine();
			}
			// line ends with \n
			else if (m_current == '\n')
			{
				Advance(); // Skip \n
				NewLine();
			}
			// other white spaces
			else
			{
				Advance();
			}
		}
	}

	virtual value_type SkipSpaceAndGetCharAndAdv() override
	{
		// 1) skip all white spaces
		SkipWhiteSpace();

		// 2) return the current non-space charater
		auto res = m_current;

		// 3) increment to next position
		Advance();

		return res;
	}

	virtual value_type SkipSpaceAndGetChar() override
	{
		// 1) skip all white spaces
		SkipWhiteSpace();

		// 2) return the current non-space charater
		return m_current;
	}

	virtual value_type GetCharAndAdv() override
	{
		// 1) return the current charater (no matter if it's space or not)
		auto res = m_current;

		// 2) increment to next position
		Advance();

		if ((res == '\n') || // case \n
			(res == '\r' && m_current != '\n') // case \r
			// case \r\n should be counted by \n
		)
		{
			NewLine();
		}

		return res;
	}

	virtual value_type GetChar() override
	{
		// return the current charater (no matter if it's space or not)
		return m_current;
	}

private:

	_InputItType m_it;
	size_t m_lineNum;
	size_t m_colNum;
	value_type m_current;

	void NewLine()
	{
		m_lineNum++;
		m_colNum = 0;
	}

	void Advance()
	{
		++m_it;
		++m_colNum;
		m_current = *m_it;
	}

}; // class InputIteratorStateMachine


/**
 * @brief An implementation of InputStateMachine interface that accepts an
 *        forward iterator
 *        NOTE: this implementation will check for the end of the string
 *        before advancing to the next position; thus, an *end* iterator is
 *        required.
 *        If the caller is getting the next charater when the end of input
 *        string is reached, a ParseError exception will be thrown
 *
 * @tparam _ForwardItType The type of the forward iterator
 */
template<typename _ForwardItType>
class ForwardIteratorStateMachine :
	public InputStateMachineIf<
		typename std::iterator_traits<_ForwardItType>::value_type>
{
public: // static members:

	using value_type = typename std::iterator_traits<_ForwardItType>::value_type;
	using Base = InputStateMachineIf<value_type>;
	using Self = ForwardIteratorStateMachine<_ForwardItType>;

public:

	ForwardIteratorStateMachine(_ForwardItType begin, _ForwardItType end):
		Base::InputStateMachineIf(),
		m_begin(begin),
		m_end(end),
		m_lineNum(0),
		m_colNum(0),
		m_current(
			((m_begin != m_end) ?
				(*m_begin) :
				throw ParseError("Input string ends unexpectedly",
					m_lineNum, m_colNum)))
	{}

	virtual ~ForwardIteratorStateMachine() = default;

	virtual size_t GetLineCount() const override
	{
		return m_lineNum;
	}

	virtual size_t GetColCount() const override
	{
		return m_colNum;
	}

	virtual bool HasEnd() const override
	{
		return true;
	}

	virtual bool IsEnd() const override
	{
		return m_begin == m_end;
	}

	void SkipWhiteSpace() override
	{
		while(!IsEnd() && Internal::IsSpaceCh(m_current))
		{
			// line ends with \r or \r\n
			if (m_current == '\r')
			{
				Advance(); // Skip \r
				if (!IsEnd() && m_current == '\n')
				{
					Advance(); // Skip \n
				}
				NewLine();
			}
			// line ends with \n
			else if (m_current == '\n')
			{
				Advance(); // Skip \n
				NewLine();
			}
			// other white spaces
			else
			{
				Advance();
			}
		}
	}

	virtual value_type SkipSpaceAndGetCharAndAdv() override
	{
		// 1) skip all white spaces
		SkipWhiteSpace();

		if (IsEnd())
		{
			// We reached the end of input; there is nothing we can return
			throw ParseError("Input string ends unexpectedly",
				m_lineNum, m_colNum);
		}

		// 2) return the current non-space charater
		auto res = m_current;

		// 3) increment to next position
		Advance();

		return res;
	}

	virtual value_type SkipSpaceAndGetChar() override
	{
		// 1) skip all white spaces
		SkipWhiteSpace();

		if (IsEnd())
		{
			// We reached the end of input; there is nothing we can return
			throw ParseError("Input string ends unexpectedly",
				m_lineNum, m_colNum);
		}

		// 2) return the current non-space charater
		return m_current;
	}

	virtual value_type GetCharAndAdv() override
	{
		if (IsEnd())
		{
			// We reached the end of input; there is nothing we can return
			throw ParseError("Input string ends unexpectedly",
				m_lineNum, m_colNum);
		}

		// 1) return the current charater (no matter if it's space or not)
		auto res = m_current;

		// 2) increment to next position
		Advance();

		if ((res == '\n') || // case \n
			(res == '\r' && IsEnd()) || // case \r
			(res == '\r' && m_current != '\n') // case \r
			// case \r\n should be counted by \n
		)
		{
			NewLine();
		}

		return res;
	}

	virtual value_type GetChar() override
	{
		// return the current charater (no matter if it's space or not)
		return (!IsEnd() ?
			m_current :
			throw ParseError("Input string ends unexpectedly",
				m_lineNum, m_colNum));
	}

private:

	_ForwardItType m_begin;
	_ForwardItType m_end;
	size_t m_lineNum;
	size_t m_colNum;
	value_type m_current;

	void NewLine()
	{
		m_lineNum++;
		m_colNum = 0;
	}

	void Advance()
	{
		++m_begin;
		++m_colNum;
		m_current = IsEnd() ? '\0' : *m_begin;
	}

}; // class ForwardIteratorStateMachine

} // namespace SimpleJson
