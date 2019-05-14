/****************************************************************************
** Copyright (C) 2019 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** Author: sergio.martins@kdab.com
** All rights reserved.
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** You may even contact us at info@kdab.com for different licensing options.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#include <QString>
#include <QVector>
#include <QGuiApplication>
#include <QQuickWindow>
#include <QQmlContext>
#include <QQuickItem>
#include <QtQml/private/qqmlengine_p.h>

extern "C"  char *qt_v4StackTrace(void *executionContext);

namespace KDAB {

QString qmlStackTrace(QV4::ExecutionEngine *engine)
{
    return QString(qt_v4StackTrace(engine->currentContext()));
}

void printQmlStackTraces()
{
    auto windows = qApp->topLevelWindows();
    for (QWindow *w : windows) {
        if (auto qw = qobject_cast<QQuickWindow*>(w)) {
            QQuickItem *item = qw->contentItem();
            QQmlContext* context = QQmlEngine::contextForObject(item);
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
