/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#ifndef KDFUNCTIONALSORTFILTERPROXYMODEL_H
#define KDFUNCTIONALSORTFILTERPROXYMODEL_H

#include <QtCore/QSortFilterProxyModel>

#include <functional>

class KDFunctionalSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_DISABLE_COPY(KDFunctionalSortFilterProxyModel)

public:
    explicit KDFunctionalSortFilterProxyModel(QObject *parent = nullptr);
    ~KDFunctionalSortFilterProxyModel();

    using AcceptsFunction = std::function<bool(const QAbstractItemModel *, int, const QModelIndex &)>;
    using LessThanFunction = std::function<bool(const QModelIndex &, const QModelIndex &)>;

    void setFilterAcceptsRowFunction(AcceptsFunction function);
    void clearFilterAcceptsRowFunction();

    void setFilterAcceptsColumnFunction(AcceptsFunction function);
    void clearFilterAcceptsColumnFunction();

    void setLessThanFunction(LessThanFunction function);
    void clearLessThanFunction();

public Q_SLOTS:
    // invalidate is already public; let's make invalidateFilter public too
    // (and a slot, while at it)
    void invalidateFilter();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const final;

private:
    AcceptsFunction m_acceptsRowFunction;
    AcceptsFunction m_acceptsColumnFunction;
    LessThanFunction m_lessThanFunction;
};

#endif // KDFUNCTIONALSORTFILTERPROXYMODEL_H
