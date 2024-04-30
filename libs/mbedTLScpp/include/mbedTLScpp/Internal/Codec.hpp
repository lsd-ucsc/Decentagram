#pragma once

#include <string>

#include "../Container.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	namespace Internal
	{
		constexpr char const gsk_hEXLUT[] = "0123456789ABCDEF";

		constexpr char const gsk_hexLUT[] = "0123456789abcdef";

		/**
		 * @brief Convert the higher bit of a single byte into a single hex
		 *        character in upper case.
		 *
		 * @exception None No exception thrown
		 * @param byte The byte to convert.
		 * @return constexpr char The hex character in upper case.
		 */
		inline constexpr char HiBit2HEX(uint8_t byte) noexcept
		{
			return gsk_hEXLUT[(byte >> 4) & 0x0F];
		}

		/**
		 * @brief Convert the lower bit of a single byte into a single hex
		 *        character in upper case.
		 *
		 * @exception None No exception thrown
		 * @param byte The byte to convert.
		 * @return constexpr char The hex character in upper case.
		 */
		inline constexpr char LoBit2HEX(uint8_t byte) noexcept
		{
			return gsk_hEXLUT[byte &        0x0F];
		}

		/**
		 * @brief Convert the higher bit of a single byte into a single hex
		 *        character in lower case.
		 *
		 * @exception None No exception thrown
		 * @param byte The byte to convert.
		 * @return constexpr char The hex character in lower case.
		 */
		inline constexpr char HiBit2Hex(uint8_t byte) noexcept
		{
			return gsk_hexLUT[(byte >> 4) & 0x0F];
		}

		/**
		 * @brief Convert the lower bit of a single byte into a single hex
		 *        character in lower case.
		 *
		 * @exception None No exception thrown
		 * @param byte The byte to convert.
		 * @return constexpr char The hex character in lower case.
		 */
		inline constexpr char LoBit2Hex(uint8_t byte) noexcept
		{
			return gsk_hexLUT[byte &        0x0F];
		}

		/**
		 * @brief Convert an array of bytes into a hex string in upper case and
		 *        big-endian.
		 *
		 * @exception std::bad_alloc Thrown when memory allocation failed.
		 * @tparam _MinWidth     The minimum width in bytes. If the actual length
		 *                       longer than it, this function will output to the
		 *                       actual length. Default to 0, so it always output
		 *                       to actual length.
		 * @tparam _PaddingVal   The value used to pad to right hand side. Default to 0.
		 * @tparam ContainerType The type of container storing the bytes.
		 * @param cnt The data container.
		 * @return std::string The hex string.
		 */
		template<size_t _MinWidth = 0, uint8_t _PaddingVal = 0, typename ContainerType, bool ContainerSecrecy>
		inline std::string Bytes2HEXBigEnd(const ContCtnReadOnlyRef<ContainerType, ContainerSecrecy>& cnt)
		{
			std::string res;
			const size_t actualWidth = cnt.GetRegionSize() * 2;
			res.reserve(actualWidth > _MinWidth ? actualWidth : _MinWidth);

			// Big Endian, padding @ left hand side
			for(size_t i = cnt.GetRegionSize(); i < _MinWidth; ++i)
			{
				res.push_back(HiBit2Hex(_PaddingVal));
				res.push_back(HiBit2Hex(_PaddingVal));
			}

			for(const uint8_t* it = cnt.EndBytePtr(); it > cnt.BeginBytePtr(); --it)
			{
				res.push_back(HiBit2HEX(*(it - 1)));
				res.push_back(LoBit2HEX(*(it - 1)));
			}

			return res;
		}

		/**
		 * @brief Convert an array of bytes into a hex string in upper case and
		 *        little-endian.
		 *
		 * @exception std::bad_alloc Thrown when memory allocation failed.
		 * @tparam _MinWidth     The minimum width in bytes. If the actual length
		 *                       longer than it, this function will output to the
		 *                       actual length. Default to 0, so it always output
		 *                       to actual length.
		 * @tparam _PaddingVal   The value used to pad to right hand side. Default to 0.
		 * @tparam ContainerType The type of container storing the bytes.
		 * @param cnt The data container.
		 * @return std::string The hex string.
		 */
		template<size_t _MinWidth = 0, uint8_t _PaddingVal = 0, typename ContainerType, bool ContainerSecrecy>
		inline std::string Bytes2HEXLitEnd(const ContCtnReadOnlyRef<ContainerType, ContainerSecrecy>& cnt)
		{
			std::string res;
			const size_t actualWidth = cnt.GetRegionSize() * 2;
			res.reserve(actualWidth > _MinWidth ? actualWidth : _MinWidth);

			for(const uint8_t* it = cnt.BeginBytePtr(); it < cnt.EndBytePtr(); ++it)
			{
				res.push_back(HiBit2HEX(*it));
				res.push_back(LoBit2HEX(*it));
			}

			// Little Endian, padding @ right hand side
			for(size_t i = cnt.GetRegionSize(); i < _MinWidth; ++i)
			{
				res.push_back(HiBit2Hex(_PaddingVal));
				res.push_back(HiBit2Hex(_PaddingVal));
			}

			return res;
		}

		/**
		 * @brief Convert an array of bytes into a hex string in lower case and
		 *        big endian.
		 *
		 * @exception std::bad_alloc Thrown when memory allocation failed.
		 * @tparam _MinWidth     The minimum width in bytes. If the actual length
		 *                       longer than it, this function will output to the
		 *                       actual length. Default to 0, so it always output
		 *                       to actual length.
		 * @tparam _PaddingVal   The value used to pad to right hand side. Default to 0.
		 * @tparam ContainerType The type of container storing the bytes.
		 * @param cnt The data container.
		 * @return std::string The hex string.
		 */
		template<size_t _MinWidth = 0, uint8_t _PaddingVal = 0, typename ContainerType, bool ContainerSecrecy>
		inline std::string Bytes2HexBigEnd(const ContCtnReadOnlyRef<ContainerType, ContainerSecrecy>& cnt)
		{
			std::string res;
			const size_t actualWidth = cnt.GetRegionSize() * 2;
			res.reserve(actualWidth > _MinWidth ? actualWidth : _MinWidth);

			// Big Endian, padding @ left hand side
			for(size_t i = cnt.GetRegionSize(); i < _MinWidth; ++i)
			{
				res.push_back(HiBit2Hex(_PaddingVal));
				res.push_back(HiBit2Hex(_PaddingVal));
			}

			for(const uint8_t* it = cnt.EndBytePtr(); it > cnt.BeginBytePtr(); --it)
			{
				res.push_back(HiBit2Hex(*(it - 1)));
				res.push_back(LoBit2Hex(*(it - 1)));
			}

			return res;
		}

		/**
		 * @brief Convert an array of bytes into a hex string in lower case and
		 *        little-endian.
		 *
		 * @exception std::bad_alloc Thrown when memory allocation failed.
		 * @tparam _MinWidth     The minimum width in bytes. If the actual length
		 *                       longer than it, this function will output to the
		 *                       actual length. Default to 0, so it output actual
		 *                       length by default.
		 * @tparam _PaddingVal   The value used to pad to right hand side. Default to 0.
		 * @tparam ContainerType The type of container storing the bytes.
		 * @param cnt The data container.
		 * @return std::string The hex string.
		 */
		template<size_t _MinWidth = 0, uint8_t _PaddingVal = 0, typename ContainerType, bool ContainerSecrecy>
		inline std::string Bytes2HexLitEnd(const ContCtnReadOnlyRef<ContainerType, ContainerSecrecy>& cnt)
		{
			std::string res;
			const size_t actualWidth = cnt.GetRegionSize() * 2;
			res.reserve(actualWidth > _MinWidth ? actualWidth : _MinWidth);

			for(const uint8_t* it = cnt.BeginBytePtr(); it < cnt.EndBytePtr(); ++it)
			{
				res.push_back(HiBit2Hex(*it));
				res.push_back(LoBit2Hex(*it));
			}

			// Little Endian, padding @ right hand side
			for(size_t i = cnt.GetRegionSize(); i < _MinWidth; ++i)
			{
				res.push_back(HiBit2Hex(_PaddingVal));
				res.push_back(HiBit2Hex(_PaddingVal));
			}

			return res;
		}

		/**
		 * @brief Convert an array of bytes into a little-endian binary string
		 *        (with \c 0 's and \c 1 's)
		 *
		 * @exception std::bad_alloc Thrown when memory allocation failed.
		 * @tparam _MinWidth     The minimum width in bytes. If the actual length
		 *                       longer than it, this function will output to the
		 *                       actual length. Default to 0, so it output actual
		 *                       length by default.
		 * @tparam _PaddingVal   The value used to pad to right hand side. Default to 0.
		 * @tparam ContainerType The type of container storing the bytes.
		 * @param cnt The data container.
		 * @return std::string The binary string.
		 */
		template<size_t _MinWidth = 0, uint8_t _PaddingVal = 0, typename ContainerType, bool ContainerSecrecy>
		inline std::string Bytes2BinLitEnd(const ContCtnReadOnlyRef<ContainerType, ContainerSecrecy>& cnt)
		{
			std::string res;
			const size_t actualWidth = cnt.GetRegionSize() * 8;
			res.reserve(actualWidth > _MinWidth ? actualWidth : _MinWidth);

			for(const uint8_t* it = cnt.BeginBytePtr(); it < cnt.EndBytePtr(); ++it)
			{
				// High bits
				res.push_back((*it & 0x80) ? '1' : '0');
				res.push_back((*it & 0x40) ? '1' : '0');
				res.push_back((*it & 0x20) ? '1' : '0');
				res.push_back((*it & 0x10) ? '1' : '0');

				// Low bits
				res.push_back((*it & 0x08) ? '1' : '0');
				res.push_back((*it & 0x04) ? '1' : '0');
				res.push_back((*it & 0x02) ? '1' : '0');
				res.push_back((*it & 0x01) ? '1' : '0');
			}

			// Little Endian, padding @ right hand side
			for(size_t i = cnt.GetRegionSize(); i < _MinWidth; ++i)
			{
				// High bits
				res.push_back((_PaddingVal & 0x80) ? '1' : '0');
				res.push_back((_PaddingVal & 0x40) ? '1' : '0');
				res.push_back((_PaddingVal & 0x20) ? '1' : '0');
				res.push_back((_PaddingVal & 0x10) ? '1' : '0');

				// Low bits
				res.push_back((_PaddingVal & 0x08) ? '1' : '0');
				res.push_back((_PaddingVal & 0x04) ? '1' : '0');
				res.push_back((_PaddingVal & 0x02) ? '1' : '0');
				res.push_back((_PaddingVal & 0x01) ? '1' : '0');
			}

			return res;
		}

		/**
		 * @brief Convert an array of bytes into a big-endian binary string
		 *        (with \c 0 's and \c 1 's)
		 *
		 * @exception std::bad_alloc Thrown when memory allocation failed.
		 * @tparam _MinWidth     The minimum width in bytes. If the actual length
		 *                       longer than it, this function will output to the
		 *                       actual length. Default to 0, so it output actual
		 *                       length by default.
		 * @tparam _PaddingVal   The value used to pad to left hand side. Default to 0.
		 * @tparam ContainerType The type of container storing the bytes.
		 * @param cnt The data container.
		 * @return std::string The binary string.
		 */
		template<size_t _MinWidth = 0, uint8_t _PaddingVal = 0, typename ContainerType, bool ContainerSecrecy>
		inline std::string Bytes2BinBigEnd(const ContCtnReadOnlyRef<ContainerType, ContainerSecrecy>& cnt)
		{
			std::string res;
			const size_t actualWidth = cnt.GetRegionSize() * 8;
			res.reserve(actualWidth > _MinWidth ? actualWidth : _MinWidth);

			// Big Endian, padding @ left hand side
			for(size_t i = cnt.GetRegionSize(); i < _MinWidth; ++i)
			{
				// High bits
				res.push_back((_PaddingVal & 0x80) ? '1' : '0');
				res.push_back((_PaddingVal & 0x40) ? '1' : '0');
				res.push_back((_PaddingVal & 0x20) ? '1' : '0');
				res.push_back((_PaddingVal & 0x10) ? '1' : '0');

				// Low bits
				res.push_back((_PaddingVal & 0x08) ? '1' : '0');
				res.push_back((_PaddingVal & 0x04) ? '1' : '0');
				res.push_back((_PaddingVal & 0x02) ? '1' : '0');
				res.push_back((_PaddingVal & 0x01) ? '1' : '0');
			}

			for(const uint8_t* it = cnt.EndBytePtr(); it > cnt.BeginBytePtr(); --it)
			{
				// High bits
				res.push_back((*(it - 1) & 0x80) ? '1' : '0');
				res.push_back((*(it - 1) & 0x40) ? '1' : '0');
				res.push_back((*(it - 1) & 0x20) ? '1' : '0');
				res.push_back((*(it - 1) & 0x10) ? '1' : '0');

				// Low bits
				res.push_back((*(it - 1) & 0x08) ? '1' : '0');
				res.push_back((*(it - 1) & 0x04) ? '1' : '0');
				res.push_back((*(it - 1) & 0x02) ? '1' : '0');
				res.push_back((*(it - 1) & 0x01) ? '1' : '0');
			}

			return res;
		}
	}
}