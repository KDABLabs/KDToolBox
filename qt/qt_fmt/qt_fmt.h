/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#pragma once

#include <fmt/format.h>

#include <type_traits>

#include <QDebug>
#include <QString>

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
                       // fmt doesn't necessarily offer these as builtins, but let's be conservative
                       detail::is_specialization_of<T, std::pair>,
                       detail::is_specialization_of<T, std::vector>,
                       detail::is_specialization_of<T, std::list>,
                       detail::is_specialization_of<T, std::map>,
                       detail::is_specialization_of<T, std::multimap>>
{
};
// clang-format on

} // namespace Qt_fmt

template<typename T>
struct fmt::formatter<T, char,
                      std::void_t<std::enable_if_t<!Qt_fmt::exclude_from_qdebug_fmt<T>::value>,
                                  decltype(std::declval<QDebug &>() << std::declval<const T &>())>>
{
    constexpr auto parse(fmt::format_parse_context &ctx)
    {
        auto it = ctx.begin();
        auto end = ctx.end();
        if (it != end && *it != '}')
            throw fmt::format_error("Only {} is supported");
        return it;
    }

    template<typename FormatContext>
    auto format(const T &t, FormatContext &ctx)
    {
        // This is really expensive (lots of allocations). Unfortunately
        // there isn't something as easy to do that also performs better.
        //
        // * storing buffer+debug as thread_local has reentrancy issues.
        //   Say you format an object of type A; this calls operator<<(QDebug d, A a).
        //   Inside that, you call d << a.b to stream a subobject of type B.
        //   operator<<(QDebug d, B b) might be implemented itself using fmt, e.g.
        //   via std::format(~~~, b.c, b.d, b.e).
        //   Now if any sub-object ends up being printed using *this*
        //   streaming (via QDebug), it won't work.
        //
        // * Using QByteArray + QBuffer to avoid the final conversion
        //   to UTF-8 doesn't really help, it'll replace it with a bunch
        //   of transient small allocations (as QDebug will convert each
        //   string streamed into it in UTF-8 and then append to its
        //   internal buffer)

        QString buffer;
        QDebug debug(&buffer);
        debug.noquote().nospace() << t;
        return fmt::format_to(ctx.out(), "{}", buffer.toStdString());
    }
};
