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

#include <mutex>

namespace {

struct RegisteredCallback {
    QtMsgType messageType;
    QRegularExpression pattern;
    std::function<void()> callback;
};

template <typename Mutex>
class MutexRelocker {
    MutexRelocker(const MutexRelocker &) = delete;
    MutexRelocker &operator=(const MutexRelocker &) = delete;
    Mutex &m;
public:
    MutexRelocker(Mutex &m) : m(m) { m.unlock(); }
    ~MutexRelocker() { m.lock(); }
};

std::once_flag oldMessageHandlerFlag;
QtMessageHandler oldMessageHandler(nullptr);

std::mutex mutex; // protects the list
std::list<RegisteredCallback> callbacks;

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
    {
        std::unique_lock<std::mutex> lock(mutex);

        for (const auto &callback : callbacks) {
            MutexRelocker<std::unique_lock<std::mutex>> l(lock);

            if (!isMessageTypeCompatibleWith(callback.messageType, type))
                continue;

            if (message.contains(callback.pattern))
                callback.callback();
        }
    }

    oldMessageHandler(type, context, message);
}

} // anonymous namespace

void KDToolBox::Private::registerMessageHandler(QtMsgType type, const QRegularExpression &pattern, const std::function<void()> &func)
{
    std::call_once(oldMessageHandlerFlag,
                   []() { oldMessageHandler = qInstallMessageHandler(&ourMessageHandler); });

    RegisteredCallback r{type, pattern, func};
    std::lock_guard<std::mutex> guard(mutex);
    callbacks.push_back(std::move(r));
}
