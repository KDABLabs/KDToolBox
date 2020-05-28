/****************************************************************************
**                                MIT License
**
** Copyright (C) 2020 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
** Author: Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>
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

#include <QApplication>
#include <QPushButton>
#include <QDebug>

#include <messagehandler.h>

int main(int argc, char **argv)
{
    qSetMessagePattern(QStringLiteral("[%{if-debug}DBUG%{endif}%{if-info}INFO%{endif}%{if-warning}WARN%{endif}%{if-critical}CRIT%{endif}%{if-fatal}FATL%{endif}] %{message}"));

    QApplication app(argc, argv);

    qWarning() << "This is a warning message; no message handler has been registered yet.";

    qDebug() << "This is a debug message. We're now installing a handler for all warnings";

    KDToolBox::handleMessage(QtWarningMsg,
                             [](){  qDebug() << "***!!!*** 1st warning handler here: a warning happened"; });

    qWarning() << "This is warning again. Before this warning, you should've had an extra print.";

    qDebug() << "This is a debug message; this did not trigger the handler because it's a debug, not a warning message.";
    qDebug() << "Now installing a handler that reacts only on warnings that begin with 'PANIC'.";

    KDToolBox::handleMessage(QtWarningMsg,
                             QRegularExpression(QStringLiteral("^PANIC")),
                             [](){ qDebug() << "***!!!*** 2nd warning handler here: a warning that begins with PANIC has been received"; });

    qWarning() << "PANIC! Both warning handlers should've fired before this message";
    qWarning() << "Another warning, this time only the first warning handler should've fired";
    qCritical() << "Critical message. Handled also by a warning handler.";

    QPushButton button(QStringLiteral("Click to quit"));
    button.resize(500, 500);
    button.show();
    QObject::connect(&button, &QPushButton::clicked, &app, &QApplication::quit);

    return app.exec();
}
