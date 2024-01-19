/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "tst_qt_hasher.h"
#include "../qt_hasher.h"

#include <QFont>
#include <QRegularExpression>
#include <QTest>

#include <unordered_map>
#include <unordered_set>

struct Foo
{
    int i;
};

void tst_QtHasher::hash()
{
    std::unordered_map<QString, Foo, KDToolBox::QtHasher<QString>> stringMap;
    stringMap.insert({QStringLiteral("123"), {123}});
    stringMap.insert({QStringLiteral("456"), {456}});
    stringMap.insert({QStringLiteral("789"), {789}});
    QCOMPARE(stringMap.size(), std::size_t(3));

    stringMap.insert({QStringLiteral("123"), {0}});
    QCOMPARE(stringMap.size(), std::size_t(3));

    stringMap.clear();
    QCOMPARE(stringMap.size(), std::size_t(0));

    constexpr int LOOPS = 195044;
    for (int i = 0; i < LOOPS; ++i)
        stringMap.insert({QString::number(i), {i}});

    QCOMPARE(stringMap.size(), std::size_t(LOOPS));

    for (const auto &v : stringMap)
        QCOMPARE(v.first.toInt(), v.second.i);

    std::unordered_set<QRegularExpression, KDToolBox::QtHasher<QRegularExpression>> regexpSet;

    regexpSet.insert(QRegularExpression(QStringLiteral("a.*b")));
    regexpSet.insert(QRegularExpression(QStringLiteral("a|b")));
    QCOMPARE(regexpSet.size(), std::size_t(2));
    QVERIFY(regexpSet.find(QRegularExpression(QStringLiteral("a|b"))) != regexpSet.end());
    QVERIFY(regexpSet.find(QRegularExpression(QStringLiteral("a+b"))) == regexpSet.end());

    regexpSet.insert(QRegularExpression(QStringLiteral("a+b")));
    QCOMPARE(regexpSet.size(), std::size_t(3));
    QVERIFY(regexpSet.find(QRegularExpression(QStringLiteral("a|b"))) != regexpSet.end());
    QVERIFY(regexpSet.find(QRegularExpression(QStringLiteral("a+b"))) != regexpSet.end());

    regexpSet.erase(QRegularExpression());
    QCOMPARE(regexpSet.size(), std::size_t(3));

    regexpSet.erase(QRegularExpression(QStringLiteral("a|b")));
    QCOMPARE(regexpSet.size(), std::size_t(2));
    QVERIFY(regexpSet.find(QRegularExpression(QStringLiteral("a|b"))) == regexpSet.end());
    QVERIFY(regexpSet.find(QRegularExpression(QStringLiteral("a+b"))) != regexpSet.end());

    std::unordered_set<QUrl, KDToolBox::QtHasher<QUrl>> urlSet;
    urlSet.insert(QUrl(QStringLiteral("https://www.kdab.com")));
    urlSet.insert(QUrl(QStringLiteral("https://www.qt.io")));
    QCOMPARE(urlSet.size(), std::size_t(2));

    urlSet.insert(QUrl(QStringLiteral("https://isocpp.org")));
    QCOMPARE(urlSet.size(), std::size_t(3));

    QVERIFY(urlSet.find(QUrl(QStringLiteral("https://www.kdab.com"))) != urlSet.end());
    QVERIFY(urlSet.find(QUrl(QStringLiteral("https://www.google.com"))) == urlSet.end());
}

namespace TestNS
{
struct Hashable
{
    int i;
};

using QHashIntReturnType = decltype(qHash(0));

QHashIntReturnType qHash(Hashable h) noexcept
{
    return QT_PREPEND_NAMESPACE(qHash)(h.i);
}

struct HashableHiddenFriend
{
    int i;
    friend QHashIntReturnType qHash(HashableHiddenFriend h) noexcept { return QT_PREPEND_NAMESPACE(qHash)(h.i); }
};
} // namespace TestNS

void tst_QtHasher::poison()
{
    Q_STATIC_ASSERT(std::is_default_constructible<KDToolBox::QtHasher<QString>>::value);
    Q_STATIC_ASSERT(std::is_default_constructible<KDToolBox::QtHasher<QUrl>>::value);
    Q_STATIC_ASSERT(std::is_default_constructible<KDToolBox::QtHasher<QFont>>::value);
    Q_STATIC_ASSERT(!std::is_default_constructible<KDToolBox::QtHasher<Foo>>::value);             // no qHash(Foo)
    Q_STATIC_ASSERT(std::is_default_constructible<KDToolBox::QtHasher<TestNS::Hashable>>::value); // found through ADL
    Q_STATIC_ASSERT(
        std::is_default_constructible<KDToolBox::QtHasher<TestNS::HashableHiddenFriend>>::value); // found through ADL
}

QTEST_MAIN(tst_QtHasher)
