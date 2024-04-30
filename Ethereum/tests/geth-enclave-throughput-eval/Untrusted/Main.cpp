// Copyright (c) 2022 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#include <ctime>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <DecentEnclave/Common/Sgx/MbedTlsInit.hpp>
#include <DecentEnclave/Untrusted/Config/AuthList.hpp>

#include <EthereumClt/Untrusted/HostBlockService.hpp>

#include <SimpleJson/SimpleJson.hpp>
#include <SimpleRlp/SimpleRlp.hpp>
#include <SimpleObjects/Internal/make_unique.hpp>
#include <SimpleObjects/SimpleObjects.hpp>

#include "EthereumCltEnclave.hpp"


using namespace DecentEnclave;
using namespace DecentEnclave::Common;
using namespace DecentEnclave::Untrusted;
using namespace EthereumClt;
using namespace SimpleObjects;


static inline uint64_t TimeNow()
{
	return static_cast<uint64_t>(std::time(nullptr));
}


int main(int argc, char* argv[])
{
	std::string configPath;
	if (argc == 1)
	{
		configPath = "../../../tests/geth-enclave-throughput-eval/components_config.json";
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
	auto configFile = SimpleSysIO::SysCall::RBinaryFile::Open(configPath);
	auto configJson = configFile->ReadBytes<std::string>();
	auto config = SimpleJson::LoadStr(configJson);


	// Host block service
	const auto& gethConfig = config.AsDict()[String("Geth")].AsDict();
	std::string gethProto = gethConfig[String("Protocol")].AsString().c_str();
	std::string gethHost = gethConfig[String("Host")].AsString().c_str();
	uint32_t gethPort = gethConfig[String("Port")].AsCppUInt32();
	std::string gethUrl =
		gethProto + "://" + gethHost + ":" + std::to_string(gethPort);
	std::shared_ptr<HostBlockService> hostBlkSvc =
		HostBlockService::Create(gethUrl);

	// Test configurations
	std::vector<double> receiptRates = {
		0.00,
		0.10,
		0.20,
		0.30,
		0.40,
		0.50,
		0.60,
		0.70,
		0.80,
		0.90,
		1.00,
	};
	uint64_t startBlockNum = 8875000;
	uint64_t endBlockNum   = 8880000;


	// Enclave
	const auto& imgConfig = config.AsDict()[String("EnclaveImage")].AsDict();
	std::string imgPath = imgConfig[String("ImagePath")].AsString().c_str();
	std::string tokenPath = imgConfig[String("TokenPath")].AsString().c_str();
	std::shared_ptr<EthereumCltEnclave> enclave =
		std::make_shared<EthereumCltEnclave>(
			hostBlkSvc,
			imgPath,
			tokenPath
		);
	hostBlkSvc->BindReceiver(enclave);


	for (const double& receiptRate: receiptRates)
	{
		enclave->SetReceiptRate(receiptRate);

		auto start = TimeNow();
		for (auto i = startBlockNum; i < endBlockNum; ++i)
		{
			hostBlkSvc->PushBlock(i);
		}
		auto end = TimeNow();
		auto duration = end - start;
		auto numBlocks = endBlockNum - startBlockNum;
		auto rate = static_cast<double>(numBlocks) / duration;

		std::cout
			<< "Receipt %:  " << receiptRate * 100 << "%" << std::endl
			<< "Pushed:     " << numBlocks << " blocks" << std::endl
			<< "Took:       " << duration  << " seconds" << std::endl
			<< "Throughput: " << rate      << " blocks / second" << std::endl;
	}
	enclave->SetReceiptRate(0.00);


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
