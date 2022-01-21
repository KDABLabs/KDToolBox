/****************************************************************************
**                                MIT License
**
** Copyright (C) 2018-2022 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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

#pragma once

#include <QAbstractListModel>

#include <vector>

/**
 * Simple vector-based test model for the unit tests. It is templated so it can operate on different
 * types of content, allowing tests with integers, doubles, strings, etc.
 * This class obviously does not have production quality, it is only meant to provide a source model
 * for unit testing.
 */
template <class T>
class VectorModel: public QAbstractListModel
{
    //cannot use Q_OBJECT but that's ok here

    int size() const noexcept { return static_cast<int>(contents.size()); }
public:
    explicit VectorModel(std::initializer_list<T> initialContents, QObject *parent = nullptr)
        : QAbstractListModel(parent),
          contents(initialContents)
    {
    }

    void setValue(int row, const T& value) {
        auto &e = contents[row];
        if (e != value) {
            e = value;
            const auto idx = index(row);
            Q_EMIT dataChanged(idx, idx, QVector<int>{Qt::DisplayRole});
        }
    }

    void append(const T &value) {
        beginInsertRows(QModelIndex(), size(), size());
        contents.push_back(value);
        endInsertRows();
    }

    void append(std::initializer_list<T> values) {
        beginInsertRows(QModelIndex(), size(), size() + static_cast<int>(values.size()) -1);

        contents.insert(contents.end(), values);
        endInsertRows();
    }

    void insert(int row, const T& value) {
        beginInsertRows({}, row, row);
        contents.insert(contents.begin() + row, value);
        endInsertRows();
    }

    void insert(int row, std::initializer_list<T> values) {
        beginInsertRows({}, row, row +  static_cast<int>(values.size()) -1);
        contents.insert(contents.begin() + row, values);
        endInsertRows();
    }

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return 0;

        return size();
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
        if (row < 0 || count < 0 || row + count >= size())
            return false;

        beginRemoveRows(parent, row, row + count -1);
        const auto first = contents.begin() + row;
        const auto last = first + count;
        contents.erase(first, last);
        endRemoveRows();

        return true;
    }

public:
    std::vector<T> contents;

    // QAbstractItemModel interface
};
