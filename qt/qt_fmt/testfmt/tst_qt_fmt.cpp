/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "../qt_fmt.h"

#include <QDebug>
#include <QRegularExpression>
#include <QTest>

#include <QDateTime>
#include <QRect>
#include <QSize>
#include <QUuid>

class QtFmtTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testQtFmt();
};

// Custom class streamable via QDebug
class CustomClass
{
};
QDebug operator<<(QDebug d, CustomClass)
{
    return d << "CustomClass()";
}

// Custom class streamable via fmt and QDebug
class BothFmtAndQDebugClass
{
};
template<>
struct fmt::formatter<BothFmtAndQDebugClass, char>
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
    auto format(const BothFmtAndQDebugClass &, FormatContext &ctx) const
    {
        return fmt::format_to(ctx.out(), "BothFmtAndQDebugClass via fmt");
    }
};

QDebug operator<<(QDebug d, BothFmtAndQDebugClass)
{
    return d << "BothFmtAndQDebugClass via QDebug";
}

template<>
struct Qt_fmt::exclude_from_qdebug_fmt<BothFmtAndQDebugClass> : std::true_type
{
};

void QtFmtTest::testQtFmt()
{
    // This is a bit sketchy because it depends on QDebug's format. Better than nothing?
    using namespace std::literals;

    // Fundamental / Standard Library types supported by libfmt _and_ QDebug;
    // should all go through fmt
    QCOMPARE(fmt::format("{}", 42), "42");
    QCOMPARE(fmt::format("{}", 10.5), "10.5");
    QCOMPARE(fmt::format("{}", "hello"), "hello");
    QCOMPARE(fmt::format("{}", (const char *)"hello"), "hello");

    QCOMPARE(fmt::format("{}", "hello"s), "hello");
    QCOMPARE(fmt::format("{}", "hello"sv), "hello");

    // Qt types
    QCOMPARE(fmt::format("{}", QStringLiteral("hello")), "hello");
    QCOMPARE(fmt::format("{}", QByteArray("hello")), "hello");
    QCOMPARE(fmt::format("{}", QDateTime(QDate(2000, 2, 29), QTime(12, 12, 12), Qt::UTC)),
             "QDateTime(2000-02-29 12:12:12.000 UTC Qt::UTC)");
    QCOMPARE(fmt::format("{}", QUuid()), "QUuid({00000000-0000-0000-0000-000000000000})");
    QCOMPARE(fmt::format("{}", QRect()), "QRect(0,0 0x0)");
    QCOMPARE(fmt::format("{}", QSizeF()), "QSizeF(-1, -1)");

    // Q_FLAG / Q_ENUM
    QCOMPARE(fmt::format("{}", Qt::Alignment(Qt::AlignLeft | Qt::AlignVCenter)),
             "QFlags<Qt::AlignmentFlag>(AlignLeading|AlignVCenter)");
    QCOMPARE(fmt::format("{}", Qt::AlignLeft), "Qt::AlignLeft");

    // User defined types
    QCOMPARE(fmt::format("{}", CustomClass()), "CustomClass()");
    QCOMPARE(fmt::format("{}", BothFmtAndQDebugClass()), "BothFmtAndQDebugClass via fmt");
}

static_assert(Qt_fmt::exclude_from_qdebug_fmt<int>::value);
static_assert(Qt_fmt::exclude_from_qdebug_fmt<const int>::value);
static_assert(Qt_fmt::exclude_from_qdebug_fmt<float>::value);
static_assert(Qt_fmt::exclude_from_qdebug_fmt<int *>::value);
static_assert(Qt_fmt::exclude_from_qdebug_fmt<const int *>::value);
static_assert(Qt_fmt::exclude_from_qdebug_fmt<char *>::value);
static_assert(Qt_fmt::exclude_from_qdebug_fmt<const char *>::value);
static_assert(Qt_fmt::exclude_from_qdebug_fmt<char[]>::value);
static_assert(Qt_fmt::exclude_from_qdebug_fmt<const char[]>::value);
static_assert(Qt_fmt::exclude_from_qdebug_fmt<char[42]>::value);
static_assert(Qt_fmt::exclude_from_qdebug_fmt<const char[42]>::value);
static_assert(Qt_fmt::exclude_from_qdebug_fmt<void *>::value);
static_assert(Qt_fmt::exclude_from_qdebug_fmt<const void *>::value);
static_assert(Qt_fmt::exclude_from_qdebug_fmt<QObject *>::value);
static_assert(Qt_fmt::exclude_from_qdebug_fmt<const QObject *>::value);
static_assert(!Qt_fmt::exclude_from_qdebug_fmt<QByteArray>::value);
static_assert(!Qt_fmt::exclude_from_qdebug_fmt<QString>::value);
static_assert(!Qt_fmt::exclude_from_qdebug_fmt<QList<int>>::value);
static_assert(!Qt_fmt::exclude_from_qdebug_fmt<QDateTime>::value);

QTEST_APPLESS_MAIN(QtFmtTest)

#include "tst_qt_fmt.moc"
