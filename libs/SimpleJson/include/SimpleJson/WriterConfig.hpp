// Copyright (c) 2022 SimpleJson
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <string>

#ifndef SIMPLEJSON_CUSTOMIZED_NAMESPACE
namespace SimpleJson
#else
namespace SIMPLEJSON_CUSTOMIZED_NAMESPACE
#endif
{

// Configurations for the writer
struct WriterConfig
{
	WriterConfig() :
		m_indent(""),
		m_lineEnd("\n"),
		m_orderDict(false)
	{}

	~WriterConfig() = default;

	std::string m_indent;
	std::string m_lineEnd;
	bool m_orderDict;

}; // struct WriterConfig

struct WriterStates
{
	WriterStates() :
		m_nestLevel(0)
	{}

	WriterStates(const WriterStates& other) :
		m_nestLevel(other.m_nestLevel)
	{}

	~WriterStates() = default;

	size_t m_nestLevel;

}; // struct WriterStates

} // namespace SimpleJson
