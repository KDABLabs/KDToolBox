/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Nicolas Arnaud-Cormos <nicolas.arnaud-cormos@kdab.com>

  SPDX-License-Identifier: MIT
*/

#ifndef TABWINDOW_H
#define TABWINDOW_H

#include <QCursor>
#include <QTabWidget>

QT_BEGIN_NAMESPACE
class QWindow;
QT_END_NAMESPACE
class TabWindow;

class TabWindowManager : public QObject
{
    Q_OBJECT

public:
    static TabWindowManager *instance();

    QList<TabWindow *> windows() const;

    TabWindow *currentWindow() const;
    QWidget *currentWidget() const;

Q_SIGNALS:
    void tabCloseRequested(QWidget *widget, TabWindow *window);

protected:
    void addWindow(TabWindow *window);
    void removeWindow(TabWindow *window);

    TabWindow *possibleWindow(TabWindow *currentWindow, QPoint globalPos = QCursor::pos());

private:
    void activateWindow(QWindow *window);
    void requestCloseTab(int index);

private:
    using QObject::QObject;
    TabWindowManager();

    friend class TabWindow;
    QList<TabWindow *> m_windows;
};

class TabWindow : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWindow(QWidget *parent = 0);
    ~TabWindow();

    bool eventFilter(QObject *object, QEvent *event) override;

private:
    bool m_isMoving = false;
    bool m_ignoreMouseEvent = false;
    TabWindow *m_movingWindow = nullptr;
    QPoint m_mouseDelta;
};

#endif // TABWINDOW_H
