/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include <QGuiApplication>
#include <QQmlContext>
#include <QQuickView>

class Controller : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    Q_INVOKABLE void crash()
    {
        delete this;
        delete this;
    }
};

int main(int a, char **b)
{
    QGuiApplication app(a, b);

    QQuickView view;
    view.rootContext()->setContextProperty(QStringLiteral("_controller"), new Controller());
    view.setSource(QUrl(QStringLiteral("qrc:/main.qml")));
    view.show();

    return app.exec();
}

#include <main.moc>
