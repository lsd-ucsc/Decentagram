// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <string>
#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
// no C++ headers needed for SGX
#else
#include <iostream>
#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED

#include <SimpleObjects/Codec/Hex.hpp>

#include "../Exceptions.hpp"
#include "../Internal/SimpleObj.hpp"
#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
#include "../../SgxEdgeSources/sys_io_t.h"
#include "../Sgx/Exceptions.hpp"
#else
// no DecentEncalve headers needed for Untrusted
#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED


namespace DecentEnclave
{
namespace Common
{
namespace Platform
{

struct Print
{

	static void Str(const std::string& str)
	{
#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
		DECENTENCLAVE_SGX_OCALL_CHECK_ERROR_E(
			ocall_decent_enclave_print_str,
			str.c_str()
		);
#else
		std::cout << str << std::flush;
#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
	}

	static void StrDebug(const std::string& str)
	{
		Str(AsmLineLeader(GetDebugLabel(), GetPlatformSymbol()) + str + "\n");
	}

	static void StrInfo(const std::string& str)
	{
		Str(AsmLineLeader(GetInfoLabel(), GetPlatformSymbol()) + str + "\n");
	}

	static void StrErr(const std::string& str)
	{
		Str(AsmLineLeader(GetErrLabel(), GetPlatformSymbol()) + str + "\n");
	}

	static void Hex(const void* data, const size_t size)
	{
		const uint8_t* byteDataPtr = static_cast<const uint8_t*>(data);
		Str(
			Common::Internal::Obj::Codec::HEX::template Encode<std::string>(
				byteDataPtr,
				byteDataPtr + size
			)
		);
	}

	static void HexDebug(const void* data, const size_t size)
	{
		const uint8_t* byteDataPtr = static_cast<const uint8_t*>(data);
		StrDebug(
			Common::Internal::Obj::Codec::HEX::template Encode<std::string>(
				byteDataPtr,
				byteDataPtr + size
			)
		);
	}

	static void Ptr(const void* ptr)
	{
		Str(Ptr2Str(ptr));
	}

	static void PtrDebug(const void* ptr)
	{
		StrDebug(Ptr2Str(ptr));
	}

	static void MemDebug(const void* data, const size_t size)
	{
		StrDebug(
			"Memory dump @ " + Ptr2Str(data) +
			", size: " + std::to_string(size) + ":"
		);
		HexDebug(data, size);
		StrDebug("\n");
	}


	// Helper functions:

	static std::string GetPlatformSymbol()
	{
#ifdef DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
		return "SGX-T";
#else
		return "APP-U";
#endif // DECENT_ENCLAVE_PLATFORM_SGX_TRUSTED
	}

	static std::string GetInfoLabel()
	{
		return "INFO";
	}

	static std::string GetDebugLabel()
	{
		return "DEBUG";
	}

	static std::string GetErrLabel()
	{
		return "ERROR";
	}

	static std::string AsmLineLeader(
		const std::string& label,
		const std::string& platSym
	)
	{
		return label + "(" + platSym + ")" + ": ";
	}

	static std::string Ptr2Str(
		const void* ptr
	)
	{
		auto val = reinterpret_cast<std::uintptr_t>(ptr);

		return Common::Internal::Obj::Codec::HEX::
			template Encode<std::string>(val);
	}

}; // struct Print


} // namespace Platform
} // namespace Common
} // namespace DecentEnclave
