// Copyright (c) 2022 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#include <memory>
#include <string>
#include <vector>

#include <DecentEnclave/Common/Platform/Print.hpp>
#include <DecentEnclave/Common/Sgx/MbedTlsInit.hpp>

#include <DecentEnclave/Untrusted/Config/AuthList.hpp>
#include <DecentEnclave/Untrusted/Config/EndpointsMgr.hpp>
#include <DecentEnclave/Untrusted/Hosting/BoostAsioService.hpp>
#include <DecentEnclave/Untrusted/Hosting/HeartbeatEmitterService.hpp>
#include <DecentEnclave/Untrusted/Hosting/LambdaFuncServer.hpp>

#include <EthereumClt/Untrusted/HostBlockServiceTasks.hpp>

#include <SimpleConcurrency/Threading/ThreadPool.hpp>
#include <SimpleJson/SimpleJson.hpp>
#include <SimpleRlp/SimpleRlp.hpp>
#include <SimpleObjects/Internal/make_unique.hpp>
#include <SimpleObjects/Codec/Hex.hpp>
#include <SimpleObjects/SimpleObjects.hpp>
#include <SimpleSysIO/SysCall/Files.hpp>

#include "EthereumCltEnclave.hpp"
#include "RunUntilSignal.hpp"


using namespace DecentEnclave;
using namespace DecentEnclave::Common;
using namespace DecentEnclave::Untrusted;
using namespace EthereumClt;
using namespace SimpleConcurrency::Threading;
using namespace SimpleObjects;
using namespace SimpleSysIO::SysCall;


std::shared_ptr<ThreadPool> GetThreadPool()
{
	static  std::shared_ptr<ThreadPool> threadPool =
		std::make_shared<ThreadPool>(5);

	return threadPool;
}


static void StartSendingBlocks(
	HostBlockService& blkSvc,
	uint64_t startBlockNum
)
{
	if (blkSvc.GetCurrBlockNum() != 0)
	{
		throw std::runtime_error("Block update service already started.");
	}

	blkSvc.SetUpdSvcStartBlock(startBlockNum);
	auto blkSvcSPtr = blkSvc.GetSharedPtr();

	auto blkUpdStatusSvc = std::unique_ptr<HostBlockStatusLogTask>(
		new HostBlockStatusLogTask(blkSvcSPtr, 10 * 1000)
	);
	auto blkUpdSvc = std::unique_ptr<BlockUpdatorServiceTask>(
		new BlockUpdatorServiceTask(blkSvcSPtr, 1 * 1000)
	);

	std::shared_ptr<ThreadPool> threadPool = GetThreadPool();

	threadPool->AddTask(std::move(blkUpdStatusSvc));
	threadPool->AddTask(std::move(blkUpdSvc));
}


int main(int argc, char* argv[])
{
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


	// Thread pool
	std::shared_ptr<ThreadPool> threadPool = GetThreadPool();


	// Read in components config
	auto configFile = RBinaryFile::Open(configPath);
	auto configJson = configFile->ReadBytes<std::string>();
	auto config = SimpleJson::LoadStr(configJson);
	std::vector<uint8_t> authListAdvRlp = Config::ConfigToAuthListAdvRlp(config);


	// Boost IO Service
	std::unique_ptr<Hosting::BoostAsioService> asioService =
		SimpleObjects::Internal::make_unique<Hosting::BoostAsioService>();


	// Endpoints Manager
	auto endpointMgr = Config::EndpointsMgr::GetInstancePtr(
		&config,
		asioService->GetIoService()
	);


	// Geth configs
	const auto& gethConfig = config.AsDict()[String("Geth")].AsDict();
	std::string gethProto = gethConfig[String("Protocol")].AsString().c_str();
	std::string gethHost = gethConfig[String("Host")].AsString().c_str();
	uint32_t gethPort = gethConfig[String("Port")].AsCppUInt32();
	std::string syncAddrHex = gethConfig[String("SyncAddr")].AsString().c_str();
	auto syncAddrBytes = Codec::Hex::Decode<std::vector<uint8_t> >(syncAddrHex);
	EclipseMonitor::Eth::ContractAddr syncAddr;
	if (syncAddrBytes.size() != syncAddr.size())
	{
		throw std::runtime_error("Invalid Sync contract address.");
	}


	// Host block service
	std::string gethUrl =
		gethProto + "://" + gethHost + ":" + std::to_string(gethPort);
	std::shared_ptr<HostBlockService> hostBlkSvc =
		HostBlockService::Create(gethUrl);


	// Pubsub configs
	const auto& pubsubConfig = config.AsDict()[String("PubSub")].AsDict();
	std::string pubsubAddrHex = pubsubConfig[String("PubSubAddr")].AsString().c_str();
	uint64_t startBlockNum = pubsubConfig[String("StartBlock")].AsCppUInt64();
	auto pubsubAddrBytes = Codec::Hex::Decode<std::vector<uint8_t> >(pubsubAddrHex);
	EclipseMonitor::Eth::ContractAddr pubsubAddr;
	if (pubsubAddrBytes.size() != pubsubAddr.size())
	{
		throw std::runtime_error("Invalid Pub-Sub contract address.");
	}
	std::copy(pubsubAddrBytes.begin(), pubsubAddrBytes.end(), pubsubAddr.begin());


	// Enclave
	const auto& imgConfig = config.AsDict()[String("EnclaveImage")].AsDict();
	std::string imgPath = imgConfig[String("ImagePath")].AsString().c_str();
	std::string tokenPath = imgConfig[String("TokenPath")].AsString().c_str();
	std::shared_ptr<EthereumCltEnclave> enclave =
		std::make_shared<EthereumCltEnclave>(
			EclipseMonitor::BuildEthereumMonitorConfig(),
			startBlockNum,
			syncAddr,
			"SyncMsg(bytes16,bytes32)",
			pubsubAddr,
			hostBlkSvc,
			authListAdvRlp,
			imgPath,
			tokenPath
		);
	hostBlkSvc->BindReceiver(enclave);
	StartSendingBlocks(*hostBlkSvc, startBlockNum);


	// API call server
	Hosting::LambdaFuncServer lambdaFuncSvr(
		endpointMgr,
		threadPool
	);
	// Setup Lambda call handlers and start to run multi-threaded-ly
	lambdaFuncSvr.AddFunction("EthereumClt", enclave);


	// Heartbeat emitter
	auto heartbeatEmitter = std::unique_ptr<Hosting::HeartbeatEmitterService>(
		new Hosting::HeartbeatEmitterService(enclave, 100)
	);
	threadPool->AddTask(std::move(heartbeatEmitter));


	// Start IO service
	threadPool->AddTask(std::move(asioService));


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


extern "C" sgx_status_t ocall_ethereum_clt_get_receipts(
	const void* host_blk_svc,
	uint64_t blk_num,
	uint8_t** out_buf,
	size_t* out_buf_size
)
{
	using _ListBytesType = SimpleObjects::ListT<SimpleObjects::Bytes>;

	const HostBlockService* blkSvc =
		static_cast<const HostBlockService*>(host_blk_svc);

	try
	{
		_ListBytesType listOfReceipts =
			blkSvc->GetReceiptsRlpByNum<_ListBytesType>(blk_num);

		std::vector<uint8_t> bytes = SimpleRlp::WriteRlp(listOfReceipts);

		*out_buf = new uint8_t[bytes.size()];
		*out_buf_size = bytes.size();

		std::copy(bytes.begin(), bytes.end(), *out_buf);

		return SGX_SUCCESS;
	}
	catch (const std::exception& e)
	{
		DecentEnclave::Common::Platform::Print::StrDebug(
			"ocall_ethereum_clt_get_receipts failed with error " +
			std::string(e.what())
		);
		return SGX_ERROR_UNEXPECTED;
	}
}

extern "C" sgx_status_t ocall_ethereum_clt_get_latest_blknum(
	const void* host_blk_svc,
	uint64_t* out_blk_num
)
{
	const HostBlockService* blkSvc =
		static_cast<const HostBlockService*>(host_blk_svc);

	try
	{
		*out_blk_num = blkSvc->GetLatestBlockNum();

		return SGX_SUCCESS;
	}
	catch (const std::exception& e)
	{
		DecentEnclave::Common::Platform::Print::StrDebug(
			"ocall_ethereum_clt_get_latest_blknum failed with error " +
			std::string(e.what())
		);
		return SGX_ERROR_UNEXPECTED;
	}
}
