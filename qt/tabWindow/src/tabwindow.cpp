/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Nicolas Arnaud-Cormos <nicolas.arnaud-cormos@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "tabwindow.h"

#include <QApplication>
#include <QList>
#include <QMouseEvent>
#include <QTabBar>
#include <QWindow>

namespace
{
const constexpr int DISTANCE_TO_DETACH = 20;

int distance(QRect r, QPoint p)
{
    if (r.contains(p))
        return 0;

    auto ref = [](int a, int b, int v) {
        if (v < a) // left/top
            return a;
        else if (v > b) // right/bottom
            return b;
        else // in-between
            return v;
    };

    const auto refPoint = QPoint(ref(r.left(), r.right(), p.x()), ref(r.top(), r.bottom(), p.y()));

    return (refPoint - p).manhattanLength();
}
}

TabWindowManager::TabWindowManager()
{
    connect(qApp, &QApplication::focusWindowChanged, this, &TabWindowManager::activateWindow);
}

TabWindowManager *TabWindowManager::instance()
{
    static TabWindowManager manager;
    return &manager;
}

QList<TabWindow *> TabWindowManager::windows() const
{
    return m_windows;
}

TabWindow *TabWindowManager::currentWindow() const
{
    if (!m_windows.isEmpty())
        return m_windows.first();
    return nullptr;
}

QWidget *TabWindowManager::currentWidget() const
{
    if (!m_windows.isEmpty())
        return m_windows.first()->currentWidget();
    return nullptr;
}

void TabWindowManager::addWindow(TabWindow *window)
{
    m_windows.append(window);
    connect(window, &TabWindow::tabCloseRequested, this, &TabWindowManager::requestCloseTab);
}

void TabWindowManager::removeWindow(TabWindow *window)
{
    m_windows.removeOne(window);
}

TabWindow *TabWindowManager::possibleWindow(TabWindow *currentWindow, QPoint globalPos)
{
    for (auto tabWindow : qAsConst(m_windows))
    {
        if (tabWindow == currentWindow)
            continue;
        // Get the possible drop rectangle, which is the rectangle at the top
        // containing the tabbar
        if (tabWindow->frameGeometry().contains(globalPos))
        {
            auto pos = tabWindow->tabBar()->mapFromGlobal(globalPos);
            auto r = tabWindow->tabBar()->rect();
            r.setWidth(tabWindow->rect().width());
            if (r.contains(pos))
                return tabWindow;
            return nullptr;
        }
    }
    return nullptr;
}

void TabWindowManager::activateWindow(QWindow *window)
{
    if (m_windows.count() < 2)
        return;

    int index = -1;
    for (int i = 1; i < m_windows.count(); ++i)
    {
        if (m_windows.at(i)->windowHandle() == window)
        {
            index = i;
            break;
        }
    }

    if (index != -1)
        m_windows.move(index, 0);
}

void TabWindowManager::requestCloseTab(int index)
{
    auto window = qobject_cast<TabWindow *>(sender());
    Q_EMIT tabCloseRequested(window->widget(index), window);
}

TabWindow::TabWindow(QWidget *parent)
    : QTabWidget(parent)
{
    Q_ASSERT(TabWindowManager::instance());
    TabWindowManager::instance()->addWindow(this);
    tabBar()->installEventFilter(this);

    setMovable(true);
    setDocumentMode(true);
    setFocusPolicy(Qt::NoFocus);

    resize(800, 600);
}

TabWindow::~TabWindow()
{
    TabWindowManager::instance()->removeWindow(this);
}

bool TabWindow::eventFilter(QObject *object, QEvent *event)
{
    if (object != tabBar())
        return QTabWidget::eventFilter(object, event);

    switch (event->type())
    {
    case QEvent::MouseMove:
    {
        if (m_ignoreMouseEvent)
            return true;
        auto mouseEvent = static_cast<QMouseEvent *>(event);

        auto sendFakeEvent = [mouseEvent](QObject *receiver, QEvent::Type type) {
            QMouseEvent newEvent(type, mouseEvent->pos(), Qt::LeftButton, mouseEvent->buttons(),
                                 mouseEvent->modifiers());
            QCoreApplication::sendEvent(receiver, &newEvent);
        };

        if (m_isMoving)
        {
            if (m_movingWindow)
            {
                auto globalPos = QCursor::pos();
                auto window = TabWindowManager::instance()->possibleWindow(m_movingWindow, globalPos);
                if (window)
                {
                    // re-attach
                    const auto pos = window->mapFromGlobal(globalPos);
                    const int index = tabBar()->tabAt(pos);
                    const auto w = m_movingWindow->widget(0);
                    const auto text = m_movingWindow->tabText(0);
                    window->raise();
                    window->activateWindow();
                    window->setCurrentIndex(window->insertTab(index, w, text));
                    m_movingWindow->deleteLater();
                    m_movingWindow = nullptr;
                    m_isMoving = false;
                    m_ignoreMouseEvent = true;
                    sendFakeEvent(object, QEvent::MouseButtonRelease);
                    sendFakeEvent(window->tabBar(), QEvent::MouseButtonPress);
                }
                else
                {
                    auto newPos = globalPos - m_mouseDelta;
                    m_movingWindow->move(newPos);
                }
                return true;
            }
        }
        else
        {
            auto r = tabBar()->rect();
            if (tabBar()->count() == 1 || distance(r, mouseEvent->pos()) > DISTANCE_TO_DETACH)
            {
                sendFakeEvent(object, QEvent::MouseButtonRelease);

                m_isMoving = true;
                const int index = currentIndex();

                auto tabRect = tabBar()->tabRect(index);
                m_mouseDelta = tabRect.center() - tabRect.topLeft() + (geometry().topLeft() - pos());

                if (tabBar()->count() >= 2)
                {
                    m_movingWindow = new TabWindow;
                    const auto w = widget(index);
                    const auto text = tabText(index);
                    const auto icon = tabIcon(index);
                    m_movingWindow->setGeometry(rect());
                    const auto globalPos = QCursor::pos();
                    m_movingWindow->move(globalPos.x() + 100, globalPos.y() + 100);
                    // BUG: it would be better to add the tab before showing, but on Windows
                    // with poor openGL driver, there is actually a crash when the tab is a QQuickWidget
                    m_movingWindow->show();
                    m_movingWindow->addTab(w, icon, text);
                }
                else
                {
                    m_movingWindow = this;
                }
                return true;
            }
        }
        break;
    }
    case QEvent::MouseButtonRelease:
        m_isMoving = false;
        m_movingWindow = nullptr;
        m_ignoreMouseEvent = false;
        break;
    case QEvent::MouseButtonPress:
        m_ignoreMouseEvent = false;
        break;
    default:
        // Remove warning
        break;
    }
    return QTabWidget::eventFilter(object, event);
}
