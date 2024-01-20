/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "KDSignalThrottler.h"

namespace KDToolBox
{

KDGenericSignalThrottler::KDGenericSignalThrottler(Kind kind, EmissionPolicy emissionPolicy, QObject *parent)
    : QObject(parent)
    , m_timer(this)
    , m_kind(kind)
    , m_emissionPolicy(emissionPolicy)
    , m_hasPendingEmission(false)
{
    // For leading throttlers we use a repeated timer. This is in order
    // to catch the case where a signal is received by the throttler
    // just after it emitted a throttled/debounced signal. Even if leading,
    // it shouldn't re-emit immediately, as it would be too close to the previous one.
    // So we keep the timer running, and stop it later if it times out
    // with no intervening timeout() emitted by it.
    switch (m_emissionPolicy)
    {
    case EmissionPolicy::Leading:
        m_timer.setSingleShot(false);
        break;
    case EmissionPolicy::Trailing:
        m_timer.setSingleShot(true);
        break;
    }
    connect(&m_timer, &QTimer::timeout, this, &KDGenericSignalThrottler::maybeEmitTriggered);
}

KDGenericSignalThrottler::~KDGenericSignalThrottler()
{
    maybeEmitTriggered();
}

KDGenericSignalThrottler::Kind KDGenericSignalThrottler::kind() const
{
    return m_kind;
}

KDGenericSignalThrottler::EmissionPolicy KDGenericSignalThrottler::emissionPolicy() const
{
    return m_emissionPolicy;
}

int KDGenericSignalThrottler::timeout() const
{
    return m_timer.interval();
}

void KDGenericSignalThrottler::setTimeout(int timeout)
{
    if (m_timer.interval() == timeout)
        return;
    m_timer.setInterval(timeout);
    Q_EMIT timeoutChanged(timeout);
}

void KDGenericSignalThrottler::setTimeout(std::chrono::milliseconds timeout)
{
    setTimeout(int(timeout.count()));
}

Qt::TimerType KDGenericSignalThrottler::timerType() const
{
    return m_timer.timerType();
}

void KDGenericSignalThrottler::setTimerType(Qt::TimerType timerType)
{
    if (m_timer.timerType() == timerType)
        return;
    m_timer.setTimerType(timerType);
    Q_EMIT timerTypeChanged(timerType);
}

void KDGenericSignalThrottler::throttle()
{
    m_hasPendingEmission = true;

    switch (m_emissionPolicy)
    {
    case EmissionPolicy::Leading:
        // Emit only if we haven't emitted already. We know if that's
        // the case by checking if the timer is running.
        if (!m_timer.isActive())
            emitTriggered();
        break;
    case EmissionPolicy::Trailing:
        break;
    }

    // The timer is started in all cases. If we got a signal,
    // and we're Leading, and we did emit because of that,
    // then we don't re-emit when the timer fires (unless we get ANOTHER
    // signal).
    switch (m_kind)
    {
    case Kind::Throttler:
        if (!m_timer.isActive())
            m_timer.start(); // = actual start, not restart
        break;
    case Kind::Debouncer:
        m_timer.start(); // = restart
        break;
    }

    Q_ASSERT(m_timer.isActive());
}

void KDGenericSignalThrottler::maybeEmitTriggered()
{
    if (m_hasPendingEmission)
        emitTriggered();
    else
        m_timer.stop();
}

void KDGenericSignalThrottler::emitTriggered()
{
    Q_ASSERT(m_hasPendingEmission);
    m_hasPendingEmission = false;
    Q_EMIT triggered();
}

// Convenience

KDSignalThrottler::KDSignalThrottler(QObject *parent)
    : KDGenericSignalThrottler(Kind::Throttler, EmissionPolicy::Trailing, parent)
{
}

KDSignalThrottler::~KDSignalThrottler() = default;

KDSignalLeadingThrottler::KDSignalLeadingThrottler(QObject *parent)
    : KDGenericSignalThrottler(Kind::Throttler, EmissionPolicy::Leading, parent)
{
}

KDSignalLeadingThrottler::~KDSignalLeadingThrottler() = default;

KDSignalDebouncer::KDSignalDebouncer(QObject *parent)
    : KDGenericSignalThrottler(Kind::Debouncer, EmissionPolicy::Trailing, parent)
{
}

KDSignalDebouncer::~KDSignalDebouncer() = default;

KDSignalLeadingDebouncer::KDSignalLeadingDebouncer(QObject *parent)
    : KDGenericSignalThrottler(Kind::Debouncer, EmissionPolicy::Leading, parent)
{
}

KDSignalLeadingDebouncer::~KDSignalLeadingDebouncer() = default;

} // namespace KDToolBox
