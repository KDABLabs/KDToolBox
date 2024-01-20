/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#ifndef KDTABLETOLISTPROXYMODEL_H
#define KDTABLETOLISTPROXYMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QByteArray>

#include <vector>

class KDTableToListProxyModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit KDTableToListProxyModel(QObject *parent = nullptr);

    QAbstractItemModel *sourceModel() const;
    void setSourceModel(QAbstractItemModel *model);

    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;

    // We will map \a column (of the original model) to \a dataRole
    // in this model (in column #0).
    // Use \a roleName to build roleNames, as extra convenience.
    // Optionally, also specify which role of the original model
    // should we be mapping to.
    void setRoleMapping(int column, int dataRole, const QByteArray &roleName = QByteArray(),
                        int columnRole = Qt::DisplayRole);
    void unsetRoleMapping(int dataRole);

    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role) override;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent,
                  int destinationChild) override;

    void fetchMore(const QModelIndex &parent = QModelIndex()) override;
    bool canFetchMore(const QModelIndex &parent = QModelIndex()) const override;

    QHash<int, QByteArray> roleNames() const override;

private:
    QAbstractItemModel *m_sourceModel;
    template<typename Func1, typename Func2>
    void connectToSourceModel(Func1 signal, Func2 slot);
    std::vector<QMetaObject::Connection> m_sourceModelConnections;

    struct RoleMapping
    {
        int dataRole;        // the role in *this* model (accepted by our data())
        int column;          // dataRole maps to this column in the *source* model
        QByteArray roleName; // name for the data role in *this* model
        int columnRole;      // dataRole also maps to this role in the *source* model
    };
    std::vector<RoleMapping> m_roleMappings;

    struct RoleMappingComparator
    {
        int m_dataRole;
        bool operator()(const RoleMapping &mapping) const noexcept { return m_dataRole == mapping.dataRole; }
    };
    std::vector<RoleMapping>::iterator findRoleMapping(int dataRole);
    std::vector<RoleMapping>::const_iterator findRoleMapping(int dataRole) const;
    std::vector<RoleMapping>::const_iterator constFindRoleMapping(int dataRole) const;

    // Slots for signals coming from the source model. We're interested in almost
    // all of QAbstractItemModel signals.
    void sourceModelDestroyed();

    void dataChangedInSourceModel(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                  const QVector<int> &roles);
    void headerDataChangedInSourceModel(Qt::Orientation orientation, int first, int last);

    void layoutAboutToBeChangedInSourceModel(const QList<QPersistentModelIndex> &parents,
                                             QAbstractItemModel::LayoutChangeHint hint);
    void layoutChangedInSourceModel(const QList<QPersistentModelIndex> &parents,
                                    QAbstractItemModel::LayoutChangeHint hint);

    void rowsAboutToBeInsertedInSourceModel(const QModelIndex &parent, int start, int end);
    void rowsInsertedInSourceModel(const QModelIndex &parent, int start, int end);

    void rowsAboutToBeRemovedInSourceModel(const QModelIndex &parent, int first, int last);
    void rowsRemovedInSourceModel(const QModelIndex &parent, int first, int last);

    void columnsInsertedInSourceModel(const QModelIndex &parent, int first, int last);

    void columnsRemovedInSourceModel(const QModelIndex &parent, int first, int last);

    void modelAboutToBeResetInSourceModel();
    void modelResetInSourceModel();

    void rowsAboutToBeMovedInSourceModel(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
                                         const QModelIndex &destinationParent, int destinationRow);
    void rowsMovedInSourceModel(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row);

    void columnsMovedInSourceModel(const QModelIndex &parent, int start, int end, const QModelIndex &destination,
                                   int column);

    // For a layout change that involves rows changing
    QModelIndexList m_ownPersistentIndexesForLayoutChange;
    std::vector<QPersistentModelIndex> m_sourcePersistentIndexesForLayoutChange;

    // For a layout change that involves *only* columns changing
    std::vector<QPersistentModelIndex> m_dummySourceIndexesForColumnLayoutChange;
};

#endif // KDTABLETOLISTPROXYMODEL_H
