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

#include <QtTest>

#include "../singleshot_connect.h"

class tst_SingleShot_Connect : public QObject
{
    Q_OBJECT

private slots:
    void singleshot();
};

class Object : public QObject
{
    Q_OBJECT
signals:
    void aSignal(int, QString);

public slots:
    void aSlot(int i, QString s)
    {
        ++m_slotCounter;
        m_i = i;
        m_s = s;
    }

public:
    int m_slotCounter = 0;
    int m_i = 0;
    QString m_s;
};

void tst_SingleShot_Connect::singleshot()
{
    {
        Object o;
        QMetaObject::Connection c;

        c = KDToolBox::connectSingleShot(&o, &Object::aSignal,
                                         &o, &Object::aSlot);
        QVERIFY(c);
        QCOMPARE(o.m_slotCounter, 0);

        o.aSignal(1, "Hello");
        QVERIFY(!c);
        QCOMPARE(o.m_i, 1);
        QCOMPARE(o.m_s, "Hello");
        QCOMPARE(o.m_slotCounter, 1);

        o.aSignal(2, "World");
        QVERIFY(!c);
        QCOMPARE(o.m_i, 1);
        QCOMPARE(o.m_s, "Hello");
        QCOMPARE(o.m_slotCounter, 1);

        c = KDToolBox::connectSingleShot(&o, &Object::aSignal,
                                         &o, &Object::aSlot);
        QVERIFY(c);
        disconnect(c);
        QVERIFY(!c);
        o.aSignal(3, "!");
        QCOMPARE(o.m_i, 1);
        QCOMPARE(o.m_s, "Hello");
        QCOMPARE(o.m_slotCounter, 1);

        c = KDToolBox::connectSingleShot(&o, &Object::aSignal,
                                         &o, &Object::aSlot);
        QVERIFY(c);
        o.aSignal(42, "The Answer");
        QVERIFY(!c);
        QCOMPARE(o.m_i, 42);
        QCOMPARE(o.m_s, "The Answer");
        QCOMPARE(o.m_slotCounter, 2);
    }
    {
        struct MoveOnlyFunctor {
            int &ri;
            QString &rs;

            MoveOnlyFunctor(int &i, QString &s) :
                ri(i), rs(s) {}
            Q_DISABLE_COPY(MoveOnlyFunctor);
            MoveOnlyFunctor(MoveOnlyFunctor &&) = default;
            MoveOnlyFunctor &operator=(MoveOnlyFunctor &&) = default;
            ~MoveOnlyFunctor() = default;
            void operator()(int i, QString s) {
                ri = i;
                rs = s;
            }
        };

        int i = 0;
        QString s;

        Object o;
        MoveOnlyFunctor f(i, s);
        QMetaObject::Connection c;
        c = KDToolBox::connectSingleShot(&o, &Object::aSignal, std::move(f));
        QVERIFY(c);
        o.aSignal(-123, "test");
        QVERIFY(!c);
        QCOMPARE(i, -123);
        QCOMPARE(s, "test");

        o.aSignal(42, "bar");
        QVERIFY(!c);
        QCOMPARE(i, -123);
        QCOMPARE(s, "test");
    }
}

QTEST_MAIN(tst_SingleShot_Connect)

#include "tst_singleshot_connect.moc"
