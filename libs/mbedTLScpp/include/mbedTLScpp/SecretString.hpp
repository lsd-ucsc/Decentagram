#pragma once

#include <string>

#include "SecretVector.hpp"

#include "LoadedFunctions.hpp"

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	template<typename _CharType,
		typename _CharTraits = std::char_traits<_CharType>,
		typename _Alloc = SecretAllocator<_CharType> >
	class SecretBasicString : public SecretVector<_CharType, _Alloc, true>
	{
	public: // Types:

		// inherited
		typedef  SecretVector<_CharType, _Alloc, true>  _Base;

		typedef  typename _Base::value_type       value_type;
		typedef  typename _Base::allocator_type   allocator_type;
		typedef  typename _Base::allocator_traits allocator_traits;
		typedef  typename _Base::size_type        size_type;
		typedef  typename _Base::difference_type  difference_type;
		typedef  typename _Base::reference        reference;
		typedef  typename _Base::const_reference  const_reference;
		typedef  typename _Base::pointer          pointer;
		typedef  typename _Base::const_pointer    const_pointer;

		typedef  typename _Base::iterator                iterator;
		typedef  typename _Base::const_iterator          const_iterator;
		typedef  typename _Base::const_reverse_iterator  const_reverse_iterator;
		typedef  typename _Base::reverse_iterator        reverse_iterator;

		// specific to string
		typedef _CharTraits          traits_type;

		static constexpr size_type npos = static_cast<size_type>(-1);

	public:
		SecretBasicString() :
			_Base::SecretVector()
		{}

		SecretBasicString(const allocator_type& alloc) :
			_Base::SecretVector(alloc)
		{}

		SecretBasicString(size_type count, value_type ch,
				const allocator_type& alloc = allocator_type()) :
			_Base(count, ch, alloc)
		{}

		SecretBasicString(const SecretBasicString& other,
				size_type pos,
				const allocator_type& alloc = allocator_type()) :
			_Base::SecretVector(other.begin() + pos, other.end(), alloc)
		{}

		SecretBasicString(const SecretBasicString& other,
				size_type pos,
				size_type count,
				const allocator_type& alloc = allocator_type()) :
			_Base::SecretVector(
				pos <= other.size() ? other.begin() + pos : throw std::out_of_range("In SecretBasicString::SecretBasicString, position exceeds string total size."),
				(count == npos || (pos + count) > other.size()) ? other.end() : other.begin() + pos + count,
				alloc)
		{}

		SecretBasicString(const value_type* s,
				size_type count,
				const allocator_type& alloc = allocator_type()) :
			_Base::SecretVector(s, s + count, alloc)
		{}

		SecretBasicString(const value_type* s,
				const allocator_type& alloc = allocator_type()) :
			_Base::SecretVector(s, s + traits_type::length(s), alloc)
		{}

		template<class _InputIterator,
			typename std::enable_if<!std::is_same<typename std::iterator_traits<_InputIterator>::value_type, void>::value, int>::type = 0>
		SecretBasicString(_InputIterator first, _InputIterator last,
				const allocator_type& alloc = allocator_type()) :
			_Base::SecretVector(first, last, alloc)
		{}

		SecretBasicString(const SecretBasicString& other) :
			_Base::SecretVector(other)
		{}

		SecretBasicString(const SecretBasicString& other, const allocator_type& alloc) :
			_Base::SecretVector(other, alloc)
		{}

		SecretBasicString(SecretBasicString&& other) noexcept :
			_Base::SecretVector(std::forward<_Base>(other))
		{}

		SecretBasicString(SecretBasicString&& other, const allocator_type& alloc) :
			_Base::SecretVector(std::forward<_Base>(other), alloc)
		{}

		SecretBasicString(std::initializer_list<value_type> init_list,
			const allocator_type& alloc = allocator_type()) :
			_Base::SecretVector(init_list.begin(), init_list.end(), alloc)
		{}

		// LCOV_EXCL_START
		virtual ~SecretBasicString() = default;
		// LCOV_EXCL_STOP

		SecretBasicString& operator=( const SecretBasicString& other )
		{
			_Base::operator=(other);
			return *this;
		}

		SecretBasicString& operator=( SecretBasicString&& other ) noexcept
		{
			_Base::operator=(std::forward<_Base>(other));
			return *this;
		}

		const_pointer c_str() const noexcept
		{
			return _Base::data();
		}

		SecretBasicString& append(size_type count, value_type ch)
		{
			_Base::insert(_Base::end(), count, ch);
			return *this;
		}

		SecretBasicString& append(const SecretBasicString& str)
		{
			_Base::insert(_Base::end(), str.begin(), str.end());
			return *this;
		}

		SecretBasicString& append(const SecretBasicString& str, size_type pos, size_type count = npos)
		{
			const auto first = str.begin() + pos;
			const auto last  = (count == npos || (pos + count) > str.size()) ? str.end() : str.begin() + pos + count;
			_Base::insert(_Base::end(), first, last);
			return *this;
		}

		SecretBasicString& append(const value_type* s, size_type count)
		{
			const auto first = s;
			const auto last  = s + count;
			_Base::insert(_Base::end(), first, last);
			return *this;
		}

		SecretBasicString& append(const value_type* s)
		{
			const auto first = s;
			const auto last  = s + traits_type::length(s);
			_Base::insert(_Base::end(), first, last);
			return *this;
		}

		template<class _InputIterator,
			typename std::enable_if<!std::is_same<typename std::iterator_traits<_InputIterator>::value_type, void>::value, int>::type = 0>
		SecretBasicString& append(_InputIterator first, _InputIterator last)
		{
			_Base::insert(_Base::end(), first, last);
			return *this;
		}

		SecretBasicString& append(std::initializer_list<value_type> ilist)
		{
			_Base::insert(_Base::end(), ilist.begin(), ilist.end());
			return *this;
		}

		SecretBasicString& operator+=(const SecretBasicString& str)
		{
			return append(str);
		}

		SecretBasicString& operator+=(value_type ch)
		{
			_Base::push_back(ch);
			return *this;
		}

		SecretBasicString& operator+=(const value_type* s)
		{
			return append(s);
		}

		SecretBasicString& operator+=(std::initializer_list<value_type> ilist)
		{
			_Base::insert(_Base::end(), ilist.begin(), ilist.end());
			return *this;
		}

		SecretBasicString& replace(size_type pos, size_type count, const SecretBasicString& str)
		{
			// replace(size_type pos, size_type count, const value_type* cstr, size_type count2);
			return replace(pos, count, str.data(), str.size());
		}

		SecretBasicString& replace(const_iterator first, const_iterator last, const SecretBasicString& str)
		{
			// replace(const_iterator first, const_iterator last, const value_type* cstr, size_type count2);
			return replace(first, last, str.data(), str.size());
		}

		SecretBasicString& replace(size_type pos, size_type count, const SecretBasicString& str, size_type pos2, size_type count2 = npos)
		{
			// replace(size_type pos, size_type count, const value_type* cstr, size_type count2);

			if (pos2 > str.size())
			{
				throw std::out_of_range("In SecretBasicString::replace, position exceeds string total size.");
			}
			const size_type elems_after2 = str.size() - pos2;
			count2 = (elems_after2 < count2) ? elems_after2 : count2;

			return replace(pos, count, str.data() + pos2, count2);
		}

		template<class _InputIterator,
			typename std::enable_if<!std::is_same<typename std::iterator_traits<_InputIterator>::value_type, void>::value, int>::type = 0>
		SecretBasicString& replace(const_iterator first, const_iterator last, _InputIterator first2, _InputIterator last2)
		{
			if (!(_Base::begin() <= first && first <= last && last <= _Base::end()))
			{
				throw std::invalid_argument("In SecretBasicString::replace, the iterator range given is outside of the range.");
			}

			const size_type pos   = static_cast<size_type>(first - _Base::begin());
			const size_type count = static_cast<size_type>(last - first);

			iterator d_first = _Base::begin() + pos;
			iterator d_last  = _Base::begin() + pos + count;

			return replace_no_check(d_first, d_last, first2, last2);
		}

		SecretBasicString& replace(size_type pos, size_type count, const value_type* cstr, size_type count2)
		{
			if (pos > _Base::size())
			{
				throw std::out_of_range("In SecretBasicString::replace, position exceeds string total size.");
			}
			const size_type elems_after = _Base::size() - pos;
			count = (elems_after < count) ? elems_after : count;

			iterator d_first = _Base::begin() + pos;
			iterator d_last  = _Base::begin() + pos + count;

			const value_type* s_first = cstr;
			const value_type* s_last  = cstr + count2;

			return replace_no_check(d_first, d_last, s_first, s_last);
		}

		SecretBasicString& replace(const_iterator first, const_iterator last, const value_type* cstr, size_type count2)
		{
			if (!(_Base::begin() <= first && first <= last && last <= _Base::end()))
			{
				throw std::invalid_argument("In SecretBasicString::replace, the iterator range given is outside of the range.");
			}

			const size_type pos   = static_cast<size_type>(first - _Base::begin());
			const size_type count = static_cast<size_type>(last - first);

			iterator d_first = _Base::begin() + pos;
			iterator d_last  = _Base::begin() + pos + count;

			const value_type* s_first = cstr;
			const value_type* s_last  = cstr + count2;

			return replace_no_check(d_first, d_last, s_first, s_last);
		}

		SecretBasicString& replace(size_type pos, size_type count, const value_type* cstr)
		{
			// replace(size_type pos, size_type count, const value_type* cstr, size_type count2);
			return replace(pos, count, cstr, traits_type::length(cstr));
		}

		SecretBasicString& replace(const_iterator first, const_iterator last, const value_type* cstr)
		{
			// replace(const_iterator first, const_iterator last, const value_type* cstr, size_type count2);
			return replace(first, last, cstr, traits_type::length(cstr));
		}

		SecretBasicString& replace(const_iterator first, const_iterator last, std::initializer_list<value_type> ilist)
		{
			// replace(const_iterator first, const_iterator last, _InputIterator first2, _InputIterator last2);
			return replace(first, last, ilist.begin(), ilist.end());
		}

		SecretBasicString& replace(size_type pos, size_type count, size_type count2, value_type ch)
		{
			if (pos > _Base::size())
			{
				throw std::out_of_range("In SecretBasicString::replace, position exceeds string total size.");
			}
			const size_type elems_after = _Base::size() - pos;
			count = (elems_after < count) ? elems_after : count;

			iterator d_first = _Base::begin() + pos;
			iterator d_last  = _Base::begin() + pos + count;

			return replace_no_check(d_first, d_last, count2, ch);
		}

		SecretBasicString& replace(const_iterator first, const_iterator last, size_type count2, value_type ch)
		{
			if (!(_Base::begin() <= first && first <= last && last <= _Base::end()))
			{
				throw std::invalid_argument("In SecretBasicString::replace, the iterator range given is outside of the range.");
			}

			const size_type pos   = static_cast<size_type>(first - _Base::begin());
			const size_type count = static_cast<size_type>(last - first);

			iterator d_first = _Base::begin() + pos;
			iterator d_last  = _Base::begin() + pos + count;

			return replace_no_check(d_first, d_last, count2, ch);
		}

		// @throw  std::out_of_range  If __pos > size().
		SecretBasicString substr(size_type pos = 0, size_type count = npos) const
		{
			if (pos > _Base::size())
			{
				throw std::out_of_range("In SecretBasicString::substr, position exceeds string total size.");
			}
			return SecretBasicString(*this, pos, count);
		}

		// @throw  std::out_of_range  If __pos > size().
		size_type copy(value_type* dest, size_type count, size_type pos = 0) const
		{
			if (pos > _Base::size())
			{
				throw std::out_of_range("In SecretBasicString::copy, position exceeds string total size.");
			}

			const_pointer src = _Base::data() + pos;
			const size_type elems_after = _Base::size() - pos;
			count = (elems_after < count) ? elems_after : count;

			if(count)
			{
				// per std: The resulting character string is not null-terminated.
				if (count == 1)
				{
					traits_type::assign(*dest, *src);
				}
				else
				{
					traits_type::copy(dest, src, count);
				}
			}

			return count;
		}

		SecretBasicString& insert(size_type index, size_type count, value_type ch)
		{
			return replace(index, size_type(0), count, ch);
		}

		SecretBasicString& insert(size_type index, const value_type* s)
		{
			return replace(index, size_type(0), s, traits_type::length(s));
		}

		SecretBasicString& insert(size_type index, const value_type* s, size_type count)
		{
			return replace(index, size_type(0), s, count);
		}

		SecretBasicString& insert(size_type index, const SecretBasicString& str)
		{
			return replace(index, size_type(0), str.data(), str.size());
		}

		SecretBasicString& insert(size_type index, const SecretBasicString& str, size_type index_str, size_type count = npos)
		{
			if (index_str > str.size())
			{
				throw std::out_of_range("In SecretBasicString::insert, position exceeds string total size.");
			}

			const size_type elems_after2 = str.size() - index_str;
			count = (elems_after2 < count) ? elems_after2 : count;

			return replace(index, size_type(0), str.data() + index_str, count);
		}


		// NOTE: The following functions (but not limited to) are not implemented yet,
		// since regular implementation could cause side channel leak.
		//
		// int compare(const SecretBasicString& str ) const noexcept;
		// int compare(size_type pos1, size_type count1, const SecretBasicString& str) const;
		// int compare(size_type pos1, size_type count1, const SecretBasicString& str, size_type pos2, size_type count2 = npos) const;
		// int compare(const value_type* s) const;
		// int compare(size_type pos1, size_type count1, const value_type* s) const;
		// int compare(size_type pos1, size_type count1, const value_type* s, size_type count2) const;

		// constexpr bool starts_with(value_type c) const noexcept;
		// constexpr bool starts_with(const value_type* s) const;
		// constexpr bool ends_with(value_type c) const noexcept;
		// constexpr bool ends_with(const value_type* s) const;

	private:

		template<class _InputIterator,
			typename std::enable_if<!std::is_same<typename std::iterator_traits<_InputIterator>::value_type, void>::value, int>::type = 0>
		SecretBasicString& replace_no_check(iterator d_first, iterator d_last, _InputIterator s_first, _InputIterator s_last)
		{
			SecretBasicString str(s_first, s_last);
			auto tmp_s_first = str.begin();
			const auto tmp_s_last  = str.end();

			for(; d_first != d_last && tmp_s_first != tmp_s_last; ++d_first, ++tmp_s_first)
			{
				traits_type::assign(*d_first, *tmp_s_first);
			}
			// At this point, either d_first == d_last, or tmp_s_first == tmp_s_last, or both.

			if (d_first != d_last)
			{
				// tmp_s_first == tmp_s_last
				// all the source string has been consumed.

				_Base::erase(d_first, d_last);
			}

			if (tmp_s_first != tmp_s_last)
			{
				// d_first == d_last
				// there are still str needed to be inserted

				_Base::insert(d_last, tmp_s_first, tmp_s_last);
			}

			return *this;
		}

		SecretBasicString& replace_no_check(iterator d_first, iterator d_last, size_type s_count, value_type s_ch)
		{
			for(; d_first != d_last && s_count > 0; ++d_first, --s_count)
			{
				traits_type::assign(*d_first, s_ch);
			}
			// At this point, either d_first == d_last, or s_count == 0, or both.

			if (d_first != d_last)
			{
				// s_count == 0
				// all s_ch have been filled in

				_Base::erase(d_first, d_last);
			}

			if (s_count > 0)
			{
				// d_first == d_last
				// there are still s_ch needed to be filled

				_Base::insert(d_last, s_count, s_ch);
			}

			return *this;
		}

	};

	template<typename _CharType, typename _CharTraits, typename _Alloc>
	SecretBasicString<_CharType, _CharTraits, _Alloc> operator+(
			const SecretBasicString<_CharType, _CharTraits, _Alloc>& lhs,
			const SecretBasicString<_CharType, _CharTraits, _Alloc>& rhs)
	{
		SecretBasicString<_CharType, _CharTraits, _Alloc> str(lhs);
		str.append(rhs);
		return str;
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc>
	SecretBasicString<_CharType, _CharTraits, _Alloc> operator+(
			const SecretBasicString<_CharType, _CharTraits, _Alloc>& lhs,
			const _CharType* rhs)
	{
		SecretBasicString<_CharType, _CharTraits, _Alloc> str(lhs);
		str.append(rhs);
		return str;
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc>
	SecretBasicString<_CharType, _CharTraits, _Alloc> operator+(
			const SecretBasicString<_CharType, _CharTraits, _Alloc>& lhs,
			_CharType rhs)
	{
		SecretBasicString<_CharType, _CharTraits, _Alloc> str(lhs);
		str.append(1, rhs);
		return str;
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc>
	SecretBasicString<_CharType, _CharTraits, _Alloc> operator+(
			const _CharType* lhs,
			const SecretBasicString<_CharType, _CharTraits, _Alloc>& rhs)
	{
		using str_type = SecretBasicString<_CharType, _CharTraits, _Alloc>;
		using size_type = typename str_type::size_type;

		const size_type len = str_type::traits_type::length(lhs);
		str_type str;
		str.reserve(len + rhs.size());
		str.append(lhs, len);
		str.append(rhs);
		return str;
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc>
	SecretBasicString<_CharType, _CharTraits, _Alloc> operator+(
			_CharType lhs,
			const SecretBasicString<_CharType, _CharTraits, _Alloc>& rhs)
	{
		using str_type = SecretBasicString<_CharType, _CharTraits, _Alloc>;
		using size_type = typename str_type::size_type;

		str_type str;
		const size_type len = rhs.size();
		str.reserve(len + 1);
		str.append(size_type(1), lhs);
		str.append(rhs);
		return str;
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc>
	SecretBasicString<_CharType, _CharTraits, _Alloc> operator+(
			SecretBasicString<_CharType, _CharTraits, _Alloc>&& lhs,
			SecretBasicString<_CharType, _CharTraits, _Alloc>&& rhs)
	{
		return std::move(
			lhs.append(
				std::make_move_iterator(rhs.begin()), std::make_move_iterator(rhs.end())));
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc>
	SecretBasicString<_CharType, _CharTraits, _Alloc> operator+(
			SecretBasicString<_CharType, _CharTraits, _Alloc>&& lhs,
			const SecretBasicString<_CharType, _CharTraits, _Alloc>& rhs)
	{
		return std::move(lhs.append(rhs));
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc>
	SecretBasicString<_CharType, _CharTraits, _Alloc> operator+(
			SecretBasicString<_CharType, _CharTraits, _Alloc>&& lhs,
			const _CharType* rhs)
	{
		return std::move(lhs.append(rhs));
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc>
	SecretBasicString<_CharType, _CharTraits, _Alloc> operator+(
			SecretBasicString<_CharType, _CharTraits, _Alloc>&& lhs,
			_CharType rhs)
	{
		return std::move(lhs.append(1, rhs));
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc>
	SecretBasicString<_CharType, _CharTraits, _Alloc> operator+(
			const SecretBasicString<_CharType, _CharTraits, _Alloc>& lhs,
			SecretBasicString<_CharType, _CharTraits, _Alloc>&& rhs)
	{
		return std::move(rhs.insert(0, lhs));
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc>
	SecretBasicString<_CharType, _CharTraits, _Alloc> operator+(
			const _CharType* lhs,
			SecretBasicString<_CharType, _CharTraits, _Alloc>&& rhs)
	{
		return std::move(rhs.insert(0, lhs));
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc>
	SecretBasicString<_CharType, _CharTraits, _Alloc> operator+(
			_CharType lhs,
			SecretBasicString<_CharType, _CharTraits, _Alloc>&& rhs)
	{
		return std::move(rhs.insert(0, 1, lhs));
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc>
	bool operator==(const SecretBasicString<_CharType, _CharTraits, _Alloc>& lhs,
					const SecretBasicString<_CharType, _CharTraits, _Alloc>& rhs)
	{
		using str_type = SecretBasicString<_CharType, _CharTraits, _Alloc>;
		return str_type::SafeCompareEqual(lhs.data(), lhs.size(), rhs.data(), rhs.size());
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc>
	bool operator!=(const SecretBasicString<_CharType, _CharTraits, _Alloc>& lhs,
					const SecretBasicString<_CharType, _CharTraits, _Alloc>& rhs)
	{
		using str_type = SecretBasicString<_CharType, _CharTraits, _Alloc>;
		return str_type::SafeCompareNotEqual(lhs.data(), lhs.size(), rhs.data(), rhs.size());
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc>
	bool operator==(const SecretBasicString<_CharType, _CharTraits, _Alloc>& lhs,
					const _CharType* rhs)
	{
		using str_type = SecretBasicString<_CharType, _CharTraits, _Alloc>;
		return str_type::SafeCompareEqual(lhs.data(), lhs.size(), rhs, str_type::traits_type::length(rhs));
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc>
	bool operator!=(const SecretBasicString<_CharType, _CharTraits, _Alloc>& lhs,
					const _CharType* rhs)
	{
		using str_type = SecretBasicString<_CharType, _CharTraits, _Alloc>;
		return str_type::SafeCompareNotEqual(lhs.data(), lhs.size(), rhs, str_type::traits_type::length(rhs));
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc>
	bool operator==(const _CharType* lhs,
					const SecretBasicString<_CharType, _CharTraits, _Alloc>& rhs)
	{
		using str_type = SecretBasicString<_CharType, _CharTraits, _Alloc>;
		return str_type::SafeCompareEqual(lhs, str_type::traits_type::length(lhs), rhs.data(), rhs.size());
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc>
	bool operator!=(const _CharType* lhs,
					const SecretBasicString<_CharType, _CharTraits, _Alloc>& rhs)
	{
		using str_type = SecretBasicString<_CharType, _CharTraits, _Alloc>;
		return str_type::SafeCompareNotEqual(lhs, str_type::traits_type::length(lhs), rhs.data(), rhs.size());
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc, typename _std_CharTraits, typename _std_Alloc>
	bool operator==(const SecretBasicString<_CharType, _CharTraits, _Alloc>& lhs,
					const std::basic_string<_CharType, _std_CharTraits, _std_Alloc>& rhs)
	{
		using str_type = SecretBasicString<_CharType, _CharTraits, _Alloc>;
		return str_type::SafeCompareEqual(lhs.data(), lhs.size(), rhs.data(), rhs.size());
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc, typename _std_CharTraits, typename _std_Alloc>
	bool operator!=(const SecretBasicString<_CharType, _CharTraits, _Alloc>& lhs,
					const std::basic_string<_CharType, _std_CharTraits, _std_Alloc>& rhs)
	{
		using str_type = SecretBasicString<_CharType, _CharTraits, _Alloc>;
		return str_type::SafeCompareNotEqual(lhs.data(), lhs.size(), rhs.data(), rhs.size());
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc, typename _std_CharTraits, typename _std_Alloc>
	bool operator==(const std::basic_string<_CharType, _std_CharTraits, _std_Alloc>& lhs,
					const SecretBasicString<_CharType, _CharTraits, _Alloc>& rhs)
	{
		using str_type = SecretBasicString<_CharType, _CharTraits, _Alloc>;
		return str_type::SafeCompareEqual(lhs.data(), lhs.size(), rhs.data(), rhs.size());
	}

	template<typename _CharType, typename _CharTraits, typename _Alloc, typename _std_CharTraits, typename _std_Alloc>
	bool operator!=(const std::basic_string<_CharType, _std_CharTraits, _std_Alloc>& lhs,
					const SecretBasicString<_CharType, _CharTraits, _Alloc>& rhs)
	{
		using str_type = SecretBasicString<_CharType, _CharTraits, _Alloc>;
		return str_type::SafeCompareNotEqual(lhs.data(), lhs.size(), rhs.data(), rhs.size());
	}

	using SecretString = SecretBasicString<char>;
}
