#pragma once

#include <array>

#include "Common.hpp"
#include "LoadedFunctions.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	/**
	 * @brief A container, which is basically a wrapper around std::array, that
	 *        is used to store secret data, so that the container will be zeroize
	 *        at destruction.
	 *
	 * @tparam _ValType The type of the value stored in array.
	 * @tparam _Size    The length (i.e., number of items) stored in array.
	 */
	template<typename _ValType, size_t _Size>
	class SecretArray
	{
	public: //Types, and static members

		/**
		 * @brief The type of the value stored in array.
		 *
		 */
		using ValType = _ValType;

		/**
		 * @brief The length (i.e., number of items) stored in array.
		 *
		 */
		static constexpr size_t sk_itemCount = _Size;

		/**
		 * @brief The size of the type of the value stored in array.
		 *
		 */
		static constexpr size_t sk_valSize = sizeof(ValType);

		using _InnerArrayT = std::array<_ValType, _Size>;

		// From std::array
		typedef typename _InnerArrayT::value_type              value_type;
		typedef typename _InnerArrayT::pointer                 pointer;
		typedef typename _InnerArrayT::const_pointer           const_pointer;
		typedef typename _InnerArrayT::reference               reference;
		typedef typename _InnerArrayT::const_reference         const_reference;
		typedef typename _InnerArrayT::iterator                iterator;
		typedef typename _InnerArrayT::const_iterator          const_iterator;
		typedef typename _InnerArrayT::size_type               size_type;
		typedef typename _InnerArrayT::difference_type         difference_type;
		typedef typename _InnerArrayT::reverse_iterator        reverse_iterator;
		typedef typename _InnerArrayT::const_reverse_iterator  const_reverse_iterator;

	public:

		/**
		 * @brief Construct a new Secret Array object
		 *
		 */
		SecretArray() :
			m_data()
		{}

		/**
		 * @brief Copy the content of a C array to this secret array.
		 *
		 * @param other The existing C array.
		 */
		SecretArray(const _ValType (&other)[_Size]) :
			SecretArray()
		{
			std::copy(std::begin(other), std::end(other), m_data.begin());
		}

		/**
		 * @brief Construct a new Secret Array object by copying a existing
		 *        Secret Array.
		 *
		 * @param other The existing Secret Array.
		 */
		SecretArray(const SecretArray& other) :
			m_data(other.m_data)
		{}

		// LCOV_EXCL_START
		/**
		 * @brief Destroy the Secret Array object
		 *
		 */
		virtual ~SecretArray()
		{
			Zeroize();
		}
		// LCOV_EXCL_STOP

		/**
		 * @brief Copy assignment
		 *
		 * @param rhs The right-hand-side value.
		 * @return SecretArray& The reference to self.
		 */
		SecretArray& operator=(const SecretArray& rhs)
		{
			m_data = rhs.m_data;
			return *this;
		}

		/**
		 * @brief Move assignment
		 *
		 * @param rhs The right-hand-size value.
		 * @return SecretArray& The reference to self.
		 */
		SecretArray& operator=(SecretArray&& rhs)
		{
			Zeroize();
			m_data = std::forward<decltype(m_data)>(rhs.m_data);
			return *this;
		}

		/**
		 * @brief Zeroize the container.
		 *
		 */
		void Zeroize() noexcept
		{
			StaticLoadedFunctions::GetInstance().SecureZeroize(m_data.data(), m_data.size());
		}

		/**
		 * @brief Get the reference to the inner array.
		 *
		 * @return std::array<_ValType, _Size>& The reference to the inner array.
		 */
		_InnerArrayT& Get() noexcept
		{
			return m_data;
		}

		/**
		 * @brief Get the const reference to the inner array.
		 *
		 * @return const std::array<_ValType, _Size>& The const reference to the inner array.
		 */
		const _InnerArrayT& Get() const noexcept
		{
			return m_data;
		}

		//=======================
		// From std::array
		//=======================

		void fill(const value_type& u)
		{ m_data.fill(u); }

		void swap(SecretArray& other) noexcept(std::swap(std::declval<_InnerArrayT&>(), std::declval<_InnerArrayT&>()))
		{ m_data.swap(other.m_data); }

		iterator begin() noexcept
		{ return m_data.begin(); }

		const_iterator begin() const noexcept
		{ return m_data.begin(); }

		iterator end() noexcept
		{ return m_data.end(); }

		const_iterator end() const noexcept
		{ return m_data.end(); }

		reverse_iterator rbegin() noexcept
		{ return m_data.rbegin(); }

		const_reverse_iterator rbegin() const noexcept
		{ return m_data.rbegin(); }

		reverse_iterator rend() noexcept
		{ return m_data.rend(); }

		const_reverse_iterator rend() const noexcept
		{ return m_data.rend(); }

		const_iterator cbegin() const noexcept
		{ return m_data.cbegin(); }

		const_iterator cend() const noexcept
		{ return m_data.cend(); }

		const_reverse_iterator crbegin() const noexcept
		{ return m_data.crbegin(); }

		const_reverse_iterator crend() const noexcept
		{ return m_data.crend(); }

		constexpr size_type size() const noexcept
		{ return sk_itemCount; }

		constexpr size_type max_size() const noexcept
		{ return sk_itemCount; }

		constexpr bool empty() const noexcept
		{ return size() == 0; }

		reference operator[](size_type n) noexcept
		{ return m_data[n]; }

		const_reference operator[](size_type n) const noexcept
		{ return m_data[n]; }

		reference at(size_type n)
		{ return m_data.at(n); }

		const_reference at(size_type n) const
		{ return m_data.at(n); }

		reference front() noexcept
		{ return m_data.front(); }

		const_reference front() const noexcept
		{ return m_data.front(); }

		reference back() noexcept
		{ return m_data.back(); }

		const_reference back() const noexcept
		{ return m_data.back(); }

		pointer data() noexcept
		{ return m_data.data(); }

		const_pointer data() const noexcept
		{ return m_data.data(); }

	private:
		_InnerArrayT m_data;
	};

	template<typename _ValType, size_t _Size>
	bool operator==(const SecretArray<_ValType, _Size>& lhs,
					const SecretArray<_ValType, _Size>& rhs)
	{
		return StaticLoadedFunctions::GetInstance().ConstTimeMemEqual(lhs.data(), rhs.data(), _Size * sizeof(_ValType));
	}

	template<typename _ValType, size_t _Size>
	bool operator!=(const SecretArray<_ValType, _Size>& lhs,
					const SecretArray<_ValType, _Size>& rhs)
	{
		return StaticLoadedFunctions::GetInstance().ConstTimeMemNotEqual(lhs.data(), rhs.data(), _Size * sizeof(_ValType));
	}

	template<typename _ValType, size_t _Size>
	void Declassify(_ValType (&out)[_Size], const SecretArray<_ValType, _Size>& in)
	{
		std::copy(in.begin(), in.end(), std::begin(out));
	}
}
