/****************************************************************************
**                                MIT License
**
** Copyright (C) 2019-2022 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
**
** This file is part of KDToolBox (https://github.com/KDAB/KDToolBox).
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, ** and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice (including the next paragraph)
** shall be included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF ** CONTRACT, TORT OR OTHERWISE,
** ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
****************************************************************************/

#include "../qt_fmt.h"

#include <QTest>
#include <QDebug>
#include <QRegularExpression>

#include <QDateTime>
#include <QUuid>
#include <QRect>
#include <QSize>

class QtFmtTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testQtFmt();
};

// Custom class streamable via QDebug
class CustomClass {};
QDebug operator<<(QDebug d, CustomClass) {
    return d << "CustomClass()";
}

// Custom class streamable via fmt and QDebug
class BothFmtAndQDebugClass {};
template <>
struct fmt::formatter<BothFmtAndQDebugClass, char> {
    constexpr auto parse(fmt::format_parse_context &ctx)
    {
        auto it = ctx.begin();
        auto end = ctx.end();
        if (it != end && *it != '}')
            throw fmt::format_error("Only {} is supported");
        return it;
    }
    template <typename FormatContext>
    auto format(const BothFmtAndQDebugClass &, FormatContext &ctx)
    {
        return fmt::format_to(ctx.out(), "BothFmtAndQDebugClass via fmt");
    }
};

QDebug operator<<(QDebug d, BothFmtAndQDebugClass)
{
    return d << "BothFmtAndQDebugClass via QDebug";
}

template <>
struct Qt_fmt::exclude_from_qdebug_fmt<BothFmtAndQDebugClass> : std::true_type {};


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
    QCOMPARE(fmt::format("{}", QString("hello")), "hello");
    QCOMPARE(fmt::format("{}", QByteArray("hello")), "hello");
    QCOMPARE(fmt::format("{}", QDateTime(QDate(2000, 2, 29), QTime(12, 12, 12), Qt::UTC)), "QDateTime(2000-02-29 12:12:12.000 UTC Qt::UTC)");
    QCOMPARE(fmt::format("{}", QUuid()), "QUuid({00000000-0000-0000-0000-000000000000})");
    QCOMPARE(fmt::format("{}", QRect()), "QRect(0,0 0x0)");
    QCOMPARE(fmt::format("{}", QSizeF()), "QSizeF(-1, -1)");
    {
        QFile f;
        const QString result = QString::fromStdString(fmt::format("{}", &f)); // pointer to QObject
        QVERIFY(result.contains(QRegularExpression("^QFile\\(0x[0-9a-z]+\\)$")));
    }

    // Q_FLAG / Q_ENUM
    QCOMPARE(fmt::format("{}", Qt::Alignment(Qt::AlignLeft | Qt::AlignVCenter)), "QFlags<Qt::AlignmentFlag>(AlignLeading|AlignVCenter)");
    QCOMPARE(fmt::format("{}", Qt::AlignLeft), "Qt::AlignLeft");

    // User defined types
    QCOMPARE(fmt::format("{}", CustomClass()), "CustomClass()");
    QCOMPARE(fmt::format("{}", BothFmtAndQDebugClass()), "BothFmtAndQDebugClass via fmt");
}

QTEST_APPLESS_MAIN(QtFmtTest)

#include "tst_qt_fmt.moc"
