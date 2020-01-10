/****************************************************************************
**                                MIT License
**
** Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
** Author: Sérgio Martins <sergio.martins@kdab.com>
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

#include "uiwatchdog.h"

#include <QDebug>
#include <QTimer>
#include <QtWidgets>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QWidget w;
    auto layout = new QVBoxLayout(&w);
    auto button = new QPushButton("Block UI completely");
    auto button2 = new QPushButton("Sleep every other second");
    auto button3 = new QPushButton("Cancel sleep");
    layout->addWidget(button);
    layout->addWidget(button2);
    layout->addStretch();
    layout->addWidget(button3);

    QObject::connect(button, &QPushButton::clicked, [] {
        qDebug() << "Blocking forever!";
        while (true);
    });

    auto sleepTimer = new QTimer();
    sleepTimer->setInterval(1000);
    QObject::connect(sleepTimer, &QTimer::timeout, [] {
        qDebug() << "Sleeping";
        QThread::sleep(1);
        qDebug() << "Waking up";
    });

    QObject::connect(button2, &QPushButton::clicked, [sleepTimer] {
        sleepTimer->start();
    });

    QObject::connect(button3, &QPushButton::clicked, [sleepTimer] {
        sleepTimer->stop();
    });

    UiWatchdog dog;
    dog.start();
    w.resize(800, 800);
    w.show();

    return app.exec();
}
