/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "KDTableToListProxyModel.h"

#include <algorithm>

KDTableToListProxyModel::KDTableToListProxyModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_sourceModel(nullptr)
{
}

QAbstractItemModel *KDTableToListProxyModel::sourceModel() const
{
    return m_sourceModel;
}

template<typename Func1, typename Func2>
void KDTableToListProxyModel::connectToSourceModel(Func1 signal, Func2 slot)
{
    Q_ASSERT(m_sourceModel);
    auto connection = connect(m_sourceModel, signal, this, slot);
    m_sourceModelConnections.push_back(std::move(connection));
}

void KDTableToListProxyModel::setSourceModel(QAbstractItemModel *model)
{
    if (m_sourceModel == model)
    {
        return;
    }

    beginResetModel();

    if (m_sourceModel)
    {
        for (const auto &connection : m_sourceModelConnections)
        {
            QObject::disconnect(connection);
        }
        m_sourceModelConnections.clear();
    }

    m_sourceModel = model;

    if (m_sourceModel)
    {
        m_sourceModelConnections.reserve(16);
        connectToSourceModel(&QObject::destroyed, &KDTableToListProxyModel::sourceModelDestroyed);

        connectToSourceModel(&QAbstractItemModel::dataChanged, &KDTableToListProxyModel::dataChangedInSourceModel);
        connectToSourceModel(&QAbstractItemModel::headerDataChanged,
                             &KDTableToListProxyModel::headerDataChangedInSourceModel);

        connectToSourceModel(&QAbstractItemModel::layoutAboutToBeChanged,
                             &KDTableToListProxyModel::layoutAboutToBeChangedInSourceModel);
        connectToSourceModel(&QAbstractItemModel::layoutChanged, &KDTableToListProxyModel::layoutChangedInSourceModel);

        connectToSourceModel(&QAbstractItemModel::rowsAboutToBeInserted,
                             &KDTableToListProxyModel::rowsAboutToBeInsertedInSourceModel);
        connectToSourceModel(&QAbstractItemModel::rowsInserted, &KDTableToListProxyModel::rowsInsertedInSourceModel);

        connectToSourceModel(&QAbstractItemModel::rowsAboutToBeRemoved,
                             &KDTableToListProxyModel::rowsAboutToBeRemovedInSourceModel);
        connectToSourceModel(&QAbstractItemModel::rowsRemoved, &KDTableToListProxyModel::rowsRemovedInSourceModel);

        connectToSourceModel(&QAbstractItemModel::columnsInserted,
                             &KDTableToListProxyModel::columnsInsertedInSourceModel);

        connectToSourceModel(&QAbstractItemModel::columnsRemoved,
                             &KDTableToListProxyModel::columnsRemovedInSourceModel);

        connectToSourceModel(&QAbstractItemModel::modelAboutToBeReset,
                             &KDTableToListProxyModel::modelAboutToBeResetInSourceModel);
        connectToSourceModel(&QAbstractItemModel::modelReset, &KDTableToListProxyModel::modelResetInSourceModel);

        connectToSourceModel(&QAbstractItemModel::rowsAboutToBeMoved,
                             &KDTableToListProxyModel::rowsAboutToBeMovedInSourceModel);
        connectToSourceModel(&QAbstractItemModel::rowsMoved, &KDTableToListProxyModel::rowsMovedInSourceModel);

        connectToSourceModel(&QAbstractItemModel::columnsMoved, &KDTableToListProxyModel::columnsMovedInSourceModel);
    }

    endResetModel();
}

QModelIndex KDTableToListProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid())
    {
        // root
        return QModelIndex();
    }

    if (sourceIndex.parent().isValid())
    {
        // non-top-level index
        return QModelIndex();
    }

    return index(sourceIndex.row(), 0);
}

void KDTableToListProxyModel::setRoleMapping(int column, int dataRole, const QByteArray &roleName, int columnRole)
{
    Q_ASSERT(column >= 0);
    Q_ASSERT(dataRole >= 0);
    Q_ASSERT(columnRole >= 0);

    Q_ASSERT(m_sourceModel);
    Q_ASSERT(column < m_sourceModel->columnCount());

    // There is no signal to notify a change in the role names... must reset the model.
    beginResetModel();

    RoleMapping mapping{dataRole, column, roleName, columnRole};

    const auto it = findRoleMapping(dataRole);
    if (it == m_roleMappings.end())
    {
        m_roleMappings.push_back(std::move(mapping));
    }
    else
    {
        *it = std::move(mapping);
    }

    endResetModel();
}

void KDTableToListProxyModel::unsetRoleMapping(int dataRole)
{
    Q_ASSERT(dataRole >= 0);
    const auto it = findRoleMapping(dataRole);
    if (it == m_roleMappings.end())
    {
        return;
    }

    beginResetModel();
    m_roleMappings.erase(it);
    endResetModel();
}

QModelIndex KDTableToListProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_ASSERT(!parent.isValid());
    Q_ASSERT(column == 0);
    return createIndex(row, column);
}

QModelIndex KDTableToListProxyModel::parent(const QModelIndex &child) const
{
    Q_ASSERT(checkIndex(child, CheckIndexOption::DoNotUseParent));
    return QModelIndex();
}

int KDTableToListProxyModel::rowCount(const QModelIndex &parent) const
{
    Q_ASSERT(checkIndex(parent));
    if (parent.isValid())
    {
        return 0;
    }

    if (m_sourceModel)
    {
        return m_sourceModel->rowCount();
    }
    return 0;
}

int KDTableToListProxyModel::columnCount(const QModelIndex &parent) const
{
    Q_ASSERT(checkIndex(parent));
    if (parent.isValid())
    {
        return 0;
    }

    if (m_sourceModel)
    {
        return 1;
    }
    return 0;
}

QVariant KDTableToListProxyModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid));
    Q_ASSERT(m_sourceModel);

    auto it = findRoleMapping(role);
    if (it == m_roleMappings.end())
    {
        return QVariant();
    }

    auto sourceIndex = m_sourceModel->index(index.row(), it->column);
    return m_sourceModel->data(sourceIndex, it->columnRole);
}

bool KDTableToListProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid));
    Q_ASSERT(m_sourceModel);

    auto it = findRoleMapping(role);
    if (it == m_roleMappings.end())
    {
        return false;
    }

    auto sourceIndex = m_sourceModel->index(index.row(), it->column);
    return m_sourceModel->setData(sourceIndex, value, it->columnRole);
}

bool KDTableToListProxyModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles_)
{
    Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid));
    Q_ASSERT(m_sourceModel);

    // We'll modify roles; take a copy
    auto roles = roles_;

    bool ok = true;

    // Partition roles based on the column they refer to.
    // The idea is to use that column to build an index in the source model,
    // then call setItemData there (with the partition of values to set).
    while (ok && !roles.empty())
    {
        auto rolesIt = roles.begin();

        // Find the column we're using as partitioning criterion
        const auto roleMappingIt = constFindRoleMapping(rolesIt.key());
        if (roleMappingIt == m_roleMappings.cend())
        {
            // Role not mapped to any column; remove and restart
            rolesIt = roles.erase(rolesIt);
            continue;
        }
        const int partitioningColumn = roleMappingIt->column;
        Q_ASSERT(partitioningColumn >= 0);

        // Find all the data roles (in this model) mapping to partitioningColumn;
        // remove them from roles, and add the corresponding ones in the source
        // model to a new map.
        QMap<int, QVariant> rolesForSameSourceIndex;
        while (rolesIt != roles.end())
        {
            const auto roleMappingIt = constFindRoleMapping(rolesIt.key());
            if (roleMappingIt == m_roleMappings.cend())
            {
                rolesIt = roles.erase(rolesIt);
                continue;
            }

            if (roleMappingIt->column == partitioningColumn)
            {
                rolesForSameSourceIndex.insert(roleMappingIt->columnRole, std::move(rolesIt.value()));
                rolesIt = roles.erase(rolesIt);
            }
            else
            {
                ++rolesIt;
            }
        }

        // Now we have all the roles to set for a specific column, get the model index
        // for it, and call setItemData on the source model.
        const QModelIndex sourceIndex = m_sourceModel->index(index.row(), partitioningColumn);
        ok = ok && m_sourceModel->setItemData(sourceIndex, rolesForSameSourceIndex);
    }

    return ok;
}

QVariant KDTableToListProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (orientation)
    {
    case Qt::Horizontal:
        break;
    case Qt::Vertical:
        if (m_sourceModel)
        {
            return m_sourceModel->headerData(section, orientation, role);
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

bool KDTableToListProxyModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    switch (orientation)
    {
    case Qt::Horizontal:
        break;
    case Qt::Vertical:
        if (m_sourceModel)
        {
            return m_sourceModel->setHeaderData(section, orientation, value, role);
        }
    }

    return QAbstractItemModel::setHeaderData(section, orientation, value, role);
}

bool KDTableToListProxyModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_ASSERT(!parent.isValid());
    if (!m_sourceModel)
    {
        return false;
    }
    return m_sourceModel->insertRows(row, count);
}

bool KDTableToListProxyModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_ASSERT(!parent.isValid());
    if (!m_sourceModel)
    {
        return false;
    }
    return m_sourceModel->removeRows(row, count);
}

bool KDTableToListProxyModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                                       const QModelIndex &destinationParent, int destinationChild)
{
    Q_ASSERT(!sourceParent.isValid());
    Q_ASSERT(!destinationParent.isValid());
    return m_sourceModel->moveRows(QModelIndex(), sourceRow, count, QModelIndex(), destinationChild);
}

void KDTableToListProxyModel::fetchMore(const QModelIndex &parent)
{
    Q_ASSERT(!parent.isValid());
    if (!m_sourceModel)
    {
        return;
    }
    m_sourceModel->fetchMore(QModelIndex());
}

bool KDTableToListProxyModel::canFetchMore(const QModelIndex &parent) const
{
    Q_ASSERT(!parent.isValid());
    if (!m_sourceModel)
    {
        return false;
    }
    return m_sourceModel->canFetchMore(QModelIndex());
}

QHash<int, QByteArray> KDTableToListProxyModel::roleNames() const
{
    QHash<int, QByteArray> result;

    for (const auto &mapping : m_roleMappings)
    {
        if (!mapping.roleName.isEmpty())
        {
            result[mapping.dataRole] = mapping.roleName;
        }
    }

    return result;
}

std::vector<KDTableToListProxyModel::RoleMapping>::iterator KDTableToListProxyModel::findRoleMapping(int dataRole)
{
    return std::find_if(m_roleMappings.begin(), m_roleMappings.end(), RoleMappingComparator{dataRole});
}

std::vector<KDTableToListProxyModel::RoleMapping>::const_iterator
KDTableToListProxyModel::findRoleMapping(int dataRole) const
{
    return std::find_if(m_roleMappings.begin(), m_roleMappings.end(), RoleMappingComparator{dataRole});
}

std::vector<KDTableToListProxyModel::RoleMapping>::const_iterator
KDTableToListProxyModel::constFindRoleMapping(int dataRole) const
{
    return findRoleMapping(dataRole);
}

void KDTableToListProxyModel::sourceModelDestroyed()
{
    setSourceModel(nullptr);
}

void KDTableToListProxyModel::dataChangedInSourceModel(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                                       const QVector<int> &roles)
{
    Q_ASSERT(topLeft.parent() == bottomRight.parent());
    if (topLeft.parent().isValid())
    {
        // Non-top level items changed, we don't display those
        return;
    }

    const int firstColumn = topLeft.column();
    const int lastColumn = bottomRight.column();

    // Are we mapping any of the columns affected by the change? And, for those
    // columns, are we mapping the passed roles? Build a vector of the mapped
    // roles affected. If empty, it means that the dataChanged does not affect
    // us.

    QVector<int> mappedRolesChanged;

    for (const auto &mapping : m_roleMappings)
    {
        if (firstColumn <= mapping.column && mapping.column <= lastColumn)
        {
            if (roles.isEmpty() || roles.contains(mapping.columnRole))
                mappedRolesChanged.append(mapping.dataRole);
        }
    }

    if (mappedRolesChanged.isEmpty())
    {
        return;
    }

    const int firstRow = topLeft.row();
    const int lastRow = bottomRight.row();
    Q_EMIT dataChanged(index(firstRow, 0), index(lastRow, 0), mappedRolesChanged);
}

void KDTableToListProxyModel::headerDataChangedInSourceModel(Qt::Orientation orientation, int first, int last)
{
    switch (orientation)
    {
    case Qt::Horizontal:
        break;
    case Qt::Vertical:
        Q_EMIT headerDataChanged(orientation, first, last);
        break;
    }
}

void KDTableToListProxyModel::layoutAboutToBeChangedInSourceModel(const QList<QPersistentModelIndex> &parents,
                                                                  QAbstractItemModel::LayoutChangeHint hint)
{
    // Does the source model have any top-level item at all?
    if (!m_sourceModel->hasChildren())
        return;

    // If it has, are they involved in the layout change?
    if (!parents.isEmpty() && !parents.contains(QModelIndex()))
        return;

    switch (hint)
    {
    case QAbstractItemModel::NoLayoutChangeHint:
        // Anything could've happened here. *Any* index could have been moved
        // to *any* other position under *any* other parent; and, rows/columns
        // could've been added/removed/moved.
        //
        // Unfortunately we don't have more granular hints, saying, for
        // instance, that entire rows have been added/removed/moved under the
        // same parent (this would e.g. happen by changing the filtering on a
        // QSortFilterProxyModel).
        //
        // This model depends on all the rows in the source, as well as the
        // subset of mapped columns; a change of ANY of them is a change
        // affecting this model. Trying to figure if we are changing data
        // or layout is super tricky so I won't do it now.
        //
        // As a simplification: do a model reset.
        beginResetModel();
        break;
    case QAbstractItemModel::VerticalSortHint:
        // In case of a layout change that ONLY changes the rows ordering:
        // get the list of persistent indexes in the current model,
        // generate and a list of corresponding (persistent) indexes in the
        // source model.
        //
        // Since there is no mapToSource function (as it would be meaningless),
        // just create persistent indexes to column 0. We will be only
        // interested in their movement.

        Q_EMIT layoutAboutToBeChanged({}, hint);

        m_ownPersistentIndexesForLayoutChange = persistentIndexList();

        m_sourcePersistentIndexesForLayoutChange.reserve(m_ownPersistentIndexesForLayoutChange.size());
        std::transform(m_ownPersistentIndexesForLayoutChange.cbegin(), m_ownPersistentIndexesForLayoutChange.cend(),
                       std::back_inserter(m_sourcePersistentIndexesForLayoutChange),
                       [this](const QModelIndex &proxyIndex) {
                           return QPersistentModelIndex(m_sourceModel->index(proxyIndex.row(), 0));
                       });

        break;

    case QAbstractItemModel::HorizontalSortHint:
        // A horizontal sorting does not involve any visible change for us;
        // we'll just need to update our internal mappings for the columns.
        // Keep a list of persistent model indexes in the source model, for the
        // columns in our mapping (using row 0 as a dummy row number).

        m_dummySourceIndexesForColumnLayoutChange.reserve(m_roleMappings.size());
        std::transform(m_roleMappings.cbegin(), m_roleMappings.cend(),
                       std::back_inserter(m_dummySourceIndexesForColumnLayoutChange),
                       [this](const RoleMapping &mapping) {
                           return QPersistentModelIndex(m_sourceModel->index(0, mapping.column));
                       });

        break;
    }
}

void KDTableToListProxyModel::layoutChangedInSourceModel(const QList<QPersistentModelIndex> &parents,
                                                         QAbstractItemModel::LayoutChangeHint hint)
{
    if (!m_sourceModel->hasChildren())
        return;

    if (!parents.isEmpty() && !parents.contains(QModelIndex()))
        return;

    switch (hint)
    {
    case QAbstractItemModel::NoLayoutChangeHint:
        endResetModel();
        break;
    case QAbstractItemModel::VerticalSortHint:
    {
        // For each saved index in this model, figure out where the corresponding
        // source index has moved to; use that to change our persistent model
        // indexes.
        QModelIndexList newOwnPersistentIndexes;
        newOwnPersistentIndexes.reserve(m_ownPersistentIndexesForLayoutChange.size());

        std::transform(m_sourcePersistentIndexesForLayoutChange.cbegin(),
                       m_sourcePersistentIndexesForLayoutChange.cend(), std::back_inserter(newOwnPersistentIndexes),
                       [this](const QPersistentModelIndex &sourceIndex) { return index(sourceIndex.row(), 0); });

        changePersistentIndexList(m_ownPersistentIndexesForLayoutChange, newOwnPersistentIndexes);

        Q_EMIT layoutChanged({}, hint);
        m_ownPersistentIndexesForLayoutChange.clear();
        m_sourcePersistentIndexesForLayoutChange.clear();
        break;
    }

    case QAbstractItemModel::HorizontalSortHint:
        // Using the dummy persistent indexes saved, update our role mappings.
        // Just iterate on both sequences (they have, by construction, same
        // number of elements) at the same time; the new column for the role
        // mapping at a given position is the column of the persistent source
        // index at the same position.
        Q_ASSERT(m_dummySourceIndexesForColumnLayoutChange.size() == m_roleMappings.size());

        auto roleMappingsIt = m_roleMappings.begin();
        const auto roleMappingsEnd = m_roleMappings.end();
        auto sourceIndexesIt = m_dummySourceIndexesForColumnLayoutChange.cbegin();

        for (; roleMappingsIt != roleMappingsEnd; ++roleMappingsIt, ++sourceIndexesIt)
        {
            // No index in the source model has been dropped
            Q_ASSERT(sourceIndexesIt->isValid());

            roleMappingsIt->column = sourceIndexesIt->column();
        }

        m_dummySourceIndexesForColumnLayoutChange.clear();
        break;
    }
}

void KDTableToListProxyModel::rowsAboutToBeInsertedInSourceModel(const QModelIndex &parent, int start, int end)
{
    if (parent.isValid())
    {
        // Added rows not below the root
        return;
    }

    beginInsertRows(QModelIndex(), start, end);
}

void KDTableToListProxyModel::rowsInsertedInSourceModel(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(start);
    Q_UNUSED(end);

    if (parent.isValid())
    {
        // Added rows not below the root
        return;
    }

    endInsertRows();
}

void KDTableToListProxyModel::rowsAboutToBeRemovedInSourceModel(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid())
    {
        // Removed rows not below the root
        return;
    }

    beginRemoveRows(QModelIndex(), first, last);
}

void KDTableToListProxyModel::rowsRemovedInSourceModel(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(first);
    Q_UNUSED(last);

    if (parent.isValid())
    {
        // Removed rows not below the root
        return;
    }

    endRemoveRows();
}

void KDTableToListProxyModel::columnsInsertedInSourceModel(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid())
    {
        // Added columns not below the root
        return;
    }

    const int columnOffset = last - first + 1;

    // Update the mappings to take the new columns into account.
    for (auto &mapping : m_roleMappings)
    {
        if (mapping.column >= first)
        {
            mapping.column += columnOffset;
        }
    }
}

void KDTableToListProxyModel::columnsRemovedInSourceModel(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid())
    {
        // Removed columns not below the root
        return;
    }

    const int columnOffset = last - first + 1;

    // Update the mappings to take the removed columns into account.
    QVector<int> mappedRolesChanged;

    auto i = m_roleMappings.begin();
    while (i != m_roleMappings.end())
    {
        if (i->column < first)
        {
            // Not affected.
            ++i;
        }
        else if (i->column <= last)
        {
            // Mapped column got removed. Remove the mapping.
            mappedRolesChanged.append(i->dataRole);
            i = m_roleMappings.erase(i);
        }
        else
        {
            // Mapped column was after the removed ones; adjust the mapping
            i->column -= columnOffset;
            ++i;
        }
    }

    if (!mappedRolesChanged.isEmpty())
    {
        // all rows havechanged
        Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), mappedRolesChanged);
    }
}

void KDTableToListProxyModel::modelAboutToBeResetInSourceModel()
{
    beginResetModel();
}

void KDTableToListProxyModel::modelResetInSourceModel()
{
    endResetModel();
}

void KDTableToListProxyModel::rowsAboutToBeMovedInSourceModel(const QModelIndex &sourceParent, int sourceStart,
                                                              int sourceEnd, const QModelIndex &destinationParent,
                                                              int destinationRow)
{
    if (sourceParent.isValid() && destinationParent.isValid())
    {
        // Moved rows not below the root
        return;
    }
    else if (sourceParent.isValid() && !destinationParent.isValid())
    {
        // Rows got moved from elsewhere to below the root: simulate an insertion
        beginInsertRows(QModelIndex(), destinationRow, destinationRow + sourceEnd - sourceStart);
    }
    else if (!sourceParent.isValid() && destinationParent.isValid())
    {
        // Rows from below the root got moved to elsewhere: simulate a removal
        beginRemoveRows(QModelIndex(), sourceStart, sourceEnd);
    }
    else
    {
        // Rows were moved below the root
        beginMoveRows(QModelIndex(), sourceStart, sourceEnd, QModelIndex(), destinationRow);
    }
}

void KDTableToListProxyModel::rowsMovedInSourceModel(const QModelIndex &sourceParent, int start, int end,
                                                     const QModelIndex &destinationParent, int row)
{
    Q_UNUSED(start);
    Q_UNUSED(end);
    Q_UNUSED(row);

    if (sourceParent.isValid() && destinationParent.isValid())
    {
        // Moved rows not below the root
        return;
    }
    else if (sourceParent.isValid() && !destinationParent.isValid())
    {
        endInsertRows();
    }
    else if (!sourceParent.isValid() && destinationParent.isValid())
    {
        endRemoveRows();
    }
    else
    {
        endMoveRows();
    }
}

void KDTableToListProxyModel::columnsMovedInSourceModel(const QModelIndex &sourceParent, int start, int end,
                                                        const QModelIndex &destinationParent, int column)
{
    if (sourceParent.isValid() && destinationParent.isValid())
    {
        // Moved columns not below the root
        return;
    }

    if (sourceParent.isValid() && !destinationParent.isValid())
    {
        // Columns were moved from elsewhere to below the root; simulate an insertion
        columnsInsertedInSourceModel(destinationParent, column, column + end - start);
        return;
    }

    if (!sourceParent.isValid() && destinationParent.isValid())
    {
        // Columns from below the root got moved to elsewhere: simulate a removal
        columnsRemovedInSourceModel(sourceParent, start, end);
        return;
    }

    // Columns from below the root got moved. Adjust the mappings.
    Q_ASSERT(end >= start);
    const int movedColumnCount = end - start + 1;

    if (start < column)
    {
        // Scenario 1: movement to the right, e.g.
        //
        //     | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
        //           \___________/             ^
        //                 \__________________/
        //          start       end             column
        //
        // In this case, for a column c:
        // * c \in [0, start) => not modified;
        // * c \in [start, end] => moved to the right by (column - end - 1) positions. (1)
        // * c \in (end, column) => moved to the left by (end - start + 1) positions.
        // * c \in [column, *) => not modified
        //
        //
        // (1) Another way to see it is: the range [start, end] gets moved just before
        // column; their new positions are going to be in the range
        // [column - length_of_moved_part, column - 1], so:
        //     c = column
        //         - (end - start + 1) // length of the moved part; i.e. position of "start" after the move
        //         + (c - start)       // relative index of c in the moved part
        //       = c + column - end - 1
        // Same result.

        Q_ASSERT(end < column);

        const int movementToTheRight = column - end - 1;

        for (auto &mapping : m_roleMappings)
        {
            auto &c = mapping.column;
            if (c < start)
            {
                /* nothing */
            }
            else if (c <= end)
            {
                c += movementToTheRight;
            }
            else if (c < column)
            {
                c -= movedColumnCount;
            }
            else
            {
                /* nothing */
            }
        }
    }
    else
    {
        // Scenario 2: movement to the left, e.g.
        //
        //     | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
        //             ^             \_______/
        //              \_______________/
        //            column       start    end
        //
        // In this case, for a column c:
        // * c \in [0, column) => not modified
        // * c \in [column, start) => moved to the right by (end - start + 1) positions
        // * c \in [start, end] => moved to the left by (start - column) positions
        // * c \in (end, *) => nothing
        Q_ASSERT(start > column);

        const int movementToTheLeft = start - column;

        for (auto &mapping : m_roleMappings)
        {
            auto &c = mapping.column;
            if (c < column)
            {
                /* nothing */
            }
            else if (c < start)
            {
                c += movedColumnCount;
            }
            else if (c < column)
            {
                c -= movementToTheLeft;
            }
            else
            {
                /* nothing */
            }
        }
    }
}
