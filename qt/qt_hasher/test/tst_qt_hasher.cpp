/****************************************************************************
**                                MIT License
**
** Copyright (C) 2019-2020 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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

#include "tst_qt_hasher.h"
#include "../qt_hasher.h"

#include <QtTest>

#include <unordered_map>
#include <unordered_set>

struct Foo { int i; };

void tst_QtHasher::hash()
{
    std::unordered_map<QString, Foo, KDToolBox::QtHasher<QString>> stringMap;
    stringMap.insert({ QStringLiteral("123"), {123} });
    stringMap.insert({ QStringLiteral("456"), {456} });
    stringMap.insert({ QStringLiteral("789"), {789} });
    QCOMPARE(stringMap.size(), std::size_t(3));

    stringMap.insert({ QStringLiteral("123"), {0} });
    QCOMPARE(stringMap.size(), std::size_t(3));

    stringMap.clear();
    QCOMPARE(stringMap.size(), std::size_t(0));

    constexpr int LOOPS = 195044;
    for (int i = 0; i < LOOPS; ++i)
        stringMap.insert({ QString::number(i), {i} });

    QCOMPARE(stringMap.size(), std::size_t(LOOPS));

    for (const auto &v : stringMap)
        QCOMPARE(v.first.toInt(), v.second.i);


    std::unordered_set<QRegularExpression, KDToolBox::QtHasher<QRegularExpression>> regexpSet;

    regexpSet.insert(QRegularExpression("a.*b"));
    regexpSet.insert(QRegularExpression("a|b"));
    QCOMPARE(regexpSet.size(), std::size_t(2));
    QVERIFY(regexpSet.find(QRegularExpression("a|b")) != regexpSet.end());
    QVERIFY(regexpSet.find(QRegularExpression("a+b")) == regexpSet.end());

    regexpSet.insert(QRegularExpression("a+b"));
    QCOMPARE(regexpSet.size(), std::size_t(3));
    QVERIFY(regexpSet.find(QRegularExpression("a|b")) != regexpSet.end());
    QVERIFY(regexpSet.find(QRegularExpression("a+b")) != regexpSet.end());

    regexpSet.erase(QRegularExpression());
    QCOMPARE(regexpSet.size(), std::size_t(3));

    regexpSet.erase(QRegularExpression("a|b"));
    QCOMPARE(regexpSet.size(), std::size_t(2));
    QVERIFY(regexpSet.find(QRegularExpression("a|b")) == regexpSet.end());
    QVERIFY(regexpSet.find(QRegularExpression("a+b")) != regexpSet.end());

    std::unordered_set<QUrl, KDToolBox::QtHasher<QUrl>> urlSet;
    urlSet.insert(QUrl("https://www.kdab.com"));
    urlSet.insert(QUrl("https://www.qt.io"));
    QCOMPARE(urlSet.size(), std::size_t(2));

    urlSet.insert(QUrl("https://isocpp.org"));
    QCOMPARE(urlSet.size(), std::size_t(3));

    QVERIFY(urlSet.find(QUrl("https://www.kdab.com")) != urlSet.end());
    QVERIFY(urlSet.find(QUrl("https://www.google.com")) == urlSet.end());
}

namespace TestNS
{
    struct Hashable { int i ; };

    using QHashIntReturnType = decltype(qHash(0));

    QHashIntReturnType qHash(Hashable h) noexcept
    {
        return QT_PREPEND_NAMESPACE(qHash)(h.i);
    }

    struct HashableHiddenFriend
    {
        int i;
        friend QHashIntReturnType qHash(HashableHiddenFriend h) noexcept
        {
            return QT_PREPEND_NAMESPACE(qHash)(h.i);
        }
    };
} // namespace TestNS

void tst_QtHasher::poison()
{
    Q_STATIC_ASSERT(std::is_default_constructible<KDToolBox::QtHasher<QString>>::value);
    Q_STATIC_ASSERT(std::is_default_constructible<KDToolBox::QtHasher<QUrl>>::value);
    Q_STATIC_ASSERT(std::is_default_constructible<KDToolBox::QtHasher<QFont>>::value);
    Q_STATIC_ASSERT(!std::is_default_constructible<KDToolBox::QtHasher<Foo>>::value); // no qHash(Foo)
    Q_STATIC_ASSERT(std::is_default_constructible<KDToolBox::QtHasher<TestNS::Hashable>>::value); // found through ADL
    Q_STATIC_ASSERT(std::is_default_constructible<KDToolBox::QtHasher<TestNS::HashableHiddenFriend>>::value); // found through ADL
}

QTEST_MAIN(tst_QtHasher)
