/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTest>

#include "../KDSqlDatabaseTransaction.h"

#define QSL(x) QStringLiteral(x)

class tst_KDSqlDatabaseTransaction : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

private Q_SLOTS:
    void initTestCase();
    void testTransactions();
};

void tst_KDSqlDatabaseTransaction::initTestCase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase(QSL("QSQLITE"));
    db.setDatabaseName(QString());
    QVERIFY(db.open());

    QSqlQuery q;
    QVERIFY(q.exec(QSL("CREATE TABLE t(x INTEGER)")));
}

static void verifyRowCount(int expectedCount)
{
    QSqlQuery q;
    QVERIFY(q.exec(QSL("SELECT COUNT(*) FROM t")));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), expectedCount);
}

void tst_KDSqlDatabaseTransaction::testTransactions()
{
    int rowCount = 0;

    verifyRowCount(rowCount);

    {
        // no transaction
        QSqlQuery q;
        QVERIFY(q.exec(QSL("INSERT INTO t VALUES(42)")));
        verifyRowCount(++rowCount);
    }

    {
        // commit
        KDToolBox::KDSqlDatabaseTransaction t;
        QSqlQuery q;
        QVERIFY(q.exec(QSL("INSERT INTO t VALUES(42)")));
        verifyRowCount(++rowCount);
        QVERIFY(t.commit());
        verifyRowCount(rowCount);
    }

    verifyRowCount(rowCount);

    {
        // commit
        KDToolBox::KDSqlDatabaseTransaction t;
        QSqlQuery q;
        QVERIFY(q.exec(QSL("INSERT INTO t VALUES(123)")));
        verifyRowCount(++rowCount);
        QVERIFY(t.commit());
        verifyRowCount(rowCount);
    }

    verifyRowCount(rowCount);

    {
        // rollback (via destructor)
        KDToolBox::KDSqlDatabaseTransaction t;
        QSqlQuery q;
        QVERIFY(q.exec(QSL("INSERT INTO t VALUES(-1)")));
        verifyRowCount(rowCount + 1);
    }

    verifyRowCount(rowCount);

    {
        // rollback (via explicit call)
        KDToolBox::KDSqlDatabaseTransaction t;
        QSqlQuery q;
        QVERIFY(q.exec(QSL("INSERT INTO t VALUES(-1)")));
        verifyRowCount(rowCount + 1);
        QVERIFY(t.rollback());
        verifyRowCount(rowCount);
    }

    verifyRowCount(rowCount);
}

QTEST_MAIN(tst_KDSqlDatabaseTransaction)

#include "tst_KDSqlDatabaseTransaction.moc"
