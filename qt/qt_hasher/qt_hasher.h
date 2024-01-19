/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>

  SPDX-License-Identifier: MIT
*/

#ifndef KDTOOLBOX_QT_HASHER_H
#define KDTOOLBOX_QT_HASHER_H

#include <QtCore/qhashfunctions.h>

#include <functional>
#include <utility>

namespace KDToolBox
{

namespace detail
{

QT_USE_NAMESPACE

template<typename...>
using void_t = void;

using QHashSeedType = decltype(qHash(0)); // which is also the return type of qHash

template<typename T, typename = void>
class QtHasherBase
{
    // poison
private:
    ~QtHasherBase();
    QtHasherBase(QtHasherBase &&);
};

template<typename T>
class QtHasherBase<T, void_t<decltype(qHash(std::declval<const T &>()))>>
{
public:
    using result_type = std::size_t;
    using argument_type = T;

    constexpr std::size_t operator()(const T &t) const noexcept
    {
        // this seeds qHash with the result of
        // std::hash applied to an int, to reap
        // any protection against predictable hash
        // values the std implementation may provide
        return static_cast<std::size_t>(qHash(t, static_cast<QHashSeedType>(std::hash<int>{}(0))));
    }

protected:
    // prevent accidental slicing, or compilers complaining about a non-virtual dtor
    QtHasherBase() = default;
    ~QtHasherBase() = default;
    QtHasherBase(const QtHasherBase &) = default;
    QtHasherBase(QtHasherBase &&) = default;
    QtHasherBase &operator=(const QtHasherBase &) = default;
    QtHasherBase &operator=(QtHasherBase &&) = default;
};

} // namespace detail

template<typename T>
struct QtHasher : detail::QtHasherBase<T>
{
};

} // namespace KDToolBox

#endif // KDTOOLBOX_QT_HASHER_H
