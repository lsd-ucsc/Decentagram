// Copyright (c) 2022 SimpleSysIO
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <SimpleObjects/SimpleObjects.hpp>


#ifndef SIMPLESYSIO_CUSTOMIZED_NAMESPACE
namespace SimpleSysIO
#else
namespace SIMPLESYSIO_CUSTOMIZED_NAMESPACE
#endif
{
namespace Internal
{

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace Obj = ::SimpleObjects;
#else
namespace Obj = ::SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE;
#endif

} // namespace Internal
} // namespace SimpleSysIO
