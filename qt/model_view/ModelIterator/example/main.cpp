/****************************************************************************
**                                MIT License
**
** Copyright (C) 2018-2020 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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

#include <QCoreApplication>
#include "../src/ModelIterator.h"
#include <QStandardItemModel>
#include <QStandardItem>
#include <QDebug>
#include <QModelIndex>
#include <algorithm>

template<typename T>
void addSubtree(T *node, int depth, int count, QString text)
{
    for (int i(0);i<count; ++i) {
        QString itemText = text + "." + QString::number(i);
        auto item = new QStandardItem(itemText);
        if (depth > 1) {
            addSubtree(item, depth - 1, count, itemText);
        }
        node->appendRow(item);
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QStandardItemModel model;

    //build up test tree
    addSubtree(&model, 3, 5, QString("node "));

    qDebug() << "root row count" << model.rowCount();

    qDebug() << "flat iterator, linear scan:";
    auto adapter = ModelAdapter<FlatIterator>(&model);
    for(const auto& index: adapter) {
        qDebug() << index.data() << index;
    }

    qDebug() << "There are" << adapter.end() - adapter.begin() << "nodes in the adapter";

    qDebug() << "\n\nDepth-first iterator, linear scan:";
    auto adapter2 = ModelAdapter<DepthFirstIterator>(&model);
    for(const auto& index: adapter2) {
        qDebug() << index.data() << index;
    }

    qDebug() << "\n\nDepth-first iterator, std::find_if:";
    auto it = std::find_if(adapter2.begin(), adapter2.end(), [](const QModelIndex& index)
    {
        return index.data().toString() == "node .3.2.4";
    });
    if (it == adapter2.end()) {
        qDebug() << "Node not found!";
    } else {
        qDebug() << "Node found:" << *it << it->data() << "\n\n";
    }

    //check if it works on an empty model
    QStandardItemModel emptyModel(0,2);
    qDebug() << "empty model count" << emptyModel.rowCount();
    auto emptyModelTest = [](auto emptyAdaptor) {
        for (const auto& index: emptyAdaptor) {
            qDebug() << index.data() << index; //Should not yield anything
        }
        auto invalidIt = std::find_if(emptyAdaptor.begin(), emptyAdaptor.end(), [](const QModelIndex& index)
                                      {
                                          return index.data().toString() == "test";
                                      });
        if (invalidIt == emptyAdaptor.end()) {
            qDebug() << "   Item not found in empty model.";
        } else {
            qWarning() << "   Item found in empty model. That is REALLY strange...";
        }
    };

    auto emptyAdapterFlat = ModelAdapter<FlatIterator>(&emptyModel);
    qDebug() << "Testing empty model with FlatIterator...";
    emptyModelTest(emptyAdapterFlat);

    auto emptyAdaptorDepthFirst = ModelAdapter<DepthFirstIterator>(&emptyModel);
    qDebug() << "Testing empty model with DepthFirstIterator...";
    emptyModelTest(emptyAdaptorDepthFirst);


    //DataValueWrapper tests
    auto it2 = DataValueWrapper<DepthFirstIterator, QString>::begin(&model);
    const auto end2 = DataValueWrapper<DepthFirstIterator, QString>::end(&model);

    qDebug() << "\n\nDepth-first iterator, std::find on DataValueWrapped iterators:";
    auto findit = std::find(it2, end2, "node .3.2.4"); //we're looking directly for the value
    if (findit == end2) {
        qDebug() << "Node not found!";
    } else {
        qDebug() << "Node found:" << *findit << findit.index() << "\n\n"; //Note that we can still access the modelindex
    }

    //Assign to DatavalueWrapper iterator. This will call setData on the model
    while (it2 != end2) {
        *it2=QStringLiteral("visited");
        ++it2;
    }

    //We can use ModelAdapter with an additional template argument to iterate over that type
    qDebug() << "\nvisited all top nodes?";
    auto adapter3 = ModelAdapter<FlatIterator, QString>(&model);
    for (const QString &label: adapter3) { //use adapter with ranged for loop
        qDebug() << label;
    }

    //And that adapter works with standard algorithms too
    if (std::all_of(adapter3.begin(), adapter3.end(), [](const QString& label) {
                    return label == QStringLiteral("visited");
    })) {
        qDebug() << "yes, all visited";
    } else {
        qDebug() << "no, not all visited!";
    }

    return 0;
}
