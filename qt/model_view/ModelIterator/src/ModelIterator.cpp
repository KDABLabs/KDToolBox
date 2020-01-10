/****************************************************************************
**                                MIT License
**
** Copyright (C) 2018-2020 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
** Author: André Somers <andre.somers@kdab.com>
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

#include "ModelIterator.h"

DepthFirstIterator::DepthFirstIterator(const QModelIndex &index, int column):
    m_column(column)
{
    m_index = index.siblingAtColumn(0);
    m_atEnd = !index.isValid();
}

DepthFirstIterator &DepthFirstIterator::operator++()
{
    //we cannot iterate forwards from the end iterator
    Q_ASSERT(!m_atEnd);

    if (model()->hasChildren(m_index)) {
        //first navigate to first child node
        m_index = model()->index(0, 0, m_index);
    } else {
        auto index = m_index;
        auto sibling = nextSibling(m_index);
        while (!sibling.isValid()) {
            if (index.parent().isValid()) {
                index = index.parent();
                sibling = nextSibling(index);
            } else {
                break;
            }
        }

        if (sibling.isValid()) {
            m_index = sibling;
        } else {
            m_atEnd = true;
        }
    }

    return *this;
}

DepthFirstIterator &DepthFirstIterator::operator--()
{
    Q_ASSERT(m_index.isValid()); // we cannot decrement from an invalid iterator

    if (m_atEnd) {
        m_atEnd = false;
    } else {
        //we cannot iterate backwards from the first node in the tree
        Q_ASSERT(m_index.parent().isValid() || m_index.row() > 0);

        const auto sibling = prevSibling(m_index);
        if (!sibling.isValid()) {
            m_index = m_index.parent();
        } else {
            m_index = lastDecendant(sibling);
        }
    }

    return *this;
}

const QModelIndex DepthFirstIterator::nextSibling(const QModelIndex &index) const
{
    if (index.row() < model()->rowCount(index.parent()) - 1) {
        return model()->index(index.row() + 1, 0, index.parent());
    }
    return {};
}

const QModelIndex DepthFirstIterator::prevSibling(const QModelIndex &index) const
{
    if (index.row() > 0) {
        return model()->index(index.row() -1, 0, index.parent());
    }
    return {};
}

const QModelIndex DepthFirstIterator::lastDecendant(const QModelIndex &index) const
{
    const auto m = model();
    if (m && m->hasChildren(index)) {
        auto lastChild = model()->index(m->rowCount(index) -1, 0, index);
        return lastDecendant(lastChild);
    } else {
        return index;
    }
}

DepthFirstIterator DepthFirstIterator::begin(QAbstractItemModel *model, int column)
{
    Q_ASSERT(model);
    Q_ASSERT(model->columnCount() > column);

    return DepthFirstIterator(model->index(0,0), column);
}

DepthFirstIterator DepthFirstIterator::end(QAbstractItemModel *model, int column)
{
    DepthFirstIterator it = begin(model, column);
    it.m_index = it.lastDecendant(QModelIndex());
    it.m_atEnd = true;

    return it;
}

////////////// FlatIterator /////////////////

FlatIterator::FlatIterator(const QModelIndex &index, int column)
{
    m_index = index.siblingAtColumn(column);
    m_atEnd = !index.isValid();
}

FlatIterator &FlatIterator::operator+=(int step)
{
    if (step < 0)
        return operator -=(-step);

    Q_ASSERT(!m_atEnd); //we cannot increase the end iterator

    const auto newRow = m_index.row() + step;
    const int rowCount = model()->rowCount(m_index.parent());
    Q_ASSERT(newRow <= rowCount); //we cannot iteratate past end; end is at rowCount

    if (newRow < rowCount) {
        m_index = m_index.sibling(newRow, m_index.column());
    } else {
        m_index = m_index.sibling(newRow -1, m_index.column());
        m_atEnd = true;
    }

    return *this;
}

FlatIterator &FlatIterator::operator-=(int step)
{
    if (step < 0)
        return operator +=(-step);

    if (m_atEnd) --step;

    Q_ASSERT(m_index.isValid());

    const auto newRow = m_index.row() - step;
    Q_ASSERT(newRow >= 0);
    m_index = m_index.sibling(newRow, m_index.column());
    m_atEnd = false;
    return *this;
}

FlatIterator FlatIterator::begin(QAbstractItemModel *model, int column)
{
    Q_ASSERT(model);
    Q_ASSERT(model->columnCount() > column);

    return FlatIterator(model->index(0, column));
}

FlatIterator FlatIterator::end(QAbstractItemModel *model, int column)
{
    Q_ASSERT(model);
    Q_ASSERT(model->columnCount() > column);

    FlatIterator it(model->index(model->rowCount() - 1, column));
    it.m_atEnd = true;
    return it;
}
