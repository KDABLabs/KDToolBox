/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include <QTest>

#include "../singleshot_connect.h"

class tst_SingleShot_Connect : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

private Q_SLOTS:
    void singleshot();
};

class Object : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

Q_SIGNALS:
    void aSignal(int, const QString &);

public Q_SLOTS:
    void aSlot(int i, const QString &s)
    {
        ++m_slotCounter;
        m_i = i;
        m_s = s;
    }

    void aSlotWithOneArgLess(int i)
    {
        m_i = i;
        ++m_slotCounter;
    }

    void noArgSlot() { ++m_slotCounter; }

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

        c = KDToolBox::connectSingleShot(&o, &Object::aSignal, &o, &Object::aSlot);
        QVERIFY(c);
        QCOMPARE(o.m_slotCounter, 0);

        Q_EMIT o.aSignal(1, QStringLiteral("Hello"));
        QVERIFY(!c);
        QCOMPARE(o.m_i, 1);
        QCOMPARE(o.m_s, QStringLiteral("Hello"));
        QCOMPARE(o.m_slotCounter, 1);

        Q_EMIT o.aSignal(2, QStringLiteral("World"));
        QVERIFY(!c);
        QCOMPARE(o.m_i, 1);
        QCOMPARE(o.m_s, QStringLiteral("Hello"));
        QCOMPARE(o.m_slotCounter, 1);

        c = KDToolBox::connectSingleShot(&o, &Object::aSignal, &o, &Object::aSlot);
        QVERIFY(c);
        disconnect(c);
        QVERIFY(!c);
        Q_EMIT o.aSignal(3, QStringLiteral("!"));
        QCOMPARE(o.m_i, 1);
        QCOMPARE(o.m_s, QStringLiteral("Hello"));
        QCOMPARE(o.m_slotCounter, 1);

        c = KDToolBox::connectSingleShot(&o, &Object::aSignal, &o, &Object::aSlot);
        QVERIFY(c);
        Q_EMIT o.aSignal(42, QStringLiteral("The Answer"));
        QVERIFY(!c);
        QCOMPARE(o.m_i, 42);
        QCOMPARE(o.m_s, QStringLiteral("The Answer"));
        QCOMPARE(o.m_slotCounter, 2);

#if __cplusplus >= 201703L
        c = KDToolBox::connectSingleShot(&o, &Object::aSignal, &o, &Object::aSlotWithOneArgLess);
        QVERIFY(c);
        Q_EMIT o.aSignal(1, QStringLiteral("Hello"));
        QVERIFY(!c);
        QCOMPARE(o.m_i, 1);
        QCOMPARE(o.m_slotCounter, 3);

        c = KDToolBox::connectSingleShot(&o, &Object::aSignal, &o, &Object::noArgSlot);
        QVERIFY(c);
        Q_EMIT o.aSignal(1, QStringLiteral("Hello"));
        QVERIFY(!c);
        QCOMPARE(o.m_i, 1);
        QCOMPARE(o.m_slotCounter, 4);

        int x = 0;
        c = KDToolBox::connectSingleShot(&o, &Object::aSignal, [&x](int i) { x = i; });
        QVERIFY(c);
        Q_EMIT o.aSignal(33, QStringLiteral("Hello"));
        QVERIFY(!c);
        QCOMPARE(x, 33);
#endif
    }
    {
        struct MoveOnlyFunctor
        {
            struct noop_deleter
            {
                void operator()(const void *) const noexcept {}
            };
            std::unique_ptr<int, noop_deleter> ri;
            std::unique_ptr<QString, noop_deleter> rs;

            explicit MoveOnlyFunctor(int &i, QString &s)
                : ri(&i)
                , rs(&s)
            {
            }

            void operator()(int i, const QString &s)
            {
                *ri = i;
                *rs = s;
            }
        };

        int i = 0;
        QString s;

        Object o;
        MoveOnlyFunctor f(i, s);
        QMetaObject::Connection c;
        c = KDToolBox::connectSingleShot(&o, &Object::aSignal, std::move(f));
        QVERIFY(c);
        Q_EMIT o.aSignal(-123, QStringLiteral("test"));
        QVERIFY(!c);
        QCOMPARE(i, -123);
        QCOMPARE(s, QStringLiteral("test"));

        Q_EMIT o.aSignal(42, QStringLiteral("bar"));
        QVERIFY(!c);
        QCOMPARE(i, -123);
        QCOMPARE(s, QStringLiteral("test"));
    }
}

QTEST_MAIN(tst_SingleShot_Connect)

#include "tst_singleshot_connect.moc"
