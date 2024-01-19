/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include <QGuiApplication>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>
#include <QString>
#include <QVector>
#include <QtQml/private/qqmlengine_p.h>

extern "C" char *qt_v4StackTrace(void *executionContext);

namespace KDAB
{

QString qmlStackTrace(QV4::ExecutionEngine *engine)
{
    return QString::fromUtf8(qt_v4StackTrace(engine->currentContext()));
}

void printQmlStackTraces()
{
    const auto windows = qApp->topLevelWindows();
    for (QWindow *w : windows)
    {
        if (auto qw = qobject_cast<QQuickWindow *>(w))
        {
            QQuickItem *item = qw->contentItem();
            QQmlContext *context = QQmlEngine::contextForObject(item);
            if (!context)
                continue;
            QQmlEnginePrivate *enginePriv = QQmlEnginePrivate::get(context->engine());
            QV4::ExecutionEngine *v4engine = enginePriv->v4engine();
            qDebug() << "Stack trace for" << qw;
            qDebug().noquote() << qmlStackTrace(v4engine);
            qDebug() << "\n";
        }
    }
}

}
