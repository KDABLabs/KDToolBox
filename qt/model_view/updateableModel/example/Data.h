/****************************************************************************
**                                MIT License
**
** Copyright (C) 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
** Author: André Somers <andre.somers@kdab.com>
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

#ifndef DATA_H
#define DATA_H

#include <QString>
#include <QVector>

#include <vector>

struct Data {
    // pre Qt 5.13: for using a QVector as data type, we need a default constructor
//    Data(): id(-1) {}

    Data(int id, QString value1, QString value2):
        id(id), value1(std::move(value1)), value2(std::move(value2)) {}


    int id;
    QString value1;
    QString value2;
};

//using DataContainer = QVector<Data>;
using DataContainer = std::vector<Data>;

#endif // DATA_H
