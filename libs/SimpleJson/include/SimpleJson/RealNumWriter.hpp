// Copyright (c) 2022 SimpleJson
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "WriterConfig.hpp"
#include "Internal/SimpleObjects.hpp"

#ifndef SIMPLEJSON_CUSTOMIZED_NAMESPACE
namespace SimpleJson
#else
namespace SIMPLEJSON_CUSTOMIZED_NAMESPACE
#endif
{

template<typename _ToStringType>
struct JsonWriterRealNumImpl
{
	template<typename _OutputIt>
	inline static void WriteBool(_OutputIt it,
		const Internal::Obj::RealNumBaseObject<_ToStringType>& obj,
		const WriterConfig&,
		const WriterStates&)
	{
		if(obj.IsTrue())
		{
			*it++ = 't';
			*it++ = 'r';
			*it++ = 'u';
			*it++ = 'e';
		}
		else
		{
			*it++ = 'f';
			*it++ = 'a';
			*it++ = 'l';
			*it++ = 's';
			*it++ = 'e';
		}
	}

	template<typename _OutputIt>
	inline static void WriteNumber(_OutputIt it,
		const Internal::Obj::RealNumBaseObject<_ToStringType>& obj,
		const WriterConfig&,
		const WriterStates&)
	{
		auto str = obj.ToString();
		std::copy(str.begin(), str.end(), it);
	}

	template<typename _OutputIt>
	inline static void Write(_OutputIt it,
		const Internal::Obj::RealNumBaseObject<_ToStringType>& obj,
		const WriterConfig& config,
		const WriterStates& state)
	{
		switch (obj.GetNumType())
		{
		case Internal::Obj::RealNumType::Bool:
			return WriteBool(it, obj, config, state);

		default:
			return WriteNumber(it, obj, config, state);
		}
	}
}; // struct JsonWriterRealNumImpl

} // namespace SimpleJson
