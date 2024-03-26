/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#pragma once

#include <type_traits>

#include <QDebug>
#include <QString>
#include <QByteArray>
#include <utility>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <string_view>

namespace Qt_fmt
{
namespace detail
{

template<typename T, template<typename...> typename Primary>
struct is_specialization_of : std::false_type
{
};

template<template<typename...> typename Primary, typename... Args>
struct is_specialization_of<Primary<Args...>, Primary> : std::true_type
{
};

} // namespace detail

// Offering this as a customization point for users who might want to
// define both QDebug streaming and fmt formatters, so they need a way
// to shut down this path.
//
// Note: keeping this in sync between fmt and QDebug sounds like a
// nightmare.
// clang-format off
template<typename T, typename Enable = void>
struct exclude_from_qdebug_fmt
    : std::disjunction<std::is_fundamental<T>,
                       // QByteArray is a thorn in the side.
                       // fmt handles automatically types that convert to const char *,
                       // but not types that convert to const void *. QByteArray by default
                       // converts to both; so be careful about it here.
                       std::conjunction<
                           std::is_convertible<T, const void *>,
                           std::negation<std::is_same<std::remove_cv_t<T>, QByteArray>>
                       >,
                       std::conjunction<
                           std::is_convertible<T, const char *>,
                           std::negation<std::is_same<std::remove_cv_t<T>, QByteArray>>
                       >,
                       // Re-include char-arrays, since the above would exclude them
                       std::conjunction<
                           std::is_array<T>,
                           std::is_same<std::remove_cv_t<T>, char>
                       >,
                       detail::is_specialization_of<T, std::basic_string>,
                       detail::is_specialization_of<T, std::basic_string_view>,
                       // fmt doesn't necessarily offer these as builtins, but let's be conservative
                       detail::is_specialization_of<T, std::pair>,
                       detail::is_specialization_of<T, std::tuple>,
                       detail::is_specialization_of<T, std::vector>,
                       detail::is_specialization_of<T, std::list>,
                       detail::is_specialization_of<T, std::map>,
                       detail::is_specialization_of<T, std::multimap>,
                       detail::is_specialization_of<T, std::set>,
                       detail::is_specialization_of<T, std::multiset>,
                       detail::is_specialization_of<T, std::unordered_map>,
                       detail::is_specialization_of<T, std::unordered_set>,
                       detail::is_specialization_of<T, std::unordered_multimap>,
                       detail::is_specialization_of<T, std::unordered_multiset>
>
{
};
// clang-format on

} // namespace Qt_fmt

