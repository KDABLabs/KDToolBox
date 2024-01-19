/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include <QTest>

#include "../pointer_cast.h"

class tst_pointer_cast : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

private Q_SLOTS:
    void makeQShared();
    void staticCast();
    void dynamicCast();
    void constCast();
};

namespace
{

struct B
{
    B() = default;
    B(const B &) = delete;
    B &operator=(const B &) = delete;
    virtual ~B() = default;
};

struct D : B
{
};

struct D2 : B
{
};

} // unnamed namespace

void tst_pointer_cast::makeQShared()
{
    {
        auto p = make_qshared<int>(42);
        Q_STATIC_ASSERT((std::is_same<decltype(p), QSharedPointer<int>>::value));
        QVERIFY(p);
        QCOMPARE(*p, 42);
    }
    {
        auto p = make_qshared<QString>(QStringLiteral("Hello"));
        Q_STATIC_ASSERT((std::is_same<decltype(p), QSharedPointer<QString>>::value));
        QVERIFY(p);
        QCOMPARE(*p, QStringLiteral("Hello"));
    }
}

void tst_pointer_cast::staticCast()
{
    auto b = make_qshared<D>();
    QVERIFY(b);
    auto d = static_pointer_cast<D>(b);
    QCOMPARE(b, d);
}

void tst_pointer_cast::dynamicCast()
{
    auto b = make_qshared<D>();
    QVERIFY(b);
    auto d = dynamic_pointer_cast<D>(b);
    QCOMPARE(b, d);
    auto d2 = dynamic_pointer_cast<D2>(b);
    QCOMPARE(d2, nullptr);
}

void tst_pointer_cast::constCast()
{
    auto cd = make_qshared<const D>();
    QVERIFY(cd);
    auto md = const_pointer_cast<D>(cd);
    QCOMPARE(md, cd);
}

QTEST_MAIN(tst_pointer_cast)

#include "tst_pointer_cast.moc"
