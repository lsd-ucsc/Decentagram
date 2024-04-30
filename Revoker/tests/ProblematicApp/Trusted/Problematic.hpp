// Copyright (c) 2022 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#include <DecentEnclave/Common/DeterministicMsg.hpp>
#include <DecentEnclave/Common/Logging.hpp>

#include <DecentEnclave/Trusted/Files.hpp>
#include <DecentEnclave/Trusted/Sgx/Random.hpp>

#include <mbedTLScpp/Hash.hpp>

#include <SimpleJson/SimpleJson.hpp>
#include <SimpleObjects/Codec/Hex.hpp>
#include <SimpleObjects/SimpleObjects.hpp>

#include "Certs.hpp"
#include "Keys.hpp"


using namespace DecentEnclave::Common;
using namespace DecentEnclave::Trusted::Sgx;


namespace ProblematicApp
{

void GenProblemCredOnce()
{
	auto logger = LoggerFactory::GetLogger("GenProblemCredOnce");

	using namespace SimpleObjects;
	Dict resJson;

	RandGenerator rand;

	// 1. output certificate
	auto certDer = DecentCert_Secp256k1::Fetch()->GetDer();
	resJson[String("appCertDer")] = Codec::Hex::Encode<String>(certDer);

	// 2. generate two messages
	Bytes eventId;
	eventId.resize(16);
	rand.Rand(&(eventId[0]), eventId.size());

	Bytes msgContent1;
	msgContent1.resize(32);
	rand.Rand(&(msgContent1[0]), msgContent1.size());

	Bytes msgContent2;
	msgContent2.resize(32);
	rand.Rand(&(msgContent2[0]), msgContent2.size());

	DetMsgId msgId;
	msgId.get_MsgType() = String("Problematic");
	msgId.get_Ext() = eventId;
	std::vector<uint8_t> msgIdBytes = AdvancedRlp::GenericWriter::Write(msgId);

	using namespace mbedTLScpp;

	auto msgIdHash = Hasher<HashType::SHA256>().Calc(CtnFullR(msgIdBytes));

	auto content1Hash = Hasher<HashType::SHA256>().Calc(
		CtnFullR(msgContent1.GetVal())
	);
	auto content2Hash = Hasher<HashType::SHA256>().Calc(
		CtnFullR(msgContent2.GetVal())
	);

	// 3. sign the messages
	auto msg1Hash = Hasher<HashType::SHA256>().Calc(
		CtnFullR(msgIdHash.m_data),
		CtnFullR(content1Hash.m_data)
	);
	auto msg2Hash = Hasher<HashType::SHA256>().Calc(
		CtnFullR(msgIdHash.m_data),
		CtnFullR(content2Hash.m_data)
	);

	BigNum r;
	BigNum s;

	std::tie(r, s) =
		DecentKey_Secp256k1::GetKey().SignInBigNum(msg1Hash, rand);
	auto sign1R = r.Bytes<false>();
	auto sign1S = s.Bytes<false>();

	std::tie(r, s) =
		DecentKey_Secp256k1::GetKey().SignInBigNum(msg2Hash, rand);
	auto sign2R = r.Bytes<false>();
	auto sign2S = s.Bytes<false>();

	// 4. output the messages, hash, and signature
	resJson[String("msgId")] = Codec::Hex::Encode<String>(msgIdBytes);
	resJson[String("msgContent1")] = Codec::Hex::Encode<String>(msgContent1);
	resJson[String("msgContent2")] = Codec::Hex::Encode<String>(msgContent2);

	resJson[String("msgIdHash")] = Codec::Hex::Encode<String>(msgIdHash.m_data);
	resJson[String("msgContent1Hash")] = Codec::Hex::Encode<String>(content1Hash.m_data);
	resJson[String("msgContent2Hash")] = Codec::Hex::Encode<String>(content2Hash.m_data);

	resJson[String("msg1SignR")] = Codec::Hex::Encode<String>(sign1R);
	resJson[String("msg1SignS")] = Codec::Hex::Encode<String>(sign1S);
	resJson[String("msg2SignR")] = Codec::Hex::Encode<String>(sign2R);
	resJson[String("msg2SignS")] = Codec::Hex::Encode<String>(sign2S);

	// 5. output the private key
	auto privKeyDer = DecentKey_Secp256k1::GetKey().GetPrivateDer();
	resJson[String("privKeyDer")] = Codec::Hex::Encode<String>(privKeyDer);

	// finally, generate and print the json string
	const BaseObj& resJsonObj = resJson;
	SimpleJson::WriterConfig writeConf;
	writeConf.m_indent = "\t";
	writeConf.m_orderDict = true;
	std::string resStr = SimpleJson::DumpStr(resJsonObj, writeConf);
	logger.Info(resStr);

	// write to file
	auto file = DecentEnclave::Trusted::
		WBUntrustedFile::Create("credentials.json");
	file->WriteBytes(resStr);
}


void GenProblemCred()
{
	auto logger = LoggerFactory::GetLogger("GenProblemCred");

	for (size_t i = 0; i < 1; ++i)
	{
		logger.Info(
			"Generating problematic credentials " + std::to_string(i) + " ..."
		);
		GenProblemCredOnce();
	}
}

} // namespace ProblematicApp
