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
#include "sortproxymodel.h"
#include <QVariant>
#include <iterator>
#include <QDebug>

SortProxyModel::SortProxyModel(QObject *parent)
    : QAbstractProxyModel(parent)
    , m_invalidatedRows(make_pair(m_rowMap.end(), m_rowMap.end()))
{
}

QModelIndex SortProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_ASSERT(!parent.isValid()); //we do not support tree models
    Q_UNUSED(parent);

    if (!sourceModel())
        return {};
    if (row >= m_rowMap.size())
        return {};
    if (column < 0 || column >= sourceModel()->columnCount())
        return {};

    return createIndex(row, column, m_rowMap[row]);
}

QModelIndex SortProxyModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);
    return {};
}

int SortProxyModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (sourceModel()) {
        return static_cast<int>(m_rowMap.size());
    } else {
        return 0;
    }
}

int SortProxyModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    const auto source = sourceModel();
    if (source) {
        return source->columnCount();
    } else {
        return 0;
    }
}

void SortProxyModel::sort(int column, Qt::SortOrder order)
{
    Q_ASSERT(column >= 0 && column < columnCount());

    if (m_sortColumn != column || m_order != order) {
        int oldColumn = m_sortColumn;
        int oldOrder = m_order;

        m_sortColumn = column;
        m_order = order;

        reorder();

        if (oldOrder != m_order)
            emit sortOrderChanged();
        if (oldColumn != m_sortColumn)
            emit sortColumnChanged();
    }
}

QVariant SortProxyModel::data(const QModelIndex &proxyIndex, int role) const
{
    if (proxyIndex.isValid() && !isInvalidedRow(proxyIndex.row())) {
        return QAbstractProxyModel::data(proxyIndex, role);
    }
    return {};
}

void SortProxyModel::setSourceModel(QAbstractItemModel *model)
{
    if (model != sourceModel()) {
        beginResetModel();
        if (sourceModel())
            sourceModel()->disconnect(this);
        QAbstractProxyModel::setSourceModel(model);
        if (model) {
            connect(model, &QAbstractItemModel::dataChanged, this, &SortProxyModel::handleDataChanged);
            connect(model, &QAbstractItemModel::rowsInserted, this, &SortProxyModel::handleRowsInserted);
            connect(model, &QAbstractItemModel::rowsRemoved, this, &SortProxyModel::handleRowsRemoved);
        }
        endResetModel();
    }
}

QModelIndex SortProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!proxyIndex.isValid())
        return {};

    Q_ASSERT(proxyIndex.model() == this);

    //no further bounds checking, out of bounds indices are a breach of contract
    return sourceModel()->index(static_cast<int>(proxyIndex.internalId()), proxyIndex.column());
}

QModelIndex SortProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid())
        return {};

    Q_ASSERT(sourceIndex.model() == sourceModel());

    if (sourceIndex.parent().isValid())
        return {};

    //no further bounds checking, out of bounds indices are a breach of contract
    auto it = std::find(m_rowMap.cbegin(), m_rowMap.cend(), sourceIndex.row());
    return index(it - m_rowMap.cbegin(), sourceIndex.column());
}

void SortProxyModel::setSortRole(int role)
{
    if (m_sortRole != role) {
        m_sortRole = role;
        emit sortRoleChanged();
        reorder();
    }
}

int SortProxyModel::sortRole() const
{
    return m_sortRole;
}

void SortProxyModel::setSortCaseSensitivity(Qt::CaseSensitivity sensitivity)
{
    if (m_caseSensitivity != sensitivity) {
        m_caseSensitivity = sensitivity;
        emit sortCaseSensitivityChanged();
        reorder();
    }
}

Qt::CaseSensitivity SortProxyModel::sortCaseSensitivity() const
{
    return m_caseSensitivity;
}

int SortProxyModel::sortColumn() const
{
    return m_sortColumn;
}

Qt::SortOrder SortProxyModel::sortOrder() const
{
    return m_order;
}

bool SortProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    const QVariant lhs = source_left.data(m_sortRole);
    const QVariant rhs = source_right.data(m_sortRole);

    if (lhs.type() == QVariant::String && rhs.type() == QVariant::String) {
        return QString::compare(lhs.toString(), rhs.toString(), m_caseSensitivity) < 0;
    } else {
        return lhs < rhs;
    }
}

void SortProxyModel::resetInternalData()
{
    rebuildRowMap();
}

void SortProxyModel::rebuildRowMap()
{
    //simple initial sort. No emitting of row moves
    m_rowMap.clear();
    if (sourceModel()) {
        m_rowMap.resize(sourceModel()->rowCount());
        std::iota(m_rowMap.begin(), m_rowMap.end(), 0);
        sortMappingContainer(m_rowMap);
    };
}

template <class Iterator>
inline Iterator predecessor(Iterator it)
{
    --it;
    return it;
}

template <class Iterator>
inline Iterator successor(Iterator it)
{
    ++it;
    return it;
}

template <class Iterator, class Predicate>
inline Iterator find_if_from_back(const Iterator& begin, const Iterator& end, Predicate predicate)
{
    auto it = end;
    do {
        auto pred = predecessor(it);
        if (predicate(*pred)) {
            return pred;
        }
        it = pred;
    } while (it != begin);

    return end;
}

template <class Iterator>
inline Iterator find_from_back(const Iterator& begin, const Iterator& end, const typename Iterator::value_type &value)
{
    const auto predicate = [value](const typename Iterator::value_type &itemValue){return itemValue == value;};

    return find_if_from_back(begin, end, predicate);
}

void SortProxyModel::reorder()
{
    //update the sort order. Emits row moves
    if (m_rowMap.empty()) //checks emptyness, doesn't empty by itself
        return;

    auto newOrder = m_rowMap; //deep copy

    sortMappingContainer(newOrder);

    auto orderedIt = predecessor( newOrder.end() );
    auto unorderedIt = predecessor ( m_rowMap.end() );

    while (orderedIt != newOrder.begin()) {
        if (*orderedIt == *unorderedIt) {
            --orderedIt; --unorderedIt;
        } else {
            auto it = find_from_back(m_rowMap.begin(), unorderedIt, *orderedIt);
            //we know it is valid, as newOrder is just a permutation of m_rowMap
            int movedRow = static_cast<int>(it - m_rowMap.begin());
            int destinationRow = static_cast<int>(unorderedIt - m_rowMap.begin()) + 1;
            int moveCount = 1;

            while (it != m_rowMap.begin() &&
                   orderedIt != newOrder.begin() &&
                   *predecessor(it) == *predecessor(orderedIt))
            {
                ++moveCount;
                --movedRow;
                --it;
                --orderedIt;
            }
            bool ok = beginMoveRows(QModelIndex(), movedRow, movedRow + moveCount - 1, QModelIndex(), destinationRow);
            if (!ok) {
                qWarning() << "moveRows from" << movedRow << "up to" << movedRow + moveCount - 1 << "to" << destinationRow;
                QStringList contents;
                contents.reserve(rowCount());
                for(int row = 0; row < rowCount(); ++row) {
                    contents << index(row).data(m_sortRole).toString();
                }
                qWarning() << "moving failed. Current contents:" << contents.join(", ");
            }
            auto rotateEnd = successor(unorderedIt);
            std::rotate(it, it + moveCount, rotateEnd);
            endMoveRows();
            --orderedIt;
            unorderedIt = rotateEnd - (moveCount + 1);
        }
    }
}

void SortProxyModel::sortMappingContainer(std::vector<int> &container)
{
    std::sort(container.begin(), container.end(), [this](int lhs, int rhs){
        return lessThan(lhs, rhs) != (m_order == Qt::DescendingOrder);
    });
}

bool SortProxyModel::lessThan(int source_left_row, int source_right_row) const
{
    return lessThan(sourceModel()->index(source_left_row, m_sortColumn),
                    sourceModel()->index(source_right_row, m_sortColumn));
}

void SortProxyModel::handleDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, QVector<int> roles)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);

    if (roles.isEmpty() || roles.contains(m_sortRole)) {
        reorder();
    }
}

void SortProxyModel::handleRowsInserted(const QModelIndex &parent, int firstNewRow, int lastNewRow)
{
    if (parent.isValid())
        return;

    //create a mapping for the new rows
    std::vector<int> newRowsMap;
    newRowsMap.resize(lastNewRow - firstNewRow + 1);
    std::iota(newRowsMap.begin(), newRowsMap.end(), firstNewRow);
    sortMappingContainer(newRowsMap);

    //update the row indices in the mapping pointing to rows that shifted backwards
    const int shift = lastNewRow - firstNewRow + 1;
    for(auto &oldPos : m_rowMap) {
        if (oldPos >= firstNewRow) {
            oldPos += shift;
        }
    }

    //assure we have enough space for the new items (and avoid reallocations that can break iterators)
    m_rowMap.reserve(m_rowMap.size() + newRowsMap.size());

    //now merge the new rows into the mapping we already have
    auto newIt = newRowsMap.begin();
    auto curIt = m_rowMap.begin();

    while(curIt != m_rowMap.end() && newIt != newRowsMap.end()) {
        if (lessThan(*newIt, *curIt)) {
            auto firstInsert = newIt;
            //see how many more items we can insert in one go
            while (successor(newIt) != newRowsMap.end() && !lessThan(*curIt, *successor(newIt))) {
                ++newIt;
            }
            const auto insertStartPos = static_cast<int>(curIt - m_rowMap.begin());
            const auto insertLength = static_cast<int>(newIt - firstInsert) + 1;
            beginInsertRows({}, insertStartPos, insertStartPos + insertLength - 1);
            curIt = m_rowMap.insert(curIt, firstInsert, successor(newIt));
            ++curIt;
            endInsertRows();
            ++newIt;
        } else {
            ++curIt;
        }
    }
}

void SortProxyModel::handleRowsRemoved(const QModelIndex &parent, int firstRemovedRow, int lastRemovedRow)
{
    if (parent.isValid())
        return;

    //update the row indices in the mapping pointing to rows that shifted forwards and build
    //  up list of rows to remove
    const int shift = lastRemovedRow - firstRemovedRow + 1;
    std::vector<int> removedRows;
    removedRows.reserve(shift);
    int row = 0;
    for(auto &oldPos : m_rowMap) {
        if (oldPos > lastRemovedRow) {
            oldPos -= shift;
        } else if (oldPos >= firstRemovedRow) {
            removedRows.push_back(row);
        }
        ++row;
    }

    std::sort(removedRows.begin(), removedRows.end());

    m_invalidatedRows = make_pair(removedRows.begin(), removedRows.end());

    //iterates backwards through the list of rows to remove so the indices in removedRows stay
    //  correct during the iteration
    auto it = predecessor(removedRows.end());
    forever {
        auto lastRowToRemove = *it;
        //see if we have consequtive rows we can remove in one go
        while (it != removedRows.begin() && *predecessor(it) == *it - 1) --it;
        auto firstRowToRemove = *it;
        beginRemoveRows({}, firstRowToRemove, lastRowToRemove);
        m_rowMap.erase(m_rowMap.begin() + firstRowToRemove, m_rowMap.begin() + lastRowToRemove + 1);
        m_invalidatedRows.second = it;
        endRemoveRows();

        if (it == removedRows.begin())
            break;
        --it;
    }

    m_invalidatedRows = make_pair(m_rowMap.end(), m_rowMap.end());
}

/**
 * @brief SortProxyModel::isInvalidedRow
 * @param row
 * @returns true if the indicated row has already been removed from the source model
 *
 * During a remove operation, we cannot always send a single rowsRemoved signal. That means
 * we will be signalling to the outside world during while the source model has already
 * removed some rows that we still have in the model. However, we cannot access these rows
 * any more, as the indexes either point to rows outside the range of the model or point to
 * rows that have shifted into the position that the row was in.) If such a row is accessed
 * via the proxy, we will return an invalid QVariant.
 */
bool SortProxyModel::isInvalidedRow(const int row) const
{
    //m_invalidatedRows only contains a valid range during a remove operation that involves multiple rows
    return std::any_of(m_invalidatedRows.first, m_invalidatedRows.second,
                       [row](int invalidated) {return invalidated==row;} );
}
