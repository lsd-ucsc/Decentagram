// Copyright (c) 2022 SimpleJson
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "WriterConfig.hpp"

#ifndef SIMPLEJSON_CUSTOMIZED_NAMESPACE
namespace SimpleJson
#else
namespace SIMPLEJSON_CUSTOMIZED_NAMESPACE
#endif
{

template<typename _ToStringType>
struct JsonWriterNullImpl
{
	template<typename _OutputIt>
	inline static void Write(_OutputIt it,
		const WriterConfig&,
		const WriterStates&)
	{
		*it++ = 'n';
		*it++ = 'u';
		*it++ = 'l';
		*it++ = 'l';
	}
}; // struct JsonWriterNullImpl

} // namespace SimpleJson
