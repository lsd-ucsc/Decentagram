// Tencent is pleased to support the open source community by making RapidJSON available.
//
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip.
//
// Licensed under the MIT License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

// imported & modified by SimpleObjects , 2022
// source: https://github.com/Tencent/rapidjson/blob/master/include/rapidjson/rapidjson.h

#pragma once

#include <cstdint>

#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{

namespace Internal {

static constexpr uint64_t RjUint64C2(uint32_t high32, uint32_t low32) {
	return ((static_cast<uint64_t>(high32) << 32) | static_cast<uint64_t>(low32));
}

} // namespace Internal
} // namespace SimpleObjects
