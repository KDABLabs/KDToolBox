/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "../qt_std_format.h"

#include <QDebug>
#include <QRegularExpression>
#include <QTest>

#include <QDateTime>
#include <QRect>
#include <QSize>
#include <QUuid>

class QtStdFormatTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testQtStdFormat();
};

// Custom class streamable via QDebug
class CustomClass
{
};
QDebug operator<<(QDebug d, CustomClass)
{
    return d << "CustomClass()";
}

// Custom class streamable via std::formatBothStdAndQDebugClass and QDebug
class BothStdAndQDebugClass
{
};
template<>
struct std::formatter<BothStdAndQDebugClass, char>
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
    auto format(const BothStdAndQDebugClass &, FormatContext &ctx) const
    {
        return std::format_to(ctx.out(), "BothStdAndQDebugClass via std");
    }
};

QDebug operator<<(QDebug d, BothStdAndQDebugClass)
{
    return d << "BothStdAndQDebugClass via QDebug";
}

template<>
struct Qt_fmt::exclude_from_qdebug_fmt<BothStdAndQDebugClass> : std::true_type
{
};

void QtStdFormatTest::testQtStdFormat()
{
    // This is a bit sketchy because it depends on QDebug's format. Better than nothing?
    using namespace std::literals;

    // Fundamental / Standard Library types supported by std::format _and_ QDebug;
    // should all go through std::format
    QCOMPARE(std::format("{}", 42), "42");
    QCOMPARE(std::format("{}", 10.5), "10.5");
    QCOMPARE(std::format("{}", "hello"), "hello");
    QCOMPARE(std::format("{}", (const char *)"hello"), "hello");

    QCOMPARE(std::format("{}", "hello"s), "hello");
    QCOMPARE(std::format("{}", "hello"sv), "hello");

    // Qt types
    QCOMPARE(std::format("{}", QStringLiteral("hello")), "hello");
    QCOMPARE(std::format("{}", QByteArray("hello")), "hello");
    QCOMPARE(std::format("{}", QDateTime(QDate(2000, 2, 29), QTime(12, 12, 12), Qt::UTC)),
             "QDateTime(2000-02-29 12:12:12.000 UTC Qt::UTC)");
    QCOMPARE(std::format("{}", QUuid()), "QUuid({00000000-0000-0000-0000-000000000000})");
    QCOMPARE(std::format("{}", QRect()), "QRect(0,0 0x0)");
    QCOMPARE(std::format("{}", QSizeF()), "QSizeF(-1, -1)");

    // Q_FLAG / Q_ENUM
    QCOMPARE(std::format("{}", Qt::Alignment(Qt::AlignLeft | Qt::AlignVCenter)),
             "QFlags<Qt::AlignmentFlag>(AlignLeading|AlignVCenter)");
    QCOMPARE(std::format("{}", Qt::AlignLeft), "Qt::AlignLeft");

    // User defined types
    QCOMPARE(std::format("{}", CustomClass()), "CustomClass()");
    QCOMPARE(std::format("{}", BothStdAndQDebugClass()), "BothStdAndQDebugClass via std");
}

static_assert(Qt_fmt::detail::IsFormattableViaQDebug<QString>);
static_assert(Qt_fmt::detail::IsFormattableViaQDebug<QByteArray>);
static_assert(Qt_fmt::detail::IsFormattableViaQDebug<QDateTime>);
static_assert(Qt_fmt::detail::IsFormattableViaQDebug<QUuid>);
static_assert(Qt_fmt::detail::IsFormattableViaQDebug<QRect>);

QTEST_APPLESS_MAIN(QtStdFormatTest)

#include "tst_qt_std_format.moc"
