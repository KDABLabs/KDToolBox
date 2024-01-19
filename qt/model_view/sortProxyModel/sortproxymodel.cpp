/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: André Somers <andre.somers@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "sortproxymodel.h"
#include <QDebug>
#include <QVariant>
#include <iterator>

#include <private/qabstractitemmodel_p.h>

namespace
{
void buildReverseMap(const std::vector<int> &aToB, std::vector<int> &bToA)
{
    const int size = int(aToB.size());
    bToA.resize(size);
    for (int i = 0; i < size; ++i)
    {
        bToA[aToB[i]] = i;
    }
}
}

SortProxyModel::SortProxyModel(QObject *parent)
    : QAbstractProxyModel(parent)
    , m_invalidatedRows(make_pair(m_proxyToSourceMap.end(), m_proxyToSourceMap.end()))
{
}

QModelIndex SortProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_ASSERT(!parent.isValid()); // we do not support tree models
    Q_UNUSED(parent)

    if (!sourceModel())
        return {};
    if (row >= static_cast<int>(m_proxyToSourceMap.size()))
        return {};
    if (column < 0 || column >= sourceModel()->columnCount())
        return {};

    return createIndex(row, column, static_cast<quintptr>(m_proxyToSourceMap[static_cast<ulong>(row)]));
}

QModelIndex SortProxyModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return {};
}

int SortProxyModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (sourceModel())
    {
        return static_cast<int>(m_proxyToSourceMap.size());
    }
    else
    {
        return 0;
    }
}

int SortProxyModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    const auto source = sourceModel();
    if (source)
    {
        return source->columnCount();
    }
    else
    {
        return 0;
    }
}

/**
 * @brief SortProxyModel::sort sorts the model
 * @param column The column to sort on.
 * @param order The order to use when sorting.
 *
 * The default @arg order is Qt::Ascending order. As per convention, if you
 * pass -1 for @arg column the sorting is disabled. The valid range for
 * @arg column is therefore -1 to columnCount() - 1.
 */
void SortProxyModel::sort(int column, Qt::SortOrder order)
{
    Q_ASSERT(column >= -1 && column < columnCount());

    if (m_sortColumn != column || m_order != order)
    {
        int oldColumn = m_sortColumn;
        int oldOrder = m_order;

        m_sortColumn = column;
        m_order = order;

        reorder();

        if (oldOrder != m_order)
            Q_EMIT sortOrderChanged();
        if (oldColumn != m_sortColumn)
            Q_EMIT sortColumnChanged();
    }
}

QVariant SortProxyModel::data(const QModelIndex &proxyIndex, int role) const
{
    if (proxyIndex.isValid() && !isInvalidedRow(proxyIndex.row()))
    {
        return QAbstractProxyModel::data(proxyIndex, role);
    }
    return {};
}

void SortProxyModel::setSourceModel(QAbstractItemModel *model)
{
    if (model != sourceModel())
    {
        beginResetModel();
        if (sourceModel())
            sourceModel()->disconnect(this);
        QAbstractProxyModel::setSourceModel(model);
        if (model)
        {
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

    // no further bounds checking, out of bounds indices are a breach of contract
    return sourceModel()->index(static_cast<int>(proxyIndex.internalId()), proxyIndex.column());
}

QModelIndex SortProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid())
        return {};

    Q_ASSERT(sourceIndex.model() == sourceModel());

    if (sourceIndex.parent().isValid())
        return {};

    // no further bounds checking, out of bounds indices are a breach of contract
    const auto proxyRow = mapToProxyRow(sourceIndex.row());
    return index(proxyRow, sourceIndex.column());
}

void SortProxyModel::setSortRole(int role)
{
    if (m_sortRole != role)
    {
        m_sortRole = role;
        Q_EMIT sortRoleChanged();
        reorder();
    }
}

int SortProxyModel::sortRole() const
{
    return m_sortRole;
}

void SortProxyModel::setSortCaseSensitivity(Qt::CaseSensitivity sensitivity)
{
    if (m_caseSensitivity != sensitivity)
    {
        m_caseSensitivity = sensitivity;
        Q_EMIT sortCaseSensitivityChanged();
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

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (lhs.typeId() == QMetaType::QString && rhs.typeId() == QMetaType::QString)
#else
    if (lhs.type() == QVariant::String && rhs.type() == QVariant::String)
#endif
    {
        return QString::compare(lhs.toString(), rhs.toString(), m_caseSensitivity) < 0;
    }
    else
    {
        return QAbstractItemModelPrivate::isVariantLessThan(lhs, rhs, m_caseSensitivity, false);
    }
}

void SortProxyModel::resetInternalData()
{
    rebuildRowMap();
}

void SortProxyModel::rebuildRowMap()
{
    // simple initial sort. No emitting of row moves
    m_proxyToSourceMap.clear();
    if (sourceModel())
    {
        m_proxyToSourceMap.resize(static_cast<ulong>(sourceModel()->rowCount()));
        std::iota(m_proxyToSourceMap.begin(), m_proxyToSourceMap.end(), 0);
        sortMappingContainer(m_proxyToSourceMap);
    }
    buildReverseMap(m_proxyToSourceMap, m_sourceToProxyMap);
}

template<class Iterator>
inline Iterator predecessor(Iterator it)
{
    --it;
    return it;
}

template<class Iterator>
inline Iterator successor(Iterator it)
{
    ++it;
    return it;
}

template<class Iterator, class Predicate>
inline Iterator find_if_from_back(const Iterator &begin, const Iterator &end, Predicate predicate)
{
    auto it = end;
    do
    {
        auto pred = predecessor(it);
        if (predicate(*pred))
        {
            return pred;
        }
        it = pred;
    } while (it != begin);

    return end;
}

template<class Iterator>
inline Iterator find_from_back(const Iterator &begin, const Iterator &end, const typename Iterator::value_type &value)
{
    const auto predicate = [value](const typename Iterator::value_type &itemValue) { return itemValue == value; };

    return find_if_from_back(begin, end, predicate);
}

void SortProxyModel::reorder()
{
    // update the sort order. Emits row moves
    if (m_proxyToSourceMap.empty()) // checks emptiness, doesn't empty by itself
        return;

    auto newOrder = m_proxyToSourceMap; // deep copy

    if (m_sortColumn == -1)
    {
        std::iota(newOrder.begin(), newOrder.end(), 0);
    }
    else
    {
        sortMappingContainer(newOrder);
    }
    // during a reorder, we don't try to keep the reverse map in order. We clear and rebuild later.
    m_sourceToProxyMap.clear();

    auto orderedIt = predecessor(newOrder.end());
    auto unorderedIt = predecessor(m_proxyToSourceMap.end());

    while (orderedIt != newOrder.begin())
    {
        if (*orderedIt == *unorderedIt)
        {
            --orderedIt;
            --unorderedIt;
        }
        else
        {
            auto it = find_from_back(m_proxyToSourceMap.begin(), unorderedIt, *orderedIt);
            // we know it is valid, as newOrder is just a permutation of m_rowMap
            int movedRow = static_cast<int>(it - m_proxyToSourceMap.begin());
            int destinationRow = static_cast<int>(unorderedIt - m_proxyToSourceMap.begin()) + 1;
            int moveCount = 1;

            while (it != m_proxyToSourceMap.begin() && orderedIt != newOrder.begin() &&
                   *predecessor(it) == *predecessor(orderedIt))
            {
                ++moveCount;
                --movedRow;
                --it;
                --orderedIt;
            }
            bool ok = beginMoveRows(QModelIndex(), movedRow, movedRow + moveCount - 1, QModelIndex(), destinationRow);
            if (!ok)
            {
                qWarning() << "moveRows from" << movedRow << "up to" << movedRow + moveCount - 1 << "to"
                           << destinationRow;
                QStringList contents;
                contents.reserve(rowCount());
                for (int row = 0; row < rowCount(); ++row)
                {
                    contents << index(row).data(m_sortRole).toString();
                }
                qWarning() << "moving failed. Current contents:" << contents.join(QLatin1String(", "));
            }
            auto rotateEnd = successor(unorderedIt);
            std::rotate(it, it + moveCount, rotateEnd);
            endMoveRows();
            --orderedIt;
            unorderedIt = rotateEnd - (moveCount + 1);
        }
    }

    buildReverseMap(m_proxyToSourceMap, m_sourceToProxyMap);
}

void SortProxyModel::sortMappingContainer(std::vector<int> &container)
{
    if (m_sortColumn == -1)
        return;

    std::sort(container.begin(), container.end(),
              [this](int lhs, int rhs) { return lessThan(lhs, rhs) != (m_order == Qt::DescendingOrder); });
}

bool SortProxyModel::lessThan(int source_left_row, int source_right_row) const
{
    if (m_sortColumn == -1)
        return false;

    return lessThan(sourceModel()->index(source_left_row, m_sortColumn),
                    sourceModel()->index(source_right_row, m_sortColumn));
}

int SortProxyModel::mapToProxyRow(int sourceRow) const
{
    if (!m_sourceToProxyMap.empty())
    {
        // we have an up-to-date reverse mapping, so use that
        return m_sourceToProxyMap[sourceRow];
    }

    // reverse mapping is not up to date, use slower linear search instead
    auto it = std::find(m_proxyToSourceMap.cbegin(), m_proxyToSourceMap.cend(), sourceRow);
    return static_cast<int>(it - m_proxyToSourceMap.cbegin());
}

void SortProxyModel::handleDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                       const QVector<int> &roles)
{
    // Map the row-range
    const int firstSrcRow = topLeft.row();
    const std::vector<int>::size_type rowCnt = bottomRight.row() - firstSrcRow + 1;
    std::vector<int> rows(rowCnt);
    for (int r = 0; r < (int)rowCnt; ++r)
    {
        rows[r] = mapToProxyRow(r + firstSrcRow);
    }
    std::sort(rows.begin(), rows.end());

    // convert the vector of ints indicating changed columns into a vector of pairs of ints indicating ranges.
    // for example, the vector {1, 2, 3, 5, 6, 9} would be converted to {{1, 3}, {5, 6}, {9, 9}}
    struct ColumnRange
    {
        int from;
        int to;
    };

    using ColumnRanges = std::vector<ColumnRange>;
    auto accumulator = [](ColumnRanges ranges, int column) {
        if (ranges.empty() || (ranges.back().to < column - 1))
        {
            ranges.push_back({column, column});
        }
        else
        {
            ranges.back().to = column;
        }
        return ranges;
    };
    const ColumnRanges ranges = std::accumulate(rows.begin(), rows.end(), ColumnRanges(), accumulator);

    // re-emit the dataChanged signals
    for (const auto &range : ranges)
    {
        QModelIndex pTopLeft = index(range.from, topLeft.column());
        QModelIndex pBottomRight = index(range.to, bottomRight.column());
        Q_EMIT dataChanged(pTopLeft, pBottomRight, roles);
    }

    // re-order if needed
    if (roles.isEmpty() || roles.contains(m_sortRole))
    {
        reorder();
    }
}

void SortProxyModel::handleRowsInserted(const QModelIndex &parent, int firstNewRow, int lastNewRow)
{
    if (parent.isValid())
        return;

    // reverse mapping is now invalid
    m_sourceToProxyMap.clear();

    // create a mapping for the new rows
    std::vector<int> newRowsMap;
    newRowsMap.resize(static_cast<ulong>(lastNewRow - firstNewRow + 1));
    std::iota(newRowsMap.begin(), newRowsMap.end(), firstNewRow);
    sortMappingContainer(newRowsMap);

    // update the row indices in the mapping pointing to rows that shifted backwards
    const int shift = lastNewRow - firstNewRow + 1;
    for (auto &oldPos : m_proxyToSourceMap)
    {
        if (oldPos >= firstNewRow)
        {
            oldPos += shift;
        }
    }

    // assure we have enough space for the new items (and avoid reallocations that can break iterators)
    m_proxyToSourceMap.reserve(m_proxyToSourceMap.size() + newRowsMap.size());

    // now merge the new rows into the mapping we already have
    auto newIt = newRowsMap.begin();
    auto curIt = m_proxyToSourceMap.begin();

    while (curIt != m_proxyToSourceMap.end() && newIt != newRowsMap.end())
    {
        if (lessThan(*newIt, *curIt))
        {
            auto firstInsert = newIt;
            // see how many more items we can insert in one go
            while (successor(newIt) != newRowsMap.end() && !lessThan(*curIt, *successor(newIt)))
            {
                ++newIt;
            }
            const auto insertStartPos = static_cast<int>(curIt - m_proxyToSourceMap.begin());
            const auto insertLength = static_cast<int>(newIt - firstInsert) + 1;
            beginInsertRows({}, insertStartPos, insertStartPos + insertLength - 1);
            curIt = m_proxyToSourceMap.insert(curIt, firstInsert, successor(newIt));
            ++curIt;
            endInsertRows();
            ++newIt;
        }
        else
        {
            ++curIt;
        }
    }
    // handle case of insert at the end of the container: insert the remaining items in newRowsMap
    if (curIt == m_proxyToSourceMap.end() && newIt != newRowsMap.end())
    {
        const auto insertStartPos = static_cast<int>(curIt - m_proxyToSourceMap.begin());
        const auto insertLength = static_cast<int>(newRowsMap.end() - newIt);
        beginInsertRows({}, insertStartPos, insertStartPos + insertLength - 1);
        m_proxyToSourceMap.insert(m_proxyToSourceMap.end(), newIt, newRowsMap.end());
        endInsertRows();
    }

    buildReverseMap(m_proxyToSourceMap, m_sourceToProxyMap);
}

void SortProxyModel::handleRowsRemoved(const QModelIndex &parent, int firstRemovedRow, int lastRemovedRow)
{
    if (parent.isValid())
        return;

    // reverse mapping is now invalid
    m_sourceToProxyMap.clear();

    // update the row indices in the mapping pointing to rows that shifted forwards and build
    //   up list of rows to remove
    const int shift = lastRemovedRow - firstRemovedRow + 1;
    std::vector<int> removedRows;
    removedRows.reserve(static_cast<ulong>(shift));
    int row = 0;
    for (auto &oldPos : m_proxyToSourceMap)
    {
        if (oldPos > lastRemovedRow)
        {
            oldPos -= shift;
        }
        else if (oldPos >= firstRemovedRow)
        {
            removedRows.push_back(row);
        }
        ++row;
    }

    std::sort(removedRows.begin(), removedRows.end());

    m_invalidatedRows = make_pair(removedRows.begin(), removedRows.end());

    // iterates backwards through the list of rows to remove so the indices in removedRows stay
    //   correct during the iteration
    auto it = predecessor(removedRows.end());
    for (;;)
    {
        auto lastRowToRemove = *it;
        // see if we have consecutive rows we can remove in one go
        while (it != removedRows.begin() && *predecessor(it) == *it - 1)
            --it;
        auto firstRowToRemove = *it;
        beginRemoveRows({}, firstRowToRemove, lastRowToRemove);
        m_proxyToSourceMap.erase(m_proxyToSourceMap.begin() + firstRowToRemove,
                                 m_proxyToSourceMap.begin() + lastRowToRemove + 1);
        m_invalidatedRows.second = it;
        endRemoveRows();

        if (it == removedRows.begin())
            break;
        --it;
    }

    m_invalidatedRows = make_pair(m_proxyToSourceMap.end(), m_proxyToSourceMap.end());

    buildReverseMap(m_proxyToSourceMap, m_sourceToProxyMap);
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
    // m_invalidatedRows only contains a valid range during a remove operation that involves multiple rows
    return std::any_of(m_invalidatedRows.first, m_invalidatedRows.second,
                       [row](int invalidated) { return invalidated == row; });
}
