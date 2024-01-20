/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>

  SPDX-License-Identifier: MIT
*/

#ifndef KDTOOLBOX_RANGES_TOCONTAINER_H
#define KDTOOLBOX_RANGES_TOCONTAINER_H

#include <iterator>
#include <type_traits>

namespace KDToolBox
{
namespace Ranges
{
namespace Private
{

// C++17 has these, but we limit ourselves to C++14
#ifdef __cpp_lib_void_t
using std::void_t;
#else
template<typename...>
using void_t = void;
#endif

#ifdef __cpp_lib_logical_traits
using std::conjunction;
#else
template<typename...>
struct conjunction : std::true_type
{
};
template<typename B>
struct conjunction<B> : B
{
};
template<typename B, typename... Bn>
struct conjunction<B, Bn...> : std::conditional_t<bool(B::value), conjunction<Bn...>, B>
{
};
#endif

// Miscellanea type traits used below to SFINAE our functions/operators.
//
// WARNING. None of these traits are
// 1. 100% accurate
// 2. matching what std::ranges do
// 3. particularly optimized to make compilation faster and reduce instantiations
//
// In a nutshell, they're "good enough".
// If you want the real deal, use ranges-v3 and its to() function.

template<typename T, typename Enable = void>
struct IsRange : std::false_type
{
};

template<typename T>
struct IsRange<
    T, void_t<
           // it's got begin()
           decltype(std::begin(std::declval<const T &>())),

           // begin() yields an input iterator
           std::enable_if_t<std::is_convertible<
               typename std::iterator_traits<decltype(std::begin(std::declval<const T &>()))>::iterator_category,
               std::input_iterator_tag>::value>,

           // it's got end()
           decltype(std::end(std::declval<const T &>())),

           // begin() != end() is OK and convertible to bool
           std::enable_if_t<std::is_convertible<
               decltype(std::begin(std::declval<const T &>()) != std::end(std::declval<const T &>())), bool>::value>>>
    : std::true_type
{
};

template<typename Range>
using RangeValueT = typename std::iterator_traits<decltype(std::begin(std::declval<const Range &>()))>::value_type;

template<typename Container, typename Range>
using IsContainerConstructibleFromRange =
    std::is_constructible<Container, decltype(std::begin(std::declval<const Range &>())),
                          decltype(std::end(std::declval<const Range &>()))>;

// There are two implementations: one for container classes and one for
// container class templates. The code is pretty much identical.
//
// We happily assume the container will reserve() in its
// constructor if it detects a forward iterator. This wouldn't be
// acceptable in general, but see above, we're aiming for "good enough"
// here.

// Pipeline operators

template<typename Container>
struct ToContainerDummyPipelineParameter;

struct ToContainerDummyPipelineParameterBase
{
    template<typename Range, typename Container,
             std::enable_if_t<conjunction<IsRange<Range>, IsContainerConstructibleFromRange<Container, Range>>::value,
                              bool> = true>
    friend constexpr auto operator|(const Range &r, ToContainerDummyPipelineParameter<Container>)
    {
        return Container(std::begin(r), std::end(r));
    }
};

template<typename Container>
struct ToContainerDummyPipelineParameter : ToContainerDummyPipelineParameterBase
{
};

template<template<typename...> typename C>
struct ToContainerTemplateDummyPipelineParameter;

struct ToContainerTemplateDummyPipelineParameterBase
{
    template<typename Range, template<typename...> class C,
             std::enable_if_t<
                 conjunction<IsRange<Range>, IsContainerConstructibleFromRange<C<RangeValueT<Range>>, Range>>::value,
                 bool> = true>
    friend constexpr auto operator|(const Range &r, ToContainerTemplateDummyPipelineParameter<C>)
    {
        return C<RangeValueT<Range>>(std::begin(r), std::end(r));
    }
};

template<template<typename...> typename C>
struct ToContainerTemplateDummyPipelineParameter : ToContainerTemplateDummyPipelineParameterBase
{
};

} // namespace Private

// *** PUBLIC API ***

// Pipeline objects

template<typename Container>
constexpr auto kdToContainer() noexcept
{
    return Private::ToContainerDummyPipelineParameter<Container>{};
}

template<template<typename...> class C>
constexpr auto kdToContainer() noexcept
{
    return Private::ToContainerTemplateDummyPipelineParameter<C>{};
}

// Converting functions

template<typename Container, typename Range, std::enable_if_t<Private::IsRange<Range>::value, bool> = true,
         std::enable_if_t<Private::IsContainerConstructibleFromRange<Container, Range>::value, bool> = true>
constexpr auto kdToContainer(const Range &r)
{
    return Container(std::begin(r), std::end(r));
}

template<template<typename...> class C, typename Range, std::enable_if_t<Private::IsRange<Range>::value, bool> = true,
         std::enable_if_t<Private::IsContainerConstructibleFromRange<C<Private::RangeValueT<Range>>, Range>::value,
                          bool> = true>
constexpr auto kdToContainer(const Range &r)
{
    return C<Private::RangeValueT<Range>>(std::begin(r), std::end(r));
}

} // namespace Ranges
} // namespace KDToolBox

#endif // KDTOOLBOX_RANGES_TOCONTAINER_H
