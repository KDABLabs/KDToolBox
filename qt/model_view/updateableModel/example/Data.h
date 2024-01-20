/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: André Somers <andre.somers@kdab.com>

  SPDX-License-Identifier: MIT
*/

#ifndef DATA_H
#define DATA_H

#include <QString>
#include <QVector>

#include <vector>

struct Data
{
    // pre Qt 5.13: for using a QVector as data type, we need a default constructor
    //    Data(): id(-1) {}

    Data(int id, QString value1, QString value2)
        : id(id)
        , value1(std::move(value1))
        , value2(std::move(value2))
    {
    }

    int id;
    QString value1;
    QString value2;
};

// using DataContainer = QVector<Data>;
using DataContainer = std::vector<Data>;

#endif // DATA_H
