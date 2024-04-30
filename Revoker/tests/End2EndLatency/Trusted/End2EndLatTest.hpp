// Copyright (c) 2023 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <stdexcept>
#include <tuple>

#include <AdvancedRlp/AdvancedRlp.hpp>

#include <DecentEnclave/Common/DecentTlsConfig.hpp>
#include <DecentEnclave/Common/Logging.hpp>
#include <DecentEnclave/Common/TlsSocket.hpp>

#include <DecentEnclave/Trusted/ComponentConnection.hpp>
#include <DecentEnclave/Trusted/DecentLambdaClt.hpp>
#include <DecentEnclave/Trusted/Files.hpp>
#include <DecentEnclave/Trusted/HeartbeatRecvMgr.hpp>
#include <DecentEnclave/Trusted/Sgx/Random.hpp>
#include <DecentEnclave/Trusted/Time.hpp>

#include <EclipseMonitor/Eth/AbiParser.hpp>

#include <SimpleJson/SimpleJson.hpp>
#include <SimpleObjects/Codec/Hex.hpp>


namespace End2EndLatency
{


inline std::vector<uint8_t> EventMsgFromReceiptData(
	const SimpleObjects::BytesBaseObj& receiptData
)
{
	std::vector<uint8_t> evMsg;

	using _MsgParser =
		EclipseMonitor::Eth::AbiParser<
			SimpleObjects::ObjCategory::Bytes,
			std::true_type
		>;
	auto abiBegin = receiptData.begin();
	auto abiEnd = receiptData.end();

	std::tie(evMsg, abiBegin) =
		_MsgParser().ToPrimitive(abiBegin, abiEnd, abiBegin);

	return evMsg;
}


inline uint64_t UIntFromReceiptData(
	const SimpleObjects::BytesBaseObj& receiptData
)
{
	uint64_t value = 0;

	using _AbiParserUint64 =
		EclipseMonitor::Eth::AbiParser<
			SimpleObjects::ObjCategory::Integer,
			EclipseMonitor::Eth::AbiUInt64
		>;
	auto abiBegin = receiptData.begin();
	auto abiEnd = receiptData.end();

	std::tie(value, abiBegin) =
		_AbiParserUint64().ToPrimitive(abiBegin, abiEnd, abiBegin);

	return value;
}


inline DecentEnclave::Common::DetMsg BuildPubSubSubscribeMsg(
	const EclipseMonitor::Eth::ContractAddr& publisherAddr
)
{
	static const SimpleObjects::String sk_labelPublisher("publisher");

	SimpleObjects::Dict msgContent;
	msgContent[sk_labelPublisher] = SimpleObjects::Bytes(
		publisherAddr.begin(),
		publisherAddr.end()
	);

	DecentEnclave::Common::DetMsg msg;
	//msg.get_Version() = 1;
	msg.get_MsgId().get_MsgType() = SimpleObjects::String("PubSub.Subscribe");
	msg.get_MsgContent() = SimpleObjects::Bytes(
		AdvancedRlp::GenericWriter::Write(msgContent)
	);

	return msg;
}


inline DecentEnclave::Common::DetMsg BuildRecSubscribeMsg(
	const EclipseMonitor::Eth::ContractAddr& publisherAddr,
	const SimpleObjects::Bytes& eventTopic
)
{
	static const SimpleObjects::String sk_labelContract("contract");
	static const SimpleObjects::String sk_labelTopics("topics");

	SimpleObjects::Dict msgContent;
	msgContent[sk_labelContract] = SimpleObjects::Bytes(
		publisherAddr.begin(),
		publisherAddr.end()
	);
	msgContent[sk_labelTopics] = SimpleObjects::List({
		eventTopic,
	});

	DecentEnclave::Common::DetMsg msg;
	//msg.get_Version() = 1;
	msg.get_MsgId().get_MsgType() = SimpleObjects::String("Receipt.Subscribe");
	msg.get_MsgContent() = SimpleObjects::Bytes(
		AdvancedRlp::GenericWriter::Write(msgContent)
	);

	return msg;
}


inline DecentEnclave::Common::DetMsg BuildPubSubSubRecSubscribeMsg(
	const EclipseMonitor::Eth::ContractAddr& publisherAddr
)
{
	static const SimpleObjects::Bytes  sk_subEventTopic(
		std::vector<uint8_t>({
			// 0x25c40554b365d6dc8604681ad8a5a09bf15c93023ee0dd19850d61a1fe8501aa
			0x25U, 0xc4U, 0x05U, 0x54U, 0xb3U, 0x65U, 0xd6U, 0xdcU,
			0x86U, 0x04U, 0x68U, 0x1aU, 0xd8U, 0xa5U, 0xa0U, 0x9bU,
			0xf1U, 0x5cU, 0x93U, 0x02U, 0x3eU, 0xe0U, 0xddU, 0x19U,
			0x85U, 0x0dU, 0x61U, 0xa1U, 0xfeU, 0x85U, 0x01U, 0xaaU,
		})
	);

	return BuildRecSubscribeMsg(publisherAddr, sk_subEventTopic);
}


inline DecentEnclave::Common::DetMsg BuildTxnSubRecSubscribeMsg(
	const EclipseMonitor::Eth::ContractAddr& publisherAddr
)
{
	static const SimpleObjects::Bytes  sk_subEventTopic(
		std::vector<uint8_t>({
			// 0x497930a5c53bf6c9ad67d88cc1f7ad2c8fcb388ba86238ebaed9943deffadf83
			0x49U, 0x79U, 0x30U, 0xa5U, 0xc5U, 0x3bU, 0xf6U, 0xc9U,
			0xadU, 0x67U, 0xd8U, 0x8cU, 0xc1U, 0xf7U, 0xadU, 0x2cU,
			0x8fU, 0xcbU, 0x38U, 0x8bU, 0xa8U, 0x62U, 0x38U, 0xebU,
			0xaeU, 0xd9U, 0x94U, 0x3dU, 0xefU, 0xfaU, 0xdfU, 0x83U,
		})
	);

	return BuildRecSubscribeMsg(publisherAddr, sk_subEventTopic);
}


inline DecentEnclave::Common::DetMsg BuildPubRecSubscribeMsg(
	const EclipseMonitor::Eth::ContractAddr& publisherAddr
)
{
	static const SimpleObjects::Bytes  sk_pubEventTopic(
		std::vector<uint8_t>({
			// 0x94dbeabe1441967db68cfd3ee62ebd5240293711320f84ee429c016193d81da7
			0x94U, 0xdbU, 0xeaU, 0xbeU, 0x14U, 0x41U, 0x96U, 0x7dU,
			0xb6U, 0x8cU, 0xfdU, 0x3eU, 0xe6U, 0x2eU, 0xbdU, 0x52U,
			0x40U, 0x29U, 0x37U, 0x11U, 0x32U, 0x0fU, 0x84U, 0xeeU,
			0x42U, 0x9cU, 0x01U, 0x61U, 0x93U, 0xd8U, 0x1dU, 0xa7U,
		})
	);

	return BuildRecSubscribeMsg(publisherAddr, sk_pubEventTopic);
}


inline
std::tuple<uint64_t, uint64_t, uint64_t>
RunPubSubTest(
	const EclipseMonitor::Eth::ContractAddr& publisherAddr,
	const EclipseMonitor::Eth::ContractAddr& subscriberAddr
)
{
	using namespace DecentEnclave;

	static auto s_logger =
		Common::LoggerFactory::GetLogger("End2EndLatency::RunPubSubTest");

	std::vector<uint8_t> expectedValue(16);
	Trusted::Sgx::RandGenerator().Rand(expectedValue.data(), expectedValue.size());
	const SimpleObjects::Bytes expectedValueBytes(expectedValue);

	std::mutex timeMutex;
	std::condition_variable cv;
	std::atomic_uint64_t pubTime(0);
	std::atomic_uint64_t subsTime(0);

	auto pubsubCallback =
		[&pubTime, &cv, expectedValueBytes]
		(std::vector<uint8_t> heartbeatMsg) -> void
	{
		static auto s_cbLogger =
			Common::LoggerFactory::GetLogger(
				"End2EndLatency::RunPubSubTest::pubsubCallback"
			);
		// s_cbLogger.Debug("Received heartbeat message");

		// 1. Get value from event
		auto msg = AdvancedRlp::Parse(heartbeatMsg);

		static const SimpleObjects::String sk_labelEvents("Events");

		const auto& msgDict = msg.AsDict();
		const auto& evQueue = msgDict[sk_labelEvents].AsList();

		if (evQueue.size() > 0)
		{
			s_cbLogger.Debug("Received " + std::to_string(evQueue.size()) + " events");
			const auto& evFields = evQueue[0].AsList();
			const auto& evData = evFields[1].AsBytes();

			// 2. Check if it is the expected value
			if (evData == expectedValueBytes)
			{
				// 3. If yes, record the time
				pubTime.store(DecentEnclave::Trusted::UntrustedTime::Timestamp());

				// 4. Notify the main thread
				cv.notify_one();
			}
		}
	};

	auto subsCallback =
		[&subsTime, &cv, expectedValue]
		(std::vector<uint8_t> heartbeatMsg) -> void
	{
		static auto s_cbLogger =
			Common::LoggerFactory::GetLogger(
				"End2EndLatency::RunPubSubTest::subsCallback"
			);

		// s_cbLogger.Debug("Received heartbeat message");

		// 1. Get value from event
		auto msg = AdvancedRlp::Parse(heartbeatMsg);

		static const SimpleObjects::String sk_labelReceipts("Receipts");

		const auto& msgDict = msg.AsDict();
		const auto& recQueue = msgDict[sk_labelReceipts].AsList();

		if (recQueue.size() > 0)
		{
			s_cbLogger.Debug("Received " + std::to_string(recQueue.size()) + " receipts");
			const auto& recFields = recQueue[0].AsList();
			const auto& recData = recFields[2].AsBytes();
			// parse ABI encoding in receipt data
			auto confirmMsg = EventMsgFromReceiptData(recData);

			// 2. Check if it is the expected value
			if (confirmMsg == expectedValue)
			{
				// 3. If yes, record the time
				subsTime.store(DecentEnclave::Trusted::UntrustedTime::Timestamp());

				// 4. Notify the main thread
				cv.notify_one();
			}
		}
	};

	// 1. Subscribe to publisher
	s_logger.Info(
		"Subscribing to publisher @" +
			SimpleObjects::Codec::Hex::Encode<std::string>(publisherAddr)
	);
	auto subMsg = BuildPubSubSubscribeMsg(publisherAddr);
	std::shared_ptr<Common::TlsSocket> pubsubTlsSocket = Trusted::MakeLambdaCall(
		"DecentEthereum",
		Common::DecentTlsConfig::MakeTlsConfig(
			false,
			"Secp256r1",
			"Secp256r1"
		),
		subMsg // lvalue reference needed
	);
	auto pubsubHbConstraint =
		std::make_shared<Trusted::HeartbeatTimeConstraint<uint64_t> >(
			1000
		);
	Trusted::HeartbeatRecvMgr::GetInstance().AddRecv(
		pubsubHbConstraint,
		pubsubTlsSocket,
		pubsubCallback,
		true
	);

	// 2. Subscribe to subscriber
	s_logger.Info(
		"Subscribing to subscriber @" +
			SimpleObjects::Codec::Hex::Encode<std::string>(subscriberAddr)
	);
	subMsg = BuildPubSubSubRecSubscribeMsg(subscriberAddr);
	std::shared_ptr<Common::TlsSocket> subTlsSocket = Trusted::MakeLambdaCall(
		"DecentEthereum",
		Common::DecentTlsConfig::MakeTlsConfig(
			false,
			"Secp256r1",
			"Secp256r1"
		),
		subMsg // lvalue reference needed
	);
	auto subHbConstraint =
		std::make_shared<Trusted::HeartbeatTimeConstraint<uint64_t> >(
			1000
		);
	Trusted::HeartbeatRecvMgr::GetInstance().AddRecv(
		subHbConstraint,
		subTlsSocket,
		subsCallback,
		true
	);

	// 3. Record publish time
	uint64_t publishTime = DecentEnclave::Trusted::UntrustedTime::Timestamp();

	// 4. Publish message to publisher contract
	auto gethPxyCon = Trusted::ComponentConnection::Connect("gethProxy");
	std::string pxyMsg =
		"{\"method\": \"OraclePublish\", \"params\": [\""+
			SimpleObjects::Codec::Hex::Encode<std::string>(expectedValue) +
			"\"]}";
	gethPxyCon->SizedSendBytes(pxyMsg);
	gethPxyCon.reset();

	// 5. Wait for results to be ready for collection
	std::unique_lock<std::mutex> lock(timeMutex);
	while (pubTime.load() == 0 || subsTime.load() == 0)
	{
		cv.wait(lock);
	}

	// 6. Gether results
	s_logger.Info("Published on:          " + std::to_string(publishTime));
	s_logger.Info("PubSub Notified on:    " + std::to_string(pubTime.load()));
	s_logger.Info("Subscriber Confirm on: " + std::to_string(subsTime.load()));

	Trusted::HeartbeatRecvMgr::GetInstance().RemoveRecv(
		pubsubHbConstraint,
		pubsubTlsSocket
	);
	pubsubHbConstraint.reset();
	pubsubTlsSocket.reset();
	Trusted::HeartbeatRecvMgr::GetInstance().RemoveRecv(
		subHbConstraint,
		subTlsSocket
	);
	subHbConstraint.reset();
	subTlsSocket.reset();

	return std::make_tuple(publishTime, pubTime.load(), subsTime.load());
}


inline
std::tuple<uint64_t, uint64_t, uint64_t>
MonitorAndReactTest(
	const EclipseMonitor::Eth::ContractAddr& publisherAddr,
	const EclipseMonitor::Eth::ContractAddr& subscriberAddr
)
{
	using namespace DecentEnclave;

	static auto s_logger =
		Common::LoggerFactory::GetLogger("End2EndLatency::MonitorAndReactTest");

	std::vector<uint8_t> expectedValue(16);
	Trusted::Sgx::RandGenerator().Rand(expectedValue.data(), expectedValue.size());
	const SimpleObjects::Bytes expectedValueBytes(expectedValue);

	std::mutex timeMutex;
	std::condition_variable cv;
	std::atomic_uint64_t pubTime(0);
	std::atomic_uint64_t subsTime(0);

	auto oracleCallback =
		[&pubTime, expectedValue]
		(std::vector<uint8_t> heartbeatMsg) -> void
	{
		static const SimpleObjects::String sk_labelReceipts("Receipts");
		static auto s_cbLogger =
			Common::LoggerFactory::GetLogger(
				"End2EndLatency::MonitorAndReactTest::oracleCallback"
			);
		// s_cbLogger.Debug("Received heartbeat message");

		// 1. Get value from event
		auto msg = AdvancedRlp::Parse(heartbeatMsg);

		const auto& msgDict = msg.AsDict();
		const auto& recQueue = msgDict[sk_labelReceipts].AsList();

		if (recQueue.size() > 0)
		{
			s_cbLogger.Debug("Received " + std::to_string(recQueue.size()) + " receipts");
			const auto& recFields = recQueue[0].AsList();
			const auto& recData = recFields[2].AsBytes();
			// parse ABI encoding in receipt data
			auto confirmMsg = EventMsgFromReceiptData(recData);

			// 2. Check if it is the expected value
			if (confirmMsg == expectedValue)
			{
				// 3. If yes, record the time
				pubTime.store(DecentEnclave::Trusted::UntrustedTime::Timestamp());

				// 4. Publish the same message to subscriber contract
				// s_cbLogger.Debug("Sending request to GethProxy");
				auto gethPxyCon = Trusted::ComponentConnection::Connect("gethProxy");
				std::string pxyMsg =
					"{\"method\": \"SubscriberTransact\", \"params\": [\""+
						SimpleObjects::Codec::Hex::Encode<std::string>(expectedValue) +
						"\"]}";
				gethPxyCon->SizedSendBytes(pxyMsg);
				s_cbLogger.Debug("Sent GethProxy request: " + pxyMsg);
				gethPxyCon.reset();
			}
		}
	};

	auto subsCallback =
		[&subsTime, &cv, expectedValue]
		(std::vector<uint8_t> heartbeatMsg) -> void
	{
		static const SimpleObjects::String sk_labelReceipts("Receipts");
		static auto s_cbLogger =
			Common::LoggerFactory::GetLogger(
				"End2EndLatency::MonitorAndReactTest::subsCallback"
			);
		// s_cbLogger.Debug("Received heartbeat message");

		// 1. Get value from event
		auto msg = AdvancedRlp::Parse(heartbeatMsg);

		const auto& msgDict = msg.AsDict();
		const auto& recQueue = msgDict[sk_labelReceipts].AsList();

		if (recQueue.size() > 0)
		{
			s_cbLogger.Debug("Received " + std::to_string(recQueue.size()) + " receipts");
			const auto& recFields = recQueue[0].AsList();
			const auto& recData = recFields[2].AsBytes();
			// parse ABI encoding in receipt data
			auto confirmMsg = EventMsgFromReceiptData(recData);

			// 2. Check if it is the expected value
			if (confirmMsg == expectedValue)
			{
				// 3. If yes, record the time
				subsTime.store(DecentEnclave::Trusted::UntrustedTime::Timestamp());

				// 4. Notify the main thread
				cv.notify_one();
			}
		}
	};

	// 1. Subscribe to publisher
	s_logger.Info(
		"Subscribing to oracle contract @" +
			SimpleObjects::Codec::Hex::Encode<std::string>(publisherAddr)
	);
	auto subMsg = BuildPubRecSubscribeMsg(publisherAddr);
	std::shared_ptr<Common::TlsSocket> pubsubTlsSocket = Trusted::MakeLambdaCall(
		"DecentEthereum",
		Common::DecentTlsConfig::MakeTlsConfig(
			false,
			"Secp256r1",
			"Secp256r1"
		),
		subMsg // lvalue reference needed
	);
	auto pubsubHbConstraint =
		std::make_shared<Trusted::HeartbeatTimeConstraint<uint64_t> >(
			1000
		);
	Trusted::HeartbeatRecvMgr::GetInstance().AddRecv(
		pubsubHbConstraint,
		pubsubTlsSocket,
		oracleCallback,
		true
	);

	// 2. Subscribe to subscriber
	s_logger.Info(
		"Subscribing to subscriber @" +
			SimpleObjects::Codec::Hex::Encode<std::string>(subscriberAddr)
	);
	subMsg = BuildTxnSubRecSubscribeMsg(subscriberAddr);
	std::shared_ptr<Common::TlsSocket> subTlsSocket = Trusted::MakeLambdaCall(
		"DecentEthereum",
		Common::DecentTlsConfig::MakeTlsConfig(
			false,
			"Secp256r1",
			"Secp256r1"
		),
		subMsg // lvalue reference needed
	);
	auto subHbConstraint =
		std::make_shared<Trusted::HeartbeatTimeConstraint<uint64_t> >(
			1000
		);
	Trusted::HeartbeatRecvMgr::GetInstance().AddRecv(
		subHbConstraint,
		subTlsSocket,
		subsCallback,
		true
	);

	// 3. Record publish time
	uint64_t publishTime = DecentEnclave::Trusted::UntrustedTime::Timestamp();

	// 4. Publish message to publisher contract
	auto gethPxyCon = Trusted::ComponentConnection::Connect("gethProxy");
	std::string pxyMsg =
		"{\"method\": \"OraclePublish\", \"params\": [\""+
			SimpleObjects::Codec::Hex::Encode<std::string>(expectedValue) +
			"\"]}";
	gethPxyCon->SizedSendBytes(pxyMsg);
	gethPxyCon.reset();

	// 5. Wait for results to be ready for collection
	std::unique_lock<std::mutex> lock(timeMutex);
	while (subsTime.load() == 0)
	{
		cv.wait(lock);
	}

	// 6. Gether results
	s_logger.Info("Published on:          " + std::to_string(publishTime));
	s_logger.Info("PubSub Notified on:    " + std::to_string(pubTime.load()));
	s_logger.Info("Subscriber Confirm on: " + std::to_string(subsTime.load()));

	Trusted::HeartbeatRecvMgr::GetInstance().RemoveRecv(
		pubsubHbConstraint,
		pubsubTlsSocket
	);
	pubsubHbConstraint.reset();
	pubsubTlsSocket.reset();
	Trusted::HeartbeatRecvMgr::GetInstance().RemoveRecv(
		subHbConstraint,
		subTlsSocket
	);
	subHbConstraint.reset();
	subTlsSocket.reset();

	return std::make_tuple(publishTime, pubTime.load(), subsTime.load());
}


inline void RunTest(
	const EclipseMonitor::Eth::ContractAddr& pubsubAddr,
	const EclipseMonitor::Eth::ContractAddr& publisherAddr,
	const EclipseMonitor::Eth::ContractAddr& subscriberAddr
)
{
	using namespace SimpleObjects;

	static constexpr size_t sk_repeatTest = 3;

	(void)pubsubAddr;

	List pubsubResult;
	List monitorAndReactResult;

	for (size_t i = 0; i < sk_repeatTest; ++i)
	{
		uint64_t publishedOn = 0;
		uint64_t oracleNotifiedOn = 0;
		uint64_t subscriberConfirmOn = 0;

		std::tie(publishedOn, oracleNotifiedOn, subscriberConfirmOn) =
			RunPubSubTest(publisherAddr, subscriberAddr);
		List pubsubResOneSet;
		pubsubResOneSet.push_back(UInt64(publishedOn));
		pubsubResOneSet.push_back(UInt64(oracleNotifiedOn));
		pubsubResOneSet.push_back(UInt64(subscriberConfirmOn));
		pubsubResult.push_back(std::move(pubsubResOneSet));

		std::tie(publishedOn, oracleNotifiedOn, subscriberConfirmOn) =
			MonitorAndReactTest(publisherAddr, subscriberAddr);
		List monitorAndReactResOneSet;
		monitorAndReactResOneSet.push_back(UInt64(publishedOn));
		monitorAndReactResOneSet.push_back(UInt64(oracleNotifiedOn));
		monitorAndReactResOneSet.push_back(UInt64(subscriberConfirmOn));
		monitorAndReactResult.push_back(std::move(monitorAndReactResOneSet));
	}

	Dict testResults;
	testResults[String("PubSub")] = std::move(pubsubResult);
	testResults[String("MonitorAndReact")] = std::move(monitorAndReactResult);

	SimpleJson::WriterConfig writeConf;
	writeConf.m_indent = "\t";
	std::string resJson = SimpleJson::DumpStr(testResults, writeConf);

	// write to file
	auto file = DecentEnclave::Trusted::
		WBUntrustedFile::Create("End2EndLatencyResult.json");
	file->WriteBytes(resJson);
}


} // namespace End2EndLatency
