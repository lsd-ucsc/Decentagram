// Copyright (c) 2022 DecentEnclave
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstddef>
#include <cstdint>

#include <tuple>
#include <vector>

#include <mbedTLScpp/Container.hpp>
#include <mbedTLScpp/RandInterfaces.hpp>
#include <mbedTLScpp/SecretVector.hpp>

#include "Platform/AesGcm.hpp"
#include "Exceptions.hpp"


namespace DecentEnclave
{
namespace Common
{


////////////////////////////
//Data Sealing:
////////////////////////////

//Structure:
// MAC                      (PlainText)         - 16  Bytes      -> 16   Bytes
// IV                       (PlainText) (MACed) - 12  Bytes      -> 28   Bytes
// Payload size             (PlainText) (MACed) - 8   Bytes      -> 36   Bytes
// Key Metadata Size        (PlainText) (MACed) - 8   Bytes      -> 44   Bytes
// Key Metadata             (PlainText) (MACed) - variable Size
// Additional Metadata size (Encrypted)         - 8   Bytes
// Data size                (Encrypted)         - 8   Bytes
// Additional Metadata      (Encrypted)         - variable Size
// Data                     (Encrypted)         - variable Size
// ----- Padding part
// Padding bytes            (Encrypted)         - variable Size

template<typename _AesGcmOneGoType>
class AesGcmPackager
{
public: // static members:

	using CryptorType = _AesGcmOneGoType;
	static constexpr size_t sk_keyBitSize = CryptorType::sk_keyBitSize;
	static constexpr size_t sk_keyByteSize = CryptorType::sk_keyByteSize;
	static constexpr size_t sk_ivSize = 12;
	static constexpr size_t sk_tagSize = 16;

	using KeyType = typename CryptorType::KeyType;
	using IVType =  uint8_t[sk_ivSize];
	using TagType = uint8_t[sk_tagSize];

	static constexpr size_t sk_knownAddSize =
		sizeof(IVType) +    // IV           - 12 Bytes
		sizeof(uint64_t) +  // Payload Size -  8 Bytes
		sizeof(uint64_t);   // Keymeta Size -  8 Bytes

	static constexpr size_t sk_sealMetaSize =
		sizeof(TagType) +  // Tag                 - 16 Bytes
		sk_knownAddSize;   // IV + pSize + KmSize - 28 Bytes

	static constexpr size_t sk_sealPkgAllKnownSize =
		sk_sealMetaSize +  // Tag + IV + pSize + KmSize - 44 Bytes
		sizeof(uint64_t) + // Metadata Size             -  8 Bytes
		sizeof(uint64_t);  // Data Size                 -  8 Bytes

	static
	std::tuple<
		size_t /* Total Size*/,
		size_t /* Add Size*/,
		size_t /* Encrypted Size */
	>
	GetTotalSealedBlockSize(
		size_t inSealedBlockSize,
		size_t inKeyMetaSize,
		size_t inMetaSize,
		size_t inDataSize
	)
	{
		const size_t totalDataSize =
			sk_sealPkgAllKnownSize +
			inKeyMetaSize +
			inMetaSize +
			inDataSize;

		const size_t totalBlockNum =
			(totalDataSize / inSealedBlockSize) +
			((totalDataSize % inSealedBlockSize) == 0 ? 0 : 1);

		const size_t totalBlockSize = totalBlockNum * inSealedBlockSize;

		const size_t padSize = totalBlockSize - totalDataSize;

		size_t encryptedSize =
			sizeof(uint64_t) + // Metadata Size - 8 Bytes
			sizeof(uint64_t) + // Data Size     - 8 Bytes
			inMetaSize + inDataSize + padSize;

		size_t addSize       = sk_knownAddSize + inKeyMetaSize;

		//encOutSize = sgx_calc_sealed_data_size(0, encInSize);
		//if (encOutSize == UINT32_MAX)
		//{
		//	throw RuntimeException("Failed to calculate seal data size.");
		//}
		//metaSize = encOutSize - encInSize;

		//const size_t res = sizeof(gsk_sgxSealedDataLabel) + encOutSize;

		// EXCEPTION_ASSERT(
		// 	totalBlockSize == sizeof(TagType) + addSize + encryptedSize,
		// 	"the calculation result is wrong."
		// );

		return std::make_tuple(totalBlockSize, addSize, encryptedSize);
	}

	static std::vector<uint8_t> GetKeyMeta(const std::vector<uint8_t>& pack)
	{
		if (pack.size() < sk_sealMetaSize)
		{
			throw Exception(
				"AesGcmPackager::GetKeyMeta - "
				"The given package's size is smaller than expected.");
		}

		constexpr size_t keyMetaSizePos =
			sizeof(TagType) +   // Tag          - 16 Bytes
			sizeof(IVType) +    // IV           - 12 Bytes
			sizeof(uint64_t);  // Payload Size -  8 Bytes

		constexpr size_t keyMetaBeginPos =
			keyMetaSizePos +   // Tag + IV + pSize  - 16 Bytes
			sizeof(uint64_t);  // Key Meta Size     -  8 Bytes
		static_assert(keyMetaBeginPos == sk_sealMetaSize, "Programming Error.");

		//uint64_t keyMetaSize = 0;
		//std::memcpy(&keyMetaSize, pack.data() + keyMetaSizePos, sizeof(keyMetaSize));
		const uint64_t keyMetaSize = *reinterpret_cast<const uint64_t*>(pack.data() + keyMetaSizePos);

		const size_t keyMetaEndPos = keyMetaBeginPos + keyMetaSize;
		if (pack.size() < keyMetaEndPos)
		{
			throw Exception(
				"AesGcmPackager::GetKeyMeta - "
				"The given package's size is smaller than expected.");
		}

		return std::vector<uint8_t>(pack.begin() + keyMetaBeginPos, pack.begin() + keyMetaEndPos);
	}

public:
	AesGcmPackager(
		KeyType key,
		size_t sealedBlockSize
	) :
		m_aesGcm(std::move(key)),
		m_sealedBlockSize(sealedBlockSize)
	{}

	~AesGcmPackager()
	{}

	AesGcmPackager(AesGcmPackager&& rhs) noexcept :
		m_aesGcm(std::move(rhs.m_aesGcm)), //noexcept
		m_sealedBlockSize(rhs.m_sealedBlockSize)
	{}

	AesGcmPackager(const AesGcmPackager& rhs) = delete;

	template<
		typename _KeyMetaCtnType, bool _KeyMetaCtnSecrecy,
		typename _MetaCtnType,    bool _MetaCtnSecrecy,
		typename _DataCtnType,    bool _DataCtnSecrecy,
		typename _AddCtnType,     bool _AddCtnSecrecy
	>
	std::pair<
		std::vector<uint8_t>,
		std::array<uint8_t, 16>
	>
	Pack(
		const mbedTLScpp::ContCtnReadOnlyRef<_KeyMetaCtnType, _KeyMetaCtnSecrecy>& keyMeta,
		const mbedTLScpp::ContCtnReadOnlyRef<_MetaCtnType, _MetaCtnSecrecy>& meta,
		const mbedTLScpp::ContCtnReadOnlyRef<_DataCtnType, _DataCtnSecrecy>& data,
		const mbedTLScpp::ContCtnReadOnlyRef<_AddCtnType, _AddCtnSecrecy>& addData,
		mbedTLScpp::RbgInterface& rand
	)
	{
		using namespace mbedTLScpp;

		size_t totalPackSize = 0;
		size_t packAddSize = 0;
		size_t encryptSize = 0;

		std::tie(totalPackSize, packAddSize, encryptSize) =
			GetTotalSealedBlockSize(
				m_sealedBlockSize,
				keyMeta.GetRegionSize(),
				meta.GetRegionSize(),
				data.GetRegionSize()
			);

		// ============ Build Input Package
		SecretVector<uint8_t> inputPkg(encryptSize, 0);

		{
			// Positions in the input packages:
			constexpr size_t ipMetaSizePos = 0;
			constexpr size_t ipDataSizePos = ipMetaSizePos + sizeof(uint64_t);
			constexpr size_t ipMetaPos     = ipDataSizePos + sizeof(uint64_t);
			//const     size_t ipDataPos     = ipMetaPos + meta.GetRegionSize();
			// Ensure we don't leak info when using local vars.

			// Meta Size
			uint64_t& metaSize =
				*reinterpret_cast<uint64_t*>(inputPkg.data() + ipMetaSizePos);
			metaSize = meta.GetRegionSize();
			// Data Size
			uint64_t& dataSize =
				*reinterpret_cast<uint64_t*>(inputPkg.data() + ipDataSizePos);
			dataSize = data.GetRegionSize();
			// Meta
			//std::copy(
			// 	meta.BeginBytePtr(),
			// 	meta.EndBytePtr(),
			// 	inputPkg.data() + ipMetaPos
			// );
			std::memcpy(
				inputPkg.data() + ipMetaPos,
				meta.BeginBytePtr(),
				meta.GetRegionSize()
			);
			// Data
			//std::copy(
			// 	data.BeginBytePtr(),
			// 	data.EndBytePtr(),
			// 	inputPkg.data() + ipMetaPos + metaSize
			// );
			std::memcpy(
				inputPkg.data() + ipMetaPos + metaSize,
				data.BeginBytePtr(),
				data.GetRegionSize()
			);
		}

		// ============ Build Final Package
		std::vector<uint8_t> finPackage(totalPackSize);  // <<= The final package to be returned.
		std::array<uint8_t, 16> tag;                     // <<= The tag/MAC to be returned.

		{
			// Positions in the final packages:
			constexpr size_t fpTagPos       = 0;
			constexpr size_t fpIvPos        = fpTagPos + sizeof(TagType);
			constexpr size_t fpPaySizePos   = fpIvPos + sizeof(IVType);
			constexpr size_t fpKMetaSizePos = fpPaySizePos + sizeof(uint64_t);
			constexpr size_t fpKMetaPos     = fpKMetaSizePos + sizeof(uint64_t);
			const     size_t fpEncDataPos   = fpKMetaPos + keyMeta.GetRegionSize();

			// Generate IV
			rand.Rand(finPackage.data() + fpIvPos, sizeof(IVType));
			// Payload Size
			uint64_t& payloadSize =
				*reinterpret_cast<uint64_t*>(finPackage.data() + fpPaySizePos);
			payloadSize = encryptSize;
			// Key Meta Size
			uint64_t& keyMetaSize =
				*reinterpret_cast<uint64_t*>(finPackage.data() + fpKMetaSizePos);
			keyMetaSize = keyMeta.GetRegionSize();
			// Key Meta
			std::memcpy(
				finPackage.data() + fpKMetaPos,
				keyMeta.BeginBytePtr(),
				keyMeta.GetRegionSize()
			);

			std::vector<uint8_t> encrypted;

			if (addData.GetRegionSize() > 0)
			{
				// Build Full Add Data
				std::vector<uint8_t> pkgAddData;
				pkgAddData.reserve(packAddSize + addData.GetRegionSize());
				pkgAddData.insert(
					pkgAddData.end(),
					finPackage.begin() + fpIvPos,
					finPackage.begin() + fpEncDataPos
				);
				pkgAddData.insert(
					pkgAddData.end(),
					addData.BeginBytePtr(),
					addData.EndBytePtr()
				);

				// Encrypt
				std::tie(encrypted, tag) = m_aesGcm.Encrypt(
					CtnByteRgR<fpIvPos, fpIvPos + sizeof(IVType)>(finPackage),
					CtnFullR(pkgAddData),
					CtnFullR(inputPkg)
				);
			}
			else
			{
				// Encrypt
				std::tie(encrypted, tag) = m_aesGcm.Encrypt(
					CtnByteRgR<fpIvPos, fpIvPos + sizeof(IVType)>(finPackage),
					CtnByteRgR(finPackage, fpIvPos, fpEncDataPos),
					CtnFullR(inputPkg)
				);
			}

			// EXCEPTION_ASSERT(
			// 	finPackage.size() == (fpEncDataPos + encrypted.size()),
			// 	"The package size calculation result is wrong."
			// );

			// Copy Tag
			std::memcpy(finPackage.data() + fpTagPos, tag.data(), tag.size());
			// Copy Encrypted Data
			std::memcpy(
				finPackage.data() + fpEncDataPos,
				encrypted.data(),
				encrypted.size()
			);
		}

		return std::make_pair(finPackage, tag);
	}


	template<
		typename _DataCtnType, bool _DataCtnSecrecy,
		typename _AddCtnType,  bool _AddCtnSecrecy
	>
	std::pair<
		mbedTLScpp::SecretVector<uint8_t> /* Data */,
		mbedTLScpp::SecretVector<uint8_t> /* Meta */
	>
	Unpack(
		const mbedTLScpp::ContCtnReadOnlyRef<_DataCtnType, _DataCtnSecrecy>& package,
		const mbedTLScpp::ContCtnReadOnlyRef<_AddCtnType, _AddCtnSecrecy>& addData,
		const std::array<uint8_t, 16>* inTag
	)
	{
		using namespace mbedTLScpp;

		// Decrypt and get output package.
		SecretVector<uint8_t> outputPkg;
		{
			// Positions in the packages:
			constexpr size_t fpTagPos       = 0;
			constexpr size_t fpIvPos        = fpTagPos + sizeof(TagType);
			constexpr size_t fpPaySizePos   = fpIvPos + sizeof(IVType);
			constexpr size_t fpKMetaSizePos = fpPaySizePos + sizeof(uint64_t);
			constexpr size_t fpKMetaPos     = fpKMetaSizePos + sizeof(uint64_t);

			const uint64_t payloadSize =
				*reinterpret_cast<const uint64_t*>(
					package.BeginBytePtr() + fpPaySizePos
				);
			const uint64_t keyMetaSize =
				*reinterpret_cast<const uint64_t*>(
					package.BeginBytePtr() + fpKMetaSizePos
				);

			const size_t fpEncDataPos = fpKMetaPos + keyMetaSize;

			if (package.GetRegionSize() != fpEncDataPos + payloadSize)
			{
				throw Exception(
					"AesGcmPackager::Unpack - "
					"The package size doesn't match the expected size."
				);
			}

			if (addData.GetRegionSize() > 0)
			{
				// Build Full Add Data
				std::vector<uint8_t> pkgAddData;
				pkgAddData.reserve(
					sk_knownAddSize + keyMetaSize + addData.GetRegionSize()
				);
				pkgAddData.insert(
					pkgAddData.end(),
					package.BeginBytePtr() + fpIvPos,
					package.BeginBytePtr() + fpEncDataPos
				);
				pkgAddData.insert(
					pkgAddData.end(),
					addData.BeginBytePtr(),
					addData.EndBytePtr()
				);

				outputPkg = m_aesGcm.Decrypt(
					CtnByteRgR<fpIvPos, fpIvPos + sizeof(IVType)>(package),
					CtnFullR(pkgAddData),
					CtnByteRgR(package, fpEncDataPos),
					CtnByteRgR<fpTagPos, fpTagPos + sizeof(TagType)>(package)
				);
			}
			else
			{
				outputPkg = m_aesGcm.Decrypt(
					CtnByteRgR<fpIvPos, fpIvPos + sizeof(IVType)>(package),
					CtnByteRgR(package, fpIvPos, fpEncDataPos),
					CtnByteRgR(package, fpEncDataPos),
					CtnByteRgR<fpTagPos, fpTagPos + sizeof(TagType)>(package)
				);
			}

			// Check input tag
			if (inTag != nullptr)
			{
				static_assert(sizeof(TagType) == 16, "Programming Error");
				if (
					std::memcmp(
						package.BeginBytePtr() + fpTagPos,
						inTag->data(),
						sizeof(TagType)
					) != 0
				)
				{
					throw Exception(
						"AesGcmPackager::Unpack - "
						"The tag/MAC contained in the message package "
						"doesn't match the given one."
					);
				}
			}
		}

		// Separating Metadata and data.
		SecretVector<uint8_t> outMeta;
		SecretVector<uint8_t> outData;
		{
			constexpr size_t opMetaSizePos = 0;
			constexpr size_t opDataSizePos = opMetaSizePos + sizeof(uint64_t);
			constexpr size_t opMetaPos     = opDataSizePos + sizeof(uint64_t);

			const uint64_t& metaSize =
				*reinterpret_cast<const uint64_t*>(
					outputPkg.data() + opMetaSizePos
				);
			const uint64_t& dataSize =
				*reinterpret_cast<const uint64_t*>(
					outputPkg.data() + opDataSizePos
				);

			if (outputPkg.size() < opMetaPos + metaSize + dataSize)
			{
				throw Exception(
					"AesGcmPackager::Unpack - "
					"The encrypted payload package size "
					"is smaller than the expected size."
				);
			}

			outMeta.insert(outMeta.end(),
				outputPkg.data() + opMetaPos,
				outputPkg.data() + opMetaPos + metaSize
			);
			outData.insert(outData.end(),
				outputPkg.data() + opMetaPos + metaSize,
				outputPkg.data() + opMetaPos + metaSize + dataSize
			);
		}

		return std::make_pair(outData, outMeta);
	}

private:

	_AesGcmOneGoType m_aesGcm;
	size_t m_sealedBlockSize;

}; // class AesGcmPackager


} // namespace Common
} // namespace DecentEnclave
