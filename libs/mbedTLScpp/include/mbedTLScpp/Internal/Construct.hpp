#pragma once

#include <memory>

#ifndef MBEDTLSCPP_CUSTOMIZED_NAMESPACE
namespace mbedTLScpp
#else
namespace MBEDTLSCPP_CUSTOMIZED_NAMESPACE
#endif
{
	namespace Internal
	{

		/**
		 * @brief destroy_at
		 *
		 */
		template<class T>
#if __cplusplus >= 202002L
		constexpr
#endif
		void destroy_at(T* p)
		{
#if __cplusplus >= 201703L
			std::destroy_at(p);
#else
			p->~T();
#endif
		}

		/**
		 * @brief destroy
		 *
		 */
		template< class ForwardIt >
#if __cplusplus >= 202002L
		constexpr
#endif
		void destroy( ForwardIt first, ForwardIt last )
		{
#if __cplusplus >= 201703L
			std::destroy(first, last);
#else
			for (; first != last; ++first)
				destroy_at(std::addressof(*first));
#endif
		}

		/**
		 * @brief uninitialized_move
		 *
		 */
		template<class InputIt, class ForwardIt>
		ForwardIt uninitialized_move(InputIt first, InputIt last, ForwardIt d_first)
		{
			typedef typename std::iterator_traits<ForwardIt>::value_type Value;
			ForwardIt current = d_first;
			try
			{
				for (; first != last; ++first, (void) ++current)
				{
					::new (static_cast<void*>(std::addressof(*current))) Value(std::move(*first));
				}
				return current;
			}
			catch (...)
			{
				destroy(d_first, current);
				throw;
			}
		}

		/**
		 * @brief uninitialized_default_construct
		 *
		 */
		template<class ForwardIt>
		void uninitialized_default_construct(ForwardIt first, ForwardIt last)
		{
			using Value = typename std::iterator_traits<ForwardIt>::value_type;
			ForwardIt current = first;
			try
			{
				for (; current != last; ++current)
				{
					::new (static_cast<void*>(std::addressof(*current))) Value;
				}
			}
			catch (...)
			{
				destroy(first, current);
				throw;
			}
		}
	}
}
