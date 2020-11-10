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

#include "messagehandler.h"

#include <QBasicMutex>

#include <mutex>
#include <forward_list>

namespace {

struct RegisteredCallback {
    const RegisteredCallback *next;
    QtMsgType messageType;
    QRegularExpression pattern;
    std::function<void()> callback;
};

std::once_flag oldMessageHandlerFlag;
QtMessageHandler oldMessageHandler(nullptr);

std::atomic<const RegisteredCallback*> callbacks;

bool isMessageTypeCompatibleWith(QtMsgType in, QtMsgType reference)
{
    switch (in) {
    case QtDebugMsg:
        if (reference == QtInfoMsg)
            return false;
        break;
    case QtWarningMsg:
        if (reference == QtDebugMsg || reference == QtInfoMsg)
            return false;
        break;
    case QtCriticalMsg:
        if (reference == QtWarningMsg || reference == QtDebugMsg || reference == QtInfoMsg)
            return false;
        break;
    case QtFatalMsg:
        if (reference == QtCriticalMsg || reference == QtWarningMsg || reference == QtDebugMsg || reference == QtInfoMsg)
            return false;
        break;
    case QtInfoMsg:
        break;
    }

    return true;
}

void ourMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    for (auto it = callbacks.load(std::memory_order_acquire); it; it = it->next) { // synchronizes-with store-release in registerMessageHandler
        if (!isMessageTypeCompatibleWith(it->messageType, type))
            continue;
        if (message.contains(it->pattern))
            it->callback();
    }

    oldMessageHandler(type, context, message);
}

} // anonymous namespace

void KDToolBox::Private::registerMessageHandler(QtMsgType type, const QRegularExpression &pattern, std::function<void()> func)
{
    std::call_once(oldMessageHandlerFlag,
                   []() { oldMessageHandler = qInstallMessageHandler(&ourMessageHandler); });

    auto tmp = new RegisteredCallback{nullptr, type, pattern, std::move(func)};
    auto expected = callbacks.load(std::memory_order_relaxed); // just the pointer value
    do {
        tmp->next = expected;
    } while (!callbacks.compare_exchange_weak(expected, tmp, std::memory_order_release)); // synchronizes-with load-acquire in outMessageHandler
}
