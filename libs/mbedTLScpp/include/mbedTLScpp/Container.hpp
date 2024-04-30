#pragma once

#include <type_traits>
#include <array>
#include <vector>
#include <stdexcept>

#include <cstddef> //size_t

#include "Common.hpp"
#include "SecretArray.hpp"
#include "SecretVector.hpp"
#include "SecretString.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	/**
	 * @brief Base class for non-contiguous container type. It also could be a
	 *        type that we don't know much about its property.
	 *        (i.e., default type)
	 *
	 */
	struct NonContiguousCtnType
	{
		/**
		 * @brief Is the container contiguous?
		 *
		 */
		static constexpr bool sk_isCtnCont = false;
	};

	/**
	 * @brief Base class for contiguous container type.
	 *
	 */
	struct ContiguousCtnType
	{
		/**
		 * @brief Is the container contiguous?
		 *
		 */
		static constexpr bool sk_isCtnCont = true;
	};

	/**
	 * @brief A struct used to represent a C style dynamic (runtime allocated)
	 *        array (e.g., data = malloc(count * sizeof(_ValType));).
	 *
	 * @tparam _ValType The type of values stored in the array.
	 */
	template<typename _ValType>
	struct CDynArray
	{
		/**
		 * @brief The pointer to the begining of the array
		 *
		 */
		_ValType* m_data;

		/**
		 * @brief Number of items that the array can hold
		 *
		 */
		size_t m_count;
	};

	/**
	 * @brief A global static constant variable that represents an empty container.
	 *
	 */
	constexpr CDynArray<uint8_t> gsk_emptyCtn { nullptr, 0 };

	/**
	 * @brief Base class for static (and contiguous) container types.
	 *
	 * @tparam _ValType     The type of values stored in the container.
	 * @tparam _ArrayLength The number of items stored in the container.
	 */
	template<class _ValType, size_t _ArrayLength>
	struct StaticCtnType : ContiguousCtnType
	{
		/**
		 * @brief Is the container size and allocation static?
		 *
		 */
		static constexpr bool sk_isCtnStatic = true;

		/**
		 * @brief Number of items in the container.
		 *
		 */
		static constexpr size_t sk_itemCount = _ArrayLength;

		/**
		 * @brief Size (in bytes) of a *single* value stored in the container.
		 *
		 */
		static constexpr size_t sk_valSize = sizeof(_ValType);

		/**
		 * @brief The size (in bytes) of the entire container, which equals to the
		 *        number of items times the size of a single value.
		 *
		 */
		static constexpr size_t sk_ctnSize = sk_valSize * sk_itemCount;

		/**
		 * @brief Type of the values stored in the container.
		 *
		 */
		typedef _ValType ValType;
	};

	/**
	 * @brief Base class for dynamic (i.e., size can change at runtime) and
	 *        contiguous container types.
	 *
	 * @tparam _ValType The type of values stored in the container.
	 */
	template<class _ValType>
	struct DynCtnType : ContiguousCtnType
	{
		/**
		 * @brief Is the container size and allocation static?
		 *
		 */
		static constexpr bool sk_isCtnStatic = false;

		/**
		 * @brief Size (in bytes) of a *single* value stored in the container.
		 *
		 */
		static constexpr size_t sk_valSize = sizeof(_ValType);

		/**
		 * @brief Type of the values stored in the container.
		 *
		 */
		typedef _ValType ValType;
	};


	/**
	 * @brief The default CtnType. It is a child type of non-contiguous
	 *        Ctn type.
	 *
	 * @tparam ContainerType Type of the container.
	 */
	template<typename ContainerType>
	struct CtnType : NonContiguousCtnType
	{};

	/**
	 * @brief The CtnType for C-style array. It is a child type of
	 *        static Ctn Type.
	 *
	 * @tparam _ValType     Type of the value stored in the container.
	 * @tparam _ArrayLength Length of the array (i.e., number of items).
	 */
	template<typename _ValType, size_t _ArrayLength>
	struct CtnType<_ValType[_ArrayLength]> : StaticCtnType<_ValType, _ArrayLength>
	{
		static_assert(_ArrayLength > 0, "The length of the array should be at least 1.");

		/**
		 * @brief Get the count of the provided container
		 *        (i.e., number of items stored in the container).
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return constexpr size_t the count of the container.
		 */
		static constexpr size_t GetItemCount(const _ValType(&/*v*/)[_ArrayLength]) noexcept
		{
			return StaticCtnType<_ValType, _ArrayLength>::sk_itemCount;
		}

		/**
		 * @brief Get the total container size in bytes (i.e., count * val_size).
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return constexpr size_t the total container size in bytes
		 */
		static constexpr size_t GetCtnSize(
			const _ValType(&/*v*/)[_ArrayLength]
		) noexcept
		{
			return StaticCtnType<_ValType, _ArrayLength>::sk_ctnSize;
		}

		/**
		 * @brief Get the const-pointer to the first available memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return const void* the pointer to the first available memory location.
		 */
		static const void* GetPtr(const _ValType(&v)[_ArrayLength]) noexcept
		{
			return &v[0];
		}

		/**
		 * @brief Get the const-byte-pointer to the requested memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return const uint8_t* the byte-pointer to the first available memory location.
		 */
		static const uint8_t* GetBytePtr(const _ValType(&v)[_ArrayLength], size_t offsetInByte = 0) noexcept
		{
			return static_cast<const uint8_t*>(GetPtr(v)) + offsetInByte;
		}

		/**
		 * @brief Get the const-pointer to the requested memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return const void* the pointer to the requested memory location.
		 */
		static const void* GetPtr(const _ValType(&v)[_ArrayLength], size_t offsetInByte) noexcept
		{
			return GetBytePtr(v, offsetInByte);
		}

		/**
		 * @brief Get the pointer to the first available memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return void* the pointer to the first available memory location.
		 */
		static void* GetPtr(_ValType(&v)[_ArrayLength]) noexcept
		{
			return &v[0];
		}

		/**
		 * @brief Get the byte-pointer to the requested memory location in the
		 *        provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return uint8_t* the byte-pointer to the requested memory location.
		 */
		static uint8_t* GetBytePtr(_ValType(&v)[_ArrayLength], size_t offsetInByte = 0) noexcept
		{
			return static_cast<uint8_t*>(GetPtr(v)) + offsetInByte;
		}

		/**
		 * @brief Get the pointer to the requested memory location in the
		 *        provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return void* the byte-pointer to the requested memory location.
		 */
		static void* GetPtr(_ValType(&v)[_ArrayLength], size_t offsetInByte) noexcept
		{
			return GetBytePtr(v, offsetInByte);
		}
	};

	/**
	 * @brief The CtnType for C++-style array. It is a child type of
	 *        static Ctn Type.
	 *
	 * @tparam _ValType     Type of the value stored in the container.
	 * @tparam _ArrayLength Length of the array (i.e., number of items).
	 */
	template<typename _ValType, size_t _ArrayLength>
	struct CtnType<std::array<_ValType, _ArrayLength> > : StaticCtnType<_ValType, _ArrayLength>
	{
		static_assert(_ArrayLength > 0, "The length of the array should be at least 1.");

		/**
		 * @brief Get the count of the provided container
		 *        (i.e., number of items stored in the container).
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return constexpr size_t the count of the container.
		 */
		static constexpr size_t GetItemCount(const std::array<_ValType, _ArrayLength>& /*v*/) noexcept
		{
			return StaticCtnType<_ValType, _ArrayLength>::sk_itemCount;
		}

		/**
		 * @brief Get the total container size in bytes (i.e., count * val_size).
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return constexpr size_t the total container size in bytes
		 */
		static constexpr size_t GetCtnSize(const std::array<_ValType, _ArrayLength>& /*v*/) noexcept
		{
			return StaticCtnType<_ValType, _ArrayLength>::sk_ctnSize;
		}

		/**
		 * @brief Get the const-pointer to the first available memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return const void* the pointer to the first available memory location.
		 */
		static const void* GetPtr(const std::array<_ValType, _ArrayLength>& v) noexcept
		{
			return v.data();
		}

		/**
		 * @brief Get the const-byte-pointer to the requested memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return const uint8_t* the byte-pointer to the first available memory location.
		 */
		static const uint8_t* GetBytePtr(const std::array<_ValType, _ArrayLength>& v, size_t offsetInByte = 0) noexcept
		{
			return static_cast<const uint8_t*>(GetPtr(v)) + offsetInByte;
		}

		/**
		 * @brief Get the const-pointer to the requested memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return const void* the pointer to the requested memory location.
		 */
		static const void* GetPtr(const std::array<_ValType, _ArrayLength>& v, size_t offsetInByte) noexcept
		{
			return GetBytePtr(v, offsetInByte);
		}

		/**
		 * @brief Get the pointer to the first available memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return void* the pointer to the first available memory location.
		 */
		static void* GetPtr(std::array<_ValType, _ArrayLength>& v) noexcept
		{
			return v.data();
		}

		/**
		 * @brief Get the byte-pointer to the requested memory location in the
		 *        provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return uint8_t* the byte-pointer to the requested memory location.
		 */
		static uint8_t* GetBytePtr(std::array<_ValType, _ArrayLength>& v, size_t offsetInByte = 0) noexcept
		{
			return static_cast<uint8_t*>(GetPtr(v)) + offsetInByte;
		}

		/**
		 * @brief Get the pointer to the requested memory location in the
		 *        provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return void* the byte-pointer to the requested memory location.
		 */
		static void* GetPtr(std::array<_ValType, _ArrayLength>& v, size_t offsetInByte) noexcept
		{
			return GetBytePtr(v, offsetInByte);
		}
	};

	/**
	 * @brief The CtnType for std::vector<bool> container. It's not necessary a
	 *        contiguous container.
	 *        Details can be found at https://en.cppreference.com/w/cpp/container/vector_bool
	 *
	 */
	template<class _Alloc>
	struct CtnType<std::vector<bool, _Alloc> > : NonContiguousCtnType
	{};

	/**
	 * @brief The CtnType for std::vector<> other than std::vector<bool> container.
	 *        It's a dynamic contiguous container.
	 *
	 * @tparam _ValType Type of the value stored in the container.
	 */
	template<typename _ValType, class _Alloc>
	struct CtnType<std::vector<_ValType, _Alloc> > : DynCtnType<_ValType>
	{
		/**
		 * @brief Get the count of the provided container
		 *        (i.e., number of items stored in the container).
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return constexpr size_t the count of the container.
		 */
		static size_t GetItemCount(const std::vector<_ValType>& v) noexcept
		{
			return v.size(); //noexcept
		}

		/**
		 * @brief Get the total container size in bytes (i.e., count * val_size).
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return constexpr size_t the total container size in bytes
		 */
		static size_t GetCtnSize(const std::vector<_ValType>& v) noexcept
		{
			return DynCtnType<_ValType>::sk_valSize * GetItemCount(v); //noexcept
		}

		/**
		 * @brief Get the const-pointer to the first available memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return const void* the pointer to the first available memory location.
		 */
		static const void* GetPtr(const std::vector<_ValType>& v) noexcept
		{
			return v.data(); //noexcept
		}

		/**
		 * @brief Get the const-byte-pointer to the requested memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return const uint8_t* the byte-pointer to the first available memory location.
		 */
		static const uint8_t* GetBytePtr(const std::vector<_ValType>& v, size_t offsetInByte = 0) noexcept
		{
			return static_cast<const uint8_t*>(GetPtr(v)) + offsetInByte;
		}

		/**
		 * @brief Get the const-pointer to the requested memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return const void* the pointer to the requested memory location.
		 */
		static const void* GetPtr(const std::vector<_ValType>& v, size_t offsetInByte) noexcept
		{
			return GetBytePtr(v, offsetInByte);
		}

		/**
		 * @brief Get the pointer to the first available memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return void* the pointer to the first available memory location.
		 */
		static void* GetPtr(std::vector<_ValType>& v) noexcept
		{
			return v.data(); //noexcept
		}

		/**
		 * @brief Get the byte-pointer to the requested memory location in the
		 *        provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return uint8_t* the byte-pointer to the requested memory location.
		 */
		static uint8_t* GetBytePtr(std::vector<_ValType>& v, size_t offsetInByte = 0) noexcept
		{
			return static_cast<uint8_t*>(GetPtr(v)) + offsetInByte; //noexcept
		}

		/**
		 * @brief Get the pointer to the requested memory location in the
		 *        provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return void* the byte-pointer to the requested memory location.
		 */
		static void* GetPtr(std::vector<_ValType>& v, size_t offsetInByte) noexcept
		{
			return GetBytePtr(v, offsetInByte);
		}
	};

	/**
	 * @brief The CtnType for C style dynamic allocated array.
	 *        It's a dynamic contiguous container.
	 *
	 * @tparam _ValType Type of the value stored in the container.
	 */
	template<typename _ValType>
	struct CtnType<CDynArray<_ValType> > : DynCtnType<_ValType>
	{
		/**
		 * @brief Get the count of the provided container
		 *        (i.e., number of items stored in the container).
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return constexpr size_t the count of the container.
		 */
		static size_t GetItemCount(const CDynArray<_ValType>& v) noexcept
		{
			return v.m_count; //noexcept
		}

		/**
		 * @brief Get the total container size in bytes (i.e., count * val_size).
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return constexpr size_t the total container size in bytes
		 */
		static size_t GetCtnSize(const CDynArray<_ValType>& v) noexcept
		{
			return DynCtnType<_ValType>::sk_valSize * GetItemCount(v); //noexcept
		}

		/**
		 * @brief Get the const-pointer to the first available memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return const void* the pointer to the first available memory location.
		 */
		static const void* GetPtr(const CDynArray<_ValType>& v) noexcept
		{
			return v.m_data; //noexcept
		}

		/**
		 * @brief Get the const-byte-pointer to the requested memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return const uint8_t* the byte-pointer to the first available memory location.
		 */
		static const uint8_t* GetBytePtr(const CDynArray<_ValType>& v, size_t offsetInByte = 0) noexcept
		{
			return static_cast<const uint8_t*>(GetPtr(v)) + offsetInByte;
		}

		/**
		 * @brief Get the const-pointer to the requested memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return const void* the pointer to the requested memory location.
		 */
		static const void* GetPtr(const CDynArray<_ValType>& v, size_t offsetInByte) noexcept
		{
			return GetBytePtr(v, offsetInByte);
		}

		/**
		 * @brief Get the pointer to the first available memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return void* the pointer to the first available memory location.
		 */
		static void* GetPtr(CDynArray<_ValType>& v) noexcept
		{
			return v.m_data; //noexcept
		}

		/**
		 * @brief Get the byte-pointer to the requested memory location in the
		 *        provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return uint8_t* the byte-pointer to the requested memory location.
		 */
		static uint8_t* GetBytePtr(CDynArray<_ValType>& v, size_t offsetInByte = 0) noexcept
		{
			return static_cast<uint8_t*>(GetPtr(v)) + offsetInByte; //noexcept
		}

		/**
		 * @brief Get the pointer to the requested memory location in the
		 *        provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return void* the byte-pointer to the requested memory location.
		 */
		static void* GetPtr(CDynArray<_ValType>& v, size_t offsetInByte) noexcept
		{
			return GetBytePtr(v, offsetInByte);
		}
	};

	/**
	 * @brief The CtnType for std::basic_string<> container.
	 *        It's a dynamic contiguous container.
	 *
	 * @tparam _Elem   character type
	 * @tparam _Traits traits class specifying the operations on the character type
	 * @tparam _Alloc  Allocator type used to allocate internal storage
	 */
	template<class _Elem, class _Traits, class _Alloc>
	struct CtnType<std::basic_string<_Elem, _Traits, _Alloc> > : DynCtnType<typename std::basic_string<_Elem, _Traits, _Alloc>::value_type>
	{
		/**
		 * @brief Get the count of the provided container
		 *        (i.e., number of items stored in the container).
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return constexpr size_t the count of the container.
		 */
		static size_t GetItemCount(const std::basic_string<_Elem, _Traits, _Alloc>& v) noexcept
		{
			return v.size(); //noexcept
		}

		/**
		 * @brief Get the total container size in bytes (i.e., count * val_size).
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return constexpr size_t the total container size in bytes
		 */
		static size_t GetCtnSize(const std::basic_string<_Elem, _Traits, _Alloc>& v) noexcept
		{
			return DynCtnType<typename std::basic_string<_Elem, _Traits, _Alloc>::value_type>::sk_valSize * GetItemCount(v);
		}

		/**
		 * @brief Get the const-pointer to the first available memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return const void* the pointer to the first available memory location.
		 */
		static const void* GetPtr(const std::basic_string<_Elem, _Traits, _Alloc>& v) noexcept
		{
			return v.data(); //noexcept
		}

		/**
		 * @brief Get the const-byte-pointer to the requested memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return const uint8_t* the byte-pointer to the first available memory location.
		 */
		static const uint8_t* GetBytePtr(const std::basic_string<_Elem, _Traits, _Alloc>& v, size_t offsetInByte = 0) noexcept
		{
			return static_cast<const uint8_t*>(GetPtr(v)) + offsetInByte;
		}

		/**
		 * @brief Get the const-pointer to the requested memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return const void* the pointer to the requested memory location.
		 */
		static const void* GetPtr(const std::basic_string<_Elem, _Traits, _Alloc>& v, size_t offsetInByte) noexcept
		{
			return GetBytePtr(v, offsetInByte);
		}

		/**
		 * @brief Get the pointer to the first available memory location
		 *        in the provided container.
		 *
		 * @exception std::out_of_range Thrown if container has zero size.
		 * @param v The container.
		 * @return void* the pointer to the first available memory location.
		 */
		static void* GetPtr(std::basic_string<_Elem, _Traits, _Alloc>& v)
		{
			return &v[0];
		}

		/**
		 * @brief Get the byte-pointer to the requested memory location in the
		 *        provided container.
		 *
		 * @exception std::out_of_range Thrown if container has zero size.
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return uint8_t* the byte-pointer to the requested memory location.
		 */
		static uint8_t* GetBytePtr(std::basic_string<_Elem, _Traits, _Alloc>& v, size_t offsetInByte = 0)
		{
			return static_cast<uint8_t*>(GetPtr(v)) + offsetInByte;
		}

		/**
		 * @brief Get the pointer to the requested memory location in the
		 *        provided container.
		 *
		 * @exception std::out_of_range Thrown if container has zero size.
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return void* the byte-pointer to the requested memory location.
		 */
		static void* GetPtr(std::basic_string<_Elem, _Traits, _Alloc>& v, size_t offsetInByte)
		{
			return GetBytePtr(v, offsetInByte);
		}
	};

	/**
	 * @brief The CtnType for C++-style array. It is a child type of
	 *        static Ctn Type.
	 *
	 * @tparam _ValType     Type of the value stored in the container.
	 * @tparam _ArrayLength Length of the array (i.e., number of items).
	 */
	template<typename _ValType, size_t _ArrayLength>
	struct CtnType<SecretArray<_ValType, _ArrayLength> > : StaticCtnType<_ValType, _ArrayLength>
	{
		static_assert(_ArrayLength > 0, "The length of the array should be at least 1.");

		/**
		 * @brief Get the count of the provided container
		 *        (i.e., number of items stored in the container).
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return constexpr size_t the count of the container.
		 */
		static constexpr size_t GetItemCount(const SecretArray<_ValType, _ArrayLength>& /*v*/) noexcept
		{
			return StaticCtnType<_ValType, _ArrayLength>::sk_itemCount;
		}

		/**
		 * @brief Get the total container size in bytes (i.e., count * val_size).
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return constexpr size_t the total container size in bytes
		 */
		static constexpr size_t GetCtnSize(const SecretArray<_ValType, _ArrayLength>& /*v*/) noexcept
		{
			return StaticCtnType<_ValType, _ArrayLength>::sk_ctnSize;
		}

		/**
		 * @brief Get the const-pointer to the first available memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return const void* the pointer to the first available memory location.
		 */
		static const void* GetPtr(const SecretArray<_ValType, _ArrayLength>& v) noexcept
		{
			return v.Get().data();
		}

		/**
		 * @brief Get the const-byte-pointer to the requested memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return const uint8_t* the byte-pointer to the first available memory location.
		 */
		static const uint8_t* GetBytePtr(const SecretArray<_ValType, _ArrayLength>& v, size_t offsetInByte = 0) noexcept
		{
			return static_cast<const uint8_t*>(GetPtr(v)) + offsetInByte;
		}

		/**
		 * @brief Get the const-pointer to the requested memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return const void* the pointer to the requested memory location.
		 */
		static const void* GetPtr(const SecretArray<_ValType, _ArrayLength>& v, size_t offsetInByte) noexcept
		{
			return GetBytePtr(v, offsetInByte);
		}

		/**
		 * @brief Get the pointer to the first available memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return void* the pointer to the first available memory location.
		 */
		static void* GetPtr(SecretArray<_ValType, _ArrayLength>& v) noexcept
		{
			return v.Get().data();
		}

		/**
		 * @brief Get the byte-pointer to the requested memory location in the
		 *        provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return uint8_t* the byte-pointer to the requested memory location.
		 */
		static uint8_t* GetBytePtr(SecretArray<_ValType, _ArrayLength>& v, size_t offsetInByte = 0) noexcept
		{
			return static_cast<uint8_t*>(GetPtr(v)) + offsetInByte;
		}

		/**
		 * @brief Get the pointer to the requested memory location in the
		 *        provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return void* the byte-pointer to the requested memory location.
		 */
		static void* GetPtr(SecretArray<_ValType, _ArrayLength>& v, size_t offsetInByte) noexcept
		{
			return GetBytePtr(v, offsetInByte);
		}
	};

	/**
	 * @brief The CtnType for SecretVector<>.
	 *        It's a dynamic contiguous container.
	 *
	 * @tparam _ValType Type of the value stored in the container.
	 */
	template<typename _ValType, class _Alloc>
	struct CtnType<SecretVector<_ValType, _Alloc> > : DynCtnType<_ValType>
	{
		/**
		 * @brief Get the count of the provided container
		 *        (i.e., number of items stored in the container).
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return constexpr size_t the count of the container.
		 */
		static size_t GetItemCount(const SecretVector<_ValType>& v) noexcept
		{
			return v.size(); //noexcept
		}

		/**
		 * @brief Get the total container size in bytes (i.e., count * val_size).
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return constexpr size_t the total container size in bytes
		 */
		static size_t GetCtnSize(const SecretVector<_ValType>& v) noexcept
		{
			return DynCtnType<_ValType>::sk_valSize * GetItemCount(v); //noexcept
		}

		/**
		 * @brief Get the const-pointer to the first available memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return const void* the pointer to the first available memory location.
		 */
		static const void* GetPtr(const SecretVector<_ValType>& v) noexcept
		{
			return v.data(); //noexcept
		}

		/**
		 * @brief Get the const-byte-pointer to the requested memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return const uint8_t* the byte-pointer to the first available memory location.
		 */
		static const uint8_t* GetBytePtr(const SecretVector<_ValType>& v, size_t offsetInByte = 0) noexcept
		{
			return static_cast<const uint8_t*>(GetPtr(v)) + offsetInByte;
		}

		/**
		 * @brief Get the const-pointer to the requested memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return const void* the pointer to the requested memory location.
		 */
		static const void* GetPtr(const SecretVector<_ValType>& v, size_t offsetInByte) noexcept
		{
			return GetBytePtr(v, offsetInByte);
		}

		/**
		 * @brief Get the pointer to the first available memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return void* the pointer to the first available memory location.
		 */
		static void* GetPtr(SecretVector<_ValType>& v) noexcept
		{
			return v.data(); //noexcept
		}

		/**
		 * @brief Get the byte-pointer to the requested memory location in the
		 *        provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return uint8_t* the byte-pointer to the requested memory location.
		 */
		static uint8_t* GetBytePtr(SecretVector<_ValType>& v, size_t offsetInByte = 0) noexcept
		{
			return static_cast<uint8_t*>(GetPtr(v)) + offsetInByte; //noexcept
		}

		/**
		 * @brief Get the pointer to the requested memory location in the
		 *        provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return void* the byte-pointer to the requested memory location.
		 */
		static void* GetPtr(SecretVector<_ValType>& v, size_t offsetInByte) noexcept
		{
			return GetBytePtr(v, offsetInByte);
		}
	};

	/**
	 * @brief The CtnType for SecretBasicString<> container.
	 *        It's a dynamic contiguous container.
	 *
	 * @tparam _Elem   character type
	 * @tparam _Traits traits class specifying the operations on the character type
	 * @tparam _Alloc  Allocator type used to allocate internal storage
	 */
	template<class _Elem, class _Traits, class _Alloc>
	struct CtnType<SecretBasicString<_Elem, _Traits, _Alloc> > : DynCtnType<typename SecretBasicString<_Elem, _Traits, _Alloc>::value_type>
	{
		/**
		 * @brief Get the count of the provided container
		 *        (i.e., number of items stored in the container).
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return constexpr size_t the count of the container.
		 */
		static size_t GetItemCount(const SecretBasicString<_Elem, _Traits, _Alloc>& v) noexcept
		{
			return v.size(); //noexcept
		}

		/**
		 * @brief Get the total container size in bytes (i.e., count * val_size).
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return constexpr size_t the total container size in bytes
		 */
		static size_t GetCtnSize(const SecretBasicString<_Elem, _Traits, _Alloc>& v) noexcept
		{
			return DynCtnType<typename SecretBasicString<_Elem, _Traits, _Alloc>::value_type>::sk_valSize * GetItemCount(v);
		}

		/**
		 * @brief Get the const-pointer to the first available memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v The container.
		 * @return const void* the pointer to the first available memory location.
		 */
		static const void* GetPtr(const SecretBasicString<_Elem, _Traits, _Alloc>& v) noexcept
		{
			return v.data(); //noexcept
		}

		/**
		 * @brief Get the const-byte-pointer to the requested memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return const uint8_t* the byte-pointer to the first available memory location.
		 */
		static const uint8_t* GetBytePtr(const SecretBasicString<_Elem, _Traits, _Alloc>& v, size_t offsetInByte = 0) noexcept
		{
			return static_cast<const uint8_t*>(GetPtr(v)) + offsetInByte;
		}

		/**
		 * @brief Get the const-pointer to the requested memory location
		 *        in the provided container.
		 *
		 * @exception None No exception thrown
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return const void* the pointer to the requested memory location.
		 */
		static const void* GetPtr(const SecretBasicString<_Elem, _Traits, _Alloc>& v, size_t offsetInByte) noexcept
		{
			return GetBytePtr(v, offsetInByte);
		}

		/**
		 * @brief Get the pointer to the first available memory location
		 *        in the provided container.
		 *
		 * @exception std::out_of_range Thrown if container has zero size.
		 * @param v The container.
		 * @return void* the pointer to the first available memory location.
		 */
		static void* GetPtr(SecretBasicString<_Elem, _Traits, _Alloc>& v)
		{
			return &v[0];
		}

		/**
		 * @brief Get the byte-pointer to the requested memory location in the
		 *        provided container.
		 *
		 * @exception std::out_of_range Thrown if container has zero size.
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return uint8_t* the byte-pointer to the requested memory location.
		 */
		static uint8_t* GetBytePtr(SecretBasicString<_Elem, _Traits, _Alloc>& v, size_t offsetInByte = 0)
		{
			return static_cast<uint8_t*>(GetPtr(v)) + offsetInByte;
		}

		/**
		 * @brief Get the pointer to the requested memory location in the
		 *        provided container.
		 *
		 * @exception std::out_of_range Thrown if container has zero size.
		 * @param v            The container.
		 * @param offsetInByte Offset from the begining of the container, in bytes.
		 * @return void* the byte-pointer to the requested memory location.
		 */
		static void* GetPtr(SecretBasicString<_Elem, _Traits, _Alloc>& v, size_t offsetInByte)
		{
			return GetBytePtr(v, offsetInByte);
		}
	};

	template<typename _ContainerType>
	struct IsSecretContainer : std::false_type
	{};

	template<typename _ValType, size_t _Size>
	struct IsSecretContainer<SecretArray<_ValType, _Size> > : std::true_type
	{};

	template<typename _ValType>
	struct IsSecretContainer<SecretVector<_ValType, SecretAllocator<_ValType> > > : std::true_type
	{};

	template<typename _ValType, typename _CharTraits>
	struct IsSecretContainer<SecretBasicString<_ValType, _CharTraits, SecretAllocator<_ValType> > > : std::true_type
	{};

	/**
	 * @brief The contiguous Container Read-Only Reference struct that
	 *        stores the const reference to a existing container, for
	 *        read-only purpose.
	 *
	 * @tparam ContainerType Type of the container.
	 * @tparam Secrecy       Can the container store secret content?
	 */
	template<typename ContainerType,
		bool Secrecy = IsSecretContainer<ContainerType>::value,
		enable_if_t<CtnType<typename remove_cvref<ContainerType>::type>::sk_isCtnCont, int> = 0>
	struct ContCtnReadOnlyRef
	{
		/**
		 * @brief The pure type of the container, which means all "const" and "&"
		 *        specification have been removed.
		 *
		 */
		using PureContainerType = typename remove_cvref<ContainerType>::type;

		/**
		 * @brief Can the container store secret content?
		 *
		 */
		static constexpr bool sk_secrecy = Secrecy;

		/**
		 * @brief The const-reference to the container.
		 *
		 */
		const PureContainerType& m_ctn;

		/**
		 * @brief The offset (in Bytes, starts from the begining of the
		 *        container) for the begining of the memory region.
		 *
		 */
		const size_t m_beginOffset;

		/**
		 * @brief The offset (in Bytes, starts from the begining of the
		 *        container) for the end of the memory region.
		 *
		 */
		const size_t m_endOffset;

		/**
		 * @brief Construct a new Contiguous Container Read Only Reference object.
		 *
		 * @exception std::invalid_argument Thrown if endOffset < beginOffset.
		 * @exception std::out_of_range Thrown if endOffset is out of the size of the container.
		 * @param ctn         The const reference to the container.
		 * @param beginOffset The offset (in Bytes, starts from the begining of
		 *                    the container) for the begining of the memory region.
		 * @param endOffset   The offset (in Bytes, starts from the begining of
		 *                    the container) for the end of the memory region.
		 */
		ContCtnReadOnlyRef(const PureContainerType& ctn, size_t beginOffset, size_t endOffset) :
			m_ctn(ctn),
			m_beginOffset(beginOffset),
			m_endOffset(endOffset)
		{
			if(endOffset < beginOffset)
			{
				throw std::invalid_argument("The end of the range is smaller than the begining of the range.");
			}

			if(endOffset > CtnType<PureContainerType>::GetCtnSize(m_ctn))
			{
				throw std::out_of_range("The end of the range is outside of the container.");
			}
		}

		/**
		 * @brief Construct a new Contiguous Container Read Only Reference object.
		 *        NOTE: Call this constructor only if you already done the safety
		 *        checks on beginOffset and endOffset!
		 *
		 * @exception None No exception thrown
		 * @param ctn         The const reference to the container.
		 * @param beginOffset The offset (in Bytes, starts from the begining of
		 *                    the container) for the begining of the memory region.
		 * @param endOffset   The offset (in Bytes, starts from the begining of
		 *                    the container) for the end of the memory region.
		 */
		ContCtnReadOnlyRef(const PureContainerType& ctn, size_t beginOffset, size_t endOffset, NoSafeCheck) noexcept :
			m_ctn(ctn),
			m_beginOffset(beginOffset),
			m_endOffset(endOffset)
		{}

		/**
		 * @brief Construct a new Contiguous Container Read Only Reference object
		 *        that references the full range of the given container.
		 *
		 * @exception None No exception thrown
		 * @tparam _dummy_ContainerType The dummy template parameter used to
		 *                              enable the contructor if container is static.
		 * @param ctn The const reference to the container.
		 */
		template<typename _dummy_ContainerType = PureContainerType,
			enable_if_t<CtnType<_dummy_ContainerType>::sk_isCtnStatic, int> = 0>
		ContCtnReadOnlyRef(const PureContainerType& ctn) noexcept :
			m_ctn(ctn),
			m_beginOffset(0),
			m_endOffset(CtnType<PureContainerType>::sk_ctnSize)
		{}

		/**
		 * @brief Construct a new Contiguous Container Read Only Reference object
		 *        that references the full range of the given container.
		 *
		 * @exception None No exception thrown
		 * @tparam _dummy_ContainerType The dummy template parameter used to
		 *                              enable the contructor if container is not static.
		 * @param ctn The const reference to the container.
		 */
		template<typename _dummy_ContainerType = PureContainerType,
			enable_if_t<!CtnType<_dummy_ContainerType>::sk_isCtnStatic, int> = 0>
		ContCtnReadOnlyRef(const PureContainerType& ctn) noexcept :
			m_ctn(ctn),
			m_beginOffset(0),
			m_endOffset(CtnType<PureContainerType>::GetCtnSize(ctn))
		{}

		/**
		 * @brief Construct a new Contiguous Container Read Only Reference object
		 *        by copying the reference from an existing instance.
		 *
		 * @exception None No exception thrown
		 * @param rhs The existing instance.
		 */
		ContCtnReadOnlyRef(const ContCtnReadOnlyRef& rhs) noexcept :
			m_ctn(rhs.m_ctn),
			m_beginOffset(rhs.m_beginOffset),
			m_endOffset(rhs.m_endOffset)
		{}

		/**
		 * @brief Construct a new Contiguous Container Read Only Reference object
		 *        by copying the reference from an existing instance, and reference
		 *        a sub-range of that instance.
		 *
		 * @exception None No exception thrown
		 * @param rhs         The existing instance.
		 * @param beginOffset The offset (in Bytes, starts from the begining of
		 *                    the given reference) for the begining of the memory region.
		 * @param endOffset   The offset (in Bytes, starts from the begining of
		 *                    the given reference) for the end of the memory region.
		 */
		ContCtnReadOnlyRef(const ContCtnReadOnlyRef& rhs,
			size_t beginOffset, size_t endOffset,
			NoSafeCheck) noexcept :
			m_ctn(rhs.m_ctn),
			m_beginOffset(rhs.m_beginOffset + beginOffset),
			m_endOffset(rhs.m_beginOffset + endOffset)
		{}
		ContCtnReadOnlyRef(const ContCtnReadOnlyRef& rhs,
			size_t beginOffset,
			NoSafeCheck) noexcept :
			m_ctn(rhs.m_ctn),
			m_beginOffset(rhs.m_beginOffset + beginOffset),
			m_endOffset(rhs.m_endOffset)
		{}
		ContCtnReadOnlyRef(const ContCtnReadOnlyRef& rhs,
			size_t beginOffset, size_t endOffset) :
			ContCtnReadOnlyRef(rhs, beginOffset, endOffset, gsk_noSafeCheck)
		{
			if(endOffset < beginOffset)
			{
				throw std::invalid_argument("The end of the range is smaller than the begining of the range.");
			}

			if(endOffset > rhs.GetRegionSize())
			{
				throw std::out_of_range("The end of the range is outside of the container.");
			}
		}
		ContCtnReadOnlyRef(const ContCtnReadOnlyRef& rhs,
			size_t beginOffset) :
			ContCtnReadOnlyRef(rhs, beginOffset, gsk_noSafeCheck)
		{
			if(beginOffset > rhs.GetRegionSize())
			{
				throw std::out_of_range("The begining of the range is outside of the container.");
			}
		}

		// LCOV_EXCL_START
		virtual ~ContCtnReadOnlyRef() = default;
		// LCOV_EXCL_STOP

		ContCtnReadOnlyRef& operator=(const ContCtnReadOnlyRef& rhs) = delete;

		ContCtnReadOnlyRef& operator=(ContCtnReadOnlyRef&& rhs) = delete;

		// /**
		//  * @brief Get the item count of the container.
		//  *
		//  * @exception None No exception thrown
		//  * @return size_t count of items.
		//  */
		// size_t GetItemCount() const
		// {
		// 	return CtnType<PureContainerType>::GetItemCount(m_ctn);
		// }

		/**
		 * @brief Get the size (in bytes) of a *single* value stored in the container.
		 *
		 * @exception None No exception thrown
		 * @return size_t the size (in bytes) of a *single* value
		 */
		virtual size_t GetValSize() const noexcept
		{
			return CtnType<PureContainerType>::sk_valSize;
		}

		// /**
		//  * @brief Get the total container size in bytes (i.e., count * val_size).
		//  *
		//  * @exception None No exception thrown
		//  * @return size_t the total container size in bytes
		//  */
		// size_t GetCtnSize() const noexcept
		// {
		// 	return CtnType<PureContainerType>::GetCtnSize(m_ctn);
		// }

		/**
		 * @brief Get the size of the memory region
		 *
		 * @exception None No exception thrown
		 * @return size_t The size of the memory region
		 */
		virtual size_t GetRegionSize() const noexcept
		{
			return m_endOffset - m_beginOffset;
		}

		/**
		 * @brief Get the void pointer to the begining of the memory region.
		 *
		 * @exception None No exception thrown
		 * @return const void* The pointer to the begining of the memory region.
		 */
		virtual const void* BeginPtr() const noexcept
		{
			return CtnType<PureContainerType>::GetPtr(m_ctn, m_beginOffset);
		}

		/**
		 * @brief Get the byte pointer to the begining of the memory region.
		 *
		 * @exception None No exception thrown
		 * @return const byte The pointer to the begining of the memory region.
		 */
		virtual const uint8_t* BeginBytePtr() const noexcept
		{
			return CtnType<PureContainerType>::GetBytePtr(m_ctn, m_beginOffset);
		}

		/**
		 * @brief Get the void pointer to the end of the memory region.
		 *
		 * @exception None No exception thrown
		 * @return const void* The pointer to the end of the memory region.
		 */
		virtual const void* EndPtr() const noexcept
		{
			return CtnType<PureContainerType>::GetPtr(m_ctn, m_endOffset);
		}

		/**
		 * @brief Get the byte pointer to the end of the memory region.
		 *
		 * @exception None No exception thrown
		 * @return const uint8_t* The pointer to the end of the memory region.
		 */
		virtual const uint8_t* EndBytePtr() const noexcept
		{
			return CtnType<PureContainerType>::GetBytePtr(m_ctn, m_endOffset);
		}
	};

	template<typename _ContainerType>
	using NormalContCtnReadOnlyRef = ContCtnReadOnlyRef<_ContainerType, false>;

	template<typename _ContainerType>
	using SecretContCtnReadOnlyRef = ContCtnReadOnlyRef<_ContainerType, true>;

	/**
	 * @brief The contiguous Container Read-Only Static Reference struct that
	 *        stores the const reference to a existing container, for
	 *        read-only purpose, and both the container size and the size of
	 *        referenced region is static.
	 *
	 * @tparam _ContainerType Type of the container.
	 * @tparam _SizeInBytes   The size of the referenced region
	 * @tparam _Secrecy       Can the container store secret content?
	 */
	template<typename _ContainerType,
			 size_t   _SizeInBytes,
			 bool     _Secrecy = IsSecretContainer<_ContainerType>::value,
			 enable_if_t<
			 	CtnType<typename remove_cvref<_ContainerType>::type>::sk_isCtnCont &&
				CtnType<typename remove_cvref<_ContainerType>::type>::sk_isCtnStatic,
			 int> = 0>
	struct ContCtnReadOnlyStRef : ContCtnReadOnlyRef<_ContainerType, _Secrecy>
	{

		/**
		 * @brief The base class type.
		 *
		 */
		using _Base = ContCtnReadOnlyRef<_ContainerType, _Secrecy>;

		/**
		 * @brief The pure type of the container, which means all "const" and "&"
		 *        specification have been removed.
		 *
		 */
		using PureContainerType = typename _Base::PureContainerType;

		/**
		 * @brief Construct a new Contiguous Container Read Only Static Reference object.
		 *
		 * @exception std::out_of_range Thrown if endOffset is out of the size of the container.
		 * @param ctn         The const reference to the container.
		 * @param beginOffset The offset (in Bytes, starts from the begining of
		 *                    the container) for the begining of the memory region.
		 */
		ContCtnReadOnlyStRef(const PureContainerType& ctn, size_t beginOffset) :
			_Base::ContCtnReadOnlyRef(ctn, beginOffset, beginOffset + _SizeInBytes, gsk_noSafeCheck)
		{
			if(_Base::m_endOffset > CtnType<PureContainerType>::sk_ctnSize)
			{
				throw std::out_of_range("The end of the range is outside of the container.");
			}
		}

		/**
		 * @brief Construct a new Contiguous Container Read Only Static Reference object.
		 *        NOTE: Call this constructor only if you already done the safety
		 *        checks on beginOffset and endOffset!
		 *
		 * @exception None No exception thrown
		 * @param ctn         The const reference to the container.
		 * @param beginOffset The offset (in Bytes, starts from the begining of
		 *                    the container) for the begining of the memory region.
		 */
		ContCtnReadOnlyStRef(const PureContainerType& ctn, size_t beginOffset, NoSafeCheck) noexcept :
			_Base::ContCtnReadOnlyRef(ctn, beginOffset, beginOffset + _SizeInBytes, gsk_noSafeCheck)
		{}

		/**
		 * @brief Construct a new Contiguous Container Read Only Static Reference object
		 *        that references the full range of the given container.
		 *
		 * @exception None No exception thrown
		 * @tparam _dummy_ContainerType The dummy template parameter used to
		 *                              enable the contructor if container's size
		 *                              is equal to the size of referenced region.
		 * @param ctn The const reference to the container.
		 */
		template<
			typename _dummy_ContainerType = PureContainerType,
			enable_if_t<
				CtnType<_dummy_ContainerType>::sk_ctnSize == _SizeInBytes,
			int> = 0>
		ContCtnReadOnlyStRef(const PureContainerType& ctn) :
			ContCtnReadOnlyStRef(ctn, 0, gsk_noSafeCheck)
		{}

		/**
		 * @brief Construct a new Contiguous Container Read Only Static Reference object
		 *        by copying the reference from an existing instance.
		 *
		 * @exception None No exception thrown
		 * @param rhs The existing instance.
		 */
		ContCtnReadOnlyStRef(const ContCtnReadOnlyStRef& rhs) noexcept :
			_Base::ContCtnReadOnlyRef(rhs)
		{}

		/**
		 * @brief Construct a new Contiguous Container Read Only Static Reference object
		 *        by copying the reference from an existing instance, and reference
		 *        a sub-range of that instance.
		 *
		 * @exception None No exception thrown
		 * @param rhs         The existing instance.
		 * @param beginOffset The offset (in Bytes, starts from the begining of
		 *                    the given reference) for the begining of the memory region.
		 * @param endOffset   The offset (in Bytes, starts from the begining of
		 *                    the given reference) for the end of the memory region.
		 */
		template<size_t _rhs_CtnSize>
		ContCtnReadOnlyStRef(const ContCtnReadOnlyStRef<_ContainerType, _rhs_CtnSize>& rhs,
			size_t beginOffset,
			size_t endOffset,
			NoSafeCheck) noexcept :
			_Base::ContCtnReadOnlyRef(rhs, beginOffset, endOffset, gsk_noSafeCheck)
		{}
		template<size_t _rhs_CtnSize>
		ContCtnReadOnlyStRef(const ContCtnReadOnlyStRef<_ContainerType, _rhs_CtnSize>& rhs,
			size_t beginOffset,
			NoSafeCheck) noexcept :
			_Base::ContCtnReadOnlyRef(rhs, beginOffset, gsk_noSafeCheck)
		{}

		// LCOV_EXCL_START
		virtual ~ContCtnReadOnlyStRef() = default;
		// LCOV_EXCL_STOP

		ContCtnReadOnlyStRef& operator=(const ContCtnReadOnlyStRef& rhs) = delete;

		ContCtnReadOnlyStRef& operator=(ContCtnReadOnlyStRef&& rhs) = delete;

		/**
		 * @brief Get the size of the memory region
		 *
		 * @exception None No exception thrown
		 * @return size_t The size of the memory region
		 */
		virtual size_t GetRegionSize() const noexcept
		{
			return _SizeInBytes;
		}
	};

	/**
	 * @brief Helper function to construct the ContCtnReadOnlyRef struct easily for
	 *        A) the entire range of the container
	 *        B) containers with static size
	 *
	 * @exception None No exception thrown
	 * @tparam ContainerType Type of the container, which will be inferred from
	 *                       the giving parameter.
	 * @param ctn The const-reference to the container.
	 * @return ContCtnReadOnlyRef<ContainerType, Secrecy> The constructed ContCtnReadOnlyRef struct
	 */
	template<// automated parts:
			 typename ContainerType,
			 enable_if_t<
			 	CtnType<ContainerType>::sk_isCtnCont &&
				!CtnType<ContainerType>::sk_isCtnStatic,
			 int> = 0>
	inline ContCtnReadOnlyRef<ContainerType>
		CtnFullR(const ContainerType& ctn) noexcept
	{
		return ContCtnReadOnlyRef<ContainerType>(ctn);
	}

	template<// automated parts:
			 typename ContainerType,
			 enable_if_t<
			 	CtnType<ContainerType>::sk_isCtnCont &&
				CtnType<ContainerType>::sk_isCtnStatic,
			 int> = 0>
	inline ContCtnReadOnlyStRef<ContainerType, CtnType<ContainerType>::sk_ctnSize>
		CtnFullR(const ContainerType& ctn) noexcept
	{
		return ContCtnReadOnlyStRef<ContainerType, CtnType<ContainerType>::sk_ctnSize>(ctn);
	}

	template<// automated parts:
			 typename ContainerType>
	inline ContCtnReadOnlyRef<ContainerType>
		CtnFullR(const ContCtnReadOnlyRef<ContainerType>& ctn) noexcept
	{
		return ctn;
	}

	template<// automated parts:
			 typename ContainerType,
			 size_t ContainerSize>
	inline ContCtnReadOnlyStRef<ContainerType, ContainerSize>
		CtnFullR(const ContCtnReadOnlyStRef<ContainerType, ContainerSize>& ctn) noexcept
	{
		return ctn;
	}




	// CtnByteRgR<x, y>(sta)
	/**
	 * @brief Helper function to construct the ContCtnReadOnlyRef struct easily for
	 *        A) a specific range of the container
	 *        B) containers with static size
	 *        C) range is specified statically
	 *
	 * @exception None No exception thrown
	 * @tparam beginOffset   The left end of the range (inclusive, in bytes).
	 * @tparam endOffset     The right end of the range (exclusive, in bytes).
	 * @tparam ContainerType Type of the container, which will be inferred from
	 *                       the giving parameter.
	 * @param ctn The const-reference to the container.
	 * @return ContCtnReadOnlyRef<ContainerType, Secrecy> The constructed ContCtnReadOnlyRef struct
	 */
	template<size_t beginOffset, size_t endOffset,
			 typename ContainerType,
			 // automated parts:
			 enable_if_t<
			 	CtnType<ContainerType>::sk_isCtnCont &&
				CtnType<ContainerType>::sk_isCtnStatic,
			 int> = 0,
			 size_t regStSize = endOffset - beginOffset>
	inline ContCtnReadOnlyStRef<ContainerType, regStSize>
		CtnByteRgR(const ContainerType& ctn) noexcept
	{
		static_assert(beginOffset <= endOffset, "The begining of the range should be smaller than or equal to the end of the range.");
		static_assert(endOffset <= CtnType<ContainerType>::sk_ctnSize, "The end of the range is outside of the container.");

		return ContCtnReadOnlyStRef<ContainerType, regStSize>(ctn, beginOffset, gsk_noSafeCheck);
	}

	template<size_t beginOffset, size_t endOffset,
			 // automated parts:
			 typename ContainerType,
			 size_t CtnRegSize,
			 size_t nRegStSize = endOffset - beginOffset>
	inline ContCtnReadOnlyStRef<ContainerType, nRegStSize>
		CtnByteRgR(const ContCtnReadOnlyStRef<ContainerType, CtnRegSize>& ctn) noexcept
	{
		static_assert(beginOffset <= endOffset, "The begining of the range should be smaller than or equal to the end of the range.");
		static_assert(endOffset <= CtnRegSize, "The end of the range is outside of the container.");

		return ContCtnReadOnlyStRef<ContainerType, nRegStSize>(ctn, beginOffset, endOffset, gsk_noSafeCheck);
	}

	// CtnByteRgR<x>(sta)
	/**
	 * @brief Helper function to construct the ContCtnReadOnlyRef struct easily for
	 *        A) a specific range of the container, where the end of range is the end of container
	 *        B) containers with static size
	 *        C) range is specified statically
	 *
	 * @exception None No exception thrown
	 * @tparam beginOffset   The left end of the range (inclusive, in bytes).
	 * @tparam ContainerType Type of the container, which will be inferred from
	 *                       the giving parameter.
	 * @param ctn The const-reference to the container.
	 * @return ContCtnReadOnlyRef<ContainerType, Secrecy> The constructed ContCtnReadOnlyRef struct
	 */
	template<size_t beginOffset,
			 typename ContainerType,
			 // automated parts:
			 enable_if_t<
			 	CtnType<ContainerType>::sk_isCtnCont &&
				CtnType<ContainerType>::sk_isCtnStatic,
			 int> = 0,
			 size_t regStSize = CtnType<ContainerType>::sk_ctnSize - beginOffset>
	inline ContCtnReadOnlyStRef<ContainerType, regStSize>
		CtnByteRgR(const ContainerType& ctn) noexcept
	{
		static_assert(beginOffset <= CtnType<ContainerType>::sk_ctnSize, "The begining of the range is outside of the container.");

		return ContCtnReadOnlyStRef<ContainerType, regStSize>(ctn, beginOffset, gsk_noSafeCheck);
	}

	template<size_t beginOffset,
			 // automated parts:
			 typename ContainerType,
			 size_t CtnRegSize,
			 size_t nRegStSize = CtnRegSize - beginOffset>
	inline ContCtnReadOnlyStRef<ContainerType, nRegStSize>
		CtnByteRgR(const ContCtnReadOnlyStRef<ContainerType, CtnRegSize>& ctn) noexcept
	{
		static_assert(beginOffset <= CtnRegSize, "The begining of the range is outside of the container.");

		return ContCtnReadOnlyStRef<ContainerType, nRegStSize>(ctn, beginOffset, gsk_noSafeCheck);
	}

	// CtnByteRgR<x, y>(dyn)
	/**
	 * @brief Helper function to construct the ContCtnReadOnlyRef struct easily for
	 *        A) a specific range of the container
	 *        B) containers with dynamic size
	 *        C) range is specified statically
	 *
	 * @exception std::out_of_range Thrown if endOffset is out of the size of the container.
	 * @tparam beginOffset   The left end of the range (inclusive, in bytes).
	 * @tparam endOffset     The right end of the range (exclusive, in bytes).
	 * @tparam ContainerType Type of the container, which will be inferred from
	 *                       the giving parameter.
	 * @param ctn The const-reference to the container.
	 * @return ContCtnReadOnlyRef<ContainerType, Secrecy> The constructed ContCtnReadOnlyRef struct
	 */
	template<size_t beginOffset, size_t endOffset,
			 // automated parts:
			 typename ContainerType,
			 enable_if_t<
			 	CtnType<ContainerType>::sk_isCtnCont &&
				!CtnType<ContainerType>::sk_isCtnStatic,
			 int> = 0>
	inline ContCtnReadOnlyRef<ContainerType>
		CtnByteRgR(const ContainerType& ctn)
	{
		static_assert(beginOffset <= endOffset, "The begining of the range should be smaller than or equal to the end of the range.");

		if(endOffset > CtnType<ContainerType>::GetCtnSize(ctn))
		{
			throw std::out_of_range("The end of the range is outside of the container.");
		}
		return ContCtnReadOnlyRef<ContainerType>(ctn, beginOffset, endOffset, gsk_noSafeCheck);
	}

	template<size_t beginOffset, size_t endOffset,
			 // automated parts:
			 typename ContainerType>
	inline ContCtnReadOnlyRef<ContainerType>
		CtnByteRgR(const ContCtnReadOnlyRef<ContainerType>& ctn)
	{
		static_assert(beginOffset <= endOffset, "The begining of the range should be smaller than or equal to the end of the range.");

		if(endOffset > ctn.GetRegionSize())
		{
			throw std::out_of_range("The end of the range is outside of the container.");
		}

		return ContCtnReadOnlyRef<ContainerType>(ctn, beginOffset, endOffset, gsk_noSafeCheck);
	}

	// CtnByteRgR<x>(dyn)
	/**
	 * @brief Helper function to construct the ContCtnReadOnlyRef struct easily for
	 *        A) a specific range of the container, where the end of range is the end of container
	 *        B) containers with dynamic size
	 *        C) range is specified statically
	 *
	 * @exception std::out_of_range Thrown if beginOffset is out of the size of the container.
	 * @tparam beginOffset   The left end of the range (inclusive, in bytes).
	 * @tparam ContainerType Type of the container, which will be inferred from
	 *                       the giving parameter.
	 * @param ctn The const-reference to the container.
	 * @return ContCtnReadOnlyRef<ContainerType, Secrecy> The constructed ContCtnReadOnlyRef struct
	 */
	template<size_t beginOffset,
			 // automated parts:
			 typename ContainerType,
			 enable_if_t<
			 	CtnType<ContainerType>::sk_isCtnCont &&
				!CtnType<ContainerType>::sk_isCtnStatic,
			 int> = 0>
	inline ContCtnReadOnlyRef<ContainerType>
		CtnByteRgR(const ContainerType& ctn)
	{
		const size_t endOffset = CtnType<ContainerType>::GetCtnSize(ctn);
		if(beginOffset > endOffset)
		{
			throw std::out_of_range("The begining of the range is outside of the container.");
		}

		return ContCtnReadOnlyRef<ContainerType>(ctn, beginOffset, endOffset, gsk_noSafeCheck);
	}

	template<size_t beginOffset,
			 // automated parts:
			 typename ContainerType>
	inline ContCtnReadOnlyRef<ContainerType>
		CtnByteRgR(const ContCtnReadOnlyRef<ContainerType>& ctn)
	{
		if(beginOffset > ctn.GetRegionSize())
		{
			throw std::out_of_range("The begining of the range is outside of the container.");
		}

		return ContCtnReadOnlyRef<ContainerType>(ctn, beginOffset, gsk_noSafeCheck);
	}

	// CtnByteRgR(dyn, x, y)
	/**
	 * @brief Helper function to construct the ContCtnReadOnlyRef struct easily for
	 *        A) a specific range of the container
	 *        B) containers with dynamic size
	 *        C) range is specified dynamically
	 *
	 * @exception std::invalid_argument Thrown if endOffset < beginOffset.
	 * @exception std::out_of_range Thrown if endOffset is out of the size of the container.
	 * @tparam ContainerType Type of the container, which will be inferred from
	 *                       the giving parameter.
	 * @param ctn         The const-reference to the container.
	 * @param beginOffset The left end of the range (inclusive, in bytes).
	 * @param endOffset   The right end of the range (exclusive, in bytes).
	 * @return ContCtnReadOnlyRef<ContainerType, Secrecy> The constructed ContCtnReadOnlyRef struct
	 */
	template<// automated parts:
			 typename ContainerType,
			 enable_if_t<CtnType<ContainerType>::sk_isCtnCont, int> = 0>
	inline ContCtnReadOnlyRef<ContainerType>
		CtnByteRgR(const ContainerType& ctn, size_t beginOffset, size_t endOffset)
	{
		return ContCtnReadOnlyRef<ContainerType>(ctn, beginOffset, endOffset);
	}

	template<// automated parts:
			 typename ContainerType>
	inline ContCtnReadOnlyRef<ContainerType>
		CtnByteRgR(const ContCtnReadOnlyRef<ContainerType>& ctn, size_t beginOffset, size_t endOffset)
	{
		return ContCtnReadOnlyRef<ContainerType>(ctn, beginOffset, endOffset);
	}

	// CtnByteRgR(dyn, x)
	/**
	 * @brief Helper function to construct the ContCtnReadOnlyRef struct easily for
	 *        A) a specific range of the container, where the end of range is the end of container
	 *        B) containers with dynamic size
	 *        C) range is specified dynamically
	 *
	 * @exception std::invalid_argument Thrown if endOffset < beginOffset.
	 * @exception std::out_of_range Thrown if begining is out of the size of the container.
	 * @tparam ContainerType Type of the container, which will be inferred from
	 *                       the giving parameter.
	 * @param ctn         The const-reference to the container.
	 * @param beginOffset The left end of the range (inclusive, in bytes).
	 * @return ContCtnReadOnlyRef<ContainerType, Secrecy> The constructed ContCtnReadOnlyRef struct
	 */
	template<// automated parts:
			 typename ContainerType,
			 enable_if_t<CtnType<ContainerType>::sk_isCtnCont, int> = 0>
	inline ContCtnReadOnlyRef<ContainerType>
		CtnByteRgR(const ContainerType& ctn, size_t beginOffset)
	{
		const size_t endOffset = CtnType<ContainerType>::GetCtnSize(ctn);
		if(beginOffset > endOffset)
		{
			throw std::out_of_range("The begining of the range is outside of the container.");
		}

		return ContCtnReadOnlyRef<ContainerType>(ctn, beginOffset, endOffset);
	}

	template<// automated parts:
			 typename ContainerType>
	inline ContCtnReadOnlyRef<ContainerType>
		CtnByteRgR(const ContCtnReadOnlyRef<ContainerType>& ctn, size_t beginOffset)
	{
		return ContCtnReadOnlyRef<ContainerType>(ctn, beginOffset);
	}




	// CtnItemRgR<x, y>(sta)
	/**
	 * @brief Helper function to construct the ContCtnReadOnlyRef struct easily for
	 *        A) a specific range of the container
	 *        B) containers with static size
	 *        C) range is specified statically
	 *
	 * @exception None No exception thrown
	 * @tparam beginCount   The left end of the range (inclusive, in item counts).
	 * @tparam endCount     The right end of the range (exclusive, in item counts).
	 * @tparam ContainerType Type of the container, which will be inferred from
	 *                       the giving parameter.
	 * @param ctn The const-reference to the container.
	 * @return ContCtnReadOnlyRef<ContainerType, Secrecy> The constructed ContCtnReadOnlyRef struct
	 */
	template<size_t   beginCount, size_t endCount,
			 typename ContainerType,
			 // automated parts:
			 enable_if_t<
			 	CtnType<ContainerType>::sk_isCtnCont &&
				CtnType<ContainerType>::sk_isCtnStatic,
			 int> = 0,
			 size_t   regStSize = (endCount - beginCount) * CtnType<ContainerType>::sk_valSize>
	inline ContCtnReadOnlyStRef<ContainerType, regStSize>
		CtnItemRgR(const ContainerType& ctn) noexcept
	{
		static_assert(beginCount <= endCount, "The begining of the range should be smaller than or equal to the end of the range.");
		static_assert(endCount <= CtnType<ContainerType>::sk_itemCount, "The end of the range is outside of the container.");

		//Should we checks for overflow? - No, if it overflows here, then there is no way that the pointer can hold the address.
		constexpr size_t beginOffset = beginCount * CtnType<ContainerType>::sk_valSize;
		constexpr size_t endOffset   = endCount *   CtnType<ContainerType>::sk_valSize;
		static_assert(endOffset - beginOffset == regStSize, "Programming Error");

		return ContCtnReadOnlyStRef<ContainerType, regStSize>(ctn, beginOffset, gsk_noSafeCheck);
	}

	template<size_t   beginCount, size_t endCount,
			 // automated parts:
			 typename ContainerType,
			 size_t CtnRegSize,
			 size_t nRegStSize = (endCount - beginCount) * CtnType<ContainerType>::sk_valSize>
	inline ContCtnReadOnlyStRef<ContainerType, nRegStSize>
		CtnItemRgR(const ContCtnReadOnlyStRef<ContainerType, CtnRegSize>& ctn) noexcept
	{
		constexpr size_t beginOffset = beginCount * CtnType<ContainerType>::sk_valSize;
		constexpr size_t endOffset   = endCount *   CtnType<ContainerType>::sk_valSize;

		static_assert(beginOffset <= endOffset, "The begining of the range should be smaller than or equal to the end of the range.");
		static_assert(endOffset <= CtnRegSize, "The end of the range is outside of the container.");

		return ContCtnReadOnlyStRef<ContainerType, nRegStSize>(ctn, beginOffset, endOffset, gsk_noSafeCheck);
	}

	// CtnItemRgR<x>(sta)
	/**
	 * @brief Helper function to construct the ContCtnReadOnlyRef struct easily for
	 *        A) a specific range of the container, where the end of range is the end of container
	 *        B) containers with static size
	 *        C) range is specified statically
	 *
	 * @exception None No exception thrown
	 * @tparam beginCount    The left end of the range (inclusive, in item counts).
	 * @tparam ContainerType Type of the container, which will be inferred from
	 *                       the giving parameter.
	 * @param ctn The const-reference to the container.
	 * @return ContCtnReadOnlyRef<ContainerType, Secrecy> The constructed ContCtnReadOnlyRef struct
	 */
	template<size_t   beginCount,
			 typename ContainerType,
			 // automated parts:
			 enable_if_t<
			 	CtnType<ContainerType>::sk_isCtnCont &&
				CtnType<ContainerType>::sk_isCtnStatic,
			 int> = 0,
			 size_t   regStSize = CtnType<ContainerType>::sk_ctnSize - (beginCount* CtnType<ContainerType>::sk_valSize)>
	inline ContCtnReadOnlyStRef<ContainerType, regStSize>
		CtnItemRgR(const ContainerType& ctn) noexcept
	{
		static_assert(beginCount <= CtnType<ContainerType>::sk_itemCount, "The begining of the range is outside of the container.");

		constexpr size_t beginOffset = beginCount * CtnType<ContainerType>::sk_valSize;
		constexpr size_t endOffset = CtnType<ContainerType>::sk_ctnSize;
		static_assert(endOffset - beginOffset == regStSize, "Programming Error");

		return ContCtnReadOnlyStRef<ContainerType, regStSize>(ctn, beginOffset, gsk_noSafeCheck);
	}

	template<size_t   beginCount,
			 // automated parts:
			 typename ContainerType,
			 size_t CtnRegSize,
			 size_t nRegStSize = CtnRegSize - (beginCount * CtnType<ContainerType>::sk_valSize)>
	inline ContCtnReadOnlyStRef<ContainerType, nRegStSize>
		CtnItemRgR(const ContCtnReadOnlyStRef<ContainerType, CtnRegSize>& ctn) noexcept
	{
		constexpr size_t beginOffset = beginCount * CtnType<ContainerType>::sk_valSize;

		static_assert(beginOffset <= CtnRegSize, "The begining of the range is outside of the container.");

		return ContCtnReadOnlyStRef<ContainerType, nRegStSize>(ctn, beginOffset, gsk_noSafeCheck);
	}

	// CtnItemRgR<x, y>(dyn)
	/**
	 * @brief Helper function to construct the ContCtnReadOnlyRef struct easily for
	 *        A) a specific range of the container
	 *        B) containers with dynamic size
	 *        C) range is specified statically
	 *
	 * @exception std::out_of_range Thrown if endCount is out of the size of the container.
	 * @tparam beginCount    The left end of the range (inclusive, in item counts).
	 * @tparam endCount      The right end of the range (exclusive, in item counts).
	 * @tparam ContainerType Type of the container, which will be inferred from
	 *                       the giving parameter.
	 * @param ctn The const-reference to the container.
	 * @return ContCtnReadOnlyRef<ContainerType, Secrecy> The constructed ContCtnReadOnlyRef struct
	 */
	template<size_t beginCount, size_t endCount,
			 // automated parts:
			 typename ContainerType,
			 enable_if_t<
			 	CtnType<ContainerType>::sk_isCtnCont &&
				!CtnType<ContainerType>::sk_isCtnStatic,
			 int> = 0>
	inline ContCtnReadOnlyRef<ContainerType>
		CtnItemRgR(const ContainerType& ctn)
	{
		static_assert(beginCount <= endCount, "The begining of the range should be smaller than or equal to the end of the range.");

		if(endCount > CtnType<ContainerType>::GetItemCount(ctn))
		{
			throw std::out_of_range("The end of the range is outside of the container.");
		}

		constexpr size_t beginOffset = beginCount * CtnType<ContainerType>::sk_valSize;
		constexpr size_t endOffset   = endCount *   CtnType<ContainerType>::sk_valSize;

		return ContCtnReadOnlyRef<ContainerType>(ctn, beginOffset, endOffset, gsk_noSafeCheck);
	}

	template<size_t   beginCount, size_t endCount,
			 // automated parts:
			 typename ContainerType>
	inline ContCtnReadOnlyRef<ContainerType>
		CtnItemRgR(const ContCtnReadOnlyRef<ContainerType>& ctn)
	{
		constexpr size_t beginOffset = beginCount * CtnType<ContainerType>::sk_valSize;
		constexpr size_t endOffset   = endCount *   CtnType<ContainerType>::sk_valSize;

		static_assert(beginOffset <= endOffset, "The begining of the range should be smaller than or equal to the end of the range.");

		if(endOffset > ctn.GetRegionSize())
		{
			throw std::out_of_range("The end of the range is outside of the container.");
		}

		return ContCtnReadOnlyRef<ContainerType>(ctn, beginOffset, endOffset, gsk_noSafeCheck);
	}

	// CtnItemRgR<x>(dyn)
	/**
	 * @brief Helper function to construct the ContCtnReadOnlyRef struct easily for
	 *        A) a specific range of the container, where the end of range is the end of container
	 *        B) containers with dynamic size
	 *        C) range is specified statically
	 *
	 * @exception std::out_of_range Thrown if beginCount is out of the size of the container.
	 * @tparam beginCount    The left end of the range (inclusive, in item counts).
	 * @tparam ContainerType Type of the container, which will be inferred from
	 *                       the giving parameter.
	 * @param ctn The const-reference to the container.
	 * @return ContCtnReadOnlyRef<ContainerType, Secrecy> The constructed ContCtnReadOnlyRef struct
	 */
	template<size_t beginCount,
			 // automated parts:
			 typename ContainerType,
			 enable_if_t<
			 	CtnType<ContainerType>::sk_isCtnCont &&
				!CtnType<ContainerType>::sk_isCtnStatic,
			 int> = 0>
	inline ContCtnReadOnlyRef<ContainerType>
		CtnItemRgR(const ContainerType& ctn)
	{
		if(beginCount > CtnType<ContainerType>::GetItemCount(ctn))
		{
			throw std::out_of_range("The begining of the range is outside of the container.");
		}

		constexpr size_t beginOffset = beginCount * CtnType<ContainerType>::sk_valSize;
		const     size_t endOffset = CtnType<ContainerType>::GetCtnSize(ctn);

		return ContCtnReadOnlyRef<ContainerType>(ctn, beginOffset, endOffset, gsk_noSafeCheck);
	}

	template<size_t   beginCount,
			 // automated parts:
			 typename ContainerType>
	inline ContCtnReadOnlyRef<ContainerType>
		CtnItemRgR(const ContCtnReadOnlyRef<ContainerType>& ctn)
	{
		constexpr size_t beginOffset = beginCount * CtnType<ContainerType>::sk_valSize;

		if(beginOffset > ctn.GetRegionSize())
		{
			throw std::out_of_range("The begining of the range is outside of the container.");
		}

		return ContCtnReadOnlyRef<ContainerType>(ctn, beginOffset, gsk_noSafeCheck);
	}

	// CtnItemRgR(dyn, x, y)
	/**
	 * @brief Helper function to construct the ContCtnReadOnlyRef struct easily for
	 *        A) a specific range of the container
	 *        B) containers with dynamic size
	 *        C) range is specified dynamically
	 *
	 * @exception std::invalid_argument Thrown if beginCount < endCount.
	 * @exception std::out_of_range Thrown if endCount is out of the size of the container.
	 * @tparam ContainerType Type of the container, which will be inferred from
	 *                       the giving parameter.
	 * @param ctn         The const-reference to the container.
	 * @param beginCount  The left end of the range (inclusive, in item counts).
	 * @param endCount    The right end of the range (exclusive, in item counts).
	 * @return ContCtnReadOnlyRef<ContainerType, Secrecy> The constructed ContCtnReadOnlyRef struct
	 */
	template<// automated parts:
			 typename ContainerType,
			 enable_if_t<CtnType<ContainerType>::sk_isCtnCont, int> = 0>
	inline ContCtnReadOnlyRef<ContainerType>
		CtnItemRgR(const ContainerType& ctn, size_t beginCount, size_t endCount)
	{
		const size_t beginOffset = beginCount * CtnType<ContainerType>::sk_valSize;
		const size_t endOffset   = endCount *   CtnType<ContainerType>::sk_valSize;

		return ContCtnReadOnlyRef<ContainerType>(ctn, beginOffset, endOffset);
	}

	template<// automated parts:
			 typename ContainerType>
	inline ContCtnReadOnlyRef<ContainerType>
		CtnItemRgR(const ContCtnReadOnlyRef<ContainerType>& ctn, size_t beginCount, size_t endCount)
	{
		const size_t beginOffset = beginCount * CtnType<ContainerType>::sk_valSize;
		const size_t endOffset   = endCount *   CtnType<ContainerType>::sk_valSize;

		return ContCtnReadOnlyRef<ContainerType>(ctn, beginOffset, endOffset);
	}

	// CtnItemRgR(dyn, x)
	/**
	 * @brief Helper function to construct the ContCtnReadOnlyRef struct easily for
	 *        A) a specific range of the container, where the end of range is the end of container
	 *        B) containers with dynamic size
	 *        C) range is specified dynamically
	 *
	 * @exception std::invalid_argument Thrown if beginCount < endCount.
	 * @exception std::out_of_range Thrown if beginCount is out of the size of the container.
	 * @tparam ContainerType Type of the container, which will be inferred from
	 *                       the giving parameter.
	 * @param ctn         The const-reference to the container.
	 * @param beginCount  The left end of the range (inclusive, in item counts).
	 * @return ContCtnReadOnlyRef<ContainerType, Secrecy> The constructed ContCtnReadOnlyRef struct
	 */
	template<// automated parts:
			 typename ContainerType,
			 enable_if_t<CtnType<ContainerType>::sk_isCtnCont, int> = 0>
	inline ContCtnReadOnlyRef<ContainerType>
		CtnItemRgR(const ContainerType& ctn, size_t beginCount)
	{
		const size_t beginOffset = beginCount * CtnType<ContainerType>::sk_valSize;
		const size_t endOffset = CtnType<ContainerType>::GetCtnSize(ctn);
		if(beginOffset > endOffset)
		{
			throw std::out_of_range("The begining of the range is outside of the container.");
		}

		return ContCtnReadOnlyRef<ContainerType>(ctn, beginOffset, endOffset);
	}

	template<// automated parts:
			 typename ContainerType>
	inline ContCtnReadOnlyRef<ContainerType>
		CtnItemRgR(const ContCtnReadOnlyRef<ContainerType>& ctn, size_t beginCount)
	{
		const size_t beginOffset = beginCount * CtnType<ContainerType>::sk_valSize;

		return ContCtnReadOnlyRef<ContainerType>(ctn, beginOffset);
	}





	/**
	 * @brief Summarize a ContCtnReadOnlyRef<ContainerType> object into InDataListItem object.
	 *
	 * @tparam ContainerType The container type wrapped by ContCtnReadOnlyRef.
	 * @param data The data.
	 * @return constexpr InDataListItem The InDataListItem object.
	 */
	template<typename ContainerType, bool Secrecy>
	inline constexpr InDataListItem ConstructInDataListItem(const ContCtnReadOnlyRef<ContainerType, Secrecy>& data)
	{
		return InDataListItem{ data.BeginPtr(), data.GetRegionSize() };
	}

	/**
	 * @brief Convert a list of ContCtnReadOnlyRef<ContainerType> object into a
	 *        list of InDataListItem object.
	 *
	 * @tparam Args The container type wrapped by ContCtnReadOnlyRef.
	 * @param args The data.
	 * @return constexpr InDataList<sizeof...(Args)> The list of InDataListItem object.
	 */
	template<class... Args>
	inline constexpr InDataList<sizeof...(Args)> ConstructInDataList(Args... args)
	{
		return InDataList<sizeof...(Args)>{ ConstructInDataListItem(args)... };
	}
}
