// Copyright (c) 2022 SimpleUtf
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include "Utf8.hpp"
#include "Utf16.hpp"
#include "Utf32.hpp"

#ifndef SIMPLEUTF_CUSTOMIZED_NAMESPACE
namespace SimpleUtf
#else
namespace SIMPLEUtf_CUSTOMIZED_NAMESPACE
#endif
{

template<typename InBoundFunc, typename OutBoundFunc, typename InputIt, typename OutputIt>
inline InputIt UtfConvertOnce(InBoundFunc inFunc, OutBoundFunc outFunc,
	InputIt begin, InputIt end,
	OutputIt dest)
{
	auto codePtRes = inFunc(begin, end);
	outFunc(codePtRes.first, dest);
	return codePtRes.second;
}

template<typename InBoundFunc, typename OutBoundFunc, typename InputIt, typename OutputIt>
inline void UtfConvert(InBoundFunc inFunc, OutBoundFunc outFunc,
	InputIt begin, InputIt end,
	OutputIt dest)
{
	while (begin != end)
	{
		begin = UtfConvertOnce(inFunc, outFunc, begin, end, dest);
	}
}

template<typename InBoundFunc, typename OutBoundFunc, typename InputIt>
inline std::pair<size_t, InputIt> UtfConvertOnceGetSize(InBoundFunc inFunc, OutBoundFunc outFunc,
	InputIt begin, InputIt end)
{
	auto codePtRes = inFunc(begin, end);
	size_t size = outFunc(codePtRes.first);
	return std::make_pair(
		size,
		codePtRes.second
	);
}

template<typename InBoundFunc, typename OutBoundFunc, typename InputIt>
inline size_t UtfConvertGetSize(InBoundFunc inFunc, OutBoundFunc outFunc,
	InputIt begin, InputIt end)
{
	size_t size = 0;
	while (begin != end)
	{
		size_t tmp = 0;
		std::tie(tmp, begin) = UtfConvertOnceGetSize(inFunc, outFunc, begin, end);
		size += tmp;
	}
	return size;
}


// ==========  UTF-8 --> UTF-16

template<typename InputIt, typename OutputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 1>::value, int> = 0>
inline void Utf8ToUtf16(InputIt begin, InputIt end, OutputIt dest)
{
	return UtfConvert(Utf8ToCodePtOnce<InputIt>, CodePtToUtf16Once<OutputIt>,
		begin, end, dest);
}

inline std::u16string Utf8ToUtf16(const std::string& utf8)
{
	std::u16string resUtfStr;

	Utf8ToUtf16(utf8.begin(), utf8.end(), std::back_inserter(resUtfStr));

	return resUtfStr;
}

template<typename InputIt, typename OutputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 1>::value, int> = 0>
inline InputIt Utf8ToUtf16Once(InputIt begin, InputIt end, OutputIt dest)
{
	return UtfConvertOnce(Utf8ToCodePtOnce<InputIt>, CodePtToUtf16Once<OutputIt>,
		begin, end, dest);
}

template<typename InputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 1>::value, int> = 0>
inline std::pair<size_t, InputIt> Utf8ToUtf16OnceGetSize(InputIt begin, InputIt end)
{
	return UtfConvertOnceGetSize(Utf8ToCodePtOnce<InputIt>, CodePtToUtf16OnceGetSize,
		begin, end);
}

template<typename InputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 1>::value, int> = 0>
inline size_t Utf8ToUtf16GetSize(InputIt begin, InputIt end)
{
	return UtfConvertGetSize(Utf8ToCodePtOnce<InputIt>, CodePtToUtf16OnceGetSize,
		begin, end);
}

// ==========  UTF-8 --> UTF-32

template<typename InputIt, typename OutputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 1>::value, int> = 0>
inline void Utf8ToUtf32(InputIt begin, InputIt end, OutputIt dest)
{
	return UtfConvert(Utf8ToCodePtOnce<InputIt>, CodePtToUtf32Once<OutputIt>,
		begin, end, dest);
}

inline std::u32string Utf8ToUtf32(const std::string& utf8)
{
	std::u32string resUtfStr;

	Utf8ToUtf32(utf8.begin(), utf8.end(), std::back_inserter(resUtfStr));

	return resUtfStr;
}

template<typename InputIt, typename OutputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 1>::value, int> = 0>
inline InputIt Utf8ToUtf32Once(InputIt begin, InputIt end, OutputIt dest)
{
	return UtfConvertOnce(Utf8ToCodePtOnce<InputIt>, CodePtToUtf32Once<OutputIt>,
		begin, end, dest);
}

template<typename InputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 1>::value, int> = 0>
inline std::pair<size_t, InputIt> Utf8ToUtf32OnceGetSize(InputIt begin, InputIt end)
{
	return UtfConvertOnceGetSize(Utf8ToCodePtOnce<InputIt>, CodePtToUtf32OnceGetSize,
		begin, end);
}

template<typename InputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 1>::value, int> = 0>
inline size_t Utf8ToUtf32GetSize(InputIt begin, InputIt end)
{
	return UtfConvertGetSize(Utf8ToCodePtOnce<InputIt>, CodePtToUtf32OnceGetSize,
		begin, end);
}

// ==========  UTF-16 --> UTF-8

template<typename InputIt, typename OutputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 2>::value, int> = 0>
inline void Utf16ToUtf8(InputIt begin, InputIt end, OutputIt dest)
{
	return UtfConvert(Utf16ToCodePtOnce<InputIt>, CodePtToUtf8Once<OutputIt>,
		begin, end, dest);
}

inline std::string Utf16ToUtf8(const std::u16string& in)
{
	std::string resUtfStr;

	Utf16ToUtf8(in.begin(), in.end(), std::back_inserter(resUtfStr));

	return resUtfStr;
}

template<typename InputIt, typename OutputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 2>::value, int> = 0>
inline InputIt Utf16ToUtf8Once(InputIt begin, InputIt end, OutputIt dest)
{
	return UtfConvertOnce(Utf16ToCodePtOnce<InputIt>, CodePtToUtf8Once<OutputIt>,
		begin, end, dest);
}

template<typename InputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 2>::value, int> = 0>
inline std::pair<size_t, InputIt> Utf16ToUtf8OnceGetSize(InputIt begin, InputIt end)
{
	return UtfConvertOnceGetSize(Utf16ToCodePtOnce<InputIt>, CodePtToUtf8OnceGetSize,
		begin, end);
}

template<typename InputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 2>::value, int> = 0>
inline size_t Utf16ToUtf8GetSize(InputIt begin, InputIt end)
{
	return UtfConvertGetSize(Utf16ToCodePtOnce<InputIt>, CodePtToUtf8OnceGetSize,
		begin, end);
}

// ==========  UTF-16 --> UTF-32

template<typename InputIt, typename OutputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 2>::value, int> = 0>
inline void Utf16ToUtf32(InputIt begin, InputIt end, OutputIt dest)
{
	return UtfConvert(Utf16ToCodePtOnce<InputIt>, CodePtToUtf32Once<OutputIt>,
		begin, end, dest);
}

inline std::u32string Utf16ToUtf32(const std::u16string& in)
{
	std::u32string resUtfStr;

	Utf16ToUtf32(in.begin(), in.end(), std::back_inserter(resUtfStr));

	return resUtfStr;
}

template<typename InputIt, typename OutputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 2>::value, int> = 0>
inline InputIt Utf16ToUtf32Once(InputIt begin, InputIt end, OutputIt dest)
{
	return UtfConvertOnce(Utf16ToCodePtOnce<InputIt>, CodePtToUtf32Once<OutputIt>,
		begin, end, dest);
}

template<typename InputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 2>::value, int> = 0>
inline std::pair<size_t, InputIt> Utf16ToUtf32OnceGetSize(InputIt begin, InputIt end)
{
	return UtfConvertOnceGetSize(Utf16ToCodePtOnce<InputIt>, CodePtToUtf32OnceGetSize,
		begin, end);
}

template<typename InputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 2>::value, int> = 0>
inline size_t Utf16ToUtf32GetSize(InputIt begin, InputIt end)
{
	return UtfConvertGetSize(Utf16ToCodePtOnce<InputIt>, CodePtToUtf32OnceGetSize,
		begin, end);
}

// ==========  UTF-32 --> UTF-8

template<typename InputIt, typename OutputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 4>::value, int> = 0>
inline void Utf32ToUtf8(InputIt begin, InputIt end, OutputIt dest)
{
	return UtfConvert(Utf32ToCodePtOnce<InputIt>, CodePtToUtf8Once<OutputIt>,
		begin, end, dest);
}

inline std::string Utf32ToUtf8(const std::u32string& in)
{
	std::string resUtfStr;

	Utf32ToUtf8(in.begin(), in.end(), std::back_inserter(resUtfStr));

	return resUtfStr;
}

template<typename InputIt, typename OutputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 4>::value, int> = 0>
inline InputIt Utf32ToUtf8Once(InputIt begin, InputIt end, OutputIt dest)
{
	return UtfConvertOnce(Utf32ToCodePtOnce<InputIt>, CodePtToUtf8Once<OutputIt>,
		begin, end, dest);
}

template<typename InputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 4>::value, int> = 0>
inline std::pair<size_t, InputIt> Utf32ToUtf8OnceGetSize(InputIt begin, InputIt end)
{
	return UtfConvertOnceGetSize(Utf32ToCodePtOnce<InputIt>, CodePtToUtf8OnceGetSize,
		begin, end);
}

template<typename InputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 4>::value, int> = 0>
inline size_t Utf32ToUtf8GetSize(InputIt begin, InputIt end)
{
	return UtfConvertGetSize(Utf32ToCodePtOnce<InputIt>, CodePtToUtf8OnceGetSize,
		begin, end);
}

// ==========  UTF-32 --> UTF-16

template<typename InputIt, typename OutputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 4>::value, int> = 0>
inline void Utf32ToUtf16(InputIt begin, InputIt end, OutputIt dest)
{
	return UtfConvert(Utf32ToCodePtOnce<InputIt>, CodePtToUtf16Once<OutputIt>,
		begin, end, dest);
}

inline std::u16string Utf32ToUtf16(const std::u32string& in)
{
	std::u16string resUtfStr;

	Utf32ToUtf16(in.begin(), in.end(), std::back_inserter(resUtfStr));

	return resUtfStr;
}

template<typename InputIt, typename OutputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 4>::value, int> = 0>
inline InputIt Utf32ToUtf16Once(InputIt begin, InputIt end, OutputIt dest)
{
	return UtfConvertOnce(Utf32ToCodePtOnce<InputIt>, CodePtToUtf16Once<OutputIt>,
		begin, end, dest);
}

template<typename InputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 4>::value, int> = 0>
inline std::pair<size_t, InputIt> Utf32ToUtf16OnceGetSize(InputIt begin, InputIt end)
{
	return UtfConvertOnceGetSize(Utf32ToCodePtOnce<InputIt>, CodePtToUtf16OnceGetSize,
		begin, end);
}

template<typename InputIt,
	Internal::EnableIfT<
		Internal::CanTHold<Internal::ItValType<InputIt>, 4>::value, int> = 0>
inline size_t Utf32ToUtf16GetSize(InputIt begin, InputIt end)
{
	return UtfConvertGetSize(Utf32ToCodePtOnce<InputIt>, CodePtToUtf16OnceGetSize,
		begin, end);
}

} // namespace SimpleUtf
