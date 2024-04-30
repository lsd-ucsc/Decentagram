// Copyright (c) 2023 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>
#include <vector>

#include <AdvancedRlp/AdvancedRlp.hpp>

#include <DecentEnclave/Common/DeterministicMsg.hpp>
#include <DecentEnclave/Common/Logging.hpp>

#include <EclipseMonitor/MonitorReport.hpp>
#include <EclipseMonitor/Eth/DataTypes.hpp>

#include <SimpleObjects/Codec/Hex.hpp>
#include <SimpleObjects/SimpleObjects.hpp>


namespace Revoker
{


inline DecentEnclave::Common::DetMsg BuildSubscribeMsg(
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


inline EclipseMonitor::Eth::BlockNumber BlkNumFromBytesBase(
	const SimpleObjects::BytesBaseObj& bytes
)
{
	SimpleObjects::Bytes blkNumBytes(
		bytes.begin(),
		bytes.end()
	);

	return EclipseMonitor::Eth::BlkNumTypeTrait::FromBytes(blkNumBytes);
}


inline void HandleRevokeEvent(const SimpleObjects::ListBaseObj& evList)
{
	static DecentEnclave::Common::Logger s_logger =
		DecentEnclave::Common::LoggerFactory::GetLogger(
			"HandleEthHeartbeatMsg"
		);

	s_logger.Debug("Received " + std::to_string(evList.size()) + " events");

	for (const auto& ev: evList)
	{
		const auto& evFields = ev.AsList();

		// each event has 2 fields: [blkNum, evData]
		const auto& blkNumRef = evFields[0].AsBytes();
		const auto& evDataRef = evFields[1].AsBytes();

		auto blkNum = BlkNumFromBytesBase(blkNumRef);

		std::string dataHex = SimpleObjects::Codec::Hex::Encode<std::string>(
			evDataRef
		);

		s_logger.Debug(
			"Received Data: " + dataHex +
			" @ block " + std::to_string(blkNum)
		);
	}
}


inline void HandleEthHeartbeatMsg(std::vector<uint8_t> msgAdvRlp)
{
	static DecentEnclave::Common::Logger s_logger =
		DecentEnclave::Common::LoggerFactory::GetLogger(
			"HandleEthHeartbeatMsg"
		);


	auto msg = AdvancedRlp::Parse(msgAdvRlp);

	static const SimpleObjects::String sk_labelSecState("SecState");
	static const SimpleObjects::String sk_labelLatestBlkNum("LatestBlkNum");
	static const SimpleObjects::String sk_labelEvents("Events");

	const auto& msgDict = msg.AsDict();

	const auto& secStateAdvRlp = msgDict[sk_labelSecState].AsBytes();
	const auto& latestBlkNumBytes = msgDict[sk_labelLatestBlkNum].AsBytes();
	const auto& evQueue = msgDict[sk_labelEvents].AsList();

	std::vector<uint8_t> secStateBytes(
		secStateAdvRlp.begin(),
		secStateAdvRlp.end()
	);
	auto secState =
		EclipseMonitor::MonitorSecStateParser().Parse(secStateBytes);
	auto latestBlkNum = BlkNumFromBytesBase(latestBlkNumBytes);

	const auto& chkptHash = secState.get_checkpointHash();
	const auto& chkptNumBytes = secState.get_checkpointNum();

	auto chkptHashHex =
		SimpleObjects::Codec::Hex::Encode<std::string>(chkptHash);
	auto chkptNum =
		EclipseMonitor::Eth::BlkNumTypeTrait::FromBytes(chkptNumBytes);
	s_logger.Info(
		std::string("Received Decent Ethereum Heartbeat:\n") +
		"Latest block number: " + std::to_string(latestBlkNum) + "\n" +
		"Checkpoint number:   " + std::to_string(chkptNum) + "\n" +
		"Checkpoint hash:     " + chkptHashHex
	);

	HandleRevokeEvent(evQueue);
}


} // namespace Revoker
