/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "../eventfilter.h"

#include <QApplication>
#include <QDebug>
#include <QFocusEvent>
#include <QPushButton>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QPushButton b(QStringLiteral("Hello World"));
    QPushButton *button = &b; // avoid cluttering the code with &

    button->setMouseTracking(true);

    // directly capturing the target in the lambda
    auto filter = KDToolBox::installEventFilter(button, QEvent::Enter, [button] {
        qDebug() << "Mouse entered button" << button;
        return false;
    });

    // taking target+event in the lambda
    auto filter2 = KDToolBox::installEventFilter(button, QEvent::FocusIn, [](QObject *target, QEvent *event) {
        qDebug() << "Focus in onto" << target << "because" << static_cast<QFocusEvent *>(event)->reason();
        return false;
    });

    // not returning anything, return false implied
    auto filter3 = KDToolBox::installEventFilter(button, QEvent::MouseMove, [button](QObject *, QEvent *event) {
        qDebug() << "Mouse moved on" << button << "at" << static_cast<QMouseEvent *>(event)->pos();
    });

    // move-only mutable lambda, not returning anything
    auto filter4 = KDToolBox::installEventFilter(
        button, QEvent::FocusOut,
        [button, x = std::make_unique<int>(0), y = 0](QObject *target, QEvent *event) mutable {
            qDebug() << "Focus out from" << target << "because" << static_cast<QFocusEvent *>(event)->reason()
                     << "invoked" << ++(y) << "times";
            *x = y;
        });

    button->show();
    button->resize(500, 500);
    return app.exec();
}
