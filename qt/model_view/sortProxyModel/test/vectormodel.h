/*
  This file is part of KDToolBox.

  Copyright (C) 2018-2019 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: André Somers <andre.somers@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
