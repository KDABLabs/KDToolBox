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

#ifndef KDTOOLBOX_MESSAGEHANDLER_H
#define KDTOOLBOX_MESSAGEHANDLER_H

#include <QtGlobal>
#include <QString>
#include <QRegularExpression>

#include <functional>

namespace KDToolBox {

namespace Private {
void registerMessageHandler(QtMsgType type, const QRegularExpression &pattern, const std::function<void()> &func);
} // namespace Private

template <typename Callable>
void handleMessage(QtMsgType type, const QRegularExpression &pattern, Callable &&callback)
{
    Private::registerMessageHandler(type,
                                    pattern,
                                    std::forward<Callable>(callback));
}

template <typename Callable>
void handleMessage(QtMsgType type, const QString &needle, Callable &&callback)
{
    Private::registerMessageHandler(type,
                                    QRegularExpression::escape(needle),
                                    std::forward<Callable>(callback));
}

template <typename Callable>
void handleMessage(QtMsgType type, Callable &&callback)
{
    Private::registerMessageHandler(type,
                                    QRegularExpression(),
                                    std::forward<Callable>(callback));
}

} // namespace KDToolBox

#endif // KDTOOLBOX_MESSAGEHANDLER_H
