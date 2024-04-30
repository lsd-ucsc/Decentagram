// Copyright (c) 2022 EclipseMonitor
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <SimpleRlp/SimpleRlp.hpp>
#include <AdvancedRlp/AdvancedRlp.hpp>

namespace EclipseMonitor
{
namespace Internal
{

#ifndef SIMPLERLP_CUSTOMIZED_NAMESPACE
namespace Rlp = ::SimpleRlp;
#else
namespace Rlp = ::SIMPLERLP_CUSTOMIZED_NAMESPACE;
#endif // !SIMPLERLP_CUSTOMIZED_NAMESPACE

#ifndef ADVANCEDRLP_CUSTOMIZED_NAMESPACE
namespace AdvRlp = ::AdvancedRlp;
#else
namespace AdvRlp = ::ADVANCEDRLP_CUSTOMIZED_NAMESPACE;
#endif // !ADVANCEDRLP_CUSTOMIZED_NAMESPACE

} // namespace Internal
} // namespace EclipseMonitor
