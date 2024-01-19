/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "messagehandler.h"

#include <QBasicMutex>

#include <atomic>
#include <forward_list>
#include <mutex>

namespace
{

struct RegisteredCallback
{
    const RegisteredCallback *next;
    QtMsgType messageType;
    QRegularExpression pattern;
    std::function<void()> callback;
};

std::once_flag oldMessageHandlerFlag;
std::atomic<QtMessageHandler> oldMessageHandler;
std::atomic<const RegisteredCallback *> callbacks;

bool isMessageTypeCompatibleWith(QtMsgType in, QtMsgType reference)
{
    switch (in)
    {
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
        if (reference == QtCriticalMsg || reference == QtWarningMsg || reference == QtDebugMsg ||
            reference == QtInfoMsg)
            return false;
        break;
    case QtInfoMsg:
        break;
    }

    return true;
}

void ourMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    for (auto it = callbacks.load(std::memory_order_acquire); it; it = it->next)
    { // synchronizes-with store-release in registerMessageHandler
        if (!isMessageTypeCompatibleWith(it->messageType, type))
            continue;
        if (message.contains(it->pattern))
            it->callback();
    }

    if (auto h = oldMessageHandler.load(
            std::memory_order_acquire)) // synchronizes-with the store-release in installOurMessageHandler()
        h(type, context, message);
}

} // anonymous namespace

void KDToolBox::Private::registerMessageHandler(QtMsgType type, const QRegularExpression &pattern,
                                                std::function<void()> func)
{
    const auto installOurMessageHandler = [] {
        oldMessageHandler.store(qInstallMessageHandler(&ourMessageHandler),
                                std::memory_order_release); // synchronizes-with the load-acquire in ourMessageHandler()
    };
    std::call_once(oldMessageHandlerFlag, installOurMessageHandler);

    auto tmp = new RegisteredCallback{nullptr, type, pattern, std::move(func)};
    auto expected = callbacks.load(std::memory_order_relaxed); // just the pointer value
    do
    {
        tmp->next = expected;
    } while (!callbacks.compare_exchange_weak(
        expected, tmp, std::memory_order_release)); // synchronizes-with load-acquire in outMessageHandler
}
