/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#pragma once

#include "qt_fmt_helpers.h"

#include <fmt/format.h>
#include <QDebug>

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
    auto format(const T &t, FormatContext &ctx) const
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
