// Copyright (c) 2022 Decentagram
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <string>
#include <vector>

#include <DecentEnclave/Untrusted/CUrl.hpp>
#include <EclipseMonitor/Eth/DataTypes.hpp>
#include <SimpleJson/SimpleJson.hpp>
#include <SimpleObjects/Codec/Hex.hpp>
#include <SimpleObjects/SimpleObjects.hpp>


namespace EthereumClt
{


class GethRequester
{
public: // static members:





public:


	GethRequester(const std::string& url) :
		m_url(url)
	{}


	~GethRequester() = default;


	std::vector<uint8_t> GetHeaderRlpByParam(
		const std::string& param
	) const
	{
		// curl "http://127.0.0.1:8545/" -X POST
		//   -H "Content-Type: application/json"
		//   --data
		//     '{ "method":"debug_getRawHeader",
		//       "params":["0x1"], "id":1, "jsonrpc":"2.0" }'

		static const SimpleObjects::String sk_reqBodyValGetHdlRlp =
			"debug_getRawHeader";

		std::string reqBodyJson = BuildRequestBody(
			sk_reqBodyValGetHdlRlp,
			{
				SimpleObjects::String(param),
			}
		);

		std::string respBodyJson = PostRequest(reqBodyJson);

		return ProcRespSingleByte(respBodyJson);
	}


	std::vector<uint8_t> GetBodyRlpByParam(
		const std::string& param
	) const
	{
		// curl "http://127.0.0.1:8545/" -X POST
		//   -H "Content-Type: application/json"
		//   --data
		//     '{ "method":"debug_getRawBlock",
		//       "params":["0x1"], "id":1, "jsonrpc":"2.0" }'

		static const SimpleObjects::String sk_reqBodyValGetBlkRlp =
			"debug_getRawBlock";

		std::string reqBodyJson = BuildRequestBody(
			sk_reqBodyValGetBlkRlp,
			{
				SimpleObjects::String(param),
			}
		);

		std::string respBodyJson = PostRequest(reqBodyJson);

		return ProcRespSingleByte(respBodyJson);
	}


	template<typename _RetType>
	_RetType GetReceiptsRlpByParam(
		const std::string& param
	) const
	{
		// curl "http://127.0.0.1:8545/" -X POST
		//   -H "Content-Type: application/json"
		//   --data
		//     '{ "method":"debug_getRawReceipts",
		//       "params":["0x1"], "id":1, "jsonrpc":"2.0" }'

		static const SimpleObjects::String sk_reqBodyValGetBlkRlp =
			"debug_getRawReceipts";

		std::string reqBodyJson = BuildRequestBody(
			sk_reqBodyValGetBlkRlp,
			{
				SimpleObjects::String(param),
			}
		);

		std::string respBodyJson = PostRequest(reqBodyJson);

		return ProcRespListOfBytes<_RetType>(respBodyJson);
	}


	std::vector<uint8_t> GetHeaderRlpByNum(
		EclipseMonitor::Eth::BlockNumber blockNum
	) const
	{
		return GetHeaderRlpByParam(ConvertBlkNumToHex(blockNum));
	}


	std::vector<uint8_t> GetBodyRlpByNum(
		EclipseMonitor::Eth::BlockNumber blockNum
	) const
	{
		return GetBodyRlpByParam(ConvertBlkNumToHex(blockNum));
	}


	template<typename _RetType>
	_RetType GetReceiptsRlpByNum(
		EclipseMonitor::Eth::BlockNumber blockNum
	) const
	{
		return GetReceiptsRlpByParam<_RetType>(ConvertBlkNumToHex(blockNum));
	}


protected:


	static std::string BuildRequestBody(
		SimpleObjects::String method,
		SimpleObjects::List params
	)
	{
		static const SimpleObjects::String sk_reqBodyLabelMethod = "method";
		static const SimpleObjects::String sk_reqBodyLabelParams = "params";
		static const SimpleObjects::String sk_reqBodyLabelId = "id";
		static const SimpleObjects::String sk_reqBodyLabelJsonRpc = "jsonrpc";

		static const SimpleObjects::UInt8 sk_reqBodyValId =
			SimpleObjects::UInt8(1);
		static const SimpleObjects::String sk_reqBodyValJsonRpc =
			"2.0";

		SimpleObjects::Dict reqBody;
		reqBody[sk_reqBodyLabelMethod]  = std::move(method);
		reqBody[sk_reqBodyLabelParams]  = std::move(params);
		reqBody[sk_reqBodyLabelId]      = sk_reqBodyValId;
		reqBody[sk_reqBodyLabelJsonRpc] = sk_reqBodyValJsonRpc;

		std::string reqBodyJson = SimpleJson::DumpStr(reqBody);

		return reqBodyJson;
	}


	std::string PostRequest(
		const std::string& reqBody
	) const
	{
		std::string respBody;
		DecentEnclave::Untrusted::CUrlContentCallBack contentCallback =
			[&respBody]
			(char* ptr, size_t size, size_t nmemb, void*) -> size_t
			{
				respBody += std::string(ptr, size * nmemb);

				return size * nmemb;
			};

		DecentEnclave::Untrusted::CUrlRequestExpectRespCode(
			m_url,
			"POST",
			{
				"Content-Type: application/json",
			},
			reqBody,
			nullptr,
			&contentCallback,
			200
		);

		return respBody;
	}


	static std::vector<uint8_t> ProcRespSingleByte(
		const std::string& respBody
	)
	{
		static const SimpleObjects::String sk_respBodyLabelResult = "result";

		auto respBodyJson = SimpleJson::LoadStr(respBody);
		const auto& resHex =
			respBodyJson.AsDict()[sk_respBodyLabelResult].AsString();

		if (
			resHex.size() < 3 ||
			resHex[0] != '0' ||
			resHex[1] != 'x'
		)
		{
			throw std::runtime_error("Invalid response from Geth.");
		}

		std::vector<uint8_t> res =
			SimpleObjects::Codec::Hex::Decode<std::vector<uint8_t> >(
				resHex.begin() + 2,
				resHex.end()
			);

		return res;
	}


	template<typename _RetType>
	static _RetType ProcRespListOfBytes(
		const std::string& respBody
	)
	{
		using _RetTypeValType = typename _RetType::value_type;

		static const SimpleObjects::String sk_respBodyLabelResult = "result";

		auto respBodyJson = SimpleJson::LoadStr(respBody);
		const auto& resList =
			respBodyJson.AsDict()[sk_respBodyLabelResult].AsList();

		_RetType res;
		res.reserve(resList.size());
		for (const auto& resHexObj : resList)
		{
			const auto& resHex = resHexObj.AsString();

			if (
				resHex.size() < 3 ||
				resHex[0] != '0' ||
				resHex[1] != 'x'
			)
			{
				throw std::runtime_error("Invalid response from Geth.");
			}

			res.push_back(_RetTypeValType(
				SimpleObjects::Codec::Hex::Decode<std::vector<uint8_t> >(
					resHex.begin() + 2,
					resHex.end()
				)
			));
		}

		return res;
	}

	static std::string ConvertBlkNumToHex(
		EclipseMonitor::Eth::BlockNumber blockNum
	)
	{
		return SimpleObjects::Codec::Hex::
			template Encode<std::string>(blockNum);
	}


private:


	std::string m_url;


}; // class GethRequester


} // namespace EthereumClt
