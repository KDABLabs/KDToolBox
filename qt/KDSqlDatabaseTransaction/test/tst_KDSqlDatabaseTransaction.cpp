/****************************************************************************
**                                MIT License
**
** Copyright (C) 2019-2021 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "../KDSqlDatabaseTransaction.h"

#define QSL(x) QStringLiteral(x)

class tst_KDSqlDatabaseTransaction : public QObject
{
    Q_OBJECT
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
