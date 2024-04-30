// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "DefaultTypes.hpp"

#ifndef SIMPLERLP_CUSTOMIZED_NAMESPACE
namespace SimpleRlp
#else
namespace SIMPLERLP_CUSTOMIZED_NAMESPACE
#endif
{

inline RetObjType ParseRlp(const InputContainerType& inBytes)
{
	return GeneralParser().Parse(inBytes);
}

inline OutputContainerType WriteRlp(const Internal::Obj::BaseObj& obj)
{
	return WriterGeneric::Write(obj);
}

inline size_t CalcRlpSize(const Internal::Obj::BaseObj& obj)
{
	return WriterGeneric::CalcSize(obj);
}

} // namespace SimpleRlp
