// Copyright (c) 2022 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#include <limits>

#include <sgx_edger8r.h>

#include <EclipseMonitor/Eth/DAA.hpp>
#include <EclipseMonitor/Eth/HeaderMgr.hpp>
#include <EclipseMonitor/Eth/ReceiptsMgr.hpp>

#include <DecentEnclave/Common/Logging.hpp>
#include <DecentEnclave/Common/Sgx/MbedTlsInit.hpp>

#include <DecentEnclave/Trusted/PlatformId.hpp>
#include <DecentEnclave/Trusted/Sgx/EnclaveIdentity.hpp>

#include <EthereumClt/Trusted/HostBlockService.hpp>


using EthChainConfig = EclipseMonitor::Eth::GoerliConfig;


namespace EthereumClt
{

static uint8_t g_receiptLimit = 0;
static size_t g_verifiedReceipts = 0;
static std::unique_ptr<Trusted::HostBlockService> g_hostBlkSvc;
static DecentEnclave::Common::Logger g_logger =
	DecentEnclave::Common::LoggerFactory::GetLogger("Enclave");

void GlobalInitialization()
{
	using namespace DecentEnclave::Common;

	// Initialize mbedTLS
	Sgx::MbedTlsInit::Init();
}


void PrintMyInfo()
{
	using namespace DecentEnclave::Common;
	using namespace DecentEnclave::Trusted;

	g_logger.Info(
		"My platform ID is              : " + PlatformId::GetIdHex()
	);

	const auto& selfHash = DecentEnclave::Trusted::Sgx::EnclaveIdentity::GetSelfHashHex();
	g_logger.Info(
		"My enclave hash is             : " + selfHash
	);
}


void Init(
	std::unique_ptr<Trusted::HostBlockService> blkSvc
)
{
	using namespace DecentEnclave::Trusted;

	GlobalInitialization();
	PrintMyInfo();

	g_hostBlkSvc = std::move(blkSvc);
}

void SetReceiptRate(double receiptRate)
{
	g_receiptLimit = std::numeric_limits<uint8_t>::max() * receiptRate;
	// (e.g., 255 * 0% = 0 -> 0)
	// (e.g., 255 * 10% = 25.5 -> 25)
	// (e.g., 255 * 100% = 255 -> 255)
	g_logger.Info(
		"Previously verified receipts: " +
			std::to_string(g_verifiedReceipts)
	);
	g_verifiedReceipts = 0;
}


void RecvBlock(const std::vector<uint8_t>& hdrRlp)
{
	EclipseMonitor::Eth::HeaderMgr headerMgr(hdrRlp, 0);

	const auto& hdrHash = headerMgr.GetHash();
	const auto& lastHashByte = hdrHash[hdrHash.size() - 1];

	if (lastHashByte < g_receiptLimit || g_receiptLimit == 255)
	{
		// verify receipt
		EclipseMonitor::Eth::ReceiptsMgr receiptsMgr(
			g_hostBlkSvc->GetReceiptsRlpByNum(
				headerMgr.GetNumber()
			).AsList()
		);
		if (
			receiptsMgr.GetRootHashBytes() !=
			headerMgr.GetRawHeader().get_ReceiptsRoot()
		)
		{
			throw std::runtime_error("Receipts root mismatch");
		}
		++g_verifiedReceipts;
	}
}


} // namespace EthereumClt


extern "C" sgx_status_t ecall_ethereum_clt_init(
	void* host_blk_svc
)
{
	using namespace EthereumClt;

	try
	{
		std::unique_ptr<Trusted::HostBlockService> blkSvc =
			SimpleObjects::Internal::
				make_unique<Trusted::HostBlockService>(host_blk_svc);

		EthereumClt::Init(
			std::move(blkSvc)
		);
		return SGX_SUCCESS;
	}
	catch(const std::exception& e)
	{
		using namespace DecentEnclave::Common;
		Platform::Print::StrErr(e.what());
		return SGX_ERROR_UNEXPECTED;
	}
}


extern "C" sgx_status_t ecall_ethereum_clt_set_receipt_rate(
	double receipt_rate
)
{
	try
	{
		EthereumClt::SetReceiptRate(receipt_rate);

		return SGX_SUCCESS;
	}
	catch(const std::exception& e)
	{
		using namespace DecentEnclave::Common;
		Platform::Print::StrErr(e.what());
		return SGX_ERROR_UNEXPECTED;
	}
}


extern "C" sgx_status_t ecall_ethereum_clt_recv_block(
	const uint8_t* hdr_rlp,
	size_t hdr_size
)
{
	try
	{
		std::vector<uint8_t> hdrRlp(hdr_rlp, hdr_rlp + hdr_size);
		EthereumClt::RecvBlock(hdrRlp);

		return SGX_SUCCESS;
	}
	catch(const std::exception& e)
	{
		using namespace DecentEnclave::Common;
		Platform::Print::StrErr(e.what());
		return SGX_ERROR_UNEXPECTED;
	}
}
