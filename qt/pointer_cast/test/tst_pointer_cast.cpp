/****************************************************************************
**                                MIT License
**
** Copyright (C) 2019-2023 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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
        auto p = make_qshared<QString>("Hello");
        Q_STATIC_ASSERT((std::is_same<decltype(p), QSharedPointer<QString>>::value));
        QVERIFY(p);
        QCOMPARE(*p, "Hello");
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
