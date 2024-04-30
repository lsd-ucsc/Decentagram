// Copyright (c) 2022 SimpleJson
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <algorithm>
#include <functional>
#include <vector>

#include "Internal/SimpleObjects.hpp"

#include "Utils.hpp"
#include "WriterConfig.hpp"

#ifndef SIMPLEJSON_CUSTOMIZED_NAMESPACE
namespace SimpleJson
#else
namespace SIMPLEJSON_CUSTOMIZED_NAMESPACE
#endif
{

template<
	typename _KeyWriter,
	typename _ObjWriter,
	typename _ToStringType,
	typename _ContainerType,
	typename _DictTraits>
struct JsonWriterDictBase
{

	template<typename _OutputIt>
	inline static void Write(_OutputIt destIt,
		const typename _DictTraits::DictBase& obj,
		const WriterConfig& config,
		const WriterStates& state)
	{
		*destIt++ = '{';

		WriterStates stateNextLevel = state;
		++(stateNextLevel.m_nestLevel);
		size_t len = obj.size();

		if (config.m_indent.size() > 0 && obj.size() > 0)
		{
			Internal::RepeatOutput(destIt, config.m_lineEnd, 1);
		}

		for(auto it = obj.cbegin(); it != obj.cend(); ++it, --len)
		{
			if (config.m_indent.size() > 0)
			{
				Internal::RepeatOutput(destIt,
					config.m_indent, stateNextLevel.m_nestLevel);
			}

			_KeyWriter::Write(destIt,
				_DictTraits::GetKey(*it), config, stateNextLevel);

			if (config.m_indent.size() > 0)
			{
				*destIt++ = ' ';
				*destIt++ = ':';
				*destIt++ = ' ';
			}
			else
			{
				*destIt++ = ':';
			}

			_ObjWriter::Write(destIt,
				_DictTraits::GetVal(*it), config, stateNextLevel);

			if (len != 1)
			{
				*destIt++ = ',';
			}

			if (config.m_indent.size() > 0)
			{
				Internal::RepeatOutput(destIt, config.m_lineEnd, 1);
			}
		}

		if (config.m_indent.size() > 0 && obj.size() > 0)
		{
			Internal::RepeatOutput(destIt, config.m_indent, state.m_nestLevel);
		}
		*destIt++ = '}';
	}

}; // struct JsonWriterDictBase

template<
	typename _KeyWriter,
	typename _ObjWriter,
	typename _ToStringType,
	typename _DictTraits>
struct JsonWriterDynOrderedDictBase
{

	template<typename _OutputIt>
	inline static void Write(_OutputIt destIt,
		const typename _DictTraits::DictBase& obj,
		const WriterConfig& config,
		const WriterStates& state)
	{
		using _KeyType = typename _DictTraits::key_type;
		std::vector<std::reference_wrapper<const _KeyType> > keys;

		// Get all keys and sort them
		keys.reserve(obj.size());
		for(auto it = obj.cbegin(); it != obj.cend(); ++it)
		{
			keys.emplace_back(_DictTraits::GetKey(*it));
		}
		std::sort(keys.begin(), keys.end(), std::less<_KeyType>());

		// Write the dict by the sorted keys
		*destIt++ = '{';

		WriterStates stateNextLevel = state;
		++(stateNextLevel.m_nestLevel);
		size_t len = obj.size();

		if (config.m_indent.size() > 0 && obj.size() > 0)
		{
			Internal::RepeatOutput(destIt, config.m_lineEnd, 1);
		}

		for(auto it = keys.cbegin(); it != keys.cend(); ++it, --len)
		{
			const auto& key = it->get();

			if (config.m_indent.size() > 0)
			{
				Internal::RepeatOutput(destIt,
					config.m_indent, stateNextLevel.m_nestLevel);
			}

			_KeyWriter::Write(destIt, key, config, stateNextLevel);

			if (config.m_indent.size() > 0)
			{
				*destIt++ = ' ';
				*destIt++ = ':';
				*destIt++ = ' ';
			}
			else
			{
				*destIt++ = ':';
			}

			const auto& val = obj[key];
			_ObjWriter::Write(destIt, val, config, stateNextLevel);

			if (len != 1)
			{
				*destIt++ = ',';
			}

			if (config.m_indent.size() > 0)
			{
				Internal::RepeatOutput(destIt, config.m_lineEnd, 1);
			}
		}

		if (config.m_indent.size() > 0 && obj.size() > 0)
		{
			Internal::RepeatOutput(destIt, config.m_indent, state.m_nestLevel);
		}
		*destIt++ = '}';
	}

}; // struct JsonWriterDynOrderedDictBase

template<typename _ToStringType>
struct DynamicDictTraits
{
	using DictBase =
		typename Internal::Obj::BaseObject<_ToStringType>::DictBase;

	typedef typename DictBase::key_type               key_type;
	typedef typename DictBase::mapped_type            mapped_type;

	typedef typename DictBase::key_iterator           key_iterator;
	typedef typename DictBase::const_mapped_iterator  const_mapped_iterator;

	typedef std::tuple<key_iterator, const_mapped_iterator>  value_type;

	static const key_type& GetKey(const value_type& it)
	{
		return *std::get<0>(it);
	}

	static const mapped_type& GetVal(const value_type& it)
	{
		return *std::get<1>(it);
	}

}; // struct DynamicDictTraits

template<typename _ToStringType>
struct StaticDictTraits
{
	using DictBase =
		typename Internal::Obj::BaseObject<_ToStringType>::StatDictBase;

	typedef typename DictBase::key_type               key_type;
	typedef typename DictBase::mapped_type            mapped_type;

	typedef typename DictBase::key_const_ref_type     key_const_ref_type;
	typedef typename DictBase::mapped_const_ref_type  mapped_const_ref_type;

	typedef std::pair<const key_const_ref_type, const mapped_const_ref_type>
		value_type;

	static const key_type& GetKey(const value_type& it)
	{
		return it.first.get();
	}

	static const mapped_type& GetVal(const value_type& it)
	{
		return it.second.get();
	}

}; // struct StaticDictTraits

template<
	typename _KeyWriter,
	typename _ObjWriter,
	typename _ToStringType,
	typename _ContainerType>
struct JsonWriterDictImpl :
	public JsonWriterDictBase<
		_KeyWriter,
		_ObjWriter,
		_ToStringType,
		_ContainerType,
		DynamicDictTraits<_ToStringType> >
{
}; // struct JsonWriterDictImpl

template<
	typename _KeyWriter,
	typename _ObjWriter,
	typename _ToStringType>
struct JsonWriterOrdDictImpl :
	public JsonWriterDynOrderedDictBase<
		_KeyWriter,
		_ObjWriter,
		_ToStringType,
		DynamicDictTraits<_ToStringType> >
{
}; // struct JsonWriterOrdDictImpl

template<
	typename _KeyWriter,
	typename _ObjWriter,
	typename _ToStringType,
	typename _ContainerType>
struct JsonWriterStaticDictImpl :
	public JsonWriterDictBase<
		_KeyWriter,
		_ObjWriter,
		_ToStringType,
		_ContainerType,
		StaticDictTraits<_ToStringType> >
{
}; // struct JsonWriterStaticDictImpl

} // namespace SimpleJson
