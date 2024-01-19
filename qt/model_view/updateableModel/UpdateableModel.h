/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: André Somers <andre.somers@kdab.com>

  SPDX-License-Identifier: MIT
*/

#ifndef UPDATEABLEMODEL_H
#define UPDATEABLEMODEL_H

#include <QModelIndex>
#include <QSet>
#include <QVector>
#include <functional>

// concepts
#define ForwardIt typename
#define RandomIt typename
#define Container typename
#define BinaryPredicate typename
#define EventHandler typename
#define QAIM typename // QAbstractItemModel-derived class

// basic algorithm

// precondition: src and target are both ordered with respect to lessThan
template<ForwardIt FwdIt, Container TargetCollection, BinaryPredicate LessThan, BinaryPredicate HasChanged,
         EventHandler OnChanged, EventHandler OnInsert, EventHandler OnRemove, EventHandler OnEqual>
void updateCollection(const FwdIt srcBegin, const FwdIt srcEnd, TargetCollection &target, LessThan lessThan,
                      HasChanged itemHasChanged, OnChanged onChanged, OnInsert onInsert, OnRemove onRemove,
                      OnEqual onEqual)
{
    auto srcIt = srcBegin;
    target.reserve(std::distance(srcBegin, srcEnd));
    auto targetIt = std::begin(target);

    while (srcIt != srcEnd)
    {
        if (targetIt == std::end(target))
        {
            // insert: src has still items left while target has no more items
            //   so, insert all remaining items from src into target
            targetIt = onInsert(srcIt, srcEnd, targetIt);
            // targetIt now points to the item after the item(s) just inserted
            srcIt = srcEnd;
        }
        else if (lessThan(*srcIt, *targetIt))
        {
            // insert: src has one or more items that need to be inserted into target
            auto srcInsertEnd = std::next(srcIt);
            while (srcInsertEnd != srcEnd && lessThan(*srcInsertEnd, *targetIt))
            {
                srcInsertEnd++;
            }
            targetIt = onInsert(srcIt, srcInsertEnd, targetIt);
            // targetIt now points to the item after the item(s) just inserted
            srcIt = srcInsertEnd;
        }
        else if (lessThan(*targetIt, *srcIt))
        {
            // removal: target has items that are not in src (any more), so remove them
            auto targetRemoveEnd = std::next(targetIt);
            auto targetEnd = std::end(target);
            while (targetRemoveEnd != targetEnd && lessThan(*targetRemoveEnd, *srcIt))
            {
                targetRemoveEnd++;
            }
            targetIt = onRemove(targetIt, targetRemoveEnd);
            // targetIt now points to the item after the items just removed,
            //   the same item targetEnd was pointing to before the call
        }
        else
        {
            // same item, check for changes
            auto changes = itemHasChanged(*srcIt, *targetIt);
            if (changes)
            {
                onChanged(srcIt, targetIt, changes);
            }
            else
            {
                onEqual(srcIt, targetIt);
            }
            srcIt++;
            targetIt++;
        }
    }

    if (targetIt != std::end(target))
    {
        // target has some items left, but we're at the end of src, so remove the rest from target
        onRemove(targetIt, std::end(target));
    }
}

// helper functions
namespace
{
// helper function to create a QSet<T> out of any container<T>.
template<typename T>
auto setFromContainer(T container) -> QSet<typename T::value_type>
{
    QSet<typename T::value_type> set;
    set.reserve(container.size());
    for (const auto value : container)
    {
        set.insert(value);
    }
    return set;
}

// helper for manual overload ranking
//   highest rank is selected first
template<size_t Rank>
struct OverloadRanker;
template<>
struct OverloadRanker<0>
{
};
template<size_t Rank>
struct OverloadRanker : OverloadRanker<Rank - 1>
{
};

// insert range into container, 2 overloads (using SFINAE) with manual ranking in case of multiple matches
constexpr size_t BestInsertRangeOverloadRank = 1;

template<Container C, RandomIt InsertIt, ForwardIt FwdIt>
auto insertRange_imp(C &container, InsertIt targetInsertBefore, FwdIt sourceBegin, FwdIt sourceEnd,
                     OverloadRanker<1>) // Best match
    -> decltype(container.insert(targetInsertBefore, sourceBegin, sourceEnd), InsertIt())
{
    // primary option: use insert with a whole range directly
    InsertIt it =
        std::next(container.insert(targetInsertBefore, sourceBegin, sourceEnd), std::distance(sourceBegin, sourceEnd));
    return it;
}

template<Container C, RandomIt InsertIt, ForwardIt FwdIt>
auto insertRange_imp(C &container, InsertIt targetInsertBefore, FwdIt sourceBegin, FwdIt sourceEnd,
                     OverloadRanker<0>) // Fallback
    -> InsertIt
{
#if (QT_VERSION < QT_VERSION_CHECK(5, 13, 0))
    // Check if, when using QVector in Qt , the value type is default constructable
    static_assert(!std::is_same<QVector<typename C::value_type>, C>::value ||
                      std::is_default_constructible<typename C::value_type>::value,
                  "When using QVector as your container type, the value type must be default-contructible. Consider "
                  "using std::vector instead or use a more recent Qt version.");
#endif

    // store the location and the length of the inserted items
    const auto location = std::distance(std::begin(container), targetInsertBefore);
    const auto originalCount = container.size();

    // append the new items to the container
    std::copy(sourceBegin, sourceEnd, std::back_inserter(container));

    // move the items into the right position in the container
    const auto b = std::begin(container);
    targetInsertBefore = std::next(b, location);
    const InsertIt originalEnd = std::next(b, originalCount);
    return std::rotate(targetInsertBefore, originalEnd, std::end(container));
}

/**
 *  Inserts a range into the target container
 *  @returns an iterator pointing one item past the last item inserted
 *  @param container is the target container
 *  @param targetInsertBefore iterator to the item the new items should be inserted before
 *  @param sourceBegin iterator to the first item to insert into @arg container
 *  @param sourceEnd iterator to the item past the last item to insert into @arg container
 *
 *  Implementation is done using SFINEA to select an efficient method for the type of
 *  container used.
 */

// "public API"
template<Container C, RandomIt InsertIt, ForwardIt FwdIt>
InsertIt insertRange(C &container, InsertIt targetInsertBefore, FwdIt sourceBegin, FwdIt sourceEnd)
{
    return insertRange_imp(container, targetInsertBefore, sourceBegin, sourceEnd,
                           OverloadRanker<BestInsertRangeOverloadRank>{});
}

// less than on data type
constexpr size_t BestDataLessThanOverloadRank = 1;

template<typename T>
auto dataLessThan_imp(const T &lhs, const T &rhs, OverloadRanker<1>) -> decltype(operator<(lhs, rhs), bool())
{
    return operator<(lhs, rhs);
}

template<typename T>
auto dataLessThan_imp(const T &lhs, const T &rhs, OverloadRanker<0>) -> decltype(bool())
{
    // fake implementation to stop the compiler from whining. User is probably providing his own via function pointer
    Q_UNUSED(lhs);
    Q_UNUSED(rhs);
    return false;
}

template<typename T>
bool dataLessThan(const T &lhs, const T &rhs)
{
    return dataLessThan_imp(lhs, rhs, OverloadRanker<BestDataLessThanOverloadRank>{});
}
}

// actual class to inherit from
template<QAIM BaseModel, typename DataType>
class UpdateableModel : public BaseModel
{
    // can't use Q_OBJECT on templates

public:
    explicit UpdateableModel(QObject *parent = nullptr)
        : BaseModel(parent)
    {
    }

protected: // types
    struct Operations
    {
        uint inserts;
        uint removals;
        uint updates;
    };

    struct DataChanges
    {
        operator bool() const { return !changedColumns.isEmpty(); }
        QVector<int> changedColumns; // important: keep ordered!
        QVector<int> changedRoles;
    };

    enum ChangeMergePolicy
    {
        AlwaysMergeNeighbouringRows,
        MergeWhenColumnsMatch,
        MergeWhenRolesMatch,
        MergeOnPerfectMatch
    };

    using HasChangesFunction = std::function<DataChanges(const DataType & /*lhs*/, const DataType & /*rhs*/)>;

protected: // methods
    void setChangeMergePolicy(ChangeMergePolicy policy) { m_changeMergePolicy = policy; }

    ChangeMergePolicy changeMergePolicy() const { return m_changeMergePolicy; }

    virtual bool lessThan(const DataType &lhs, const DataType &rhs) const { return dataLessThan(lhs, rhs); }

    virtual DataChanges itemHasChanged(const DataType &lhs, const DataType &rhs) const
    {
        Q_UNUSED(lhs);
        Q_UNUSED(rhs);

        return {};
    }

    template<ForwardIt Iterator, Container DataContainer, BinaryPredicate LessThan>
    Operations updateData(Iterator srcBegin, Iterator srcEnd, DataContainer &targetContainer, LessThan lessThan,
                          HasChangesFunction itemHasChanged)
    {
        Operations ops{0, 0, 0};

        // events
        auto onChanged = [this, &targetContainer, &ops](Iterator lhs, typename DataContainer::iterator rhs,
                                                        const DataChanges &changes) {
            *rhs = *lhs;

            int row = std::distance(targetContainer.begin(), rhs);
            addChange(row, changes.changedColumns, changes.changedRoles);

            ops.updates++;
        };

        auto onInsert = [this, &targetContainer, &ops](Iterator lhsBegin, Iterator lhsEnd,
                                                       typename DataContainer::iterator rhsInsertAt) {
            flushCachedChanges();
            int firstNewRow = std::distance(targetContainer.begin(), rhsInsertAt);
            int lastNewRow = firstNewRow + std::distance(lhsBegin, lhsEnd) - 1;
            beginInsertRows(firstNewRow, lastNewRow);
            auto rangeEnd = insertRange(targetContainer, rhsInsertAt, lhsBegin, lhsEnd);
            endInsertRows();

            ops.inserts += lhsEnd - lhsBegin;

            return rangeEnd;
        };

        auto onRemove = [this, &targetContainer, &ops](typename DataContainer::iterator rhsBegin,
                                                       typename DataContainer::iterator rhsEnd) {
            flushCachedChanges();
            const auto targetContainerBegin = std::begin(targetContainer);
            int firstRemovedRow = std::distance(targetContainerBegin, rhsBegin);
            int lastRemovedRow = std::distance(targetContainerBegin, rhsEnd) - 1;
            beginRemoveRows(firstRemovedRow, lastRemovedRow);
            auto newRhsEnd = targetContainer.erase(rhsBegin, rhsEnd);
            endRemoveRows();

            ops.removals += std::distance(rhsBegin, rhsEnd);

            return newRhsEnd;
        };

        auto onEqual = [this](Iterator /*lhs*/, typename DataContainer::iterator /*rhs*/) { flushCachedChanges(); };

        updateCollection(srcBegin, srcEnd, targetContainer, lessThan, itemHasChanged, onChanged, onInsert, onRemove,
                         onEqual);
        flushCachedChanges();

        return ops;
    }

    template<ForwardIt Iterator, Container Data>
    Operations updateData(Iterator srcBegin, Iterator srcEnd, Data &targetContainer, HasChangesFunction hasChanged = {})
    {
        auto lessThanFunction = [this](const DataType &lhs, const DataType &rhs) { return this->lessThan(lhs, rhs); };
        if (!hasChanged)
        {
            hasChanged = [this](const DataType &lhs, const DataType &rhs) -> DataChanges {
                return this->itemHasChanged(lhs, rhs);
            };
        }

        return updateData(srcBegin, srcEnd, targetContainer, lessThanFunction, hasChanged);
    }

protected:
    // Shadowing the methods in the base model, because MSVC thinks the onRemove and onInsert
    // lambdas in updateData are not allowed to access the protected member functions in BaseClass.
    // GCC is ok with it...
    void beginInsertRows(int firstRow, int lastRow) { BaseModel::beginInsertRows(QModelIndex(), firstRow, lastRow); }

    void endInsertRows() { BaseModel::endInsertRows(); }

    void beginRemoveRows(int firstRow, int lastRow) { BaseModel::beginRemoveRows(QModelIndex(), firstRow, lastRow); }

    void endRemoveRows() { BaseModel::endRemoveRows(); }

private:
    void flushCachedChanges()
    {
        if (m_firstChangedRow == -1)
            return;

        // reconstruct vector of changed roles
        QVector<int> roles;
        roles.reserve(m_changedRoles.count());
        for (int role : qAsConst(m_changedRoles))
        {
            roles.append(role);
        }

        // convert the vector of ints indicating changed columns into a vector of pairs of ints indicating ranges.
        // for example, the vector {1, 2, 3, 5, 6, 9} would be converted to {{1, 3}, {5, 6}, {9, 9}}
        typedef QVector<QPair<int, int>> ColumnRanges;
        auto accumulator = [](ColumnRanges ranges, int column) {
            if (ranges.isEmpty() || (ranges.last().second < column - 1))
            {
                ranges.append(qMakePair(column, column));
            }
            else
            {
                ranges[ranges.count() - 1].second = column;
            }
            return ranges;
        };
        const auto ranges =
            std::accumulate(m_changedColumns.cbegin(), m_changedColumns.cend(), ColumnRanges(), accumulator);

        for (const auto &columnRange : ranges)
        {
            QModelIndex topLeft = BaseModel::index(m_firstChangedRow, columnRange.first);
            QModelIndex bottomRight = BaseModel::index(m_lastChangedRow, columnRange.second);
            Q_EMIT BaseModel::dataChanged(topLeft, bottomRight, roles);
        }

        m_firstChangedRow = -1;
    }

    void addChange(int row, const QVector<int> &columns, const QVector<int> &roles)
    {
        auto mergeColumns = [this](const QVector<int> &columns) {
            QVector<int> columnUnion;
            // we have to cast this to QAbstractItemModel, to work around QAbstractListModel making columnCount private
            columnUnion.reserve(static_cast<QAbstractItemModel *>(this)->columnCount());
            auto inserter = std::back_inserter(columnUnion);
            std::set_union(m_changedColumns.cbegin(), m_changedColumns.cend(), columns.cbegin(), columns.cend(),
                           inserter);
            m_changedColumns = std::move(columnUnion);
        };

        if (m_firstChangedRow > -1)
        {
            // from here rows are guaranteed to be sequential, so no need to check for that
            m_lastChangedRow = row; // optimistic, but lets see...

            switch (m_changeMergePolicy)
            {
            case AlwaysMergeNeighbouringRows:
                m_changedRoles = m_changedRoles.intersect(setFromContainer(roles));
                mergeColumns(columns);
                return;
            case MergeWhenColumnsMatch:
                if (m_changedColumns == columns)
                {
                    m_changedRoles = m_changedRoles.intersect(setFromContainer(roles));
                    return;
                }
                break;
            case MergeWhenRolesMatch:
                if (m_changedRoles == setFromContainer(roles))
                {
                    mergeColumns(columns);
                    return;
                }
                break;
            case MergeOnPerfectMatch:
                if (m_changedColumns == columns && m_changedRoles == setFromContainer(roles))
                {
                    return;
                }
                break;
            }

            m_lastChangedRow = row - 1; // we were too optimistic
            flushCachedChanges();
        }

        // just copy the details of the first change
        m_firstChangedRow = row;
        m_lastChangedRow = row;
        m_changedColumns = columns;
        m_changedRoles = setFromContainer(roles);
    }

    ChangeMergePolicy m_changeMergePolicy = MergeOnPerfectMatch;
    int m_firstChangedRow = -1;
    int m_lastChangedRow = -1;
    QVector<int> m_changedColumns;
    QSet<int> m_changedRoles;
};

#undef ForwardIt
#undef RandomIt
#undef Container
#undef BinaryPredicate
#undef EventHandler
#undef QAIM

#endif // UPDATEABLEMODEL_H
