/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: André Somers <andre.somers@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "../src/ModelIterator.h"
#include <QCoreApplication>
#include <QDebug>
#include <QModelIndex>
#include <QStandardItem>
#include <QStandardItemModel>
#include <algorithm>

template<typename T>
void addSubtree(T *node, int depth, int count, const QString &text)
{
    for (int i = 0; i < count; ++i)
    {
        QString itemText = text + QLatin1Char('.') + QString::number(i);
        auto item = new QStandardItem(itemText);
        if (depth > 1)
        {
            addSubtree(item, depth - 1, count, itemText);
        }
        node->appendRow(item);
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QStandardItemModel model;

    // build up test tree
    addSubtree(&model, 3, 5, QStringLiteral("node "));

    qDebug() << "root row count" << model.rowCount();

    qDebug() << "flat iterator, linear scan:";
    auto adapter = ModelAdapter<FlatIterator>(&model);
    for (const auto &index : adapter)
    {
        qDebug() << index.data() << index;
    }

    qDebug() << "There are" << adapter.end() - adapter.begin() << "nodes in the adapter";

    qDebug() << "\n\nDepth-first iterator, linear scan:";
    auto adapter2 = ModelAdapter<DepthFirstIterator>(&model);
    for (const auto &index : adapter2)
    {
        qDebug() << index.data() << index;
    }

    // The following should not compile, as adaptor2 wraps a DepthFirstIterator that doesn't support getting the size
    // qDebug() << "node count:" << adapter2.size();

    qDebug() << "\n\nDepth-first iterator, std::find_if:";
    auto it = std::find_if(adapter2.begin(), adapter2.end(),
                           [](const QModelIndex &index) { return index.data().toString() == u"node .3.2.4"; });
    if (it == adapter2.end())
    {
        qDebug() << "Node not found!";
    }
    else
    {
        qDebug() << "Node found:" << *it << it->data() << "\n\n";
    }

    // check if it works on an empty model
    QStandardItemModel emptyModel(0, 2);
    qDebug() << "empty model count" << emptyModel.rowCount();
    auto emptyModelTest = [](auto emptyAdaptor) {
        for (const auto &index : emptyAdaptor)
        {
            qDebug() << index.data() << index; // Should not yield anything
        }
        auto invalidIt = std::find_if(emptyAdaptor.begin(), emptyAdaptor.end(),
                                      [](const QModelIndex &index) { return index.data().toString() == u"test"; });
        if (invalidIt == emptyAdaptor.end())
        {
            qDebug() << "   Item not found in empty model.";
        }
        else
        {
            qWarning() << "   Item found in empty model. That is REALLY strange...";
        }
    };

    auto emptyAdapterFlat = ModelAdapter<FlatIterator>(&emptyModel);
    qDebug() << "Testing empty model with FlatIterator...";
    emptyModelTest(emptyAdapterFlat);

    auto emptyAdaptorDepthFirst = ModelAdapter<DepthFirstIterator>(&emptyModel);
    qDebug() << "Testing empty model with DepthFirstIterator...";
    emptyModelTest(emptyAdaptorDepthFirst);

    // DataValueWrapper tests
    auto it2 = DataValueWrapper<DepthFirstIterator, QString>::begin(&model);
    const auto end2 = DataValueWrapper<DepthFirstIterator, QString>::end(&model);

    qDebug() << "\n\nDepth-first iterator, std::find on DataValueWrapped iterators:";
    auto findit = std::find(it2, end2, QStringLiteral("node .3.2.4")); // we're looking directly for the value
    if (findit == end2)
    {
        qDebug() << "Node not found!";
    }
    else
    {
        qDebug() << "Node found:" << *findit << findit.index() << "\n\n"; // Note that we can still access the
                                                                          // modelindex
    }

    // Assign to DatavalueWrapper iterator. This will call setData on the model
    while (it2 != end2)
    {
        *it2 = QStringLiteral("visited");
        ++it2;
    }

    // We can use ModelAdapter with an additional template argument to iterate over that type
    qDebug() << "\nvisited all top nodes?";
    auto adapter3 = ModelAdapter<FlatIterator, QString>(&model);
    for (const QString &label : adapter3)
    { // use adapter with ranged for loop
        qDebug() << label;
    }

    // And that adapter works with standard algorithms too
    if (std::all_of(adapter3.begin(), adapter3.end(),
                    [](const QString &label) { return label == QStringLiteral("visited"); }))
    {
        qDebug() << "yes, all visited";
    }
    else
    {
        qDebug() << "no, not all visited!";
    }

    qDebug() << "We visited" << adapter3.size() << "nodes";
    return 0;
}
