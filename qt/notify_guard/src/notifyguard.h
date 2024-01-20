/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: André Somers <andre.somers@kdab.com>

  SPDX-License-Identifier: MIT
*/

#ifndef KDTOOLBOX_NOTIFYGUARD_H
#define KDTOOLBOX_NOTIFYGUARD_H

#include <QMetaMethod>
#include <memory>

namespace KDToolBox
{

namespace Internal
{
struct SignalData;
using SignalDataSPtr = std::shared_ptr<SignalData>;
}

/** Generic property notification guard
 *
 * This class is a generic property notification guard, that can
 * monitor if a property was modified between the point of construction
 * and destruction of the guard object. If the property was modified,
 * the property's notifySignal will be emitted on destruction of
 * the guard object.
 *
 * NotifyGuard requires the property to have a notify signal, and
 * that the notify signal has either no or one argument that must
 * then match the type of the property itself.
 *
 * A NotifyGuard can be constructed using a property name or using
 * a pointer to a notify signal. In the first case, only the
 * indicated property will be monitored for changes. When using a
 * pointer to a notify signal, all properties using that notification
 * signal will be monitored for changes.
 *
 * NotifyGuard can work either in SingleScope or in RecursiveScope
 * mode (the default). In SingleScope, NotifyGuard only cares about
 * itself and will always emit the notify signal if the value of the
 * property has changed on destruction of the guard object. In
 * RecursiveScope mode, NotifyGuard will make sure that a notification
 * signal is only emitted once, from the outermost scope created for
 * the given notify signal. This minimizes the number of signal
 * emissions to prevent potentially expensive updates.
 */
class NotifyGuard
{
public:
    enum GuardOptions
    {
        RecursiveScope, //!< This guard will only activate if there isn't already another guard on the same property
                        //!< active (in an outer scope).
        SingleScope,    //!< This guard only considers its own scope.
    };

public:
    NotifyGuard() = default;
    explicit NotifyGuard(QObject *target, const char *property, GuardOptions options = RecursiveScope);
    template<typename PointerToMemberFunction>
    explicit NotifyGuard(QObject *target, PointerToMemberFunction notifySignal, GuardOptions options = RecursiveScope)
        : NotifyGuard(target, QMetaMethod::fromSignal(notifySignal), options)
    {
    }
    // we allow moving from a NotifyGuard
    NotifyGuard(NotifyGuard &&) = default;
    NotifyGuard &operator=(NotifyGuard &&) = default;

    ~NotifyGuard();

    inline bool isActive() const { return bool(m_signalData); }

private: // methods
    Q_DISABLE_COPY(NotifyGuard)
    explicit NotifyGuard(QObject *target, QMetaMethod notifySignal, GuardOptions options);

private: // members
    Internal::SignalDataSPtr m_signalData;
};

} // namespace KDToolBox

#endif // KDTOOLBOX_NOTIFYGUARD_H
