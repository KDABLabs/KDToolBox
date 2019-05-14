#include "ModelIterator.h"

DepthFirstIterator::DepthFirstIterator(const QModelIndex &index, int column):
    m_column(column)
{
    if (column == 0) {
        m_index = index;
    } else {
        m_index = index.sibling(index.row(), 0);
    }
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
    if (m_atEnd) {
        m_atEnd = false;
    } else {
        //we cannot iterate backwards from the first node in the tree
        Q_ASSERT(m_index.parent().isValid() || m_index.row() > 0);

        auto index = m_index;
        auto sibling = prevSibling(m_index);
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
    if (model()->hasChildren(index)) {
        auto lastChild = model()->index(model()->rowCount(index) -1, 0, index);
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
    Q_ASSERT(model);
    Q_ASSERT(model->columnCount() > column);

    DepthFirstIterator it = begin(model, column);
    it.m_index = it.lastDecendant(QModelIndex());
    it.m_atEnd = true;
    return it;
}

////////////// FlatIterator /////////////////

FlatIterator::FlatIterator(const QModelIndex &index, int column)
{
    Q_ASSERT(index.isValid());
    m_index = index.model()->index(index.row(), column, index.parent());
    m_atEnd = false;
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
