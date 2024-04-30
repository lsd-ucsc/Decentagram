// Copyright (c) 2022 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#include <memory>
#include <string>

#include <DecentEnclave/Common/Platform/Print.hpp>
#include <DecentEnclave/Common/Sgx/MbedTlsInit.hpp>
#include <DecentEnclave/Untrusted/Config/AuthList.hpp>
#include <DecentEnclave/Untrusted/Config/EndpointsMgr.hpp>
#include <DecentEnclave/Untrusted/Hosting/BoostAsioService.hpp>
#include <SimpleConcurrency/Threading/ThreadPool.hpp>
#include <SimpleJson/SimpleJson.hpp>
#include <SimpleObjects/Internal/make_unique.hpp>
#include <SimpleObjects/Codec/Hex.hpp>
#include <SimpleObjects/SimpleObjects.hpp>
#include <SimpleSysIO/SysCall/Files.hpp>

#include "Revoker.hpp"
#include "RunUntilSignal.hpp"


using namespace DecentEnclave;
using namespace DecentEnclave::Common;
using namespace DecentEnclave::Untrusted;
using namespace Revoker;
using namespace SimpleConcurrency::Threading;
using namespace SimpleObjects;
using namespace SimpleSysIO::SysCall;


int main(int argc, char* argv[]) {
	std::string configPath;
	if (argc == 1)
	{
		configPath = "../../src/components_config.json";
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


	// Thread pool
	std::shared_ptr<ThreadPool> threadPool = std::make_shared<ThreadPool>(5);


	// Boost IO Service
	std::unique_ptr<Hosting::BoostAsioService> asioService =
		SimpleObjects::Internal::make_unique<Hosting::BoostAsioService>();
	auto asioIoService = asioService->GetIoService();
	threadPool->AddTask(std::move(asioService));


	// Endpoints Manager
	auto endpointMgr = Config::EndpointsMgr::GetInstancePtr(
		&config,
		asioIoService
	);


	// Publisher
	const auto& pubConfig = config.AsDict()[String("Publisher")].AsDict();
	std::string pubAddrHex = pubConfig[String("Addr")].AsString().c_str();
	auto pubAddrBytes = Codec::Hex::Decode<std::vector<uint8_t> >(pubAddrHex);
	EclipseMonitor::Eth::ContractAddr pubAddr;
	if (pubAddrBytes.size() != pubAddr.size())
	{
		throw std::runtime_error("Invalid Publisher contract address.");
	}
	std::copy(pubAddrBytes.begin(), pubAddrBytes.end(), pubAddr.begin());

	// Create enclave
	const auto& imgConfig = config.AsDict()[String("EnclaveImage")].AsDict();
	std::string imgPath = imgConfig[String("ImagePath")].AsString().c_str();
	std::string tokenPath = imgConfig[String("TokenPath")].AsString().c_str();
	auto enclave = std::make_shared<Revoker::Revoker>(
		pubAddr,
		authListAdvRlp,
		imgPath,
		tokenPath
	);


	RunUntilSignal(
		[&]()
		{
			threadPool->Update();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	);


	threadPool->Terminate();


	return 0;
}
