/****************************************************************************
**                                MIT License
**
** Copyright (C) 2020-2021 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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

#pragma once

#include <QSharedPointer>

#include <utility>

inline namespace KDToolBox {

template <typename To, typename From>
QSharedPointer<To> static_pointer_cast(const QSharedPointer<From>& from)
{
    return from.template staticCast<To>();
}

template <typename To, typename From>
QSharedPointer<To> dynamic_pointer_cast(const QSharedPointer<From>& from)
{
    return from.template dynamicCast<To>();
}

template <typename To, typename From>
QSharedPointer<To> const_pointer_cast(const QSharedPointer<From>& from)
{
    return from.template constCast<To>();
}

template <typename T, typename...Args>
QSharedPointer<T> make_qshared(Args&&...args)
{
    return QSharedPointer<T>::create(std::forward<Args>(args)...);
}

} // namespace KDToolBox
