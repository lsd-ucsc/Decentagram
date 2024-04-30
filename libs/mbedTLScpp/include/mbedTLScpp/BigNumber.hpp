// Copyright (c) 2022 mbedTLScpp
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include "ObjectBase.hpp"

#include <mbedtls/bignum.h>

#include <cstring>

#include "Common.hpp"
#include "Container.hpp"
#include "Exceptions.hpp"
#include "RandInterfaces.hpp"

#include "Internal/Codec.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{

	/**
	 * @brief Normal Big Number allocator.
	 *
	 */
	struct BigNumAllocator : DefaultAllocBase
	{
		typedef mbedtls_mpi      CObjType;

		using DefaultAllocBase::NewObject;
		using DefaultAllocBase::DelObject;

		static void Init(CObjType* ptr)
		{
			return mbedtls_mpi_init(ptr);
		}

		static void Free(CObjType* ptr) noexcept
		{
			return mbedtls_mpi_free(ptr);
		}
	};

	/**
	 * @brief Normal Big Number trait.
	 *
	 */
	using DefaultBigNumObjTrait = ObjTraitBase<BigNumAllocator,
									false,
									false>;

	/**
	 * @brief Borrower Big Number trait.
	 *
	 */
	using BorrowerBigNumTrait = ObjTraitBase<BorrowAllocBase<mbedtls_mpi>,
									true,
									false>;

	/**
	 * @brief The base class for big number objects. It defines all the basic and
	 *        constant (immutable) operations.
	 *
	 * @tparam _BigNumTrait The trait of big number.
	 */
	template<typename _BigNumTrait,
		enable_if_t<std::is_same<typename _BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
	class BigNumberBase : public ObjectBase<_BigNumTrait>
	{
	public: // Types:

		using _Base = ObjectBase<_BigNumTrait>;

	public:

		using _Base::ObjectBase;

		/**
		 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
		 *
		 * @exception None No exception thrown
		 * @param rhs The other BigNumberBase instance.
		 */
		BigNumberBase(BigNumberBase&& rhs) noexcept :
			_Base::ObjectBase(std::forward<_Base>(rhs)) //noexcept
		{}

		BigNumberBase(const BigNumberBase& rhs) = delete;

		// LCOV_EXCL_START
		/**
		 * @brief Destroy the Big Number Base object
		 *
		 */
		virtual ~BigNumberBase() = default;
		// LCOV_EXCL_STOP

		/**
		 * @brief Move assignment. The `rhs` will be empty/null afterwards.
		 *
		 * @exception None No exception thrown
		 * @param rhs The other BigNumberBase instance.
		 * @return BigNumberBase& A reference to this instance.
		 */
		BigNumberBase& operator=(BigNumberBase&& rhs) noexcept
		{
			_Base::operator=(std::forward<_Base>(rhs)); //noexcept

			return *this;
		}

		BigNumberBase& operator=(const BigNumberBase& other) = delete;

		/**
		 * @brief Check if the current instance is holding a null pointer for
		 *        the mbedTLS object. If so, exception will be thrown. Helper
		 *        function to be called before accessing the mbedTLS object.
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 */
		virtual void NullCheck() const
		{
			_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(BigNumberBase));
		}

		using _Base::Get;
		using _Base::NonVirtualGet;
		using _Base::Swap;

		/**
		 * @brief Swap the internal pointer of a Big Number base object with the
		 *        same trait.
		 *
		 * @exception None No exception thrown
		 * @param other The other big number object to swap with
		 */
		virtual void Swap(BigNumberBase& other) noexcept
		{
			ObjectBase<_BigNumTrait>::Swap(other);
		}

		/**
		 * @brief Is the big number positive?
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @return true If it's positive number (or zero)
		 * @return false If it's negative number
		 */
		bool IsPositive() const
		{
			NullCheck();
			int cmpRes = mbedtls_mpi_cmp_int(Get(), 0);
			return cmpRes >= 0;
		}

		/**
		 * @brief Get the size of the number in granularity of bytes.
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @return size_t the size of the number in bytes.
		 */
		size_t GetSize() const
		{
			NullCheck();
			return mbedtls_mpi_size(Get());
		}

		/**
		 * @brief Get the size of the number in granularity of bits.
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @return size_t the size of the number in bits.
		 */
		size_t GetBitSize() const
		{
			NullCheck();
			return mbedtls_mpi_bitlen(Get());
		}

		/**
		 * @brief Get the value of a individual bit.
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @param pos The position of the bit to get.
		 * @return bool true - 1, false - 0;
		 */
		bool GetBit(const size_t pos) const
		{
			NullCheck();
			return mbedtls_mpi_get_bit(Get(), pos) == 1;
		}

		/**
		 * @brief Compare this big number with another big number.
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @tparam _rhs_BigNumTrait The trait used by the other big number in
		 *                          right hand side.
		 * @param rhs The right hand side of the comparasion.
		 *
		 * @return \c 1  if \p this is greater than \p rhs.
		 * @return \c -1 if \p this is lesser than  \p rhs.
		 * @return \c 0  if \p this is equal to     \p rhs.
		 */
		template<typename _rhs_BigNumTrait,
				enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
		int Compare(const BigNumberBase<_rhs_BigNumTrait> & rhs) const
		{
			NullCheck();
			rhs.NullCheck();
			return mbedtls_mpi_cmp_mpi(Get(), rhs.Get());
		}

		/**
		 * @brief Overloading \p operator== .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @tparam _rhs_BigNumTrait The trait used by the other big number in
		 *                          right hand side.
		 * @param rhs The right hand side.
		 * @return bool \c true if both side are equal; \c false if otherwise.
		 */
		template<typename _rhs_BigNumTrait,
				enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
		bool operator==(const BigNumberBase<_rhs_BigNumTrait> & rhs) const
		{
			return Compare(rhs) == 0;
		}

		/**
		 * @brief Overloading \p operator!= .
		 *
		 * @tparam _rhs_BigNumTrait The trait used by the other big number in
		 *                          right hand side.
		 * @param rhs The right hand side.
		 * @return bool \c true if both side are not equal; \c false if otherwise.
		 */
		template<typename _rhs_BigNumTrait,
				enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
		bool operator!=(const BigNumberBase<_rhs_BigNumTrait> & rhs) const
		{
			return Compare(rhs) != 0;
		}

		/**
		 * @brief Overloading \p operator< .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @tparam _rhs_BigNumTrait The trait used by the other big number in
		 *                          right hand side.
		 * @param rhs The right hand side.
		 * @return bool \c true if \p LHS is less than \p RHS; \c false if otherwise.
		 */
		template<typename _rhs_BigNumTrait,
				enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
		bool operator<(const BigNumberBase<_rhs_BigNumTrait> & rhs) const
		{
			return Compare(rhs) < 0;
		}

		/**
		 * @brief Overloading \p operator<= .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @tparam _rhs_BigNumTrait The trait used by the other big number in
		 *                          right hand side.
		 * @param rhs The right hand side.
		 * @return bool \c true if \p LHS is less than or equal to \p RHS; \c false if otherwise.
		 */
		template<typename _rhs_BigNumTrait,
				enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
		bool operator<=(const BigNumberBase<_rhs_BigNumTrait> & rhs) const
		{
			return Compare(rhs) <= 0;
		}

		/**
		 * @brief Overloading \p operator> .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @tparam _rhs_BigNumTrait The trait used by the other big number in
		 *                          right hand side.
		 * @param rhs The right hand side.
		 * @return bool \c true if \p LHS is greater than \p RHS; \c false if otherwise.
		 */
		template<typename _rhs_BigNumTrait,
				enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
		bool operator>(const BigNumberBase<_rhs_BigNumTrait> & rhs) const
		{
			return Compare(rhs) > 0;
		}

		/**
		 * @brief Overloading \p operator>= .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @tparam _rhs_BigNumTrait The trait used by the other big number in
		 *                          right hand side.
		 * @param rhs The right hand side.
		 * @return bool \c true if \p LHS is greater than or equal to \p RHS; \c false if otherwise.
		 */
		template<typename _rhs_BigNumTrait,
				enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
		bool operator>=(const BigNumberBase<_rhs_BigNumTrait> & rhs) const
		{
			return Compare(rhs) >= 0;
		}

		/**
		 * @brief Compare this big number with a integral number.
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @tparam _ValType The type of the integral number.
		 * @param rhs The right hand side of the comparasion.
		 *
		 * @return \c 1  if \p this is greater than \p rhs.
		 * @return \c -1 if \p this is lesser than  \p rhs.
		 * @return \c 0  if \p this is equal to     \p rhs.
		 */
		template<typename _ValType,
			enable_if_t<(std::is_integral<_ValType>::value && std::is_signed<_ValType>::value && sizeof(_ValType) <= sizeof(mbedtls_mpi_sint)) ||
			(std::is_integral<_ValType>::value && std::is_unsigned<_ValType>::value && sizeof(_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0>
		int Compare(_ValType rhs) const
		{
			NullCheck();
			const mbedtls_mpi_sint rhsVal = static_cast<mbedtls_mpi_sint>(rhs);
			return mbedtls_mpi_cmp_int(Get(), rhsVal);
		}

		/**
		 * @brief Overloading \p operator== .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @tparam _ValType The type of the integral number.
		 * @param rhs The right hand side.
		 * @return bool \c true if both side are equal; \c false if otherwise.
		 */
		template<typename _ValType,
			enable_if_t<(std::is_integral<_ValType>::value && std::is_signed<_ValType>::value && sizeof(_ValType) <= sizeof(mbedtls_mpi_sint)) ||
			(std::is_integral<_ValType>::value && std::is_unsigned<_ValType>::value && sizeof(_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0>
		bool operator==(_ValType rhs) const
		{
			return Compare(rhs) == 0;
		}

		/**
		 * @brief Overloading \p operator!= .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @tparam _ValType The type of the integral number.
		 * @param rhs The right hand side.
		 * @return bool \c true if both side are not equal; \c false if otherwise.
		 */
		template<typename _ValType,
			enable_if_t<(std::is_integral<_ValType>::value && std::is_signed<_ValType>::value && sizeof(_ValType) <= sizeof(mbedtls_mpi_sint)) ||
			(std::is_integral<_ValType>::value && std::is_unsigned<_ValType>::value && sizeof(_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0>
		bool operator!=(_ValType rhs) const
		{
			return Compare(rhs) != 0;
		}

		/**
		 * @brief Overloading \p operator< .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @tparam _ValType The type of the integral number.
		 * @param rhs The right hand side.
		 * @return bool \c true if \p LHS is less than \p RHS; \c false if otherwise.
		 */
		template<typename _ValType,
			enable_if_t<(std::is_integral<_ValType>::value && std::is_signed<_ValType>::value && sizeof(_ValType) <= sizeof(mbedtls_mpi_sint)) ||
			(std::is_integral<_ValType>::value && std::is_unsigned<_ValType>::value && sizeof(_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0>
		bool operator<(_ValType rhs) const
		{
			return Compare(rhs) < 0;
		}

		/**
		 * @brief Overloading \p operator<= .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @tparam _ValType The type of the integral number.
		 * @param rhs The right hand side.
		 * @return bool \c true if \p LHS is less than or equal to \p RHS; \c false if otherwise.
		 */
		template<typename _ValType,
			enable_if_t<(std::is_integral<_ValType>::value && std::is_signed<_ValType>::value && sizeof(_ValType) <= sizeof(mbedtls_mpi_sint)) ||
			(std::is_integral<_ValType>::value && std::is_unsigned<_ValType>::value && sizeof(_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0>
		bool operator<=(_ValType rhs) const
		{
			return Compare(rhs) <= 0;
		}

		/**
		 * @brief Overloading \p operator> .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @tparam _ValType The type of the integral number.
		 * @param rhs The right hand side.
		 * @return bool \c true if \p LHS is greater than \p RHS; \c false if otherwise.
		 */
		template<typename _ValType,
			enable_if_t<(std::is_integral<_ValType>::value && std::is_signed<_ValType>::value && sizeof(_ValType) <= sizeof(mbedtls_mpi_sint)) ||
			(std::is_integral<_ValType>::value && std::is_unsigned<_ValType>::value && sizeof(_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0>
		bool operator>(_ValType rhs) const
		{
			return Compare(rhs) > 0;
		}

		/**
		 * @brief Overloading \p operator>= .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @tparam _ValType The type of the integral number.
		 * @param rhs The right hand side.
		 * @return bool \c true if \p LHS is greater than or equal to \p RHS; \c false if otherwise.
		 */
		template<typename _ValType,
			enable_if_t<(std::is_integral<_ValType>::value && std::is_signed<_ValType>::value && sizeof(_ValType) <= sizeof(mbedtls_mpi_sint)) ||
			(std::is_integral<_ValType>::value && std::is_unsigned<_ValType>::value && sizeof(_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0>
		bool operator>=(_ValType rhs) const
		{
			return Compare(rhs) >= 0;
		}

		/**
		 * @brief Calculate the modulo value with a given integral number.
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @tparam _ValType The type of the integral number.
		 * @param rhs The right hand side.
		 * @return _ValType The result of calculation.
		 */
		template<typename _ValType,
			enable_if_t<(std::is_integral<_ValType>::value && std::is_signed<_ValType>::value && sizeof(_ValType) <= sizeof(mbedtls_mpi_sint)) ||
			(std::is_integral<_ValType>::value && std::is_unsigned<_ValType>::value && sizeof(_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0>
		_ValType Mod(_ValType rhs) const
		{
			NullCheck();

			mbedtls_mpi_sint rhsVal = static_cast<mbedtls_mpi_sint>(rhs);
			mbedtls_mpi_uint res = 0;
			MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumberBase::Mod, mbedtls_mpi_mod_int, &res, Get(), rhsVal);

			return static_cast<_ValType>(res);
		}

		/**
		 * @brief Convert this big number to a hex string. This string doesn't
		 *        contain the \c '0x' prefix.
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception std::bad_alloc Thrown when memory allocation failed.
		 * @return std::string The output hex string.
		 */
		std::string Hex() const
		{
			return ToNumString<16>();
		}

		/**
		 * @brief Convert this big number to a binary string with \c 0 's and \c 1 's.
		 *        This string doesn't contain \c '0b' prefix.
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception std::bad_alloc Thrown when memory allocation failed.
		 * @return std::string The output binary string.
		 */
		std::string Bin() const
		{
			return ToNumString<2>();
		}

		/**
		 * @brief Convert this big number to a human-readable decimal number string.
		 *        This string contain the negative sign.
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @exception std::bad_alloc Thrown when memory allocation failed.
		 * @return std::string The output string.
		 */
		std::string Dec() const
		{
			return ToNumString<10>();
		}

		/**
		 * @brief Convert this big number to an array of bytes.
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @exception std::bad_alloc Thrown when memory allocation failed.
		 * @tparam _LitEndian Should output in little-endian format? (Default to \c true )
		 * @return std::vector<uint8_t> The output array of bytes.
		 */
		template<bool _LitEndian = true>
		std::vector<uint8_t> Bytes() const
		{
			NullCheck();
			const size_t size = GetSize();
			std::vector<uint8_t> res(size);

			if (_LitEndian) // Little Endian
			{
				MBEDTLSCPP_MAKE_C_FUNC_CALL(
					BigNumberBase::Bytes,
					mbedtls_mpi_write_binary_le,
					Get(),
					res.data(),
					size
				);
			}
			else            // Big Endian
			{
				MBEDTLSCPP_MAKE_C_FUNC_CALL(
					BigNumberBase::Bytes,
					mbedtls_mpi_write_binary,
					Get(),
					res.data(),
					size
				);
			}

			return res;
		}

		/**
		 * @brief Convert this big number to an secret array of bytes.
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @exception std::bad_alloc Thrown when memory allocation failed.
		 * @tparam _LitEndian Should output in little-endian format? (Default to \c true )
		 * @return SecretVector<uint8_t> The output array of bytes.
		 */
		template<bool _LitEndian = true>
		SecretVector<uint8_t> SecretBytes() const
		{
			NullCheck();
			const size_t size = GetSize();
			SecretVector<uint8_t> res(size);

			if (_LitEndian) // Little Endian
			{
				MBEDTLSCPP_MAKE_C_FUNC_CALL(
					BigNumberBase::Bytes,
					mbedtls_mpi_write_binary_le,
					Get(),
					res.data(),
					size
				);
			}
			else            // Big Endian
			{
				MBEDTLSCPP_MAKE_C_FUNC_CALL(
					BigNumberBase::Bytes,
					mbedtls_mpi_write_binary,
					Get(),
					res.data(),
					size
				);
			}

			return res;
		}

		protected:

			using _Base::NullCheck;

		private:

			template<size_t _Raidx>
			std::string ToNumString() const
			{
				NullCheck();

				size_t outLen = 0;
				auto getLenRes = mbedtls_mpi_write_string(
					Get(),
					_Raidx,
					nullptr,
					0,
					&outLen
				);
				if (getLenRes != MBEDTLS_ERR_MPI_BUFFER_TOO_SMALL)
				{
					throw mbedTLSRuntimeError(
						getLenRes,
						mbedTLSRuntimeError::ConstructWhatMsg(
							getLenRes,
							"BigNumberBase::ToNumString",
							"mbedtls_mpi_write_string"
						)
					);
				}

				std::string res(outLen, '\0');
				MBEDTLSCPP_MAKE_C_FUNC_CALL(
					BigNumberBase::ToNumString,
					mbedtls_mpi_write_string,
					Get(),
					_Raidx,
					&res[0],
					res.size(),
					&outLen
				);

				res.resize(outLen - 1); // -1 to remove null terminator

				return res;
			}
	};


	/**
	 * @brief Constant Big Number allocator.
	 *
	 */
	struct ConstBigNumAllocator : DefaultAllocBase
	{
		typedef mbedtls_mpi      CObjType;

		using DefaultAllocBase::NewObject;
		using DefaultAllocBase::DelObject;

		static void Init(CObjType* /* ptr */)
		{}

		static void Free(CObjType* /* ptr */) noexcept
		{}
	};


	/**
	 * @brief The class for a normal Big Number object.
	 *
	 */
	template<typename _ObjTrait = DefaultBigNumObjTrait,
			 enable_if_t<!_ObjTrait::sk_isConst, int> = 0>
	class BigNumber : public BigNumberBase<_ObjTrait>
	{
	public: // Static members:

		using _Base = BigNumberBase<_ObjTrait>;

		/**
		 * @brief Construct a new Big Number object and fill with random value.
		 *
		 * @param size The number of random bytes to generate.
		 * @param rand The random bit generator.
		 * @return BigNumber The new random big number.
		 */
		static BigNumber<DefaultBigNumObjTrait> Rand(
			size_t size,
			RbgInterface& rand
		)
		{
			BigNumber<DefaultBigNumObjTrait> rd;
			MBEDTLSCPP_MAKE_C_FUNC_CALL(
				BigNumber::Rand,
				mbedtls_mpi_fill_random,
				rd.Get(),
				size,
				&RbgInterface::CallBack,
				&rand
			);

			return rd;
		}

		static const BigNumber<DefaultBigNumObjTrait>& Zero()
		{
			static BigNumber<DefaultBigNumObjTrait> zero;
			return zero;
		}

		static const BigNumber<DefaultBigNumObjTrait>& NegativeOne()
		{
			static auto constructFunc = []() -> BigNumber<DefaultBigNumObjTrait>
			{
				BigNumber<DefaultBigNumObjTrait> res;

				MBEDTLSCPP_MAKE_C_FUNC_CALL(
					BigNumber::NegativeOne,
					mbedtls_mpi_lset,
					res.Get(),
					-1
				);
				return res;
			};

			static BigNumber<DefaultBigNumObjTrait> nOne = constructFunc();
			return nOne;
		}

	public:

		/**
		 * @brief Construct a new Big Number object, which is initialized, but
		 *        with zero value.
		 *
		 * @exception std::bad_alloc Thrown when memory allocation failed.
		 *
		 */
		template<typename _dummy_Trait = _ObjTrait,
				 enable_if_t<!_dummy_Trait::sk_isBorrower, int> = 0>
		BigNumber() :
			_Base::BigNumberBase()
		{}

		/**
		 * @brief Construct a new Big Number object, which is initialized, but
		 *        with zero value.
		 *
		 * @exception std::bad_alloc Thrown when memory allocation failed.
		 *
		 */
		template<typename _dummy_Trait = _ObjTrait,
				 enable_if_t<_dummy_Trait::sk_isBorrower, int> = 0>
		BigNumber(mbedtls_mpi* other) :
			_Base::BigNumberBase(other)
		{}

		/**
		 * @brief Construct a new Big Number object by copying other BigNumber.
		 *        If \c other is null, then this instance will be null as well.
		 *        Otherwise, deep copy will be performed.
		 *
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @exception std::bad_alloc Thrown when memory allocation failed.
		 * @tparam _other_BigNumTrait The trait used by the other big number.
		 * @param other The other big number to copy from.
		 */
		template<typename _other_BigNumTrait,
				 typename _dummy_Trait = _ObjTrait,
				 enable_if_t<std::is_same<typename _other_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0,
				 enable_if_t<!_dummy_Trait::sk_isBorrower, int> = 0>
		BigNumber(const BigNumberBase<_other_BigNumTrait>& other, const void* = nullptr) :
			_Base::BigNumberBase()
		{
			if(other.IsNull())
			{
				_Base::FreeBaseObject();
			}
			else
			{
				MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::BigNumber, mbedtls_mpi_copy, NonVirtualGet(), other.Get());
			}
		}

		BigNumber(const BigNumber& other) :
			BigNumber(static_cast<const BigNumberBase<_ObjTrait>&>(other), (const void*)nullptr)
		{}

		/**
		 * @brief Construct a new Big Number object by copying other big number
		 *        C object.
		 *
		 *
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @exception std::bad_alloc Thrown when memory allocation failed.
		 * @param other The other big number to copy from.
		 */
		template<typename _dummy_Trait = _ObjTrait,
				 enable_if_t<!_dummy_Trait::sk_isBorrower, int> = 0>
		BigNumber(const mbedtls_mpi& other) :
			_Base::BigNumberBase()
		{
			MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::BigNumber, mbedtls_mpi_copy, NonVirtualGet(), &other);
		}

		/**
		 * @brief Construct a new Big Number object by copying bytes from a byte array.
		 *
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @exception std::bad_alloc Thrown when memory allocation failed.
		 * @tparam ContainerType The type of the data container.
		 * @param data The reference to data container.
		 * @param isPositive Should the constructed big number be positive?
		 *                   (since we assume the byte array only stores
		 *                   unsigned value)
		 * @param isLittleEndian Is the input bytes in little-endian format?
		 */
		template<typename ContainerType, bool ContainerSecrecy,
				 typename _dummy_Trait = _ObjTrait,
				 enable_if_t<!_dummy_Trait::sk_isBorrower, int> = 0>
		BigNumber(const ContCtnReadOnlyRef<ContainerType, ContainerSecrecy>& data, bool isPositive = true, bool isLittleEndian = true) :
			_Base::BigNumberBase()
		{
			if (isLittleEndian)
			{
				MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::BigNumber,
					mbedtls_mpi_read_binary_le,
					NonVirtualGet(),
					static_cast<const unsigned char*>(data.BeginPtr()),
					data.GetRegionSize());
			}
			else
			{
				MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::BigNumber,
					mbedtls_mpi_read_binary,
					NonVirtualGet(),
					static_cast<const unsigned char*>(data.BeginPtr()),
					data.GetRegionSize());
			}

			if (!isPositive)
			{
				BigNumber x;
				MBEDTLSCPP_MAKE_C_FUNC_CALL(
					BigNumber::BigNumber,
					mbedtls_mpi_sub_mpi,
					x.NonVirtualGet(),
					Zero().NonVirtualGet(),
					NonVirtualGet()
				);
				mbedtls_mpi_swap(x.NonVirtualGet(), NonVirtualGet());
			}
		}

		/**
		 * @brief Construct a new Big Number object by copying value from a native integral value.
		 *
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @exception std::bad_alloc Thrown when memory allocation failed.
		 * @param val The value to copy from.
		 * @param isPositive Should the constructed big number be positive?
		 *                   (since here we accpet an unsigned value)
		 */
		template<typename _dummy_Trait = _ObjTrait,
				 enable_if_t<!_dummy_Trait::sk_isBorrower, int> = 0>
		BigNumber(mbedtls_mpi_uint val, bool isPositive = true) :
			BigNumber(
				CtnFullR(CDynArray<mbedtls_mpi_uint>{
					&val,
					1
				}),
				isPositive,
				true  // TODO: get endianness from platform
			)
		{}

		/**
		 * @brief Construct a new Big Number object by copying value from a native integral value.
		 *
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @exception std::bad_alloc Thrown when memory allocation failed.
		 * @tparam _ValType The type of the integral number.
		 *                  It should be an unsigned type.
		 *                  Thus, the constructed big number will be positive.
		 * @param val The value to copy from.
		 */
		template<typename _ValType,
				 typename _dummy_Trait = _ObjTrait,
				 enable_if_t<
				 	std::is_integral<_ValType>::value &&
					std::is_unsigned<_ValType>::value &&
					sizeof(_ValType) <= sizeof(mbedtls_mpi_uint),
				 int> = 0,
				 enable_if_t<!_dummy_Trait::sk_isBorrower, int> = 0>
		BigNumber(_ValType val) :
			BigNumber(static_cast<mbedtls_mpi_uint>(val), true)
		{}

		/**
		 * @brief Construct a new Big Number object by copying value from a native integral value.
		 *
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @exception std::bad_alloc Thrown when memory allocation failed.
		 * @tparam _ValType The type of the integral number. It should be an signed type.
		 * @param val The value to copy from.
		 */
		template<typename _ValType,
				 typename _dummy_Trait = _ObjTrait,
				 enable_if_t<
				 	std::is_integral<_ValType>::value &&
					std::is_signed<_ValType>::value &&
					sizeof(_ValType) <= sizeof(mbedtls_mpi_uint),
				 int> = 0,
				 enable_if_t<!_dummy_Trait::sk_isBorrower, int> = 0>
		BigNumber(_ValType val) :
			BigNumber(static_cast<mbedtls_mpi_uint>(val >= 0 ? val : -val), val >= 0)
		{}

		/**
		 * @brief Move Constructor. The `rhs` will be empty/null afterwards.
		 *
		 * @exception None No exception thrown
		 * @param rhs The other BigNumber instance.
		 */
		BigNumber(BigNumber&& rhs) noexcept :
			_Base::BigNumberBase(std::forward<_Base>(rhs)) //noexcept
		{}

		// LCOV_EXCL_START
		/**
		 * @brief Destroy the Big Number Base object
		 *
		 */
		virtual ~BigNumber() = default;
		// LCOV_EXCL_STOP

		/**
		 * @brief Move assignment. The `rhs` will be empty/null afterwards.
		 *
		 * @exception None No exception thrown
		 * @param rhs The other BigNumber instance.
		 * @return BigNumber& A reference to this instance.
		 */
		BigNumber& operator=(BigNumber&& rhs) noexcept
		{
			_Base::operator=(std::forward<_Base>(rhs)); //noexcept

			return *this;
		}

		/**
		 * @brief Copy assignment. If \c RHS is null, then this instance will become
		 *        null as well. Otherwise, deep copy will be performed.
		 *
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @tparam _rhs_BigNumTrait The trait used by the \c RHS big number.
		 * @param rhs The number in right hand side.
		 * @return BigNumber& The reference to this instance.
		 */
		template<typename _rhs_BigNumTrait,
			enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
		BigNumber& operator=(const BigNumberBase<_rhs_BigNumTrait>& rhs)
		{
			if (static_cast<const void*>(this) != static_cast<const void*>(&rhs))
			{
				if(rhs.IsNull())
				{
					_Base::FreeBaseObject();
				}
				else
				{
					MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::operator=, mbedtls_mpi_copy, Get(), rhs.Get());
				}
			}
			return *this;
		}

		BigNumber& operator=(const BigNumber& rhs)
		{
			return operator=<_ObjTrait>(rhs);
		}

		using _Base::Swap;
		using _Base::Get;
		using _Base::NonVirtualGet;
		using _Base::NullCheck;

		/**
		 * @brief Check if the current instance is holding a null pointer for
		 *        the mbedTLS object. If so, exception will be thrown. Helper
		 *        function to be called before accessing the mbedTLS object.
		 *
		 * @exception InvalidObjectException Thrown when the current instance is
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 */
		virtual void NullCheck() const
		{
			_Base::NullCheck(MBEDTLSCPP_CLASS_NAME_STR(BigNumber));
		}

		/**
		 * @brief Flip the sign of the big number.
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @return BigNumber& A reference to this instance.
		 */
		BigNumber& FlipSign()
		{
			NullCheck();

			BigNumber<DefaultBigNumObjTrait> x;
			MBEDTLSCPP_MAKE_C_FUNC_CALL(
				BigNumber::FlipSign,
				mbedtls_mpi_sub_mpi,
				x.Get(),
				Zero().Get(),
				Get()
			);
			mbedtls_mpi_swap(Get(), x.Get());

			return *this;
		}

		/**
		 * @brief Swap the content of big number, by calling the mbedtls_mpi_swap
		 *        function. Different from the normal \c Swap method, this method
		 *        requires \c this and \c other instances are not null.
		 *
		 * @tparam _other_Traits The trait used by other instance.
		 * @param other The other instance to swap with.
		 */
		template<typename _other_Traits>
		void SwapContent(BigNumber<_other_Traits>& other)
		{
			NullCheck();
			other.NullCheck();

			MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::SwapContent,
				mbedtls_mpi_swap, Get(), other.Get());
		}

		/**
		 * @brief Swap the content of big number with the MPI C object, by
		 *        calling the mbedtls_mpi_swap function.
		 *        Different from the normal \c Swap method, this method requires
		 *        \c this instance is not null.
		 *
		 * @tparam _other_Traits The trait used by other instance.
		 * @param other The other instance to swap with.
		 */
		void SwapContent(mbedtls_mpi& other)
		{
			NullCheck();

			mbedtls_mpi_swap(Get(), &other);
		}

		/**
		 * @brief Overloading \p operator<<= .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @param rhs The value on right hand side.
		 * @return BigNumber& A reference to this instance.
		 */
		BigNumber & operator<<=(size_t rhs)
		{
			NullCheck();

			MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::operator<<=, mbedtls_mpi_shift_l, Get(), rhs);

			return *this;
		}

		/**
		 * @brief Overloading \p operator>>= .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @param rhs The value on right hand side.
		 * @return BigNumber& A reference to this instance.
		 */
		BigNumber & operator>>=(size_t rhs)
		{
			NullCheck();

			MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::operator>>=, mbedtls_mpi_shift_r, Get(), rhs);

			return *this;
		}

		/**
		 * @brief Overloading \p operator+= .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @tparam _rhs_BigNumTrait The trait used by the other big number on
		 *                          right hand side.
		 * @param rhs The value on right hand side.
		 * @return BigNumber& A reference to this instance.
		 */
		template<typename _rhs_BigNumTrait,
			enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
		BigNumber& operator+=(const BigNumberBase<_rhs_BigNumTrait>& rhs)
		{
			NullCheck();
			rhs.NullCheck();

			// Can call it as A = A + B, see bignum.c
			MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::operator+=, mbedtls_mpi_add_mpi, Get(), Get(), rhs.Get());

			return *this;
		}

		/**
		 * @brief Overloading \p operator+= .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @tparam _rhs_ValType The type of the integral number.
		 * @param rhs The right hand side.
		 * @return BigNumber& A reference to this instance.
		 */
		template<typename _rhs_ValType,
			enable_if_t<(std::is_integral<_rhs_ValType>::value && std::is_signed<_rhs_ValType>::value && sizeof(_rhs_ValType) <= sizeof(mbedtls_mpi_sint)) ||
			(std::is_integral<_rhs_ValType>::value && std::is_unsigned<_rhs_ValType>::value && sizeof(_rhs_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0>
		BigNumber& operator+=(_rhs_ValType rhs)
		{
			const mbedtls_mpi_sint rhsVal = static_cast<mbedtls_mpi_sint>(rhs);
			NullCheck();

			// Can call it as A = A + B, see bignum.c
			MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::operator+=, mbedtls_mpi_add_int, Get(), Get(), rhsVal);

			return *this;
		}

		/**
		 * @brief Overloading \p operator-= .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @tparam _rhs_BigNumTrait The trait used by the other big number on
		 *                          right hand side.
		 * @param rhs The value on right hand side.
		 * @return BigNumber& A reference to this instance.
		 */
		template<typename _rhs_BigNumTrait,
			enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
		BigNumber& operator-=(const BigNumberBase<_rhs_BigNumTrait>& rhs)
		{
			NullCheck();
			rhs.NullCheck();

			// Can call it as A = A - B, see bignum.c
			MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::operator-=, mbedtls_mpi_sub_mpi, Get(), Get(), rhs.Get());

			return *this;
		}

		/**
		 * @brief Overloading \p operator-= .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @tparam _rhs_ValType The type of the integral number.
		 * @param rhs The right hand side.
		 * @return BigNumber& A reference to this instance.
		 */
		template<typename _rhs_ValType,
			enable_if_t<(std::is_integral<_rhs_ValType>::value && std::is_signed<_rhs_ValType>::value && sizeof(_rhs_ValType) <= sizeof(mbedtls_mpi_sint)) ||
			(std::is_integral<_rhs_ValType>::value && std::is_unsigned<_rhs_ValType>::value && sizeof(_rhs_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0>
		BigNumber& operator-=(_rhs_ValType rhs)
		{
			const mbedtls_mpi_sint rhsVal = static_cast<mbedtls_mpi_sint>(rhs);
			NullCheck();

			// Can call it as A = A - B, see bignum.c
			MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::operator-=, mbedtls_mpi_sub_int, Get(), Get(), rhsVal);

			return *this;
		}

		/**
		 * @brief Overloading \p operator*= .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @tparam _rhs_BigNumTrait The trait used by the other big number on
		 *                          right hand side.
		 * @param rhs The value on right hand side.
		 * @return BigNumber& A reference to this instance.
		 */
		template<typename _rhs_BigNumTrait,
			enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
		BigNumber& operator*=(const BigNumberBase<_rhs_BigNumTrait>& rhs)
		{
			NullCheck();
			rhs.NullCheck();

			// Can call it as A = A * B, see bignum.c
			MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::operator*=, mbedtls_mpi_mul_mpi, Get(), Get(), rhs.Get());

			return *this;
		}

		/**
		 * @brief Overloading \p operator*= .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @tparam _rhs_ValType The type of the integral number.
		 * @param rhs The right hand side.
		 * @return BigNumber& A reference to this instance.
		 */
		template<typename _rhs_ValType,
			enable_if_t<(std::is_integral<_rhs_ValType>::value && std::is_signed<_rhs_ValType>::value && sizeof(_rhs_ValType) <= sizeof(mbedtls_mpi_uint)) ||
			(std::is_integral<_rhs_ValType>::value && std::is_unsigned<_rhs_ValType>::value && sizeof(_rhs_ValType) <= sizeof(mbedtls_mpi_uint)), int> = 0>
		BigNumber& operator*=(_rhs_ValType rhs)
		{
			const bool isPos = rhs >= 0;
			const mbedtls_mpi_uint rhsVal = static_cast<mbedtls_mpi_uint>(isPos ? rhs : -rhs );
			NullCheck();

			// Can call it as A = A * B, see bignum.c
			MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::operator*=, mbedtls_mpi_mul_int, Get(), Get(), rhsVal);
			if(!isPos)
			{
				FlipSign();
			}

			return *this;
		}

		/**
		 * @brief Overloading \p operator/= .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @tparam _rhs_BigNumTrait The trait used by the other big number on
		 *                          right hand side.
		 * @param rhs The value on right hand side.
		 * @return BigNumber& A reference to this instance.
		 */
		template<typename _rhs_BigNumTrait,
			enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
		BigNumber& operator/=(const BigNumberBase<_rhs_BigNumTrait>& rhs)
		{
			NullCheck();
			rhs.NullCheck();

			BigNumber res;

			MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::operator/=, mbedtls_mpi_div_mpi, res.Get(), nullptr, Get(), rhs.Get());
			Swap(res);

			return *this;
		}

		/**
		 * @brief Overloading \p operator/= .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @tparam _rhs_ValType The type of the integral number.
		 * @param rhs The right hand side.
		 * @return BigNumber& A reference to this instance.
		 */
		template<typename _rhs_ValType,
			enable_if_t<(std::is_integral<_rhs_ValType>::value && std::is_signed<_rhs_ValType>::value && sizeof(_rhs_ValType) <= sizeof(mbedtls_mpi_sint)) ||
			(std::is_integral<_rhs_ValType>::value && std::is_unsigned<_rhs_ValType>::value && sizeof(_rhs_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0>
		BigNumber& operator/=(_rhs_ValType rhs)
		{
			const mbedtls_mpi_sint rhsVal = static_cast<mbedtls_mpi_sint>(rhs);
			NullCheck();

			BigNumber res;

			MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::operator/=, mbedtls_mpi_div_int, res.Get(), nullptr, Get(), rhsVal);
			Swap(res);

			return *this;
		}

		/**
		 * @brief Overloading \p operator%= .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @tparam _rhs_BigNumTrait The trait used by the other big number on
		 *                          right hand side.
		 * @param rhs The value on right hand side.
		 * @return BigNumber& A reference to this instance.
		 */
		template<typename _rhs_BigNumTrait,
			enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
		BigNumber& operator%=(const BigNumberBase<_rhs_BigNumTrait>& rhs)
		{
			NullCheck();
			rhs.NullCheck();

			BigNumber res;

			MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::operator%=, mbedtls_mpi_div_mpi, nullptr, res.Get(), Get(), rhs.Get());
			Swap(res);

			return *this;
		}

		/**
		 * @brief Overloading \p operator%= .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @tparam _rhs_ValType The type of the integral number.
		 * @param rhs The right hand side.
		 * @return BigNumber& A reference to this instance.
		 */
		template<typename _rhs_ValType,
			enable_if_t<(std::is_integral<_rhs_ValType>::value && std::is_signed<_rhs_ValType>::value && sizeof(_rhs_ValType) <= sizeof(mbedtls_mpi_sint)) ||
			(std::is_integral<_rhs_ValType>::value && std::is_unsigned<_rhs_ValType>::value && sizeof(_rhs_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0>
		BigNumber& operator%=(_rhs_ValType rhs)
		{
			const mbedtls_mpi_sint rhsVal = static_cast<mbedtls_mpi_sint>(rhs);
			NullCheck();

			BigNumber res;

			MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::operator%=, mbedtls_mpi_div_int, nullptr, res.Get(), Get(), rhsVal);
			Swap(res);

			return *this;
		}

		/**
		 * @brief Overloading \p operator++ .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @return BigNumber& A reference to this instance.
		 */
		BigNumber& operator++()
		{
			*this += 1;

			return *this;
		}

		/**
		 * @brief Overloading \p operator-- .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @return BigNumber& A reference to this instance.
		 */
		BigNumber& operator--()
		{
			*this -= 1;

			return *this;
		}

		/**
		 * @brief Overloading \p operator++ .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @return BigNumber& A reference to this instance.
		 */
		BigNumber operator++(int)
		{
			NullCheck();

			BigNumber res;

			MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::operator++, mbedtls_mpi_add_int, res.Get(), Get(), 1);
			Swap(res);

			return res;
		}

		/**
		 * @brief Overloading \p operator-- .
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @return BigNumber& A reference to this instance.
		 */
		BigNumber operator--(int)
		{
			NullCheck();

			BigNumber res;

			MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::operator--, mbedtls_mpi_sub_int, res.Get(), Get(), 1);
			Swap(res);

			return res;
		}

		/**
		 * @brief Set an individual bit in the big number.
		 *
		 * @exception InvalidObjectException Thrown when one or more given objects are
		 *                                   holding a null pointer for the C mbed TLS
		 *                                   object.
		 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
		 * @param pos The position of the bit to set.
		 * @param bit The bit value: \c true - 1, \c false - 0.
		 * @return BigNumber& A reference to this instance.
		 */
		BigNumber & SetBit(size_t pos, bool bit)
		{
			NullCheck();

			MBEDTLSCPP_MAKE_C_FUNC_CALL(BigNumber::SetBit, mbedtls_mpi_set_bit, Get(), pos, bit ? 1 : 0);

			return *this;
		}
	};

	using BigNum = BigNumber<>;

	/**
	 * @brief Overloading \p operator== .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _ValType The type of the native integral number on left hand side.
	 * @tparam _rhs_BigNumTrait The trait used by the other big number in
	 *                          right hand side.
	 * @param lhs The left hand side.
	 * @param rhs The right hand side.
	 * @return bool \c true if both side are equal; \c false if otherwise.
	 */
	template<typename _ValType, typename _BigNumTrait,
		enable_if_t<(std::is_integral<_ValType>::value && std::is_signed<_ValType>::value && sizeof(_ValType) <= sizeof(mbedtls_mpi_sint)) ||
		(std::is_integral<_ValType>::value && std::is_unsigned<_ValType>::value && sizeof(_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0,
		enable_if_t<std::is_same<typename _BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
	inline bool operator==(_ValType lhs, const BigNumberBase<_BigNumTrait> & rhs)
	{
		return rhs.operator==(lhs);
	}

	/**
	 * @brief Overloading \p operator!= .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _ValType The type of the native integral number on left hand side.
	 * @tparam _rhs_BigNumTrait The trait used by the other big number in
	 *                          right hand side.
	 * @param lhs The left hand side.
	 * @param rhs The right hand side.
	 * @return bool \c true if both side are not equal; \c false if otherwise.
	 */
	template<typename _ValType, typename _BigNumTrait,
		enable_if_t<(std::is_integral<_ValType>::value && std::is_signed<_ValType>::value && sizeof(_ValType) <= sizeof(mbedtls_mpi_sint)) ||
		(std::is_integral<_ValType>::value && std::is_unsigned<_ValType>::value && sizeof(_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0,
		enable_if_t<std::is_same<typename _BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
	inline bool operator!=(_ValType lhs, const BigNumberBase<_BigNumTrait> & rhs)
	{
		return rhs.operator!=(lhs);
	}

	/**
	 * @brief Overloading \p operator>= .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _ValType The type of the native integral number on left hand side.
	 * @tparam _rhs_BigNumTrait The trait used by the other big number in
	 *                          right hand side.
	 * @param lhs The left hand side.
	 * @param rhs The right hand side.
	 * @return bool \c true if \p LHS is greater than or equal to \p RHS; \c false if otherwise.
	 */
	template<typename _ValType, typename _BigNumTrait,
		enable_if_t<(std::is_integral<_ValType>::value && std::is_signed<_ValType>::value && sizeof(_ValType) <= sizeof(mbedtls_mpi_sint)) ||
		(std::is_integral<_ValType>::value && std::is_unsigned<_ValType>::value && sizeof(_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0,
		enable_if_t<std::is_same<typename _BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
	inline bool operator>=(_ValType lhs, const BigNumberBase<_BigNumTrait> & rhs)
	{
		// lhs >= rhs
		return rhs.operator<=(lhs);
	}

	/**
	 * @brief Overloading \p operator> .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _ValType The type of the native integral number on left hand side.
	 * @tparam _rhs_BigNumTrait The trait used by the other big number in
	 *                          right hand side.
	 * @param lhs The left hand side.
	 * @param rhs The right hand side.
	 * @return bool \c true if \p LHS is greater than \p RHS; \c false if otherwise.
	 */
	template<typename _ValType, typename _BigNumTrait,
		enable_if_t<(std::is_integral<_ValType>::value && std::is_signed<_ValType>::value && sizeof(_ValType) <= sizeof(mbedtls_mpi_sint)) ||
		(std::is_integral<_ValType>::value && std::is_unsigned<_ValType>::value && sizeof(_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0,
		enable_if_t<std::is_same<typename _BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
	inline bool operator>(_ValType lhs, const BigNumberBase<_BigNumTrait> & rhs)
	{
		// lhs > rhs
		return rhs.operator<(lhs);
	}

	/**
	 * @brief Overloading \p operator< .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _ValType The type of the native integral number on left hand side.
	 * @tparam _rhs_BigNumTrait The trait used by the other big number in
	 *                          right hand side.
	 * @param lhs The left hand side.
	 * @param rhs The right hand side.
	 * @return bool \c true if \p LHS is less than or equal to \p RHS; \c false if otherwise.
	 */
	template<typename _ValType, typename _BigNumTrait,
		enable_if_t<(std::is_integral<_ValType>::value && std::is_signed<_ValType>::value && sizeof(_ValType) <= sizeof(mbedtls_mpi_sint)) ||
		(std::is_integral<_ValType>::value && std::is_unsigned<_ValType>::value && sizeof(_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0,
		enable_if_t<std::is_same<typename _BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
	inline bool operator<=(_ValType lhs, const BigNumberBase<_BigNumTrait> & rhs)
	{
		// lhs <= rhs
		return rhs.operator>=(lhs);
	}

	/**
	 * @brief Overloading \p operator< .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _ValType The type of the native integral number on left hand side.
	 * @tparam _rhs_BigNumTrait The trait used by the other big number in
	 *                          right hand side.
	 * @param lhs The left hand side.
	 * @param rhs The right hand side.
	 * @return bool \c true if \p LHS is less than \p RHS; \c false if otherwise.
	 */
	template<typename _ValType, typename _BigNumTrait,
		enable_if_t<(std::is_integral<_ValType>::value && std::is_signed<_ValType>::value && sizeof(_ValType) <= sizeof(mbedtls_mpi_sint)) ||
		(std::is_integral<_ValType>::value && std::is_unsigned<_ValType>::value && sizeof(_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0,
		enable_if_t<std::is_same<typename _BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
	inline bool operator<(_ValType lhs, const BigNumberBase<_BigNumTrait> & rhs)
	{
		// lhs < rhs
		return rhs.operator>(lhs);
	}

	/**
	 * @brief Overloading \p operator+ .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _lhs_BigNumTrait The trait used by the other big number on
	 *                          left hand side.
	 * @tparam _rhs_BigNumTrait The trait used by the other big number on
	 *                          right hand side.
	 * @param lhs The value on left hand side.
	 * @param rhs The value on right hand side.
	 * @return BigNum The result of calculation, a new Big Number object.
	 */
	template<typename _lhs_BigNumTrait, typename _rhs_BigNumTrait,
		enable_if_t<std::is_same<typename _lhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0,
		enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
	inline BigNum operator+(const BigNumberBase<_lhs_BigNumTrait>& lhs, const BigNumberBase<_rhs_BigNumTrait>& rhs)
	{
		lhs.NullCheck();
		rhs.NullCheck();

		BigNum res;

		MBEDTLSCPP_MAKE_C_FUNC_CALL(::operator+_lhsBigNum-_rhsBigNum, mbedtls_mpi_add_mpi, res.Get(), lhs.Get(), rhs.Get());

		return res;
	}

	/**
	 * @brief Overloading \p operator- .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _lhs_BigNumTrait The trait used by the other big number on
	 *                          left hand side.
	 * @tparam _rhs_BigNumTrait The trait used by the other big number on
	 *                          right hand side.
	 * @param lhs The value on left hand side.
	 * @param rhs The value on right hand side.
	 * @return BigNum The result of calculation, a new Big Number object.
	 */
	template<typename _lhs_BigNumTrait, typename _rhs_BigNumTrait,
		enable_if_t<std::is_same<typename _lhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0,
		enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
	inline BigNum operator-(const BigNumberBase<_lhs_BigNumTrait>& lhs, const BigNumberBase<_rhs_BigNumTrait>& rhs)
	{
		lhs.NullCheck();
		rhs.NullCheck();

		BigNum res;

		MBEDTLSCPP_MAKE_C_FUNC_CALL(::operator-_lhsBigNum-_rhsBigNum, mbedtls_mpi_sub_mpi, res.Get(), lhs.Get(), rhs.Get());

		return res;
	}

	/**
	 * @brief Overloading \p operator- (negation operator).
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _rhs_BigNumTrait The trait used by the other big number on
	 *                          right hand side.
	 * @param rhs The value on right hand side.
	 * @return BigNum The result of calculation, a new Big Number object.
	 */
	template<typename _rhs_BigNumTrait,
		enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
	inline BigNum operator-(const BigNumberBase<_rhs_BigNumTrait>& rhs)
	{
		BigNum cpy(rhs);
		cpy.FlipSign();
		return cpy;
	}

	/**
	 * @brief Overloading \p operator* .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _lhs_BigNumTrait The trait used by the other big number on
	 *                          left hand side.
	 * @tparam _rhs_BigNumTrait The trait used by the other big number on
	 *                          right hand side.
	 * @param lhs The value on left hand side.
	 * @param rhs The value on right hand side.
	 * @return BigNum The result of calculation, a new Big Number object.
	 */
	template<typename _lhs_BigNumTrait, typename _rhs_BigNumTrait,
		enable_if_t<std::is_same<typename _lhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0,
		enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
	inline BigNum operator*(const BigNumberBase<_lhs_BigNumTrait>& lhs, const BigNumberBase<_rhs_BigNumTrait>& rhs)
	{
		lhs.NullCheck();
		rhs.NullCheck();

		BigNum res;

		MBEDTLSCPP_MAKE_C_FUNC_CALL(::operator*_lhsBigNum-_rhsBigNum, mbedtls_mpi_mul_mpi, res.Get(), lhs.Get(), rhs.Get());

		return res;
	}

	/**
	 * @brief Overloading \p operator/ .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _lhs_BigNumTrait The trait used by the other big number on
	 *                          left hand side.
	 * @tparam _rhs_BigNumTrait The trait used by the other big number on
	 *                          right hand side.
	 * @param lhs The value on left hand side.
	 * @param rhs The value on right hand side.
	 * @return BigNum The result of calculation, a new Big Number object.
	 */
	template<typename _lhs_BigNumTrait, typename _rhs_BigNumTrait,
		enable_if_t<std::is_same<typename _lhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0,
		enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
	inline BigNum operator/(const BigNumberBase<_lhs_BigNumTrait>& lhs, const BigNumberBase<_rhs_BigNumTrait>& rhs)
	{
		lhs.NullCheck();
		rhs.NullCheck();

		BigNum res;

		MBEDTLSCPP_MAKE_C_FUNC_CALL(::operator/_lhsBigNum-_rhsBigNum, mbedtls_mpi_div_mpi, res.Get(), nullptr, lhs.Get(), rhs.Get());

		return res;
	}

	/**
	 * @brief Overloading \p operator% .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _lhs_BigNumTrait The trait used by the other big number on
	 *                          left hand side.
	 * @tparam _rhs_BigNumTrait The trait used by the other big number on
	 *                          right hand side.
	 * @param lhs The value on left hand side.
	 * @param rhs The value on right hand side.
	 * @return BigNum The result of calculation, a new Big Number object.
	 */
	template<typename _lhs_BigNumTrait, typename _rhs_BigNumTrait,
		enable_if_t<std::is_same<typename _lhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0,
		enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
	inline BigNum operator%(const BigNumberBase<_lhs_BigNumTrait>& lhs, const BigNumberBase<_rhs_BigNumTrait>& rhs)
	{
		lhs.NullCheck();
		rhs.NullCheck();

		BigNum res;

		MBEDTLSCPP_MAKE_C_FUNC_CALL(::operator%_lhsBigNum-_rhsBigNum, mbedtls_mpi_div_mpi, nullptr, res.Get(), lhs.Get(), rhs.Get());

		return res;
	}

	/**
	 * @brief Calculate modulo of two big numbers.
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _lhs_BigNumTrait The trait used by the other big number on
	 *                          left hand side.
	 * @tparam _rhs_BigNumTrait The trait used by the other big number on
	 *                          right hand side.
	 * @param lhs The value on left hand side.
	 * @param rhs The value on right hand side.
	 * @return BigNum The result of calculation, a new Big Number object.
	 */
	template<typename _lhs_BigNumTrait, typename _rhs_BigNumTrait,
		enable_if_t<std::is_same<typename _lhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0,
		enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
	inline BigNum Mod(const BigNumberBase<_lhs_BigNumTrait>& lhs, const BigNumberBase<_rhs_BigNumTrait>& rhs)
	{
		lhs.NullCheck();
		rhs.NullCheck();

		BigNum res;

		MBEDTLSCPP_MAKE_C_FUNC_CALL(::Mod_lhsBigNum-_rhsBigNum, mbedtls_mpi_mod_mpi, res.Get(), lhs.Get(), rhs.Get());

		return res;
	}

	/**
	 * @brief Overloading \p operator+ .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _lhs_BigNumTrait The trait used by the other big number on
	 *                          left hand side.
	 * @tparam _rhs_ValType     The type of the native integral number on
	 *                          right hand side.
	 * @param lhs The value on left hand side.
	 * @param rhs The value on right hand side.
	 * @return BigNum The result of calculation, a new Big Number object.
	 */
	template<typename _lhs_BigNumTrait, typename _rhs_ValType,
		enable_if_t<std::is_same<typename _lhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0,
		enable_if_t<(std::is_integral<_rhs_ValType>::value && std::is_signed<_rhs_ValType>::value && sizeof(_rhs_ValType) <= sizeof(mbedtls_mpi_sint)) ||
		(std::is_integral<_rhs_ValType>::value && std::is_unsigned<_rhs_ValType>::value && sizeof(_rhs_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0>
	inline BigNum operator+(const BigNumberBase<_lhs_BigNumTrait>& lhs, _rhs_ValType rhs)
	{
		const mbedtls_mpi_sint rhsVal = static_cast<mbedtls_mpi_sint>(rhs);
		lhs.NullCheck();

		BigNum res;

		MBEDTLSCPP_MAKE_C_FUNC_CALL(::operator+_lhsBigNum-_rhsInt, mbedtls_mpi_add_int, res.Get(), lhs.Get(), rhsVal);

		return res;
	}

	/**
	 * @brief Overloading \p operator- .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _lhs_BigNumTrait The trait used by the other big number on
	 *                          left hand side.
	 * @tparam _rhs_ValType     The type of the native integral number on
	 *                          right hand side.
	 * @param lhs The value on left hand side.
	 * @param rhs The value on right hand side.
	 * @return BigNum The result of calculation, a new Big Number object.
	 */
	template<typename _lhs_BigNumTrait, typename _rhs_ValType,
		enable_if_t<std::is_same<typename _lhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0,
		enable_if_t<(std::is_integral<_rhs_ValType>::value && std::is_signed<_rhs_ValType>::value && sizeof(_rhs_ValType) <= sizeof(mbedtls_mpi_sint)) ||
		(std::is_integral<_rhs_ValType>::value && std::is_unsigned<_rhs_ValType>::value && sizeof(_rhs_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0>
	inline BigNum operator-(const BigNumberBase<_lhs_BigNumTrait>& lhs, _rhs_ValType rhs)
	{
		const mbedtls_mpi_sint rhsVal = static_cast<mbedtls_mpi_sint>(rhs);
		lhs.NullCheck();

		BigNum res;

		MBEDTLSCPP_MAKE_C_FUNC_CALL(::operator-_lhsBigNum-_rhsInt, mbedtls_mpi_sub_int, res.Get(), lhs.Get(), rhsVal);

		return res;
	}

	/**
	 * @brief Overloading \p operator* .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _lhs_BigNumTrait The trait used by the other big number on
	 *                          left hand side.
	 * @tparam _rhs_ValType     The type of the native integral number on
	 *                          right hand side.
	 * @param lhs The value on left hand side.
	 * @param rhs The value on right hand side.
	 * @return BigNum The result of calculation, a new Big Number object.
	 */
	template<typename _lhs_BigNumTrait, typename _rhs_ValType,
		enable_if_t<std::is_same<typename _lhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0,
		enable_if_t<(std::is_integral<_rhs_ValType>::value && std::is_signed<_rhs_ValType>::value && sizeof(_rhs_ValType) <= sizeof(mbedtls_mpi_uint)) ||
		(std::is_integral<_rhs_ValType>::value && std::is_unsigned<_rhs_ValType>::value && sizeof(_rhs_ValType) <= sizeof(mbedtls_mpi_uint)), int> = 0>
	inline BigNum operator*(const BigNumberBase<_lhs_BigNumTrait>& lhs, _rhs_ValType rhs)
	{
		const bool isPos = rhs >= 0;
		const mbedtls_mpi_uint rhsVal = static_cast<mbedtls_mpi_uint>(isPos ? rhs : -rhs );
		lhs.NullCheck();

		BigNum res;

		MBEDTLSCPP_MAKE_C_FUNC_CALL(::operator*_lhsBigNum-_rhsInt, mbedtls_mpi_mul_int, res.Get(), lhs.Get(), rhsVal);

		if(!isPos)
		{
			res.FlipSign();
		}

		return res;
	}

	/**
	 * @brief Overloading \p operator/ .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _lhs_BigNumTrait The trait used by the other big number on
	 *                          left hand side.
	 * @tparam _rhs_ValType     The type of the native integral number on
	 *                          right hand side.
	 * @param lhs The value on left hand side.
	 * @param rhs The value on right hand side.
	 * @return BigNum The result of calculation, a new Big Number object.
	 */
	template<typename _lhs_BigNumTrait, typename _rhs_ValType,
		enable_if_t<std::is_same<typename _lhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0,
		enable_if_t<(std::is_integral<_rhs_ValType>::value && std::is_signed<_rhs_ValType>::value && sizeof(_rhs_ValType) <= sizeof(mbedtls_mpi_sint)) ||
		(std::is_integral<_rhs_ValType>::value && std::is_unsigned<_rhs_ValType>::value && sizeof(_rhs_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0>
	inline BigNum operator/(const BigNumberBase<_lhs_BigNumTrait>& lhs, _rhs_ValType rhs)
	{
		const mbedtls_mpi_sint rhsVal = static_cast<mbedtls_mpi_sint>(rhs);
		lhs.NullCheck();

		BigNum res;

		MBEDTLSCPP_MAKE_C_FUNC_CALL(::operator/_lhsBigNum-_rhsInt, mbedtls_mpi_div_int, res.Get(), nullptr, lhs.Get(), rhsVal);

		return res;
	}

	/**
	 * @brief Overloading \p operator% .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _lhs_BigNumTrait The trait used by the other big number on
	 *                          left hand side.
	 * @tparam _rhs_ValType     The type of the native integral number on
	 *                          right hand side.
	 * @param lhs The value on left hand side.
	 * @param rhs The value on right hand side.
	 * @return BigNum The result of calculation, a new Big Number object.
	 */
	template<typename _lhs_BigNumTrait, typename _rhs_ValType,
		enable_if_t<std::is_same<typename _lhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0,
		enable_if_t<(std::is_integral<_rhs_ValType>::value && std::is_signed<_rhs_ValType>::value && sizeof(_rhs_ValType) <= sizeof(mbedtls_mpi_sint)) ||
		(std::is_integral<_rhs_ValType>::value && std::is_unsigned<_rhs_ValType>::value && sizeof(_rhs_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0>
	inline BigNum operator%(const BigNumberBase<_lhs_BigNumTrait>& lhs, _rhs_ValType rhs)
	{
		const mbedtls_mpi_sint rhsVal = static_cast<mbedtls_mpi_sint>(rhs);
		lhs.NullCheck();

		BigNum res;

		MBEDTLSCPP_MAKE_C_FUNC_CALL(::operator%_lhsBigNum-_rhsInt, mbedtls_mpi_div_int, nullptr, res.Get(), lhs.Get(), rhsVal);

		return res;
	}

	/**
	 * @brief Overloading \p operator+ .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _lhs_ValType     The type of the native integral number on
	 *                          left hand side.
	 * @tparam _rhs_BigNumTrait The trait used by the other big number on
	 *                          right hand side.
	 * @param lhs The value on left hand side.
	 * @param rhs The value on right hand side.
	 * @return BigNum The result of calculation, a new Big Number object.
	 */
	template<typename _rhs_BigNumTrait, typename _lhs_ValType,
		enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0,
		enable_if_t<(std::is_integral<_lhs_ValType>::value && std::is_signed<_lhs_ValType>::value && sizeof(_lhs_ValType) <= sizeof(mbedtls_mpi_sint)) ||
		(std::is_integral<_lhs_ValType>::value && std::is_unsigned<_lhs_ValType>::value && sizeof(_lhs_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0>
	inline BigNum operator+(_lhs_ValType lhs, const BigNumberBase<_rhs_BigNumTrait>& rhs)
	{
		return rhs + lhs;
	}

	/**
	 * @brief Overloading \p operator- .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _lhs_ValType     The type of the native integral number on
	 *                          left hand side.
	 * @tparam _rhs_BigNumTrait The trait used by the other big number on
	 *                          right hand side.
	 * @param lhs The value on left hand side.
	 * @param rhs The value on right hand side.
	 * @return BigNum The result of calculation, a new Big Number object.
	 */
	template<typename _rhs_BigNumTrait, typename _lhs_ValType,
		enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0,
		enable_if_t<(std::is_integral<_lhs_ValType>::value && std::is_signed<_lhs_ValType>::value && sizeof(_lhs_ValType) <= sizeof(mbedtls_mpi_sint)) ||
		(std::is_integral<_lhs_ValType>::value && std::is_unsigned<_lhs_ValType>::value && sizeof(_lhs_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0>
	inline BigNum operator-(_lhs_ValType lhs, const BigNumberBase<_rhs_BigNumTrait>& rhs)
	{
		return lhs + (-rhs);
	}

	/**
	 * @brief Overloading \p operator* .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _lhs_ValType     The type of the native integral number on
	 *                          left hand side.
	 * @tparam _rhs_BigNumTrait The trait used by the other big number on
	 *                          right hand side.
	 * @param lhs The value on left hand side.
	 * @param rhs The value on right hand side.
	 * @return BigNum The result of calculation, a new Big Number object.
	 */
	template<typename _rhs_BigNumTrait, typename _lhs_ValType,
		enable_if_t<std::is_same<typename _rhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0,
		enable_if_t<(std::is_integral<_lhs_ValType>::value && std::is_signed<_lhs_ValType>::value && sizeof(_lhs_ValType) <= sizeof(mbedtls_mpi_sint)) ||
		(std::is_integral<_lhs_ValType>::value && std::is_unsigned<_lhs_ValType>::value && sizeof(_lhs_ValType) < sizeof(mbedtls_mpi_sint)), int> = 0>
	inline BigNum operator*(_lhs_ValType lhs, const BigNumberBase<_rhs_BigNumTrait>& rhs)
	{
		return rhs * lhs;
	}

	/**
	 * @brief Overloading \p operator<< .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _lhs_BigNumTrait The trait used by the other big number on
	 *                          left hand side.
	 * @param lhs The value on left hand side.
	 * @param rhs The value on right hand side.
	 * @return BigNum The result of calculation, a new Big Number object.
	 */
	template<typename _lhs_BigNumTrait,
		enable_if_t<std::is_same<typename _lhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
	inline BigNum operator<<(const BigNumberBase<_lhs_BigNumTrait>& lhs, size_t rhs)
	{
		BigNum res(lhs);
		res.operator<<=(rhs);
		return res;
	}

	/**
	 * @brief Overloading \p operator>> .
	 *
	 * @exception InvalidObjectException Thrown when one or more given objects are
	 *                                   holding a null pointer for the C mbed TLS
	 *                                   object.
	 * @exception mbedTLSRuntimeError    Thrown when mbed TLS C function call failed.
	 * @tparam _lhs_BigNumTrait The trait used by the other big number on
	 *                          left hand side.
	 * @param lhs The value on left hand side.
	 * @param rhs The value on right hand side.
	 * @return BigNum The result of calculation, a new Big Number object.
	 */
	template<typename _lhs_BigNumTrait,
		enable_if_t<std::is_same<typename _lhs_BigNumTrait::CObjType, mbedtls_mpi>::value, int> = 0>
	inline BigNum operator>>(const BigNumberBase<_lhs_BigNumTrait>& lhs, size_t rhs)
	{
		BigNum res(lhs);
		res.operator>>=(rhs);
		return res;
	}
}
