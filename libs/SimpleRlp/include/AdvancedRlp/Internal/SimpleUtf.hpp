// Copyright (c) 2022 SimpleRlp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <SimpleUtf/Utf.hpp>

#ifndef ADVANCEDRLP_CUSTOMIZED_NAMESPACE
namespace AdvancedRlp
#else
namespace ADVANCEDRLP_CUSTOMIZED_NAMESPACE
#endif
{
namespace Internal
{

#ifndef SIMPLEUTF_CUSTOMIZED_NAMESPACE
namespace Utf = ::SimpleUtf;
#else
namespace Utf = ::SIMPLEUTF_CUSTOMIZED_NAMESPACE;
#endif

} // namespace Internal
} // namespace AdvancedRlp
