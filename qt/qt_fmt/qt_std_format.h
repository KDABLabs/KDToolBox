/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#pragma once

#if __cplusplus < 202002L
#error "This header requires C++20"
#endif

#include <format>
#include <QDebug>
#include "qt_fmt_helpers.h"

namespace Qt_fmt::detail
{
template<typename T>
concept IsFormattableViaQDebug = requires(QDebug &d, const T &t)
{
    d << t;
    requires !Qt_fmt::exclude_from_qdebug_fmt<T>::value;
};
}

template<typename T>
    requires Qt_fmt::detail::IsFormattableViaQDebug<T>
struct std::formatter<T, char>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        auto it = ctx.begin();
        auto end = ctx.end();
        if (it != end && *it != '}')
            throw std::format_error("Only {} is supported");
        return it;
    }

    template<typename FormatContext>
    auto format(const T &t, FormatContext &ctx) const
    {
        // See the comment in the libfmt formatter
        QString buffer;
        QDebug debug(&buffer);
        debug.noquote().nospace() << t;
        return std::format_to(ctx.out(), "{}", buffer.toStdString());
    }
};
