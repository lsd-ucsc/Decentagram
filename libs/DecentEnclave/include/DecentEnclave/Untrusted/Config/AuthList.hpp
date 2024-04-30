// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <vector>

#include <SimpleObjects/Codec/Hex.hpp>

#include "../../Common/AuthList.hpp"
#include "../../Common/Internal/SimpleObj.hpp"


namespace DecentEnclave
{
namespace Untrusted
{
namespace Config
{


std::vector<uint8_t> ConfigToAuthListAdvRlp(
	const Common::Internal::Obj::Object& config
)
{
	using namespace Common::Internal::Obj;
	const auto& hashMap =
		config.AsDict()[String("AuthorizedComponents")].AsDict();

	Common::AuthList authList;

	authList.get_SVN() = Common::sk_authListSVN();

	for(const auto& pair : hashMap)
	{
		const auto& hashHex = std::get<0>(pair)->AsString();
		const auto& cmpConfig = std::get<1>(pair)->AsDict();
		const auto& name = cmpConfig[String("Name")].AsString();

		Bytes hash(
			Codec::Hex::Decode<std::vector<uint8_t> >(hashHex)
		);

		authList.get_HashToName()[hash] = name;
	}

	return Common::Internal::AdvRlp::GenericWriter::Write(authList);
}


} // namespace Hosting
} // namespace Untrusted
} // namespace DecentEnclave
