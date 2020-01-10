/****************************************************************************
**                                MIT License
**
** Copyright (C) 2018-2020 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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

#include "tableModel.h"
#include <QVector>
#include <QDebug>

TableModel::TableModel(QObject *parent)
    :UpdateableModel<QAbstractTableModel, Data>(parent)
{

}

// Implementing this model is the crux of the example of using UpdateableModel
void TableModel::update(const DataContainer &updatedData)
{
    //tests
    //allow comparing items. Could also have used an operator< on Data
    auto lessThan = [](const Data& lhs, const Data& rhs) {
        return lhs.id < rhs.id;
    };
    //check if the item has changed data, but is otherwise the same object. Needs to return the actual changes made.
    auto itemHasChanged = [](const Data& lhs, const Data& rhs) {
        DataChanges changes;
        if (lhs.value1 != rhs.value1)
            changes.changedColumns.append(1);
        if (lhs.value2 != rhs.value2)
            changes.changedColumns.append(2);
        changes.changedRoles = QVector<int>{Qt::DisplayRole};
        return changes;
    };

    // call the updateData method to trigger merging in the changes to the model
    auto changes = updateData(updatedData.cbegin(), updatedData.cend(), m_data, lessThan, itemHasChanged);

    // the method above returns some stats, so lets print them.
    qDebug() << "changes in model: inserts:" << changes.inserts <<
                " deletes:" << changes.removals <<
                " updates:" << changes.updates;
}

//Under here you'll find the normal QAIM method reimplementations to make a model work
int TableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_data.size();
}

int TableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 3;
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if (!hasIndex(index.row(), index.column(), index.parent()))
        return {};

    if (role != Qt::DisplayRole)
        return {};

    const Data& item = m_data[index.row()];

    switch (index.column()) {
    case 0: return item.id;
    case 1: return item.value1;
    case 2: return item.value2;
    }

    return {};
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical || role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
    case 0: return QStringLiteral("ID");
    case 1: return QStringLiteral("Value 1");
    case 2: return QStringLiteral("Value 2");
    }

    return QVariant();
}
