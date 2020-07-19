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

#pragma once
#include <QAbstractItemModel>
#include <iterator>

// Collection of tools to allow iterating over QAbstractItemModel using standard iterator interfaces
// Author: André Somers <andre.somers@kdab.com>

// Depth-first iterator
// This class iterates depth-first over a QAbstractItemModel. Given the tree:
// R
// |- N1
// |  |- N1.1
// |  |- N1.2
// |
// |- N2
// |  |- N2.1
// |
// |- N3
//
// the visiting order for the tree above will be N1, N1.1, N1.2, N2, N2.1, N3. That is: children are
// visited after parents, but before the next sibling of the parent. Only a single column of the tree
// is visited in the iteration, but the parent-child structure is assumed to be represented in column
// 0 of the model only (as is standard practice in Qt).
//
// You can construct an instance of the iterator using the static @ref begin and @ref end methods or
// by using @ref ModelAdapter.
// The Iterator is a bi-directional iterator. This has an impact over the efficiency of some algorithms
// over using a random-access iterator.
class DepthFirstIterator
{
    QModelIndex m_index;
    int m_column = 0;
    bool m_atEnd = false;

public: //types
    struct pseudo_ptr {
        QModelIndex t;
        QModelIndex operator*()&&{return t;}
        QModelIndex* operator->(){ return &t; }
    };

    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = QModelIndex;
    using difference_type = int;
    using pointer = value_type*;
    using reference = value_type&;

public: //methods
    explicit DepthFirstIterator(const QModelIndex& index, int column = 0);

    DepthFirstIterator& operator++();
    inline DepthFirstIterator operator++(int) {DepthFirstIterator tmp(*this); operator++(); return tmp;}
    DepthFirstIterator& operator--();
    inline DepthFirstIterator operator--(int) {DepthFirstIterator tmp(*this); operator--(); return tmp;}

    inline bool operator==(const DepthFirstIterator& other) {return m_index == other.m_index && m_column == other.m_column && m_atEnd == other.m_atEnd;}
    inline bool operator!=(const DepthFirstIterator& other) {return !operator ==(other);}

    inline QModelIndex operator*() {return m_atEnd ? QModelIndex() :
                                                      m_index.siblingAtColumn(m_column);}
    inline pseudo_ptr operator->() {Q_ASSERT(!m_atEnd); return pseudo_ptr{m_index.siblingAtColumn(m_column)};}

    static DepthFirstIterator begin(QAbstractItemModel *model, int column = 0);
    static DepthFirstIterator end(QAbstractItemModel *model, int column = 0);

private:
    inline const QAbstractItemModel *model() const {return m_index.model();}
    const QModelIndex nextSibling(const QModelIndex& index) const;
    const QModelIndex prevSibling(const QModelIndex& index) const;
    const QModelIndex lastDecendant(const QModelIndex& index) const;
};



// Flat iterator
// This class iterates only the root level of a QAbstractItemModel. Given the tree:
// R
// |- N1
// |  |- N1.1
// |  |- N1.2
// |
// |- N2
// |  |- N2.1
// |
// |- N3
//
// the visiting order for the tree above will be N1, N2, N3. That is: children are not visited. Only
// a single column of the tree is visited in the iteration.
//
// You can construct an instance of the iterator using the static @ref begin and @ref end methods or
// by using @ref ModelAdapter.
// FlatIterator is a random-access iterator, which allows algortihms to operate in the most effecient
// way.
class FlatIterator
{
    QModelIndex m_index;
    bool m_atEnd = true;

public: //types
    using iterator_category = std::random_access_iterator_tag;
    using value_type = QModelIndex;
    using difference_type = int;
    using pointer = value_type*;
    using reference = value_type&;

public:
    explicit FlatIterator(const QModelIndex& index, int column = 0);

    FlatIterator& operator+=(difference_type step);
    FlatIterator& operator-=(difference_type step);
    inline FlatIterator operator+(difference_type step) {FlatIterator tmp(*this); tmp += step; return tmp;}
    inline FlatIterator operator-(difference_type step) {FlatIterator tmp(*this); tmp -= step; return tmp;}

    inline FlatIterator& operator++() {return operator+=(1);}
    inline FlatIterator operator++(int) {FlatIterator tmp(*this); operator++(); return tmp;}
    inline FlatIterator& operator--() {return operator-=(1);}
    inline FlatIterator operator--(int) {FlatIterator tmp(*this); operator--(); return tmp;}
    inline difference_type operator-(const FlatIterator& other) const {Q_ASSERT(canCompare(*this, other)); return row() - other.row();}

    inline bool operator==(const FlatIterator& other) {return m_index == other.m_index && m_atEnd == other.m_atEnd;}
    inline bool operator!=(const FlatIterator& other) {return !operator==(other);}
    inline friend bool operator<(const FlatIterator& lhs, const FlatIterator& rhs) {Q_ASSERT(canCompare(lhs, rhs)); return lhs.row() < rhs.row();}
    inline friend bool operator>(const FlatIterator& lhs, const FlatIterator& rhs) {return operator<(rhs, lhs);}
    inline friend bool operator<=(const FlatIterator& lhs, const FlatIterator& rhs) {return !operator>(lhs, rhs);}
    inline friend bool operator>=(const FlatIterator& lhs, const FlatIterator& rhs) {return !operator<(lhs, rhs);}

    inline QModelIndex operator*() {return !m_atEnd ? m_index : QModelIndex();}
    inline QModelIndex *operator->() {Q_ASSERT(!m_atEnd); return &m_index;}

    static FlatIterator begin(QAbstractItemModel *model, int column = 0);
    static FlatIterator end(QAbstractItemModel *model, int column = 0);

private:
    inline const QAbstractItemModel *model() const {return m_index.model();}
    inline int row() const {int row = m_index.row(); if (m_atEnd) ++row; return row;}
    inline friend bool canCompare(const FlatIterator& it1, const FlatIterator& it2) {
        return it1.m_index.model() == it2.m_index.model() &&
               it1.m_index.column() == it2.m_index.column() &&
               it1.m_index.parent() == it2.m_index.parent();
    }
};

//DataValueWrapper
//DepthFirstIterator and FlatIterator iterate exposing QModelIndex as their value_type. That means
//you will then need to dereference this index in order to access the actual data. As a convenience,
//you can use DataValueWrapper (or have @ref ModelAdaptor use it) to de-reference the model indices
//directly, and in that way allow read/write operations on the data in the model directly.
//
//This iterator wraps another iterator.
template <typename ModelIterator, typename T, int role = Qt::DisplayRole>
class DataValueWrapper
{
    ModelIterator it;

public: //types
    struct pseudo_val {
        QModelIndex index;
        T t;

        pseudo_val(const QModelIndex& index): index(index), t(index.data(role).value<T>()) {}
        inline T* operator->(){return &t;}
        inline operator T() {return t;}
        inline void operator=(const T &newValue) {const_cast<QAbstractItemModel*>(index.model())->setData(index, QVariant::fromValue(newValue), role);}
        template <typename ComparedType> inline bool operator==(const ComparedType& value) const {return t == value;}
    };

    using value_type = T;
    using iterator_category = typename ModelIterator::iterator_category;
    using reference = pseudo_val;
    using pointer = pseudo_val;
    using difference_type = typename ModelIterator::difference_type;

public: //methods
    explicit DataValueWrapper(ModelIterator it): it(it){}

    DataValueWrapper& operator+=(int step) {it+= step; return *this;}
    DataValueWrapper& operator-=(int step) {it-= step; return *this;}
    inline DataValueWrapper operator+(int step) {return DataValueWrapper(it + step);}
    inline DataValueWrapper operator-(int step) {return DataValueWrapper(it - step);}

    inline DataValueWrapper& operator++() {it++; return *this;}
    inline DataValueWrapper operator++(int) {DataValueWrapper tmp(*this); operator++(); return tmp;}
    inline DataValueWrapper& operator--() {it--; return *this;}
    inline DataValueWrapper operator--(int) {DataValueWrapper tmp(*this); operator--(); return tmp;}

    inline bool operator==(const DataValueWrapper& other) {return it == other.it;}
    inline bool operator!=(const DataValueWrapper& other) {return it != other.it;}
    inline friend bool operator<(const DataValueWrapper& lhs, const DataValueWrapper& rhs) {return lhs.it < rhs.it;}
    inline friend bool operator>(const DataValueWrapper& lhs, const DataValueWrapper& rhs) {return lhs.it > rhs.it;}
    inline friend bool operator<=(const DataValueWrapper& lhs, const DataValueWrapper& rhs) {return lhs.it <= rhs.it;}
    inline friend bool operator>=(const DataValueWrapper& lhs, const DataValueWrapper& rhs) {return lhs.it >= rhs.it;}
    inline difference_type operator-(const DataValueWrapper& other) const {return this->it - other.it;}

    inline pseudo_val operator*()  {return pseudo_val{(*it)};}
    inline pseudo_val operator->() {return pseudo_val{(*it)};}
    inline QModelIndex index() {return (*it);}

    static DataValueWrapper begin(QAbstractItemModel *model, int column = 0) {auto it = ModelIterator::begin(model, column); return DataValueWrapper(it);}
    static DataValueWrapper end(QAbstractItemModel *model, int column = 0) {auto it = ModelIterator::end(model, column); return DataValueWrapper(it);}
};

// Adaptor class for QAIM to make it useable in contexts where a container is exected that
// provides an adaptor pair via begin() and end(). This template class allows you to use
// standard algorithms and ranged for loops on QAbstractItemModels.
//
// Use with the iterator type as the first template argument. As a second template argument,
// you can pass the type you want as the value_type, allowing you to automatically de-reference
// the model-indices with the role you can pass as the third template argument (Qt::DisplayRole
// by default.) If you only pass the iterator type, the value_type will be QModelIndex which are
// then not de-referenced.
//
// Pass the model and optionally column as the constructor arguments.The default column is column 0.
//
// General wrapper that returns iterators that dereference to T for the specified role
template <typename ModelIterator, typename T = void, int role = Qt::DisplayRole>
class ModelAdapter
{
public: //types
    using Wrapper = DataValueWrapper<ModelIterator, T, role>;
    using value_type = typename Wrapper::value_type;
    using iterator = Wrapper;
    using const_iterator = Wrapper;

public:
    explicit ModelAdapter(QAbstractItemModel *model, int column = 0) : m_model(model), m_column(column) {}

    Wrapper begin() const {return Wrapper::begin(m_model, m_column);}
    Wrapper end() const {return Wrapper::end(m_model, m_column);}

private:
    QAbstractItemModel *m_model;
    int m_column;
};

/**
 * Partial template specialization that returns iterators that dereference to a QModelIndex
 */
template <typename ModelIterator>
class ModelAdapter<ModelIterator, void>
{
public: //types
    using value_type = typename ModelIterator::value_type;
    using iterator = ModelIterator;
    using const_iterator = ModelIterator;

public:
    explicit ModelAdapter(QAbstractItemModel *model, int column = 0) : m_model(model), m_column(column) {}

    ModelIterator begin() const {return ModelIterator::begin(m_model, m_column);}
    ModelIterator end() const {return ModelIterator::end(m_model, m_column);}

private:
    QAbstractItemModel *m_model;
    int m_column;
};
