/****************************************************************************
**                                MIT License
**
** Copyright (C) 2020 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
** Author: Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>
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

#ifndef KDTOOLBOX_QT_HASHER_H
#define KDTOOLBOX_QT_HASHER_H

#include <QtCore/qhashfunctions.h>

#include <utility>
#include <functional>

namespace KDToolBox {

namespace detail {

QT_USE_NAMESPACE

template <typename ...> using void_t = void;

using QHashSeedType = decltype(qHash(0)); // which is also the return type of qHash

template <typename T, typename = void>
class QtHasherBase
{
    // poison
private:
    ~QtHasherBase();
    QtHasherBase(QtHasherBase &&);
};

template <typename T>
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

template <typename T>
struct QtHasher : detail::QtHasherBase<T>
{
};

} // namespace KDToolBox

#endif // KDTOOLBOX_QT_HASHER_H
