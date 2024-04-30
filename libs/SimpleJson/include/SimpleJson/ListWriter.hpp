// Copyright (c) 2022 SimpleJson
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "Internal/SimpleObjects.hpp"

#include "Utils.hpp"
#include "WriterConfig.hpp"

#ifndef SIMPLEJSON_CUSTOMIZED_NAMESPACE
namespace SimpleJson
#else
namespace SIMPLEJSON_CUSTOMIZED_NAMESPACE
#endif
{

template<typename _ObjWriter, typename _ToStringType, typename _ContainerType>
struct JsonWriterListImpl
{

	using ListBase =
		typename Internal::Obj::BaseObject<_ToStringType>::ListBase;

	template<typename _OutputIt>
	inline static void Write(_OutputIt destIt,
		const ListBase& obj,
		const WriterConfig& config,
		const WriterStates& state)
	{
		*destIt++ = '[';

		if (config.m_indent.size() > 0 && obj.size() > 0)
		{
			Internal::RepeatOutput(destIt, config.m_lineEnd, 1);
		}

		WriterStates stateNextLevel = state;
		++(stateNextLevel.m_nestLevel);
		size_t len = obj.size();

		for(auto it = obj.cbegin(); it != obj.cend(); ++it, --len)
		{
			if (config.m_indent.size() > 0)
			{
				Internal::RepeatOutput(destIt,
					config.m_indent, stateNextLevel.m_nestLevel);
			}

			_ObjWriter::Write(destIt, *it, config, stateNextLevel);

			if (len != 1)
			{
				*destIt++ = ',';
			}

			if (config.m_indent.size() > 0)
			{
				Internal::RepeatOutput(destIt, config.m_lineEnd, 1);
			}
		}

		if (config.m_indent.size() > 0 && obj.size() > 0)
		{
			Internal::RepeatOutput(destIt, config.m_indent, state.m_nestLevel);
		}
		*destIt++ = ']';
	}

}; // struct JsonWriterListImpl

} // namespace SimpleJson
