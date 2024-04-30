// Copyright (c) 2023 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <memory>
#include <vector>

#include "../Common/AuthList.hpp"


namespace DecentEnclave
{
namespace Trusted
{


class AuthListMgr
{
public: // static member:

	static const AuthListMgr& GetInstance(
		const std::vector<uint8_t>* authListAdvRlp = nullptr
	)
	{
		static const AuthListMgr sk_inst(authListAdvRlp);
		return sk_inst;
	}

public:

	~AuthListMgr() = default;


	const std::vector<uint8_t>& GetAuthListAdvRlp() const
	{
		return m_authListAdvRlp;
	}

	const Common::AuthList& GetAuthList() const
	{
		return m_authList;
	}


private:

	AuthListMgr(
		const std::vector<uint8_t>* authListAdvRlp
	) :
		m_authListAdvRlp(
			authListAdvRlp == nullptr ?
				std::vector<uint8_t>() :
				*authListAdvRlp
		),
		m_authList(
			Common::AuthListParser().Parse(m_authListAdvRlp)
		)
	{}


	std::vector<uint8_t> m_authListAdvRlp;
	Common::AuthList m_authList;
}; // class AuthListMgr


} // namespace Trusted
} // namespace DecentEnclave
