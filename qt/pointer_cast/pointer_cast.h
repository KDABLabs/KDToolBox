/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>

  SPDX-License-Identifier: MIT
*/

#pragma once

#include <QSharedPointer>

#include <utility>

inline namespace KDToolBox
{

template<typename To, typename From>
QSharedPointer<To> static_pointer_cast(const QSharedPointer<From> &from)
{
    return from.template staticCast<To>();
}

template<typename To, typename From>
QSharedPointer<To> dynamic_pointer_cast(const QSharedPointer<From> &from)
{
    return from.template dynamicCast<To>();
}

template<typename To, typename From>
QSharedPointer<To> const_pointer_cast(const QSharedPointer<From> &from)
{
    return from.template constCast<To>();
}

template<typename T, typename... Args>
QSharedPointer<T> make_qshared(Args &&...args)
{
    return QSharedPointer<T>::create(std::forward<Args>(args)...);
}

} // namespace KDToolBox
