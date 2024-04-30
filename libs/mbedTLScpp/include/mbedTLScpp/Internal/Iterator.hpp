#pragma once

#include <iterator>
#include <type_traits>

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	namespace Internal
	{
		template<typename _Pointer>
		class random_acc_iterator
		{
		protected:
			_Pointer m_ptr;

			typedef std::iterator_traits<_Pointer>		it_traits_type;

		public:
			typedef _Pointer                                    iterator_type;
			typedef typename it_traits_type::iterator_category  iterator_category;
			typedef typename it_traits_type::value_type         value_type;
			typedef typename it_traits_type::difference_type    difference_type;
			typedef typename it_traits_type::reference          reference;
			typedef typename it_traits_type::pointer            pointer;

			template<typename _some_Pointer>
			using add_const_pointed = std::add_pointer<         // 3rd, add pointer
				typename std::add_const<                        // 2nd, add const
					typename std::remove_pointer<_some_Pointer> // 1st, remove pointer
					::type>::type>;

			constexpr random_acc_iterator() noexcept :
				m_ptr(_Pointer())
			{}

			explicit random_acc_iterator(const _Pointer& ptr) noexcept :
				m_ptr(ptr)
			{}

			template<typename _other_Pointer,
				typename std::enable_if<
					std::is_same<_Pointer, _other_Pointer                                   >::value || // If it's the same pointer
					std::is_same<_Pointer, typename add_const_pointed<_other_Pointer>::type >::value    // If the other one is non-const, and to convert to const.
				, int>::type = 0>
			random_acc_iterator(const random_acc_iterator<_other_Pointer>& other) noexcept :
				m_ptr(other.base())
			{}

			reference operator*() const noexcept
			{
				return *m_ptr;
			}

			pointer operator->() const noexcept
			{
				return m_ptr;
			}

			random_acc_iterator& operator++() noexcept
			{
				++m_ptr;
				return *this;
			}

			random_acc_iterator operator++(int) noexcept
			{
				return random_acc_iterator(m_ptr++);
			}

			random_acc_iterator& operator--() noexcept
			{
				--m_ptr;
				return *this;
			}

			random_acc_iterator operator--(int) noexcept
			{
				return random_acc_iterator(m_ptr--);
			}

			reference operator[](difference_type idx) const noexcept
			{
				return m_ptr[idx];
			}

			random_acc_iterator& operator+=(difference_type count) noexcept
			{
				m_ptr += count;
				return *this;
			}

			random_acc_iterator operator+(difference_type count) const noexcept
			{
				return random_acc_iterator(m_ptr + count);
			}

			random_acc_iterator& operator-=(difference_type count) noexcept
			{
				m_ptr -= count;
				return *this;
			}

			random_acc_iterator operator-(difference_type count) const noexcept
			{
				return random_acc_iterator(m_ptr - count);
			}

			const _Pointer& base() const noexcept
			{
				return m_ptr;
			}
		};

		template<typename _PointerL, typename _PointerR>
		inline bool operator==(const random_acc_iterator<_PointerL>& lhs, const random_acc_iterator<_PointerR>& rhs) noexcept
		{
			return lhs.base() == rhs.base();
		}

		template<typename _Pointer>
		inline bool operator==(const random_acc_iterator<_Pointer>& lhs, const random_acc_iterator<_Pointer>& rhs) noexcept
		{
			return lhs.base() == rhs.base();
		}

		template<typename _PointerL, typename _PointerR>
		inline bool operator!=(const random_acc_iterator<_PointerL>& lhs, const random_acc_iterator<_PointerR>& rhs) noexcept
		{
			return lhs.base() != rhs.base();
		}

		template<typename _Pointer>
		inline bool operator!=(const random_acc_iterator<_Pointer>& lhs, const random_acc_iterator<_Pointer>& rhs) noexcept
		{
			return lhs.base() != rhs.base();
		}

		template<typename _PointerL, typename _PointerR>
		inline bool operator<(const random_acc_iterator<_PointerL>& lhs, const random_acc_iterator<_PointerR>& rhs) noexcept
		{
			return lhs.base() < rhs.base();
		}

		template<typename _Pointer>
		inline bool operator<(const random_acc_iterator<_Pointer>& lhs, const random_acc_iterator<_Pointer>& rhs) noexcept
		{
			return lhs.base() < rhs.base();
		}

		template<typename _PointerL, typename _PointerR>
		inline bool operator>(const random_acc_iterator<_PointerL>& lhs, const random_acc_iterator<_PointerR>& rhs) noexcept
		{
			return lhs.base() > rhs.base();
		}

		template<typename _Pointer>
		inline bool operator>(const random_acc_iterator<_Pointer>& lhs, const random_acc_iterator<_Pointer>& rhs) noexcept
		{
			return lhs.base() > rhs.base();
		}

		template<typename _PointerL, typename _PointerR>
		inline bool operator<=(const random_acc_iterator<_PointerL>& lhs, const random_acc_iterator<_PointerR>& rhs) noexcept
		{
			return lhs.base() <= rhs.base();
		}

		template<typename _Pointer>
		inline bool operator<=(const random_acc_iterator<_Pointer>& lhs, const random_acc_iterator<_Pointer>& rhs) noexcept
		{
			return lhs.base() <= rhs.base();
		}

		template<typename _PointerL, typename _PointerR>
		inline bool operator>=(const random_acc_iterator<_PointerL>& lhs, const random_acc_iterator<_PointerR>& rhs) noexcept
		{
			return lhs.base() >= rhs.base();
		}

		template<typename _Pointer>
		inline bool operator>=(const random_acc_iterator<_Pointer>& lhs, const random_acc_iterator<_Pointer>& rhs) noexcept
		{
			return lhs.base() >= rhs.base();
		}

		template<typename _PointerL, typename _PointerR>
		inline auto operator-(const random_acc_iterator<_PointerL>& lhs, const random_acc_iterator<_PointerR>& rhs) noexcept
			-> decltype(lhs.base() - rhs.base())
		{
			return lhs.base() - rhs.base();
		}

		template<typename _Pointer>
		inline typename random_acc_iterator<_Pointer>::difference_type operator-(const random_acc_iterator<_Pointer>& lhs, const random_acc_iterator<_Pointer>& rhs) noexcept
		{
			return lhs.base() - rhs.base();
		}

		template<typename _Pointer>
		inline random_acc_iterator<_Pointer> operator+(typename random_acc_iterator<_Pointer>::difference_type count, const random_acc_iterator<_Pointer>& it) noexcept
		{
			return random_acc_iterator<_Pointer>(it.base() + count);
		}
	}
}
