/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include <QApplication>
#include <QDebug>
#include <QPushButton>

#include <messagehandler.h>

int main(int argc, char **argv)
{
    qSetMessagePattern(QStringLiteral("[%{if-debug}DBUG%{endif}%{if-info}INFO%{endif}%{if-warning}WARN%{endif}%{if-"
                                      "critical}CRIT%{endif}%{if-fatal}FATL%{endif}] %{message}"));

    QApplication app(argc, argv);

    qWarning() << "This is a warning message; no message handler has been registered yet.";

    qDebug() << "This is a debug message. We're now installing a handler for all warnings";

    KDToolBox::handleMessage(QtWarningMsg,
                             []() { qDebug() << "***!!!*** 1st warning handler here: a warning happened"; });

    qWarning() << "This is warning again. Before this warning, you should've had an extra print.";

    qDebug()
        << "This is a debug message; this did not trigger the handler because it's a debug, not a warning message.";
    qDebug() << "Now installing a handler that reacts only on warnings that begin with 'PANIC'.";

    KDToolBox::handleMessage(QtWarningMsg, QRegularExpression(QStringLiteral("^PANIC")), []() {
        qDebug() << "***!!!*** 2nd warning handler here: a warning that begins with PANIC has been received";
    });

    qWarning() << "PANIC! Both warning handlers should've fired before this message";
    qWarning() << "Another warning, this time only the first warning handler should've fired";
    qCritical() << "Critical message. Handled also by a warning handler.";

    QPushButton button(QStringLiteral("Click to quit"));
    button.resize(500, 500);
    button.show();
    QObject::connect(&button, &QPushButton::clicked, &app, &QApplication::quit);

    return app.exec();
}
