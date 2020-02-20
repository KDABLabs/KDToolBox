/****************************************************************************
**                                MIT License
**
** Copyright (C) 2020 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
** Author: Nicolas Arnaud-Cormos <nicolas.arnaud-cormos@kdab.com>
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

#ifndef TABWINDOW_H
#define TABWINDOW_H

#include <QTabWidget>
#include <QCursor>

QT_BEGIN_NAMESPACE
class QWindow;
QT_END_NAMESPACE
class TabWindow;

class TabWindowManager : public QObject
{
    Q_OBJECT

public:
    static TabWindowManager *instance();

    QList<TabWindow*> windows() const;

    TabWindow *currentWindow() const;
    QWidget *currentWidget() const;

Q_SIGNALS:
    void tabCloseRequested(QWidget *widget, TabWindow *window);

protected:
    void addWindow(TabWindow *window);
    void removeWindow(TabWindow *window);

    TabWindow *possibleWindow(TabWindow *currentWindow, const QPoint &globalPos = QCursor::pos());

private:
    void activateWindow(QWindow *window);
    void requestCloseTab(int index);

private:
    explicit TabWindowManager();

    friend class TabWindow;
    QList<TabWindow*> m_windows;
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
