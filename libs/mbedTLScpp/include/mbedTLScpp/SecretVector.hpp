#pragma once

#include <cstring>

#include <limits>
#include <vector>
#include <iterator>

#include "Common.hpp"
#include "SecretAllocator.hpp"

#include "Internal/Construct.hpp"
#include "Internal/Iterator.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	template<typename _ValType, typename _Alloc = SecretAllocator<_ValType>, bool _NullTerminated = false >
	class SecretVector
	{
	public: // Member types

		typedef  _ValType                                  value_type;
		typedef  _Alloc                                    allocator_type;
		typedef  std::allocator_traits<allocator_type>     allocator_traits;
		typedef  typename allocator_type::size_type        size_type;
		typedef  typename allocator_type::difference_type  difference_type;
		typedef  typename allocator_type::reference        reference;
		typedef  typename allocator_type::const_reference  const_reference;
		typedef  typename allocator_type::pointer          pointer;
		typedef  typename allocator_type::const_pointer    const_pointer;

		typedef  Internal::random_acc_iterator<pointer>       iterator;
		typedef  Internal::random_acc_iterator<const_pointer> const_iterator;
		typedef  std::reverse_iterator<const_iterator>        const_reverse_iterator;
		typedef  std::reverse_iterator<iterator>              reverse_iterator;

		static constexpr size_type sk_extra_cap = _NullTerminated ? 1 : 0;


	public: // Static members:

		template<typename _value_type = value_type,
			enable_if_t<IsCTypeAlike<_value_type>::value, int> = 0>
		static void BuildEqualLengthString(uint8_t* a_buf, uint8_t* b_buf,
			const _value_type* a, size_type a_size,
			const _value_type* b, size_type b_size)
		{
			const uint8_t*       a_size_first = reinterpret_cast<const uint8_t*>(&a_size);
			const uint8_t* const a_size_last  = a_size_first + sizeof(size_type);

			const uint8_t*       a_first = reinterpret_cast<const uint8_t*>(a);
			const uint8_t* const a_last  = a_first + (a_size * sizeof(_value_type));
			const uint8_t*       b_first = reinterpret_cast<const uint8_t*>(b);
			const uint8_t* const b_last  = b_first + (b_size * sizeof(_value_type));

			// 1. fill a_buf = size | a | b
			for(; a_size_first != a_size_last; ++a_buf, ++a_size_first)
			{
				*a_buf = *a_size_first;
			}
			for(; a_first != a_last; ++a_buf, ++a_first)
			{
				*a_buf = *a_first;
			}
			for(; b_first != b_last; ++a_buf, ++b_first)
			{
				*a_buf = *b_first;
			}

			const uint8_t*       b_size_first = reinterpret_cast<const uint8_t*>(&b_size);
			const uint8_t* const b_size_last  = b_size_first + sizeof(size_type);

			b_first = reinterpret_cast<const uint8_t*>(b);
			a_first = reinterpret_cast<const uint8_t*>(a);

			// 2. fill b_buf = size | b | a
			for(; b_size_first != b_size_last; ++b_buf, ++b_size_first)
			{
				*b_buf = *b_size_first;
			}
			for(; b_first != b_last; ++b_buf, ++b_first)
			{
				*b_buf = *b_first;
			}
			for(; a_first != a_last; ++b_buf, ++a_first)
			{
				*b_buf = *a_first;
			}
		}

		template<typename _value_type = value_type,
			enable_if_t<IsCTypeAlike<_value_type>::value, int> = 0>
		static bool SafeCompareEqual(const _value_type* a, size_type a_size, const _value_type* b, size_type b_size)
		{
			using buf_allocator = typename allocator_traits::template rebind_alloc<uint8_t>;
			using buf_allocator_traits = typename allocator_traits::template rebind_traits<uint8_t>;

			buf_allocator alloc;

			const size_t buf_size = sizeof(size_type) + (a_size * sizeof(_value_type)) + (b_size * sizeof(_value_type));

			// allocate all buffer space at once
			uint8_t* const buf = buf_allocator_traits::allocate(alloc, buf_size + buf_size);

			try
			{
				uint8_t* a_buf = buf;
				uint8_t* b_buf = buf + buf_size;

				BuildEqualLengthString(a_buf, b_buf, a, a_size, b, b_size);

				bool res = StaticLoadedFunctions::GetInstance().ConstTimeMemEqual(a_buf, b_buf, buf_size);
				buf_allocator_traits::deallocate(alloc, buf, buf_size + buf_size);
				return res;
			}
			catch(...)
			{
				buf_allocator_traits::deallocate(alloc, buf, buf_size + buf_size);
				throw;
			}
		}

		template<typename _value_type = value_type,
			enable_if_t<IsCTypeAlike<_value_type>::value, int> = 0>
		static bool SafeCompareNotEqual(const _value_type* a, size_type a_size, const _value_type* b, size_type b_size)
		{
			using buf_allocator = typename allocator_traits::template rebind_alloc<uint8_t>;
			using buf_allocator_traits = typename allocator_traits::template rebind_traits<uint8_t>;

			buf_allocator alloc;

			const size_t buf_size = sizeof(size_type) + (a_size * sizeof(_value_type)) + (b_size * sizeof(_value_type));

			// allocate all buffer space at once
			uint8_t* const buf = buf_allocator_traits::allocate(alloc, buf_size + buf_size);

			try
			{
				uint8_t* a_buf = buf;
				uint8_t* b_buf = buf + buf_size;

				BuildEqualLengthString(a_buf, b_buf, a, a_size, b, b_size);

				bool res = StaticLoadedFunctions::GetInstance().ConstTimeMemNotEqual(a_buf, b_buf, buf_size);
				buf_allocator_traits::deallocate(alloc, buf, buf_size + buf_size);
				return res;
			}
			catch(...)
			{
				buf_allocator_traits::deallocate(alloc, buf, buf_size + buf_size);
				throw;
			}
		}

	private: // methods that are needed by constructors

		pointer allocate(size_type cap)
		{
			return allocator_traits::allocate(m_alloc, cap + sk_extra_cap);
		}

		void deallocate(pointer data, size_type cap) noexcept
		{
			try
			{
				allocator_traits::deallocate(m_alloc, data, cap + sk_extra_cap);
			}
			catch(...)
			{}
		}

		void null_terminate() noexcept
		{
			if (_NullTerminated)
			{
				try
				{
					if (m_data != nullptr)
					{
						std::memset(
							static_cast<void*>(m_data + m_size),
							0,
							sk_extra_cap * sizeof(value_type)
						);
					}
				}
				catch(...)
				{}
			}
		}

		void clear_only() noexcept
		{
			try
			{
				Internal::destroy(m_data, m_data + m_size);
			}
			catch(...)
			{}

			m_size = 0;
		}

		void clear_and_deallocate() noexcept
		{
			clear_only();
			deallocate(m_data, m_capacity);
			m_data = nullptr;
			m_capacity = 0;
		}

	public:

		SecretVector() :
			m_alloc(allocator_type()),
			m_capacity(0),
			m_data(nullptr),
			m_size(0)
		{}

		SecretVector(const allocator_type& alloc) :
			m_alloc(alloc),
			m_capacity(0),
			m_data(nullptr),
			m_size(0)
		{}

		SecretVector(size_type count, const allocator_type& alloc = allocator_type()) :
			m_alloc(alloc),
			m_capacity(count),
			m_data(allocate(count)),
			m_size(0)
		{
			try
			{
				Internal::uninitialized_default_construct(m_data, m_data + count);
				m_size = count;
			}
			catch(...)
			{
				// uninitialized_default_construct will destroy any constructed object when throw.
				clear_and_deallocate();
				throw;
			}
			null_terminate();
		}

		SecretVector(size_type count, const value_type& value, const allocator_type& alloc = allocator_type()) :
			m_alloc(alloc),
			m_capacity(count),
			m_data(allocate(count)),
			m_size(0)
		{
			try
			{
				std::uninitialized_fill_n(m_data, count, value);
				m_size = count;
			}
			catch(...)
			{
				// uninitialized_fill_n will destroy any constructed object when throw.
				clear_and_deallocate();
				throw;
			}
			null_terminate();
		}

		template<typename _InputIterator,
			typename std::enable_if<!std::is_same<typename std::iterator_traits<_InputIterator>::value_type, void>::value, int>::type = 0>
		SecretVector(_InputIterator first, _InputIterator last, const allocator_type& alloc = allocator_type()) :
			SecretVector(alloc)
		{
			const auto signed_count = std::distance(first, last);
			if (signed_count < 0)
			{
				throw std::invalid_argument("In SecretVector::SecretVector, the iterator range given may be in flipped side.");
			}

			const size_type count = static_cast<size_type>(signed_count);
			m_data = allocate(count);
			m_capacity = count;

			try
			{
				std::uninitialized_copy(first, last, m_data);
				m_size = m_capacity;
			}
			catch(...)
			{
				// uninitialized_fill_n will destroy any constructed object when throw.
				clear_and_deallocate();
				throw;
			}
			null_terminate();
		}

		SecretVector(std::initializer_list<value_type> init_list, const allocator_type& alloc = allocator_type()) :
			SecretVector(init_list.begin(), init_list.end(), alloc)
		{}

		SecretVector(const SecretVector& other, const allocator_type& alloc) :
			m_alloc(alloc),
			m_capacity(other.m_capacity),
			m_data(allocate(other.m_capacity)),
			m_size(0)
		{
			try
			{
				std::uninitialized_copy(other.m_data, other.m_data + other.m_size, m_data);
				m_size = other.m_size;
			}
			catch(...)
			{
				// uninitialized_copy will destroy any constructed object when throw.
				clear_and_deallocate();
				throw;
			}
			null_terminate();
		}

		SecretVector(const SecretVector& other) :
			m_alloc(other.m_alloc),
			m_capacity(other.m_capacity),
			m_data(allocate(other.m_capacity)),
			m_size(0)
		{
			try
			{
				std::uninitialized_copy(other.m_data, other.m_data + other.m_size, m_data);
				m_size = other.m_size;
			}
			catch(...)
			{
				// uninitialized_copy will destroy any constructed object when throw.
				clear_and_deallocate();
				throw;
			}
			null_terminate();
		}

		SecretVector(SecretVector&& other) noexcept :
			m_alloc(std::move(other.m_alloc)),
			m_capacity(std::move(other.m_capacity)),
			m_data(std::move(other.m_data)),
			m_size(std::move(other.m_size))
		{
			other.m_data = nullptr;
			other.m_size = 0;
			other.m_capacity = 0;
		}

		SecretVector(SecretVector&& other, const allocator_type& alloc) :
			m_alloc(alloc),
			m_capacity(std::move(other.m_capacity)),
			m_data(std::move(other.m_data)),
			m_size(std::move(other.m_size))
		{
			other.m_data = nullptr;
			other.m_size = 0;
			other.m_capacity = 0;
		}

		// LCOV_EXCL_START
		virtual ~SecretVector()
		{
			clear_and_deallocate();
		}
		// LCOV_EXCL_STOP

		SecretVector& operator=( const SecretVector& other )
		{
			if(this != &other)
			{
				clear_and_deallocate();

				m_alloc = other.m_alloc;
				relocate(other.m_capacity);
				std::uninitialized_copy(other.m_data, other.m_data + other.m_size, m_data);
				m_size = other.m_size;

				null_terminate();
			}
			return *this;
		}

		SecretVector& operator=( SecretVector&& other ) noexcept
		{
			if(this != &other)
			{
				clear_and_deallocate();

				m_alloc = std::move(other.m_alloc);
				std::swap(m_capacity, other.m_capacity);
				std::swap(m_data, other.m_data);
				std::swap(m_size, other.m_size);
			}
			return *this;
		}

		void swap( SecretVector& other ) noexcept
		{
			std::swap(m_alloc, other.m_alloc);
			std::swap(m_capacity, other.m_capacity);
			std::swap(m_data, other.m_data);
			std::swap(m_size, other.m_size);
		}

		void clear() noexcept
		{
			clear_only();

			null_terminate();
		}

		allocator_type get_allocator() const noexcept
		{
			return allocator_type(m_alloc);
		}

		iterator begin() noexcept
		{ return iterator(m_data); }

		const_iterator begin() const noexcept
		{ return const_iterator(m_data); }

		iterator end() noexcept
		{ return iterator(m_data + m_size); }

		const_iterator end() const noexcept
		{ return const_iterator(m_data + m_size); }

		reverse_iterator rbegin() noexcept
		{ return reverse_iterator(end()); }

		const_reverse_iterator rbegin() const noexcept
		{ return const_reverse_iterator(end()); }

		reverse_iterator rend() noexcept
		{ return reverse_iterator(begin()); }

		const_reverse_iterator rend() const noexcept
		{ return const_reverse_iterator(begin()); }

		const_iterator cbegin() const noexcept
		{ return const_iterator(m_data); }

		const_iterator cend() const noexcept
		{ return const_iterator(m_data + m_size); }

		const_reverse_iterator crbegin() const noexcept
		{ return const_reverse_iterator(end()); }

		const_reverse_iterator crend() const noexcept
		{ return const_reverse_iterator(begin()); }

		reference operator[](size_type n)
		{
			return *(m_data + n);
		}

		const_reference operator[](size_type n) const
		{
			return *(m_data + n);
		}

		reference at(size_type pos)
		{
			range_check(pos);

			return (*this)[pos];
		}

		const_reference at(size_type pos) const
		{
			range_check(pos);

			return (*this)[pos];
		}

		pointer data() noexcept
		{
			return m_data;
		}

		const_pointer data() const noexcept
		{
			return m_data;
		}

		reference front()
		{
			return *begin();
		}

		const_reference front() const
		{
			return *begin();
		}

		reference back()
		{
			return *(end() - 1);
		}

		const_reference back() const
		{
			return *(end() - 1);
		}

		size_type size() const noexcept
		{
			return m_size;
		}

		size_type capacity() const noexcept
		{
			return m_capacity;
		}

		bool empty() const noexcept
		{
			return m_size == 0;
		}

		void reserve(size_type new_cap)
		{
			if(new_cap > capacity())
			{
				relocate(new_cap);

				null_terminate();
			}
		}

		void resize( size_type count )
		{
			if(count < m_size)
			{
				// Reduce size
				// Deconstruct tail elements
				Internal::destroy(m_data + count, m_data + m_size);
				m_size = count;
				// Vector capacity is never reduced when resizing to smaller size

				null_terminate();
			}
			else if(count > m_size)
			{
				// Increase size
				if (count > capacity())
				{
					// relocation needed
					relocate(count);
				}
				// Initialize values
				Internal::uninitialized_default_construct(m_data + m_size, m_data + count);
				m_size = count;

				null_terminate();
			}
			// count == m_size, do nothing
		}

		void resize( size_type count, const value_type& value )
		{
			if(count < m_size)
			{
				// Reduce size
				resize(count);
			}
			else if(count > m_size)
			{
				// Increase size
				if (count > capacity())
				{
					// relocation needed
					relocate(count);
				}
				// fill values
				std::uninitialized_fill_n(m_data + m_size, count - m_size, value);
				m_size = count;

				null_terminate();
			}
			// count == m_size, do nothing
		}

		void shrink_to_fit()
		{
			relocate(size());

			null_terminate();
		}

		size_type max_size() const noexcept
		{
			return (std::numeric_limits<size_type>::max)() - sk_extra_cap;
		}

		void push_back(const value_type& value)
		{
			// NOTE: Don't reallocate the space before storing the value some where
			// since it can be part of the array!
			if (m_size < m_capacity)
			{
				// There is enough space
				std::allocator_traits<allocator_type>::construct(m_alloc, m_data + m_size, value);
				++m_size;
			}
			else
			{
				// No enough space, we need to reallocate
				// Construct the value in a *safe* place first.
				SecretVector secure_tmp(1, value, m_alloc);
				realloc_move_insert(end() - begin(), secure_tmp.data(), secure_tmp.size());
			}

			null_terminate();
		}

		void push_back(value_type&& value)
		{
			emplace_back(std::move(value));
		}

		void pop_back()
		{
			if(size() > 0)
			{
				std::allocator_traits<allocator_type>::destroy(m_alloc, m_data + m_size - 1);
				--m_size;

				null_terminate();
			}
		}

		template<typename... _Args>
#if __cplusplus > 201402L
		reference
#else
		void
#endif
		emplace_back(_Args&&... __args)
		{
			// NOTE: Don't reallocate the space before storing the value some where
			// since it can be part of the array!
			if (m_size < m_capacity)
			{
				// There is enough space
				std::allocator_traits<allocator_type>::construct(m_alloc, m_data + m_size, std::forward<_Args>(__args)...);
				++m_size;
			}
			else
			{
				// No enough space, we need to reallocate
				// Construct the value in a *safe* place first.
				SecretVector secure_tmp(m_alloc);
				secure_tmp.reserve(1);
				secure_tmp.emplace_back(std::forward<_Args>(__args)...);
				realloc_move_insert(end() - begin(), secure_tmp.data(), secure_tmp.size());
			}

			null_terminate();
#if __cplusplus > 201402L
			return back();
#endif
		}

		template< class... _Args >
		iterator emplace(const_iterator pos, _Args&&... args)
		{
			if (pos == cend())
			{
				emplace_back(std::forward<_Args>(args)...);

				null_terminate();
				return end() - 1;
			}
			else
			{
				// Construct the value in a *safe* place first.
				SecretVector secure_tmp(m_alloc);
				secure_tmp.reserve(1);
				secure_tmp.emplace_back(std::forward<_Args>(args)...);

				if (m_size < m_capacity)
				{
					// There is enough space
					auto it = insert(pos, std::move(secure_tmp.front()));

					null_terminate();
					return it;
				}
				else
				{
					const size_type offset = pos - cbegin();
					// No enough space, we need to reallocate
					realloc_move_insert(offset, secure_tmp.data(), secure_tmp.size());

					null_terminate();
					return iterator(m_data + offset);
				}
			}

		}

		iterator insert(const_iterator pos, const value_type& value)
		{
			const size_type offset = pos - cbegin();
			if (m_size < m_capacity)
			{
				if (pos == end())
				{
					std::allocator_traits<allocator_type>::construct(m_alloc, m_data + m_size, value);
					++m_size;
				}
				else
				{
					// Construct the value in a *safe* place first.
					SecretVector secure_tmp(1, value, m_alloc);
					noalloc_move_insert_one(begin() + offset, std::move(secure_tmp.front()));
				}
			}
			else
			{
				// No enough space, we need to reallocate
				// Construct the value in a *safe* place first.
				SecretVector secure_tmp(1, value, m_alloc);
				realloc_move_insert(pos - cbegin(), secure_tmp.data(), secure_tmp.size());
			}

			null_terminate();
			return iterator(m_data + offset);
		}

		iterator insert(const_iterator pos, value_type&& value)
		{
			const size_type offset = pos - cbegin();
			if (m_size < m_capacity)
			{
				if (pos == cend())
				{
					std::allocator_traits<allocator_type>::construct(m_alloc, m_data + m_size, std::move(value));
					++m_size;
				}
				else
				{
					// Construct the value in a *safe* place first.
					SecretVector secure_tmp(m_alloc);
					secure_tmp.reserve(1);
					secure_tmp.emplace_back(std::move(value));
					noalloc_move_insert_one(begin() + offset, std::move(secure_tmp.front()));
				}
			}
			else
			{
				// No enough space, we need to reallocate
				// Construct the value in a *safe* place first.
				SecretVector secure_tmp(m_alloc);
				secure_tmp.reserve(1);
				secure_tmp.emplace_back(std::move(value));
				realloc_move_insert(offset, secure_tmp.data(), secure_tmp.size());
			}

			null_terminate();
			return iterator(m_data + offset);
		}

		iterator insert(iterator pos, size_type count, const value_type& value)
		{
			const size_type offset = pos - begin();

			SecretVector secure_tmp(1, value, m_alloc);
			const value_type& value_copy = secure_tmp.front();

			if (count != 0)
			{
				if (size_type(m_capacity - m_size) >= count)
				{
					noalloc_fill_insert(pos, value_copy, count);
				}
				else
				{
					realloc_fill_insert(pos - begin(), value_copy, count);
				}

				null_terminate();
			}

			return begin() + offset;
		}

		iterator insert( const_iterator pos, size_type count, const value_type& value )
		{
			const size_type n = pos - cbegin();
			iterator mut_pos = begin() + n;

			return insert(mut_pos, count, value);
		}

		template<typename _InputIt,
			typename std::enable_if<!std::is_same<typename std::iterator_traits<_InputIt>::value_type, void>::value, int>::type = 0>
		iterator insert(iterator pos, _InputIt first, _InputIt last)
		{
			const size_type n = pos - begin();
			const auto signed_count = std::distance(first, last);
			if (signed_count < 0)
			{
				throw std::invalid_argument("In SecretVector::insert, the iterator range given may be in flipped side.");
			}

			const size_type count = static_cast<size_type>(signed_count);

			if(first != last)
			{
				if (size_type(m_capacity - m_size) >= count)
				{
					// there is enough space - directly copy
					noalloc_copy_insert(pos, first, last, count);
				}
				else
				{
					// there is no enough space - realloc + copy
					SecretVector secure_tmp(first, last, m_alloc);
					realloc_move_insert(pos - begin(), secure_tmp.data(), secure_tmp.size());
				}

				null_terminate();
			}

			return iterator(m_data + n);
		}

		template<class _InputIt,
			typename std::enable_if<!std::is_same<typename std::iterator_traits<_InputIt>::value_type, void>::value, int>::type = 0>
		iterator insert(const_iterator pos, _InputIt first, _InputIt last)
		{
			const size_type n = pos - cbegin();
			iterator mut_pos = begin() + n;

			return insert(mut_pos, first, last);
		}

		iterator insert(iterator pos, const value_type* first, const value_type* last)
		{
			const size_type n = pos - begin();

			if (first > last)
			{
				throw std::invalid_argument("In SecretVector::insert, the pointer range given may be in flipped side.");
			}

			const size_type count = last - first;

			if(count > 0)
			{
				if (size_type(m_capacity - m_size) >= count)
				{
					// there is enough space - copy only

					if (m_data <= first && first < (m_data + m_size))
					{
						// Source overlap with this vector

						SecretVector secure_tmp;
						secure_tmp.assign(first, last);
						noalloc_copy_insert(pos,
							std::make_move_iterator(secure_tmp.begin()),
							std::make_move_iterator(secure_tmp.end()),
							count);
					}
					else
					{
						// no overlap

						noalloc_copy_insert(pos, first, last, count);
					}
				}
				else
				{
					// there is no enough space - realloc + copy

					// Overlap is OK
					realloc_copy_insert(pos - begin(), first, count);
				}

				null_terminate();
			}

			return iterator(m_data + n);
		}

		iterator insert(const_iterator pos, const value_type* first, const value_type* last)
		{
			const size_type n = pos - cbegin();
			iterator mut_pos = begin() + n;

			return insert(mut_pos, first, last);
		}

		iterator erase(iterator pos)
		{
			if(pos != end() && m_size > 0)
			{
				if (pos + 1 != end())
					std::move(pos + 1, end(), pos);

				--m_size;
				std::allocator_traits<allocator_type>::destroy(m_alloc, m_data + m_size);

				null_terminate();
			}
			return pos;
		}

		iterator erase(const_iterator pos)
		{
			iterator mut_pos = begin() + (pos - cbegin());
			return erase(mut_pos);
		}

		iterator erase(iterator first, iterator last)
		{
			if (first != last)
			{
				if (last != end())
				{
					std::move(last, end(), first);
				}
				erase_at_end(first.base() + (end() - last));

				null_terminate();
			}
			return first;
		}

		iterator erase(const_iterator first, const_iterator last)
		{
			iterator mut_first = begin() + (first - cbegin());
			iterator mut_last  = begin() + (last  - cbegin());
			return erase(mut_first, mut_last);
		}

		void assign(size_type count, const value_type& value)
		{
			if(count <= m_capacity)
			{
				// there is enough space
				// 1. clear;
				clear_only();
			}
			else
			{
				// there is no enough space
				// 1. clear and deallocate
				clear_and_deallocate();

				// 2. reserve
				reserve(count);
			}

			// fill
			std::uninitialized_fill_n(m_data, count, value);
			m_size = count;

			null_terminate();
		}

		template<class _InputIt,
			typename std::enable_if<!std::is_same<typename std::iterator_traits<_InputIt>::value_type, void>::value, int>::type = 0>
		void assign(_InputIt first, _InputIt last)
		{
			const auto signed_count = std::distance(first, last);
			if (signed_count < 0)
			{
				throw std::invalid_argument("In SecretVector::insert, the iterator range given may be in flipped side.");
			}

			const size_type count = static_cast<size_type>(signed_count);
			if(count <= m_capacity)
			{
				// there is enough space
				// 1. clear;
				clear_only();
			}
			else
			{
				// there is no enough space
				// 1. clear and deallocate
				clear_and_deallocate();

				// 2. reserve
				reserve(count);
			}

			// copy
			std::uninitialized_copy(first, last, m_data);
			m_size = count;

			null_terminate();
		}

	protected:

		void range_check(size_type n) const
		{
			if (n >= size())
				throw std::out_of_range("SecretVector::range_check out of range.");
		}

		// Do nothing if new_cap == m_capacity || new_cap < m_size
		void relocate(size_type new_cap)
		{
			if (new_cap != m_capacity && // If we need a different capacity
				new_cap >= m_size)       // If new capacity is enough to hold current data
			{
				// allocate new space
				pointer new_data = allocate(new_cap);

				try
				{
					Internal::uninitialized_move(m_data, m_data + m_size, new_data);
				}
				catch(...)
				{
					// already moved data is destroyed by uninitialized_move
					// deallocate the new memory region
					deallocate(new_data, new_cap);

					// re-throw exception
					throw;
				}

				// destroy invlid moved data in old space
				Internal::destroy(m_data, m_data + m_size);
				// deallocate old space
				deallocate(m_data, m_capacity);

				// Set new context
				m_data = new_data;
				m_capacity = new_cap;
			}
		}

		size_type get_recommend_cap(size_type add) const
		{
			const size_type cap        = capacity();
			const size_type ms         = max_size(); // NOTE: max_size counted sk_extra_cap
			const size_type new_size   = m_size + add;

			if (new_size > ms)
			{
				throw std::length_error("SecretVector::get_recommend_cap new capacity exceed the max_size().");
			}
			if (new_size <= cap)
			{
				return cap;
			}
			if (cap >= ms / 2)
			{
				return ms;
			}
			return (std::max<size_type>)((2 * cap), new_size);
		}

	private:

		allocator_type m_alloc;
		size_type      m_capacity;
		pointer        m_data;
		size_type      m_size;

	private:

		void noalloc_move_insert_one(iterator pos, value_type&& arg)
		{
			Internal::uninitialized_move(end() - 1, end(), m_data + m_size);
			++m_size;
			std::move_backward(pos.base(), m_data + m_size - 2, m_data + m_size - 1);
			*pos = std::forward<value_type>(arg);
		}

		void noalloc_fill_insert(iterator pos, const value_type& value, size_type count)
		{
			if (count != 0)
			{
				const size_type elems_after = end() - pos;
				const size_type old_size = m_size;

				if (elems_after > count)
				{
					Internal::uninitialized_move(m_data + m_size - count, m_data + m_size, m_data + m_size);
					m_size += count;
					std::move_backward(pos.base(), m_data + old_size - count, m_data + old_size);
					std::fill(pos.base(), pos.base() + count, value);
				}
				else
				{
					std::uninitialized_fill_n(m_data + m_size, count - elems_after, value);
					m_size += (count - elems_after);

					Internal::uninitialized_move(pos.base(), m_data + old_size, m_data + m_size);
					m_size += elems_after;

					std::fill(pos.base(), m_data + old_size, value);
				}
			}
		}

		template<typename _InputIt,
			typename std::enable_if<!std::is_same<typename std::iterator_traits<_InputIt>::value_type, void>::value, int>::type = 0>
		void noalloc_copy_insert(iterator pos, _InputIt src_first, _InputIt src_last, size_type elems_count)
		{
			if (elems_count != 0)
			{
				const size_type elems_after = end() - pos;
				const size_type old_size = m_size;

				if (elems_after > elems_count)
				{
					Internal::uninitialized_move(end() - elems_count, end(), m_data + m_size);
					m_size += elems_count;
					std::move_backward(pos.base(), m_data + old_size - elems_count, m_data + old_size);
					std::copy(src_first, src_last, pos);
				}
				else
				{
					_InputIt src_first_ele_after = src_first;
					std::advance(src_first_ele_after, elems_after);
					std::uninitialized_copy(src_first_ele_after, src_last, m_data + m_size);
					m_size += (elems_count - elems_after);

					Internal::uninitialized_move(pos.base(), m_data + old_size, m_data + m_size);
					m_size += elems_after;

					std::copy(src_first, src_first_ele_after, pos);
				}
			}
		}

		void realloc_move_insert(size_type elems_before, pointer src_first, size_type elems_count)
		{
			const size_type new_cap = get_recommend_cap(elems_count);
			pointer new_data = allocate(new_cap);

			// Move new elems
			try
			{
				Internal::uninitialized_move(src_first, src_first + elems_count, new_data + elems_before);
			}
			catch(...)
			{
				// Deallocate
				deallocate(new_data, new_cap);

				// Rethrow
				throw;
			}

			//Move front part
			try
			{
				Internal::uninitialized_move(begin(), begin() + elems_before, new_data);
			}
			catch(...)
			{
				// Failed. Destroy already moved elems
				Internal::destroy(new_data + elems_before, new_data + elems_before + elems_count); // destroy middle part

				// Deallocate
				deallocate(new_data, new_cap);

				// Rethrow
				throw;
			}

			//Move tail part
			try
			{
				Internal::uninitialized_move(begin() + elems_before, end(), new_data + elems_before + elems_count);
			}
			catch(...)
			{
				// Failed. Destroy already moved elems
				Internal::destroy(new_data, new_data + elems_before + elems_count); // destroy front + middle part

				// Deallocate
				deallocate(new_data, new_cap);

				// Rethrow
				throw;
			}

			Internal::destroy(begin(), end());
			deallocate(m_data, m_capacity);
			m_data = new_data;
			m_size += elems_count;
			m_capacity = new_cap;
		}

		// overlap is OK, since source will be copied to the new space first.
		void realloc_copy_insert(size_type elems_before, const_pointer src_first, size_type elems_count)
		{
			const size_type new_cap = get_recommend_cap(elems_count);
			pointer new_data = allocate(new_cap);

			// Move new elems
			try
			{
				std::uninitialized_copy(src_first, src_first + elems_count, new_data + elems_before);
			}
			catch(...)
			{
				// Deallocate
				deallocate(new_data, new_cap);

				// Rethrow
				throw;
			}

			//Move front part
			try
			{
				Internal::uninitialized_move(begin(), begin() + elems_before, new_data);
			}
			catch(...)
			{
				// Failed. Destroy already moved elems
				Internal::destroy(new_data + elems_before, new_data + elems_before + elems_count); // destroy middle part

				// Deallocate
				deallocate(new_data, new_cap);

				// Rethrow
				throw;
			}

			//Move tail part
			try
			{
				Internal::uninitialized_move(begin() + elems_before, end(), new_data + elems_before + elems_count);
			}
			catch(...)
			{
				// Failed. Destroy already moved elems
				Internal::destroy(new_data, new_data + elems_before + elems_count); // destroy front + middle part

				// Deallocate
				deallocate(new_data, new_cap);

				// Rethrow
				throw;
			}

			Internal::destroy(begin(), end());
			deallocate(m_data, m_capacity);
			m_data = new_data;
			m_size += elems_count;
			m_capacity = new_cap;
		}

		void realloc_fill_insert(size_type elems_before, const value_type& value, size_type elems_count)
		{
			const size_type new_cap = get_recommend_cap(elems_count);
			pointer new_data = allocate(new_cap);

			// Fill new elems
			try
			{
				std::uninitialized_fill(new_data + elems_before, new_data + elems_before + elems_count, value);
			}
			catch(...)
			{
				// Deallocate
				deallocate(new_data, new_cap);

				// Rethrow
				throw;
			}

			//Move front part
			try
			{
				Internal::uninitialized_move(begin(), begin() + elems_before, new_data);
			}
			catch(...)
			{
				// Failed. Destroy already moved elems
				Internal::destroy(new_data + elems_before, new_data + elems_before + elems_count); // destroy middle part

				// Deallocate
				deallocate(new_data, new_cap);

				// Rethrow
				throw;
			}

			//Move tail part
			try
			{
				Internal::uninitialized_move(begin() + elems_before, end(), new_data + elems_before + elems_count);
			}
			catch(...)
			{
				// Failed. Destroy already moved elems
				Internal::destroy(new_data, new_data + elems_before + elems_count); // destroy front + middle part

				// Deallocate
				deallocate(new_data, new_cap);

				// Rethrow
				throw;
			}

			Internal::destroy(begin(), end());
			deallocate(m_data, m_capacity);
			m_data = new_data;
			m_size += elems_count;
			m_capacity = new_cap;
		}

		void erase_at_end(pointer pos) noexcept
		{
			if (size_type remove_count = m_data + m_size - pos)
			{
				Internal::destroy(pos, m_data + m_size);
				m_size -= remove_count;
			}
		}
	};

	template<typename _ValType, typename _Alloc,
		enable_if_t<IsCTypeAlike<_ValType>::value, int> = 0>
	bool operator==(const SecretVector<_ValType, _Alloc>& lhs, const SecretVector<_ValType, _Alloc>& rhs)
	{
		using vec_type = SecretVector<_ValType, _Alloc>;
		return vec_type::SafeCompareEqual(lhs.data(), lhs.size(), rhs.data(), rhs.size());
	}

	template<typename _ValType, typename _Alloc,
		enable_if_t<IsCTypeAlike<_ValType>::value, int> = 0>
	bool operator!=(const SecretVector<_ValType, _Alloc>& lhs, const SecretVector<_ValType, _Alloc>& rhs)
	{
		using vec_type = SecretVector<_ValType, _Alloc>;
		return vec_type::SafeCompareNotEqual(lhs.data(), lhs.size(), rhs.data(), rhs.size());
	}

	template<typename _ValType, typename _Alloc, typename _std_Alloc,
		enable_if_t<IsCTypeAlike<_ValType>::value, int> = 0>
	bool operator==(const SecretVector<_ValType, _Alloc>& lhs, const std::vector<_ValType, _std_Alloc>& rhs)
	{
		using vec_type = SecretVector<_ValType, _Alloc>;
		return vec_type::SafeCompareEqual(lhs.data(), lhs.size(), rhs.data(), rhs.size());
	}

	template<typename _ValType, typename _Alloc, typename _std_Alloc,
		enable_if_t<IsCTypeAlike<_ValType>::value, int> = 0>
	bool operator!=(const SecretVector<_ValType, _Alloc>& lhs, const std::vector<_ValType, _std_Alloc>& rhs)
	{
		using vec_type = SecretVector<_ValType, _Alloc>;
		return vec_type::SafeCompareNotEqual(lhs.data(), lhs.size(), rhs.data(), rhs.size());
	}

	template<typename _ValType, typename _Alloc, typename _std_Alloc,
		enable_if_t<IsCTypeAlike<_ValType>::value, int> = 0>
	bool operator==(const std::vector<_ValType, _std_Alloc>& lhs, const SecretVector<_ValType, _Alloc>& rhs)
	{
		using vec_type = SecretVector<_ValType, _Alloc>;
		return vec_type::SafeCompareEqual(lhs.data(), lhs.size(), rhs.data(), rhs.size());
	}

	template<typename _ValType, typename _Alloc, typename _std_Alloc,
		enable_if_t<IsCTypeAlike<_ValType>::value, int> = 0>
	bool operator!=(const std::vector<_ValType, _std_Alloc>& lhs, const SecretVector<_ValType, _Alloc>& rhs)
	{
		using vec_type = SecretVector<_ValType, _Alloc>;
		return vec_type::SafeCompareNotEqual(lhs.data(), lhs.size(), rhs.data(), rhs.size());
	}
}
