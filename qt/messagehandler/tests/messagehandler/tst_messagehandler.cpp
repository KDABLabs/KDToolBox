/****************************************************************************
**                                MIT License
**
** Copyright (C) 2020-2023 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
** Author: Marc Mutz <marc.mutz@kdab.com>
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

#include <messagehandler.h>

#include <QDebug>
#include <QTest>

#include <algorithm>
#include <thread>
#include <vector>

class tst_MessageHandler : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

private Q_SLOTS:
    void threading();
};

void tst_MessageHandler::threading()
{
    // to be run with tsan
    static const size_t num_iterations = 100;
    static const size_t max_threads = 16;
    static const QtMsgType msg_types[] = {QtDebugMsg, QtWarningMsg, QtCriticalMsg, /*QtFatalMsg,*/ QtInfoMsg};
    static const auto num_msg_types = sizeof msg_types / sizeof *msg_types;
    static const auto num_threads = qBound(num_msg_types, size_t{std::thread::hardware_concurrency()} * 2, max_threads);
    qDebug() << "num_threads" << num_threads;

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    std::vector<size_t> counts(num_threads);

    for (size_t thread = 0; thread < num_threads; ++thread)
    {
        auto &count = counts[thread];
        threads.emplace_back([thread, &count] {
            const auto msg_type = msg_types[thread % num_msg_types];
            const QString pattern = QStringLiteral("^thread %1 iteration").arg(thread);
            KDToolBox::handleMessage(msg_type, QRegularExpression{pattern}, [&count] { ++count; });
            for (size_t i = 0; i < num_iterations; ++i)
            {
                [=] {
                    switch (msg_type)
                    {
                    case QtDebugMsg:
                        return qDebug();
                    case QtWarningMsg:
                        return qWarning();
                    case QtCriticalMsg:
                        return qCritical();
                    case QtInfoMsg:
                        return qInfo();
                    case QtFatalMsg:;
                    };
                    Q_UNREACHABLE();
                }() << "thread"
                    << thread << "iteration" << i + 1;
            }
        });
    }

    for (auto &t : threads)
        t.join();

    for (size_t count : counts)
        QCOMPARE(count, num_iterations);
}

QTEST_APPLESS_MAIN(tst_MessageHandler)
#include "tst_messagehandler.moc"
