/****************************************************************************
**                                MIT License
**
** Copyright (C) 2019-2023 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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
