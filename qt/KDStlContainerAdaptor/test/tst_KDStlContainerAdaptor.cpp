/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "../KDStlContainerAdaptor.h"

#include <QTest>

class tst_KDStlContainerAdaptor : public QObject
{
    Q_OBJECT
public:
    explicit tst_KDStlContainerAdaptor(QObject *parent = nullptr);

private Q_SLOTS:
    void vectorAdaptorConstruction();
    void vectorAdaptorIterators();
    void vectorAdaptorDataAccess();
    void vectorAdaptorSizeCapacity();
    void vectorAdaptorInsertion();
    void vectorAdaptorRemoval();
    void vectorAdaptorSearch();
    void vectorAdaptorMisc();
    void vectorAdaptorOperators();
};

using IntVec = KDToolBox::StlContainerAdaptor::StdVectorAdaptor<int>;
using StringVec = KDToolBox::StlContainerAdaptor::StdVectorAdaptor<QString>;

tst_KDStlContainerAdaptor::tst_KDStlContainerAdaptor(QObject *parent)
    : QObject{parent}
{
}

void tst_KDStlContainerAdaptor::vectorAdaptorConstruction()
{
    IntVec v1;
    QCOMPARE(v1.size(), 0);

    IntVec v2 = {1, 2, 3};
    QCOMPARE(v2.size(), 3);

    IntVec v3(IntVec::size_type(123), -1);
    QCOMPARE(v3.size(), 123);

    IntVec v4 = v3.clone();
    QCOMPARE(v4.size(), 123);

    v4.assignFrom(v2);
    QCOMPARE(v4.size(), 3);

    IntVec v5 = std::move(v4);
    QCOMPARE(v5.size(), 3);

    v5 = std::move(v3);
    QCOMPARE(v5.size(), 123);
}

void tst_KDStlContainerAdaptor::vectorAdaptorIterators()
{
    IntVec v{1, 2, 3, 4, 5};
    static_assert(std::is_same<decltype(*v.constBegin()), const int &>::value, "Wrong types");
    static_assert(std::is_same<decltype(*v.constEnd()), const int &>::value, "Wrong types");
    auto result = std::accumulate(v.constBegin(), v.constEnd(), 0);
    QCOMPARE(result, 15);
}

void tst_KDStlContainerAdaptor::vectorAdaptorDataAccess()
{
    IntVec v{1, 2, 3, 4, 5};

    for (IntVec::size_type i = 0; i < v.size(); ++i)
    {
        const int expected = i + 1;
        QCOMPARE(v.constData()[i], expected);
        QCOMPARE(v.at(i), expected);
        QCOMPARE(v[i], expected);
        QCOMPARE(qAsConst(v)[i], expected);
        QCOMPARE(v.value(i), expected);
    }

    QCOMPARE(v.value(1'000'000), 0);
    QCOMPARE(v.value(-1'000'000), 0);
    QCOMPARE(v.value(1'000'000, 123), 123);
    QCOMPARE(v.value(-1'000'000, 123), 123);

    v[0] = -1;
    QCOMPARE(v.first(), -1);
    v.first() = 123;
    QCOMPARE(v.first(), 123);
    QCOMPARE(qAsConst(v).first(), 123);
    QCOMPARE(v.constFirst(), 123);

    v[4] = -1;
    QCOMPARE(v.last(), -1);
    v.last() = 456;
    QCOMPARE(v.last(), 456);
    QCOMPARE(qAsConst(v).last(), 456);
    QCOMPARE(v.constLast(), 456);
}

void tst_KDStlContainerAdaptor::vectorAdaptorSizeCapacity()
{
    IntVec v1;
    QVERIFY(v1.isEmpty());
    QCOMPARE(v1.size(), 0);
    QCOMPARE(v1.count(), 0);
    QCOMPARE(v1.length(), 0);
    QCOMPARE(v1.capacity(), 0);

    IntVec v2{1, 2, 3, 4, 5};
    QVERIFY(!v2.isEmpty());
    QCOMPARE(v2.size(), 5);
    QCOMPARE(v2.count(), 5);
    QCOMPARE(v2.length(), 5);
    QVERIFY(v2.capacity() >= 5);

    v2.reserve(100);
    QVERIFY(!v2.isEmpty());
    QCOMPARE(v2.size(), 5);
    QCOMPARE(v2.count(), 5);
    QCOMPARE(v2.length(), 5);
    QVERIFY(v2.capacity() >= 100);

    v2.reserve(0);
    QVERIFY(!v2.isEmpty());
    QCOMPARE(v2.size(), 5);
    QCOMPARE(v2.count(), 5);
    QCOMPARE(v2.length(), 5);
    QVERIFY(v2.capacity() >= 5);

    v2.squeeze();
    QVERIFY(!v2.isEmpty());
    QCOMPARE(v2.size(), 5);
    QCOMPARE(v2.count(), 5);
    QCOMPARE(v2.length(), 5);
    QVERIFY(v2.capacity() >= 5);
}

void tst_KDStlContainerAdaptor::vectorAdaptorInsertion()
{
    IntVec v{1, 2, 3};
    v.append(4);
    QCOMPARE(v, (IntVec{1, 2, 3, 4}));
    v.prepend(0);
    QCOMPARE(v, (IntVec{0, 1, 2, 3, 4}));
    v.insert(2, 42);
    QCOMPARE(v, (IntVec{0, 1, 42, 2, 3, 4}));
    v.insert(0, 42);
    QCOMPARE(v, (IntVec{42, 0, 1, 42, 2, 3, 4}));
    v.insert(v.size(), 42);
    QCOMPARE(v, (IntVec{42, 0, 1, 42, 2, 3, 4, 42}));
}

void tst_KDStlContainerAdaptor::vectorAdaptorRemoval()
{
    IntVec v{0, 1, 2, 3, 4, 5, 6, 7, 8};
    v.removeFirst();
    QCOMPARE(v, (IntVec{1, 2, 3, 4, 5, 6, 7, 8}));
    v.removeLast();
    QCOMPARE(v, (IntVec{1, 2, 3, 4, 5, 6, 7}));
    v.remove(2);
    QCOMPARE(v, (IntVec{1, 2, 4, 5, 6, 7}));
    v.removeAt(3);
    QCOMPARE(v, (IntVec{1, 2, 4, 6, 7}));
    v.remove(1, 2);
    QCOMPARE(v, (IntVec{1, 6, 7}));

    v = {1, 2, 3, 2, 4, 2, 2, 4, 6, 2};
    QCOMPARE(v.removeAll(0), 0);
    QCOMPARE(v, (IntVec{1, 2, 3, 2, 4, 2, 2, 4, 6, 2}));
    QCOMPARE(v.removeAll(2), 5);
    QCOMPARE(v, (IntVec{1, 3, 4, 4, 6}));
    QCOMPARE(v.removeAll(2), 0);
    QCOMPARE(v, (IntVec{1, 3, 4, 4, 6}));

    QVERIFY(!v.removeOne(0));
    QCOMPARE(v, (IntVec{1, 3, 4, 4, 6}));
    QVERIFY(v.removeOne(1));
    QCOMPARE(v, (IntVec{3, 4, 4, 6}));
    QVERIFY(!v.removeOne(1));
    QCOMPARE(v, (IntVec{3, 4, 4, 6}));
    QVERIFY(v.removeOne(4));
    QCOMPARE(v, (IntVec{3, 4, 6}));
    QVERIFY(v.removeOne(4));
    QCOMPARE(v, (IntVec{3, 6}));
    QVERIFY(!v.removeOne(4));
    QCOMPARE(v, (IntVec{3, 6}));

    v = {1, 2, 2, 3, 1, 3, 5, 4, 5, 4};
    QCOMPARE(v.removeIf([](auto &&i) { return (i % 2) == 0; }), 4);
    QCOMPARE(v, (IntVec{1, 3, 1, 3, 5, 5}));

    v = {1, 2, 3, 4, 5, 6, 7, 8};
    QCOMPARE(v.takeFirst(), 1);
    QCOMPARE(v, (IntVec{2, 3, 4, 5, 6, 7, 8}));
    QCOMPARE(v.takeLast(), 8);
    QCOMPARE(v, (IntVec{2, 3, 4, 5, 6, 7}));
    QCOMPARE(v.takeAt(2), 4);
    QCOMPARE(v, (IntVec{2, 3, 5, 6, 7}));
    QCOMPARE(v.takeAt(3), 6);
    QCOMPARE(v, (IntVec{2, 3, 5, 7}));
    QCOMPARE(v.takeAt(3), 7);
    QCOMPARE(v, (IntVec{2, 3, 5}));
    QCOMPARE(v.takeAt(0), 2);
    QCOMPARE(v.takeAt(0), 3);
    QCOMPARE(v.takeAt(0), 5);
    QCOMPARE(v, (IntVec{}));

    StringVec sv = {
        QStringLiteral("foo"), QStringLiteral("bar"), QStringLiteral("baz"),
        QStringLiteral("fie"), QStringLiteral("fax"),
    };
    QCOMPARE(sv.removeIf([](auto &&s) { return s.startsWith(QLatin1Char('f')); }), 3);
    QCOMPARE(sv, (StringVec{QStringLiteral("bar"), QStringLiteral("baz")}));
}

void tst_KDStlContainerAdaptor::vectorAdaptorSearch()
{
    const IntVec v{1, 2, 3, 4, 5, 3, 4, 5};

    QVERIFY(v.contains(1));
    QVERIFY(v.contains(2));
    QVERIFY(v.contains(3));
    QVERIFY(v.contains(4));
    QVERIFY(v.contains(5));
    QVERIFY(!v.contains(0));
    QVERIFY(!v.contains(100));

    QCOMPARE(v.indexOf(1), 0);
    QCOMPARE(v.indexOf(2), 1);
    QCOMPARE(v.indexOf(3), 2);
    QCOMPARE(v.indexOf(4), 3);
    QCOMPARE(v.indexOf(5), 4);
    QCOMPARE(v.indexOf(0), -1);
    QCOMPARE(v.indexOf(100), -1);

    QCOMPARE(v.indexOf(1, 0), 0);
    QCOMPARE(v.indexOf(1, 1), -1);
    QCOMPARE(v.indexOf(1, 100), -1);
    QCOMPARE(v.indexOf(1, -100), 0);
    QCOMPARE(v.indexOf(1, -1), -1);

    QCOMPARE(v.indexOf(2, 0), 1);
    QCOMPARE(v.indexOf(2, 1), 1);
    QCOMPARE(v.indexOf(2, 2), -1);
    QCOMPARE(v.indexOf(2, 100), -1);
    QCOMPARE(v.indexOf(2, -100), 1);
    QCOMPARE(v.indexOf(2, -1), -1);

    QCOMPARE(v.indexOf(3, 0), 2);
    QCOMPARE(v.indexOf(3, 1), 2);
    QCOMPARE(v.indexOf(3, 2), 2);
    QCOMPARE(v.indexOf(3, 3), 5);
    QCOMPARE(v.indexOf(3, 4), 5);
    QCOMPARE(v.indexOf(3, 5), 5);
    QCOMPARE(v.indexOf(3, 6), -1);
    QCOMPARE(v.indexOf(3, 100), -1);
    QCOMPARE(v.indexOf(3, -100), 2);
    QCOMPARE(v.indexOf(3, -1), -1);
    QCOMPARE(v.indexOf(3, -2), -1);
    QCOMPARE(v.indexOf(3, -3), 5);
    QCOMPARE(v.indexOf(3, -4), 5);
    QCOMPARE(v.indexOf(3, -5), 5);
    QCOMPARE(v.indexOf(3, -6), 2);
    QCOMPARE(v.indexOf(3, -7), 2);
    QCOMPARE(v.indexOf(3, -8), 2);

    QCOMPARE(v.lastIndexOf(1), 0);
    QCOMPARE(v.lastIndexOf(2), 1);
    QCOMPARE(v.lastIndexOf(3), 5);
    QCOMPARE(v.lastIndexOf(4), 6);
    QCOMPARE(v.lastIndexOf(5), 7);
    QCOMPARE(v.lastIndexOf(0), -1);
    QCOMPARE(v.lastIndexOf(100), -1);

    QCOMPARE(v.lastIndexOf(3, 8), 5);
    QCOMPARE(v.lastIndexOf(3, 7), 5);
    QCOMPARE(v.lastIndexOf(3, 6), 5);
    QCOMPARE(v.lastIndexOf(3, 4), 2);
    QCOMPARE(v.lastIndexOf(3, 3), 2);
    QCOMPARE(v.lastIndexOf(3, 2), 2);
    QCOMPARE(v.lastIndexOf(3, 1), -1);
    QCOMPARE(v.lastIndexOf(3, 0), -1);
    QCOMPARE(v.lastIndexOf(3, -1), 5);
    QCOMPARE(v.lastIndexOf(3, -2), 5);
    QCOMPARE(v.lastIndexOf(3, -3), 5);
    QCOMPARE(v.lastIndexOf(3, -4), 2);
    QCOMPARE(v.lastIndexOf(3, -5), 2);
    QCOMPARE(v.lastIndexOf(3, -6), 2);
    QCOMPARE(v.lastIndexOf(3, -7), -1);
    QCOMPARE(v.lastIndexOf(3, -8), -1);

    QVERIFY(v.startsWith(1));
    QVERIFY(!v.startsWith(0));
    QVERIFY(v.endsWith(5));
    QVERIFY(!v.endsWith(0));
}

void tst_KDStlContainerAdaptor::vectorAdaptorMisc()
{
    {
        IntVec v;
        v.fill(123);
        QVERIFY(v.isEmpty());
    }
    {
        IntVec v;
        v.fill(1, 6);
        QCOMPARE(v, (IntVec{1, 1, 1, 1, 1, 1}));
        v.fill(2, 3);
        QCOMPARE(v, (IntVec{2, 2, 2}));
        v.fill(3, 2);
        QCOMPARE(v, (IntVec{3, 3}));
        v.fill(4, 5);
        QCOMPARE(v, (IntVec{4, 4, 4, 4, 4}));
        v.fill(10);
        QCOMPARE(v, (IntVec{10, 10, 10, 10, 10}));
    }

    {
        IntVec v;
        QCOMPARE(v.mid(0), (IntVec{}));
    }
    {
        IntVec v{1, 2, 3, 4, 5};
        QCOMPARE(v.mid(0), (IntVec{1, 2, 3, 4, 5}));
        QCOMPARE(v.mid(1), (IntVec{2, 3, 4, 5}));
        QCOMPARE(v.mid(2), (IntVec{3, 4, 5}));
        QCOMPARE(v.mid(3), (IntVec{4, 5}));
        QCOMPARE(v.mid(4), (IntVec{5}));
        QCOMPARE(v.mid(5), (IntVec{}));

        QCOMPARE(v.mid(0, -1), (IntVec{1, 2, 3, 4, 5}));
        QCOMPARE(v.mid(0, 0), (IntVec{}));
        QCOMPARE(v.mid(0, 1), (IntVec{1}));
        QCOMPARE(v.mid(0, 2), (IntVec{1, 2}));
        QCOMPARE(v.mid(0, 3), (IntVec{1, 2, 3}));
        QCOMPARE(v.mid(0, 4), (IntVec{1, 2, 3, 4}));
        QCOMPARE(v.mid(0, 5), (IntVec{1, 2, 3, 4, 5}));
        QCOMPARE(v.mid(0, 6), (IntVec{1, 2, 3, 4, 5}));

        QCOMPARE(v.mid(2, -1), (IntVec{3, 4, 5}));
        QCOMPARE(v.mid(2, 0), (IntVec{}));
        QCOMPARE(v.mid(2, 1), (IntVec{3}));
        QCOMPARE(v.mid(2, 2), (IntVec{3, 4}));
        QCOMPARE(v.mid(2, 3), (IntVec{3, 4, 5}));
        QCOMPARE(v.mid(2, 4), (IntVec{3, 4, 5}));

        QCOMPARE(v.mid(5, -1), (IntVec{}));
        QCOMPARE(v.mid(5, 0), (IntVec{}));
        QCOMPARE(v.mid(5, 1), (IntVec{}));
    }
}

void tst_KDStlContainerAdaptor::vectorAdaptorOperators()
{
    IntVec v{1, 2, 3};
    v << 4 << 5 << 6;
    QCOMPARE(v, (IntVec{1, 2, 3, 4, 5, 6}));

    v = IntVec{1, 2, 3};
    v << 4 << IntVec{5, 6} << 7;
    QCOMPARE(v, (IntVec{1, 2, 3, 4, 5, 6, 7}));

    IntVec v2{8, 9};
    QCOMPARE(v + v2, (IntVec{1, 2, 3, 4, 5, 6, 7, 8, 9}));
    QCOMPARE(v2 + v, (IntVec{8, 9, 1, 2, 3, 4, 5, 6, 7}));

    v = IntVec{1, 2, 3};
    v += IntVec{4, 5, 6};
    QCOMPARE(v, (IntVec{1, 2, 3, 4, 5, 6}));

    v = IntVec{1, 2, 3};
    v += v;
    QCOMPARE(v, (IntVec{1, 2, 3, 1, 2, 3}));

    StringVec sv;
    sv << sv;
    QVERIFY(sv.isEmpty());
    sv << QStringLiteral("hello") << QStringLiteral("world");
    QCOMPARE(sv, (StringVec{QStringLiteral("hello"), QStringLiteral("world")}));
}

QTEST_MAIN(tst_KDStlContainerAdaptor)

#include "tst_KDStlContainerAdaptor.moc"
