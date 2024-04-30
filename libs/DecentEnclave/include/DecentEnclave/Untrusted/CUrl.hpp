// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <functional>
#include <string>
#include <vector>

#include <curl/curl.h>

#include "../Common/Exceptions.hpp"
#include "../Common/Utils.hpp"


namespace DecentEnclave
{
namespace Untrusted
{


using CUrlContentCallBack =
	std::function<size_t(char*, size_t, size_t, void*)>;
using CUrlHeaderCallBack =
	std::function<size_t(char*, size_t, size_t, void*)>;


inline size_t CUrlHeaderCallbackWrapper(
	char *ptr,
	size_t size,
	size_t nitems,
	void *userdata
)
{
	if (userdata == nullptr)
	{
		// This callback is optional, so we do nothing here

		// If returned amount differs from the amount passed in,
		// it will signal an error to the library and cause the transfer
		// to get aborted
		// - https://curl.se/libcurl/c/CURLOPT_HEADERFUNCTION.html
		return size * nitems;
	}
	CUrlHeaderCallBack& callbackFunc =
		*static_cast<CUrlHeaderCallBack*>(userdata);
	return callbackFunc(ptr, size, nitems, nullptr);
}


inline size_t CUrlContentCallbackWrapper(
	char *ptr,
	size_t size,
	size_t nmemb,
	void *userdata
)
{
	if (userdata == nullptr)
	{
		// This callback is optional, so we do nothing here

		// If returned amount differs from the amount passed in,
		// it will signal an error to the library and cause the transfer
		// to get aborted
		// - https://curl.se/libcurl/c/CURLOPT_HEADERFUNCTION.html
		return size * nmemb;
	}
	CUrlContentCallBack& callbackFunc =
		*static_cast<CUrlContentCallBack*>(userdata);
	return callbackFunc(ptr, size, nmemb, nullptr);
}


inline uint16_t CUrlRequest(
	const std::string& url,
	const std::string& method,
	const std::vector<std::string>& headerStrs,
	const std::string& body,
	CUrlHeaderCallBack* headerCallback,
	CUrlContentCallBack* contentCallback
)
{
	// Initialize curl
	CURL *hnd = curl_easy_init();
	if (hnd == nullptr)
	{
		throw Common::Exception("Failed to initialize curl");
	}

	// Initialize curl headers
	curl_slist* headers = nullptr;
	for (const auto& headerStr : headerStrs)
	{
		curl_slist* tmp = curl_slist_append(headers, headerStr.c_str());
		if (tmp == nullptr)
		{
			curl_slist_free_all(headers);
			curl_easy_cleanup(hnd);
			throw Common::Exception("Failed to initialize curl headers");
		}
		headers = tmp;
	}

	// Set curl options
	if (
		// curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L)
		//	!= CURLE_OK || // Turn this on for debugging
		curl_easy_setopt(hnd, CURLOPT_TCP_NODELAY, 1)
			!= CURLE_OK ||
		curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, method.c_str())
			!= CURLE_OK ||
		curl_easy_setopt(hnd, CURLOPT_URL, url.c_str())
			!= CURLE_OK ||
		curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L)
			!= CURLE_OK ||
		curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L)
			!= CURLE_OK ||
		curl_easy_setopt(
			hnd, CURLOPT_HEADERFUNCTION, &CUrlHeaderCallbackWrapper
		)
			!= CURLE_OK ||
		curl_easy_setopt(hnd, CURLOPT_HEADERDATA, headerCallback)
			!= CURLE_OK ||
		curl_easy_setopt(
			hnd, CURLOPT_WRITEFUNCTION, &CUrlContentCallbackWrapper
		)
			!= CURLE_OK ||
		curl_easy_setopt(hnd, CURLOPT_WRITEDATA, contentCallback)
			!= CURLE_OK ||
		curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers)
			!= CURLE_OK
	)
	{
		curl_slist_free_all(headers);
		curl_easy_cleanup(hnd);
		throw Common::Exception("Failed to set curl options");
	}

	if (body.size() > 0)
	{
		if (
			curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, body.c_str())
				!= CURLE_OK ||
			curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE, body.size())
				!= CURLE_OK
		)
		{
			curl_slist_free_all(headers);
			curl_easy_cleanup(hnd);
			throw Common::Exception("Failed to set curl request body");
		}
	}

	long response_code = 0;
	if (
		curl_easy_perform(hnd) != CURLE_OK ||
		curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &response_code)
			!= CURLE_OK
	)
	{
		curl_slist_free_all(headers);
		curl_easy_cleanup(hnd);
		throw Common::Exception("Failed to perform curl request");
	}

	curl_slist_free_all(headers);
	curl_easy_cleanup(hnd);

	return static_cast<uint16_t>(response_code);
}


inline void CUrlRequestExpectRespCode(
	const std::string& url,
	const std::string& method,
	const std::vector<std::string>& headerStrs,
	const std::string& body,
	CUrlHeaderCallBack* headerCallback,
	CUrlContentCallBack* contentCallback,
	uint16_t expectedRespCode
)
{
	uint16_t respCode = CUrlRequest(
		url,
		method,
		headerStrs,
		body,
		headerCallback,
		contentCallback
	);

	if (respCode != expectedRespCode)
	{
		throw Common::Exception(
			"CURL request received unexpected response code (response code=" +
			std::to_string(respCode) + ")"
		);
	}
}


inline std::string& CUrlParseHeaderValue(std::string& s)
{
	s = s.substr(s.find_first_of(':') + 1);

	return Common::Strip(s);
}


inline std::string& CUrlUnescape(std::string& s)
{
	int outLen = 0;
	char* resStr = curl_easy_unescape(
		nullptr, // Since curl 7.82.0, this parameter is ignored
		s.c_str(),
		static_cast<int>(s.size()),
		&outLen
	);
	if (resStr == nullptr)
	{
		throw Common::Exception("Failed to do CURL unescape");
	}
	std::copy(resStr, resStr + outLen, s.begin());
	curl_free(resStr);
	s.resize(outLen);

	return s;
}

} // namespace Untrusted
} // namespace DecentEnclave
