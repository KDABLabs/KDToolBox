/****************************************************************************
**                                MIT License
**
** Copyright (C) 2019-2021 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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
