// Copyright (c) 2022 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#include <memory>
#include <string>

#include <DecentEnclave/Common/Sgx/MbedTlsInit.hpp>
#include <DecentEnclave/Untrusted/Config/AuthList.hpp>
#include <DecentEnclave/Untrusted/Config/EndpointsMgr.hpp>
#include <SimpleJson/SimpleJson.hpp>
#include <SimpleObjects/SimpleObjects.hpp>
#include <SimpleSysIO/SysCall/Files.hpp>

#include "ProblematicApp.hpp"


using namespace DecentEnclave;
using namespace DecentEnclave::Common;
using namespace DecentEnclave::Untrusted;
using namespace ProblematicApp;
using namespace SimpleObjects;
using namespace SimpleSysIO::SysCall;


int main(int argc, char* argv[]) {
	std::string configPath;
	if (argc == 1)
	{
		configPath = "../../../tests/ProblematicApp/components_config.json";
	}
	else if (argc == 2)
	{
		configPath = argv[1];
	}
	else
	{
		Common::Platform::Print::StrErr("Unexpected number of arguments.");
		Common::Platform::Print::StrErr(
			"Only the path to the components configuration file is needed."
		);
		return -1;
	}

	// Init MbedTLS
	Common::Sgx::MbedTlsInit::Init();


	// Read in components config
	auto configFile = RBinaryFile::Open(configPath);
	auto configJson = configFile->ReadBytes<std::string>();
	auto config = SimpleJson::LoadStr(configJson);
	std::vector<uint8_t> authListAdvRlp = Config::ConfigToAuthListAdvRlp(config);


	// Endpoints Manager
	auto endpointMgr = Config::EndpointsMgr::GetInstancePtr(&config);


	// Create enclave
	const auto& imgConfig = config.AsDict()[String("EnclaveImage")].AsDict();
	std::string imgPath = imgConfig[String("ImagePath")].AsString().c_str();
	std::string tokenPath = imgConfig[String("TokenPath")].AsString().c_str();
	auto enclave = std::make_shared<ProblematicApp::ProblematicApp>(
		authListAdvRlp,
		imgPath,
		tokenPath
	);


	return 0;
}
