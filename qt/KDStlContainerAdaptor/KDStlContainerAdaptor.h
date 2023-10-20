/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>

  SPDX-License-Identifier: MIT
*/

#ifndef KDTOOLBOX_STLCONTAINERADAPTOR_H
#define KDTOOLBOX_STLCONTAINERADAPTOR_H

#include <QtGlobal>

#include <algorithm>
#include <iterator>
#include <vector>

namespace KDToolBox
{
namespace StlContainerAdaptor
{

namespace Private
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
using qt_size_type = int;
#else
using qt_size_type = qsizetype;
#endif
} // namespace Private

template<typename T, typename... Args>
struct StdVectorAdaptor : std::vector<T, Args...>
{
    using base_container = std::vector<T, Args...>;
    using base_size_type = typename base_container::size_type;
    using size_type = Private::qt_size_type;
    static_assert(std::is_signed_v<size_type>);
    using value_type = typename base_container::value_type;

    using Iterator = typename base_container::iterator;
    using ConstIterator = typename base_container::const_iterator;

    // Construction / RO5
    using base_container::base_container;

    StdVectorAdaptor() = default; // work around broken compilers in C++17 mode
    explicit StdVectorAdaptor(size_type count)
        : base_container(base_size_type(count))
    {
    }
    StdVectorAdaptor(size_type count, const value_type &v)
        : base_container(base_size_type(count), v)
    {
    }

    [[deprecated("Use clone() instead; copies of non-Qt containers are not cheap")]] StdVectorAdaptor(
        const StdVectorAdaptor &) = default;
    [[deprecated("Use assignFrom() instead; copies of non-Qt containers are not cheap")]] StdVectorAdaptor &
    operator=(const StdVectorAdaptor &) = default;

    StdVectorAdaptor(StdVectorAdaptor &&) = default;
    StdVectorAdaptor &operator=(StdVectorAdaptor &&) = default;

    ~StdVectorAdaptor() = default;

    StdVectorAdaptor clone() const
    {
        QT_WARNING_PUSH
        QT_WARNING_DISABLE_DEPRECATED
        return *this;
        QT_WARNING_POP
    }

    StdVectorAdaptor &assignFrom(const StdVectorAdaptor &other)
    {
        QT_WARNING_PUSH
        QT_WARNING_DISABLE_DEPRECATED
        return *this = other;
        QT_WARNING_POP
    }
    StdVectorAdaptor &assignFrom(StdVectorAdaptor &&other) { return *this = std::move(other); }

    // Iterators
    decltype(auto) constBegin() const { return this->cbegin(); }
    decltype(auto) constEnd() const { return this->cend(); }

    // Data access
    decltype(auto) constData() const { return this->data(); }
    decltype(auto) at(size_type i) const { return base_container::operator[](base_size_type(i)); }
    decltype(auto) operator[](size_type i) { return base_container::operator[](base_size_type(i)); }
    decltype(auto) operator[](size_type i) const { return base_container::operator[](base_size_type(i)); }

    auto value(size_type i, const T &defaultValue) const
    {
        if (i >= 0 && i < size())
            return at(i);
        return defaultValue;
    }
    auto value(size_type i) const
    {
        if (i >= 0 && i < size())
            return at(i);
        return value_type();
    }

    decltype(auto) first() { return this->front(); }
    decltype(auto) first() const { return this->front(); }
    decltype(auto) constFirst() const { return this->front(); }

    decltype(auto) last() { return this->back(); }
    decltype(auto) last() const { return this->back(); }
    decltype(auto) constLast() const { return this->back(); }

    // Size and capacity
    Q_REQUIRED_RESULT decltype(auto) isEmpty() const { return this->empty(); }
    Q_REQUIRED_RESULT decltype(auto) size() const { return size_type(base_container::size()); }
    Q_REQUIRED_RESULT decltype(auto) count() const { return size(); }
    Q_REQUIRED_RESULT decltype(auto) length() const { return size(); }
    Q_REQUIRED_RESULT decltype(auto) capacity() const { return size_type(base_container::capacity()); }
    void reserve(size_type s) { base_container::reserve(base_size_type(s)); }
    void squeeze() { this->shrink_to_fit(); }

    // Insertion
    void append(const value_type &v) { this->push_back(v); }
    void append(value_type &&v) { this->push_back(std::move(v)); }
    void append(const StdVectorAdaptor &other)
    {
        if (this != &other)
        {
            this->insert(this->end(), other.begin(), other.end());
        }
        else
        {
            this->reserve(2 * size());
            std::copy(this->begin(), this->end(), std::back_inserter(*this));
        }
    }
    void prepend(const value_type &v) { this->insert(this->begin(), v); }
    void prepend(value_type &&v) { this->insert(this->begin(), std::move(v)); }

    using base_container::insert;
    decltype(auto) insert(size_type position, const value_type &v) { return this->insert(this->begin() + position, v); }
    decltype(auto) insert(size_type position, value_type &&v)
    {
        return this->insert(this->begin() + position, std::move(v));
    }

    // Removal
    void removeFirst() { this->erase(this->begin()); }
    void removeLast() { this->pop_back(); }
    void remove(size_type position) { this->erase(this->begin() + position); }
    void remove(size_type position, size_type count)
    {
        const auto b = this->begin();
        this->erase(b + position, b + position + count);
    }
    void removeAt(size_type position) { this->erase(this->begin() + position); }

    template<typename AT>
    decltype(auto) removeAll(const AT &v)
    {
        const auto b = this->begin();
        const auto e = this->end();
        const auto i = std::remove(b, e, v);
        const auto result = size_type(e - i);
        this->erase(i, e);
        return result;
    }

    template<typename AT>
    bool removeOne(const AT &v)
    {
        const auto b = this->begin();
        const auto e = this->end();
        const auto i = std::find(b, e, v);
        if (i == e)
            return false;
        this->erase(i);
        return true;
    }

    template<typename Predicate>
    size_type removeIf(Predicate p)
    {
        const auto b = this->begin();
        const auto e = this->end();
        const auto it = std::remove_if(b, e, std::ref(p));
        const auto result = size_type(std::distance(it, e));
        this->erase(it, e);
        return result;
    }

    decltype(auto) takeAt(size_type i)
    {
        const auto it = this->begin() + i;
        const auto result = std::move(*it);
        this->erase(it);
        return result;
    }
    decltype(auto) takeLast() { return takeAt(size() - 1); }
    decltype(auto) takeFirst() { return takeAt(0); }

    // Search
    template<typename AT>
    Q_REQUIRED_RESULT bool contains(const AT &v) const
    {
        return indexOf(v) >= 0;
    }

    template<typename AT>
    Q_REQUIRED_RESULT size_type indexOf(const AT &v, size_type from = 0) const
    {
        const auto s = size();
        if (from < 0)
            from = std::max(from + s, size_type(0));
        if (from < s)
        {
            const auto b = this->begin();
            const auto e = this->end();
            const auto i = std::find(b + from, e, v);
            if (i != e)
                return size_type(i - b);
        }
        return size_type(-1);
    }

    template<typename AT>
    Q_REQUIRED_RESULT size_type lastIndexOf(const AT &v, size_type from = -1) const
    {
        const auto s = size();
        if (from < 0)
            from += s;
        else if (from >= s)
            from = s - 1;

        if (from >= 0)
        {
            const auto b = this->begin();

            const auto revB = std::make_reverse_iterator(b + from + 1);
            const auto revE = std::make_reverse_iterator(b);
            const auto i = std::find(revB, revE, v);
            if (i != revE)
                return size_type(i.base() - b) - 1;
        }
        return size_type(-1);
    }

    template<typename AT>
    Q_REQUIRED_RESULT bool startsWith(const AT &v) const
    {
        return this->front() == v;
    }

    template<typename AT>
    Q_REQUIRED_RESULT bool endsWith(const AT &v) const
    {
        return this->back() == v;
    }

    // Miscellanea
    StdVectorAdaptor &fill(const value_type &v, size_type i = -1)
    {
        if (i < 0)
            i = size();
        this->assign(base_size_type(i), v);
        return *this;
    }

    StdVectorAdaptor mid(size_type pos, size_type len = -1)
    {
        const auto s = size();

        if (len < 0)
            len = s;
        len = std::min(len, s - pos);

        const auto b = this->begin() + pos;
        const auto e = b + len;
        return StdVectorAdaptor(b, e);
    }

    void move(size_type from, size_type to)
    {
        const auto b = this->begin();
        if (from < to)
            std::rotate(b + from, b + from + 1, b + to + 1);
        else
            std::rotate(b + to, b + from, b + from + 1);
    }

    void replace(size_type pos, const value_type &v) { at(pos) = v; }

    void swapItemsAt(size_type i, size_type j) { qSwap(at(i), at(j)); }

    // Misc operators
    decltype(auto) operator+=(const StdVectorAdaptor & other)
    {
        append(other);
        return *this;
    }
};

template<typename T>
decltype(auto) operator<<(StdVectorAdaptor<T> &adaptor, const typename StdVectorAdaptor<T>::value_type & v)
{
    adaptor.push_back(v);
    return adaptor;
}

template<typename T>
decltype(auto) operator<<(StdVectorAdaptor<T> &adaptor, typename StdVectorAdaptor<T>::value_type && v)
{
    adaptor.push_back(std::move(v));
    return adaptor;
}

template<typename T>
decltype(auto) operator<<(StdVectorAdaptor<T> &lhs, const StdVectorAdaptor<T> &rhs)
{
    lhs.append(rhs);
    return lhs;
}

template<typename T>
decltype(auto) operator+(const StdVectorAdaptor<T> &lhs, const StdVectorAdaptor<T> &rhs)
{
    StdVectorAdaptor<T> result;
    result.reserve(lhs.size() + rhs.size());
    result.insert(result.end(), lhs.begin(), lhs.end());
    result.insert(result.end(), rhs.begin(), rhs.end());
    return result;
}

} // namespace StlContainerAdaptor
} // namespace KDToolBox

#endif // KDTOOLBOX_STLCONTAINERADAPTOR_H
