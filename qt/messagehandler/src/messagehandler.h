/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>

  SPDX-License-Identifier: MIT
*/

#ifndef KDTOOLBOX_MESSAGEHANDLER_H
#define KDTOOLBOX_MESSAGEHANDLER_H

#include <QRegularExpression>
#include <QString>
#include <QtGlobal>

#include <functional>

namespace KDToolBox
{

namespace Private
{
void registerMessageHandler(QtMsgType type, const QRegularExpression &pattern, std::function<void()> func);
} // namespace Private

template<typename Callable>
void handleMessage(QtMsgType type, const QRegularExpression &pattern, Callable &&callback)
{
    Private::registerMessageHandler(type, pattern, std::forward<Callable>(callback));
}

template<typename Callable>
void handleMessage(QtMsgType type, const QString &needle, Callable &&callback)
{
    Private::registerMessageHandler(type, QRegularExpression{QRegularExpression::escape(needle)},
                                    std::forward<Callable>(callback));
}

template<typename Callable>
void handleMessage(QtMsgType type, Callable &&callback)
{
    Private::registerMessageHandler(type, QRegularExpression(), std::forward<Callable>(callback));
}

} // namespace KDToolBox

#endif // KDTOOLBOX_MESSAGEHANDLER_H
