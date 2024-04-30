// Copyright 2022 SimpleObjects
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#ifndef SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
namespace SimpleObjects
#else
namespace SIMPLEOBJECTS_CUSTOMIZED_NAMESPACE
#endif
{

enum class ObjCategory
{
	Null,
	Bool,
	Integer,
	Real,
	String,
	List,
	Dict,
	StaticDict,
	Bytes,
};

enum class RealNumType
{
	Bool,
	Int8,
	Int16,
	Int32,
	Int64,
	UInt8,
	UInt16,
	UInt32,
	UInt64,
	Float,
	Double,
	// All enum type above are RESERVED for RealNumImpl<> class ONLY

	Other,  // for future extensions, use this
};





namespace Internal
{

template<ObjCategory _Cat>
struct CategoryTraits;

template<>
struct CategoryTraits<ObjCategory::Null>
{
	static constexpr ObjCategory sk_cat()
	{ return ObjCategory::Null; }

	static constexpr const char* sk_name()
	{ return "Null"; }
}; // struct CategoryTraits<ObjCategory::Null>

template<>
struct CategoryTraits<ObjCategory::Bool>
{
	static constexpr ObjCategory sk_cat()
	{ return ObjCategory::Bool; }

	static constexpr const char* sk_catName()
	{ return "Bool";  }
}; // struct CategoryTraits<ObjCategory::Bool>

template<>
struct CategoryTraits<ObjCategory::Integer>
{
	static constexpr ObjCategory sk_cat()
	{ return ObjCategory::Integer; }

	static constexpr const char* sk_catName()
	{ return "Integer";  }
}; // struct CategoryTraits<ObjCategory::Integer>

template<>
struct CategoryTraits<ObjCategory::Real>
{
	static constexpr ObjCategory sk_cat()
	{ return ObjCategory::Real; }

	static constexpr const char* sk_catName()
	{ return "Real";  }
}; // struct CategoryTraits<ObjCategory::Real>


template<>
struct CategoryTraits<ObjCategory::String>
{
	static constexpr ObjCategory sk_cat()
	{ return ObjCategory::String; }

	static constexpr const char* sk_catName()
	{ return "String";  }
}; // struct CategoryTraits<ObjCategory::String>

template<>
struct CategoryTraits<ObjCategory::List>
{
	static constexpr ObjCategory sk_cat()
	{ return ObjCategory::List; }

	static constexpr const char* sk_catName()
	{ return "List";  }
}; // struct CategoryTraits<ObjCategory::List>

template<>
struct CategoryTraits<ObjCategory::Dict>
{
	static constexpr ObjCategory sk_cat()
	{ return ObjCategory::Dict; }

	static constexpr const char* sk_catName()
	{ return "Dict";  }
}; // struct CategoryTraits<ObjCategory::Dict>

template<>
struct CategoryTraits<ObjCategory::StaticDict>
{
	static constexpr ObjCategory sk_cat()
	{ return ObjCategory::StaticDict; }

	static constexpr const char* sk_catName()
	{ return "StaticDict";  }
}; // struct CategoryTraits<ObjCategory::StaticDict>

template<>
struct CategoryTraits<ObjCategory::Bytes>
{
	static constexpr ObjCategory sk_cat()
	{ return ObjCategory::Bytes; }

	static constexpr const char* sk_catName()
	{ return "Bytes";  }
}; // struct CategoryTraits<ObjCategory::Bytes>

} // namespace Internal

} // namespace SimpleObjects
