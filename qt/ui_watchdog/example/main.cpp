/* UiWatchdog
Copyright (C) 2017 Sérgio Martins <iamsergio@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
        while(true);
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
