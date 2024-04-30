// Copyright (c) 2022 mbedTLScpp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstddef>
#include <cstdint>
#include <cstring>

//#include <vector>

#include <mbedtls/asn1.h>
//#include <mbedtls/bignum.h>

#include "../Exceptions.hpp"


//==============================================================================
// ASN.1 Write size estimation functions
//==============================================================================

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
namespace Internal
{


/**
 * @brief
 *
 * @exception InvalidArgumentException
 *
 * @return size_t
 */
inline constexpr size_t asn1_write_len_est_size(size_t len)
{
	return
		 len <        0x80 ? 1 :
		(len <=       0xFF ? 2 :
		(len <=     0xFFFF ? 3 :
		(len <=   0xFFFFFF ? 4 :
#if SIZE_MAX > 0xFFFFFFFF
		(len <= 0xFFFFFFFF ? 5 :
			throw InvalidArgumentException(
				"mbedTLScpp::Internal::asn1_write_len_est_size"
				" - Invalid length is given."
			)
		)
#else
			5
#endif
		)
		)
		);
}


inline constexpr size_t asn1_write_tag_est_size(
	unsigned char /* tag */
) noexcept
{
	return 1;
}


/**
 * @brief
 *
 * @exception InvalidArgumentException
 *
 * @return size_t
 */
inline constexpr size_t asn1_write_null_est_size()
{
	return asn1_write_len_est_size(0) +
		asn1_write_tag_est_size(MBEDTLS_ASN1_NULL);
}

/**
 * @brief
 *
 * @exception InvalidArgumentException
 *
 * @return size_t
 */
inline constexpr size_t asn1_write_bool_est_size(int /* boolean */)
{
	return static_cast<size_t>(1) +
		asn1_write_len_est_size(static_cast<size_t>(1)) +
		asn1_write_tag_est_size(MBEDTLS_ASN1_BOOLEAN);
}

inline constexpr size_t asn1_write_int_est_size_est_val_len(int val) noexcept
{
	return sizeof(decltype(val)) + 1;
}

/**
 * @brief
 *
 * @exception InvalidArgumentException
 *
 * @return size_t
 */
inline constexpr size_t asn1_write_int_est_size(int val)
{
	return asn1_write_int_est_size_est_val_len(val) +
			+ asn1_write_len_est_size(asn1_write_int_est_size_est_val_len(val))
			+ asn1_write_tag_est_size(MBEDTLS_ASN1_INTEGER);
}

inline constexpr size_t asn1_write_raw_buffer_est_size(
	const void* /* buf */,
	size_t size
) noexcept
{
	return size;
}

inline constexpr size_t asn1_write_oid_est_size_est_oid_len(
	const void *oid,
	size_t oid_len
) noexcept
{
	return asn1_write_raw_buffer_est_size(oid, oid_len);
}

/**
 * @brief
 *
 * @exception InvalidArgumentException
 *
 * @return size_t
 */
inline constexpr size_t asn1_write_oid_est_size(
	const void *oid,
	size_t oid_len
)
{
	return asn1_write_oid_est_size_est_oid_len(oid, oid_len) +
		asn1_write_len_est_size(
			asn1_write_oid_est_size_est_oid_len(oid, oid_len)
		) +
		asn1_write_tag_est_size(MBEDTLS_ASN1_OID);
}


/**
 * @brief
 *
 * @exception InvalidArgumentException
 *
 * @return size_t
 */
inline constexpr size_t asn1_write_mpi_est_size_given_mpi_size(
	size_t mpiSize
) noexcept
{
	return (mpiSize + (mpiSize == 0 ? 1 : 0) + 1) +
		asn1_write_len_est_size(
			(mpiSize + (mpiSize == 0 ? 1 : 0) + 1)
		) +
		asn1_write_tag_est_size(MBEDTLS_ASN1_INTEGER);
}


/**
 * @brief
 *
 * @exception InvalidArgumentException
 *
 * @return size_t
 */
inline size_t asn1_write_mpi_est_size(const mbedtls_mpi & X)
{
	return asn1_write_mpi_est_size_given_mpi_size(mbedtls_mpi_size(&X));
}

inline constexpr size_t asn1_write_algorithm_identifier_param_est_size(
	size_t par_len
)
{
	return (
		(par_len == 0) ?
			asn1_write_null_est_size() :
			par_len
		);
}

/**
 * @brief
 *
 * @exception InvalidArgumentException
 *
 * @return size_t
 */
inline constexpr size_t asn1_write_algorithm_identifier_est_size_est_param_oid(
	const void *oid,
	size_t oid_len,
	size_t par_len
)
{
	return asn1_write_algorithm_identifier_param_est_size(par_len) +
		asn1_write_oid_est_size(oid, oid_len);
}

/**
 * @brief
 *
 * @exception InvalidArgumentException
 *
 * @return size_t
 */
inline constexpr size_t asn1_write_algorithm_identifier_est_size(
	const char *oid,
	size_t oid_len,
	size_t par_len
)
{
	return
		asn1_write_algorithm_identifier_est_size_est_param_oid(
			oid,
			oid_len,
			par_len
		) +
		asn1_write_len_est_size(
			asn1_write_algorithm_identifier_est_size_est_param_oid(
				oid,
				oid_len,
				par_len
			)
		) +
		asn1_write_tag_est_size(
			MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE
		);
}

/**
 * @brief
 *
 * @exception InvalidArgumentException
 *
 * @return size_t
 */
inline constexpr size_t asn1_write_tagged_string_est_size(
	unsigned char tag,
	const void *text,
	size_t text_len
)
{
	return asn1_write_raw_buffer_est_size(text, text_len) +
		asn1_write_len_est_size(
			asn1_write_raw_buffer_est_size(text, text_len)
		) +
		asn1_write_tag_est_size(tag);
}

} // namespace Internal
} // namespace mbedTLScpp


//==============================================================================
// ASN.1 DeepCopy functions
//==============================================================================

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
namespace Internal
{


inline mbedtls_asn1_named_data* Asn1NamedDataAlloc()
{
	mbedtls_asn1_named_data* res = static_cast<mbedtls_asn1_named_data*>(
		mbedtls_calloc(1, sizeof(mbedtls_asn1_named_data))
	);
	MBEDTLSCPP_MEMALLOC_NULLPTR_CHECK(res);

	return res;
}


/**
 * @brief
 *
 * @param dest
 * @param src
 * @exception std::bad_alloc
 */
inline void Asn1DeepCopy(mbedtls_asn1_buf& dest, const mbedtls_asn1_buf& src)
{
	if (dest.p != nullptr)
	{
		mbedtls_free(dest.p);
	}

	if (src.p == nullptr)
	{
		dest.p = nullptr;
	}
	else
	{
		dest.p = static_cast<unsigned char *>(mbedtls_calloc(1, src.len));
		if (dest.p == nullptr)
		{
			throw std::bad_alloc();
		}
	}

	dest.len = src.len;
	dest.tag = src.tag;
	std::memcpy(dest.p, src.p, src.len);
}


/**
 * @brief
 *
 * @param dest Dest is a pointer holding named data. If there is already
 *             something stored in dest (not-null), it will be freed.
 * @param src
 * @exception std::bad_alloc
 */
inline void Asn1DeepCopy(
	mbedtls_asn1_named_data*& dest,
	const mbedtls_asn1_named_data* src
)
{
	mbedtls_asn1_free_named_data_list(&dest);

	if (src == nullptr)
	{
		return;
	}

	dest = Asn1NamedDataAlloc();

	const mbedtls_asn1_named_data* curSrc = src;
	mbedtls_asn1_named_data* curDest = dest;

	while (curSrc != nullptr)
	{
		Asn1DeepCopy(curDest->oid, curSrc->oid);
		Asn1DeepCopy(curDest->val, curSrc->val);
		curDest->MBEDTLS_PRIVATE(next_merged) =
			curSrc->MBEDTLS_PRIVATE(next_merged);

		if (curSrc->next != nullptr)
		{
			curDest->next = Asn1NamedDataAlloc();
		}

		curSrc = curSrc->next;
		curDest = curDest->next;
	}
}


inline void Asn1ReverseNamedDataList(mbedtls_asn1_named_data*& dest)
{
	std::vector<mbedtls_asn1_named_data*> stack;

	mbedtls_asn1_named_data* cur = dest;
	while(cur != nullptr)
	{
		stack.push_back(cur);
		cur = cur->next;
	}

	if (stack.size() > 0)
	{
		dest = stack.back();
		stack.pop_back();
		dest->next = nullptr;
	}

	cur = dest;

	while(stack.size() > 0 && cur != nullptr)
	{
		cur->next = stack.back();
		stack.pop_back();
		cur = cur->next;
		cur->next = nullptr;
	}
}


template<typename _ByteType>
inline size_t CalcLeadingZeroBitsInByte(const _ByteType& b)
{
	size_t leadingZeroBits = 0;
	for (int i = 7; i > 0; --i)
	{
		if ((b >> i) & 0x01U)
		{
			return leadingZeroBits;
		}
		++leadingZeroBits;
	}
	return leadingZeroBits +
		((b & 0x01U) ? 0 : 1);
}


template<typename _ByteType, typename _InIt>
inline _ByteType FillWritingBitsImpl(
	uint8_t    fillLen,
	_ByteType& lastByte,
	size_t&    lastByteBitLen,
	_InIt&     bytesBegin,
	_InIt      bytesEnd
)
{
	_ByteType writingByte = 0;
	size_t    writingByteBitLen = 0;
	size_t    bitLenToBeWrite = fillLen;

	while(writingByteBitLen < fillLen)
	{
		if (lastByteBitLen == 0)
		{
			// last byte is empty, fill it with new byte

			if (bytesBegin == bytesEnd)
			{
				// We need more bytes, but there is no more input bytes
				throw InvalidArgumentException(
					"mbedTLScpp::Internal::FillWritingBitsImpl"
					" - unexpected ending of input bytes"
				);
			}
			lastByte = *bytesBegin++;
			lastByteBitLen = 8;
		}
		else if (lastByteBitLen > bitLenToBeWrite)
		{
			// we only need to use part of lastByte
			// so there will be leftovers in lastByte
			size_t leftOverBitLen = (lastByteBitLen - bitLenToBeWrite);

			_ByteType usedBits = lastByte >> leftOverBitLen;
			writingByte |= usedBits;
			writingByteBitLen += bitLenToBeWrite;

			// make used bits to 0
			_ByteType usedBitsMask = usedBits << leftOverBitLen;
			lastByte ^= usedBitsMask;
			lastByteBitLen = leftOverBitLen;
		}
		else if (lastByteBitLen <= bitLenToBeWrite)
		{
			// The bits in lastByte are less than the length of bits we need, OR
			// The bits in lastByte are exactly the length of bits we need

			// bits to be filled in next round
			size_t nextRoundBitLen = (bitLenToBeWrite - lastByteBitLen);

			writingByte |= lastByte << nextRoundBitLen;
			writingByteBitLen += lastByteBitLen;

			lastByte = 0;
			lastByteBitLen = 0;
		}
		bitLenToBeWrite = (fillLen - writingByteBitLen);
	}

	return writingByte;
}


template<typename _ByteType, typename _InIt>
inline _ByteType FillWritingBits(
	uint8_t    fillLen,
	_ByteType& lastByte,
	size_t&    lastByteBitLen,
	_InIt&     bytesBegin,
	_InIt      bytesEnd
)
{
	if (fillLen > 8)
	{
		throw InvalidArgumentException(
			"mbedTLScpp::Internal::FillWritingBits"
			" - fillLen must be within the size of a byte"
		);
	}
	return FillWritingBitsImpl(
		fillLen,
		lastByte,
		lastByteBitLen,
		bytesBegin,
		bytesEnd
	);
}


template<uint8_t _FillLen, typename _ByteType, typename _InIt>
inline _ByteType FillWritingBits(
	_ByteType& lastByte,
	size_t&    lastByteBitLen,
	_InIt&     bytesBegin,
	_InIt      bytesEnd
)
{
	static_assert(_FillLen <= 8, "_FillLen must be within the size of a byte");

	return FillWritingBitsImpl(
		_FillLen,
		lastByte,
		lastByteBitLen,
		bytesBegin,
		bytesEnd
	);
}


template<typename _OutValType, typename _OutIt, typename _InIt>
inline void Asn1MultiBytesOidEncode(
	_OutIt out,
	_InIt begin,
	_InIt end,
	size_t totalBytes
)
{
	static constexpr size_t sk_validBitsPerByte = 7;
	static constexpr _OutValType sk_leadingBitOne = ~(_OutValType(0x7F));

	if (begin == end)
	{
		throw InvalidArgumentException(
			"mbedTLScpp::Internal::Asn1MultiBytesOidEncode"
			" - At least one byte should be given"
		);
	}

	auto firstByte = *begin++;

	if (firstByte == 0U)
	{
		throw InvalidArgumentException(
			"mbedTLScpp::Internal::Asn1MultiBytesOidEncode"
			" - There are too many leading zeros"
		);
	}
	if (firstByte < 128U && totalBytes <= 1)
	{
		throw InvalidArgumentException(
			"mbedTLScpp::Internal::Asn1MultiBytesOidEncode"
			" - This OID is not multi-bytes"
		);
	}

	size_t leadingZeroBits = CalcLeadingZeroBitsInByte(firstByte);
	size_t totalBits = (totalBytes * 8) - leadingZeroBits;
	size_t validBitsInFirstByte = totalBits % sk_validBitsPerByte;
	validBitsInFirstByte =
		(validBitsInFirstByte == 0) ?
			sk_validBitsPerByte : // there is no leftover, use all 7 bits
			validBitsInFirstByte;
	size_t totalBytesNeeded =
		(totalBits + (sk_validBitsPerByte - 1)) / sk_validBitsPerByte;

	auto   lastByte = firstByte;
	size_t lastByteBitLen = 8 - leadingZeroBits;

	*out++ =
		sk_leadingBitOne |
		FillWritingBits(
			static_cast<uint8_t>(validBitsInFirstByte),
			lastByte,
			lastByteBitLen,
			begin,
			end
		);

	for(size_t i = 1; i < totalBytesNeeded; ++i)
	{
		bool isEnding = (i == (totalBytesNeeded - 1));
		_OutValType leadingBit = isEnding ?
			0 : // The leftmost bit of ending byte is 0
			sk_leadingBitOne; // The leftmost bit of non-ending byte is 1

		*out++ =
			leadingBit |
			FillWritingBits<7>(
				lastByte,
				lastByteBitLen,
				begin,
				end
			);
	}
}


template<typename _OutValType, typename _OutIt, typename _InIt>
inline void Asn1MultiBytesOidEncode(
	_OutIt out,
	_InIt begin,
	_InIt end
)
{
	return Asn1MultiBytesOidEncode<_OutValType, _OutIt, _InIt>(
		out,
		begin,
		end,
		std::distance(begin, end)
	);
}


} // namespace Internal
} // namespace mbedTLScpp
