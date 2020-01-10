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

#ifndef VECTORMODEL_H
#define VECTORMODEL_H
#include <QAbstractListModel>
#include <algorithm>

/**
 * Simple vector-based test model for the unit tests. It is templated so it can operate on different
 * types of content, allowing tests with integers, doubles, strings, etc.
 * This class obviously does not have production quality, it is only meant to provide a source model
 * for unit testing.
 */
template <class value_type>
class VectorModel: public QAbstractListModel
{
    //cannot use Q_OBJECT but that's ok here

public:
    VectorModel(std::initializer_list<value_type> initialContents) {
        contents.reserve(static_cast<int>(initialContents.size()));
        std::copy(initialContents.begin(), initialContents.end(), std::back_inserter(contents));
    }

    void setValue(int row, value_type value) {
        if (contents[row] != value) {
            contents[row] = value;
            const auto idx = index(row);
            emit dataChanged(idx, idx, QVector<int>{Qt::DisplayRole});
        }
    }

    void append(value_type value) {
        beginInsertRows(QModelIndex(), contents.count(), contents.count());
        contents.append(value);
        endInsertRows();
    }

    void append(std::initializer_list<value_type> values) {
        beginInsertRows(QModelIndex(), contents.count(), contents.count() + static_cast<int>(values.size()) -1);
        contents.append(values);
        endInsertRows();
    }

    void insert(int row, value_type value) {
        beginInsertRows({}, row, row);
        contents.insert(row, value);
        endInsertRows();
    }

    void insert(int row, std::initializer_list<value_type> values) {
        beginInsertRows({}, row, row +  static_cast<int>(values.size()) -1);
        contents.reserve( contents.size() + static_cast<int>(values.size()));
        auto oldEnd = contents.end();
        contents.append(values);
        std::rotate(contents.begin() + row, oldEnd, contents.end());
        endInsertRows();
    }

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return 0;

        return contents.count();
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (role == Qt::DisplayRole) {
            return QVariant::fromValue(contents[index.row()]);
        }

        return {};
    }

    bool removeRows(int row, int count, const QModelIndex &parent = {}) override
    {
        if (row < 0 || count < 0 || row + count >= contents.count())
            return false;

        beginRemoveRows(parent, row, row + count -1);
        auto it = std::rotate(contents.begin() + row, contents.begin() + row + count, contents.end());
        contents.erase(it, contents.end());
        endRemoveRows();

        return true;
    }

public:
    QVector<value_type> contents;

    // QAbstractItemModel interface
};

#endif // VECTORMODEL_H
