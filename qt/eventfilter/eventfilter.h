/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: MIT
*/

#ifndef KDTOOLBOX_EVENTFILTER_H
#define KDTOOLBOX_EVENTFILTER_H

///@file
///@brief Implements KDToolBox::EventFilter

#include <QEvent>
#include <QObject>
#include <QPointer>

#include <memory>
#include <type_traits>

namespace KDToolBox
{

namespace KDToolBoxPrivate
{

template<typename...>
using void_t = void;

// TODO: refactor in two use cases to avoid combinatorial explosion
// (1: invoke with or without parameters; 2: always return bool)
template<typename C, typename Enable = void_t<>>
struct CallbackInvoker
{
    static bool invoke(C &c, QObject *, QEvent *)
    {
        (void)c();
        return false;
    }
};

template<typename C>
struct CallbackInvoker<C,
                       typename std::enable_if<std::is_convertible<decltype(std::declval<C>()()), bool>::value>::type>
{
    static bool invoke(C &c, QObject *, QEvent *) { return c(); }
};

template<typename C>
struct CallbackInvoker<C, typename std::enable_if<std::is_void<decltype(std::declval<C>()(
                              std::declval<QObject *>(), std::declval<QEvent *>()))>::value>::type>
{
    static bool invoke(C &c, QObject *target, QEvent *event)
    {
        (void)c(target, event);
        return false;
    }
};

template<typename C>
struct CallbackInvoker<
    C, typename std::enable_if<std::is_convertible<
           decltype(std::declval<C>()(std::declval<QObject *>(), std::declval<QEvent *>())), bool>::value>::type>
{
    static bool invoke(C &c, QObject *target, QEvent *event) { return c(target, event); }
};

} // namespace KDToolBoxPrivate

template<typename Callback>
class EventFilter : public QObject
{
    Q_DISABLE_COPY(EventFilter)

private:
    template<typename C>
    explicit EventFilter(QObject *target, QEvent::Type eventType, C &&callback, QObject *parent)
        : QObject(parent)
        , m_eventType(eventType)
        , m_callback(std::forward<C>(callback))
        , m_target(target)
    {
        Q_ASSERT(target);
        target->installEventFilter(this);
    }

    bool eventFilter(QObject *target, QEvent *event) override
    {
        if (m_eventType == event->type() || m_eventType == QEvent::None)
            return KDToolBoxPrivate::CallbackInvoker<Callback>::invoke(m_callback, target, event);

        return QObject::eventFilter(target, event);
    }

    template<typename C>
    friend EventFilter<typename std::remove_reference<C>::type> *
    installEventFilter(QObject *target, QEvent::Type eventType, C &&callback, QObject *parent);

    const QEvent::Type m_eventType;
    Callback m_callback;
    const QPointer<const QObject> m_target;
};

/**
 * @brief Installs an event filter which will call the specified function @p callback when receiving
 * events of type @p eventType.
 *
 * It's common that classes sometimes miss signals and we're forced to write boilerplate event filters.
 * For example, QWidget::isEnabled() doesn't emit any signal when enabled changed.
 * You can now write simply:
 *
 * auto filter = KDToolBox::installEventFilter(button, QEvent::EnabledChange, [button] {
 *       qDebug() << "Enabled changed" << button->isEnabled();
 *       return false; // propagate the event
 * });
 *
 * @param target the target object on which we'll intercept all events
 * @param eventType the event type we're interested in. QEvent::None will listen to all events.
 * @param callback the function to call once we receive an event. It may have 0 parameters or 2 parameters
 * (in which case, they must be QObject * and QEvent *, matching the signature of QObject::eventFilter).
 * The callback may also return a boolean. If it returns true, the event will be filtered out and it will
 * not reach the target. If it returns false, the event processing will continue. If the callback
 * does not return anything, return false is implied.
 * @param parent Optional QObject parent for ownership purposes
 */
template<typename Callback>
EventFilter<typename std::remove_reference<Callback>::type> *installEventFilter(QObject *target, QEvent::Type eventType,
                                                                                Callback &&callback, QObject *parent)
{
    return new EventFilter<Callback>(target, eventType, std::forward<Callback>(callback), parent);
}

/**
 * @brief Overload which returns a std::unique_ptr instead of receiving a QObject parent.
 */
template<typename Callback>
Q_REQUIRED_RESULT std::unique_ptr<EventFilter<typename std::remove_reference<Callback>::type>>
installEventFilter(QObject *target, QEvent::Type eventType, Callback &&callback)
{
    return std::unique_ptr<EventFilter<typename std::remove_reference<Callback>::type>>(
        installEventFilter(target, eventType, std::forward<Callback>(callback), nullptr));
}

} // namespace KDToolBox

#endif
