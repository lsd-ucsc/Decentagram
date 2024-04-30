// Copyright (c) 2022 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#include <memory>
#include <string>

#include <DecentEnclave/Common/Sgx/MbedTlsInit.hpp>
#include <DecentEnclave/Untrusted/Config/AuthList.hpp>
#include <DecentEnclave/Untrusted/Config/EndpointsMgr.hpp>
#include <DecentEnclave/Untrusted/Hosting/BoostAsioService.hpp>

#include <SimpleConcurrency/Threading/ThreadPool.hpp>

#include <SimpleJson/SimpleJson.hpp>

#include <SimpleObjects/Codec/Hex.hpp>
#include <SimpleObjects/SimpleObjects.hpp>

#include <SimpleSysIO/SysCall/Files.hpp>

#include "End2EndLatTester.hpp"


using namespace DecentEnclave;
using namespace DecentEnclave::Common;
using namespace DecentEnclave::Untrusted;
using namespace End2EndLatency;
using namespace SimpleConcurrency::Threading;
using namespace SimpleObjects;
using namespace SimpleSysIO::SysCall;


int main(int argc, char* argv[]) {
	std::string configPath;
	if (argc == 1)
	{
		configPath = "../../../tests/End2EndLatency/components_config.json";
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


	// Testing contracts configurations
	const auto& pubsubConfig = config.AsDict()[String("PubSub")].AsDict();
	const auto& pubsubAddrHex = pubsubConfig[String("PubSubAddr")].AsString();
	const auto& publisherAddrHex = pubsubConfig[String("PublisherAddr")].AsString();
	const auto& subscriberAddrHex = pubsubConfig[String("SubscriberAddr")].AsString();


	EclipseMonitor::Eth::ContractAddr pubsubAddr;
	Codec::Hex::Decode(pubsubAddr.begin(), pubsubAddrHex.begin(), pubsubAddrHex.end());
	EclipseMonitor::Eth::ContractAddr publisherAddr;
	Codec::Hex::Decode(publisherAddr.begin(), publisherAddrHex.begin(), publisherAddrHex.end());
	EclipseMonitor::Eth::ContractAddr subscriberAddr;
	Codec::Hex::Decode(subscriberAddr.begin(), subscriberAddrHex.begin(), subscriberAddrHex.end());


	// Create enclave
	const auto& imgConfig = config.AsDict()[String("EnclaveImage")].AsDict();
	std::string imgPath = imgConfig[String("ImagePath")].AsString().c_str();
	std::string tokenPath = imgConfig[String("TokenPath")].AsString().c_str();
	auto enclave = std::make_shared<End2EndLatency::End2EndLatTester>(
		pubsubAddr,
		publisherAddr,
		subscriberAddr,
		authListAdvRlp,
		imgPath,
		tokenPath
	);


	threadPool->Terminate();


	return 0;
}
