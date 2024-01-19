/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "KDFunctionalSortFilterProxyModel.h"

KDFunctionalSortFilterProxyModel::KDFunctionalSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

KDFunctionalSortFilterProxyModel::~KDFunctionalSortFilterProxyModel() = default;

void KDFunctionalSortFilterProxyModel::setFilterAcceptsRowFunction(AcceptsFunction function)
{
    m_acceptsRowFunction = std::move(function);
    invalidateFilter();
}

void KDFunctionalSortFilterProxyModel::clearFilterAcceptsRowFunction()
{
    setFilterAcceptsRowFunction({});
}

void KDFunctionalSortFilterProxyModel::setFilterAcceptsColumnFunction(AcceptsFunction function)
{
    m_acceptsColumnFunction = std::move(function);
    invalidateFilter();
}

void KDFunctionalSortFilterProxyModel::clearFilterAcceptsColumnFunction()
{
    setFilterAcceptsColumnFunction({});
}

void KDFunctionalSortFilterProxyModel::setLessThanFunction(LessThanFunction function)
{
    m_lessThanFunction = std::move(function);
    invalidate();
}

void KDFunctionalSortFilterProxyModel::clearLessThanFunction()
{
    setLessThanFunction({});
}

void KDFunctionalSortFilterProxyModel::invalidateFilter()
{
    QSortFilterProxyModel::invalidateFilter();
}

bool KDFunctionalSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (m_acceptsRowFunction && !m_acceptsRowFunction(sourceModel(), source_row, source_parent))
        return false;

    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

bool KDFunctionalSortFilterProxyModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
    if (m_acceptsColumnFunction && !m_acceptsColumnFunction(sourceModel(), source_column, source_parent))
        return false;

    return QSortFilterProxyModel::filterAcceptsColumn(source_column, source_parent);
}

bool KDFunctionalSortFilterProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (m_lessThanFunction)
        return m_lessThanFunction(source_left, source_right);

    return QSortFilterProxyModel::lessThan(source_left, source_right);
}
