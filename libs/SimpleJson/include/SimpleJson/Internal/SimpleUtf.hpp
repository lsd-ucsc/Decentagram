// Copyright (c) 2022 SimpleJson
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <SimpleUtf/Utf.hpp>

#ifndef SIMPLEJSON_CUSTOMIZED_NAMESPACE
namespace SimpleJson
#else
namespace SIMPLEJSON_CUSTOMIZED_NAMESPACE
#endif
{
namespace Internal
{

#ifndef SIMPLEUTF_CUSTOMIZED_NAMESPACE
namespace Utf = ::SimpleUtf;
#else
namespace Utf = ::SIMPLEUtf_CUSTOMIZED_NAMESPACE;
#endif

} // namespace Internal
} // namespace SimpleJson
