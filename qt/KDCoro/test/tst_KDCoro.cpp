/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include <QPoint>
#include <QTimer>
#include <QTest>

#include "../KDCoro.h"

using namespace KDToolBox;
using namespace Qt::StringLiterals;
using namespace std::chrono;

class MyLibrary : public QObject
{
    Q_OBJECT
public:
    MyLibrary() {
        auto timer = new QTimer;
        timer->setSingleShot(false); // kdCoroSignal() behaves like a singleShot connection, you can only await it one time
        timer->setInterval(1s);
        timer->start();

        QObject::connect(timer, &QTimer::timeout, this, [this] {
            Q_EMIT textChanged(u"some text"_s);
        });
    }

    KDCoroExpected<QPoint> doSomethingLater(bool error) {

        auto timer = new QTimer;
        timer->setInterval(1s);
        timer->start();

        KDCoroExpected<QPoint> awaitable(timer);

        auto exp = awaitable.callbackExpected;
        auto unexp = awaitable.callbackUnexpected;

        QObject::connect(timer, &QTimer::timeout, [exp, unexp, error, timer] {
            timer->deleteLater();

            if (error) {
                unexp(u"finished with error"_s);
            } else {
                exp(QPoint{800, 600});
            }
        });

        return awaitable;
    }

    KDCoroExpected<QPointF> doSomethingError() {

        auto timer = new QTimer;
        timer->setInterval(1s);
        timer->start();

        KDCoroExpected<QPointF> awaitable(timer);

        auto cb = awaitable.callbackUnexpected;

        QObject::connect(timer, &QTimer::timeout, [cb, timer] {
            timer->deleteLater();

            cb(u"finished with error"_s);
        });

        return awaitable;
    }

Q_SIGNALS:
    void textChanged(const QString &text);
};

class tst_KDCoro : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

private Q_SLOTS:
    void initTestCase();
    void testCoro();
    void testSignals();

private:
    void coroFinished() {
        if (--m_coroRunning == 0) {
            qApp->quit();
        }
    }
    int m_coroRunning = 0;
};

void tst_KDCoro::initTestCase()
{

}

void tst_KDCoro::testCoro()
{
    QEventLoop loop;

           // This lambda is a coroutine
    auto coroTests = [&loop, this]() -> KDCoroTerminator {
        MyLibrary lib;

               // Expected
        auto resultTrue = co_await lib.doSomethingLater(false); // Wait till started timer finish
        KDCOROCOMPARE_EQ(resultTrue.has_value(), true);
        KDCOROCOMPARE_EQ(resultTrue.value(), QPoint(800, 600));

               // Unexpected
        auto awaitableError = lib.doSomethingError(); // Timer false starts now

        auto resultFalse = co_await awaitableError; // Wait till started timer finish
        KDCOROCOMPARE_EQ(resultFalse.has_value(), false);
        KDCOROCOMPARE_EQ(resultFalse.error(), u"finished with error"_s);

        loop.quit();
    };

    ++m_coroRunning;
    coroTests();

    loop.exec();
    coroFinished();
}

void tst_KDCoro::testSignals()
{
    QEventLoop loop;

    // This lambda is a coroutine
    auto coroTests = [&loop, this]() -> KDCoroTerminator {
        auto lib = new MyLibrary;

        // MyLibrary emits a QString each 1s
        auto text = co_await kdCoroSignal<QString>(lib, &MyLibrary::textChanged);
        KDCOROCOMPARE_EQ(text.has_value(), true);
        KDCOROCOMPARE_EQ(text.value(), u"some text"_s);

        loop.quit();
    };

    ++m_coroRunning;
    coroTests();

    loop.exec();
    coroFinished();
}

QTEST_GUILESS_MAIN(tst_KDCoro)

// int main(int argc, char *argv[])
// {
//     QCoreApplication app(argc, argv);

//     tst_KDCoro tst;
//     tst.testSignals();
//     return app.exec();
// }

#include "tst_KDCoro.moc"
