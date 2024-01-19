/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Nicolas Arnaud-Cormos <nicolas.arnaud-cormos@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "../src/tabwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    TabWindow w;
    w.addTab(new QWidget, QStringLiteral("Tab 1"));
    w.addTab(new QWidget, QStringLiteral("Tab 2"));
    w.addTab(new QWidget, QStringLiteral("Tab 3"));
    w.addTab(new QWidget, QStringLiteral("Tab 4"));
    w.show();

    return a.exec();
}
