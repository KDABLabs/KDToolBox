/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>

  SPDX-License-Identifier: MIT
*/

#ifndef KDSIGNALTHROTTLER_H
#define KDSIGNALTHROTTLER_H

#include <QObject>
#include <QTimer>

#include <chrono>

namespace KDToolBox
{

class KDGenericSignalThrottler : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Kind kind READ kind CONSTANT)
    Q_PROPERTY(EmissionPolicy emissionPolicy READ emissionPolicy CONSTANT)
    Q_PROPERTY(int timeout READ timeout WRITE setTimeout NOTIFY timeoutChanged)
    Q_PROPERTY(Qt::TimerType timerType READ timerType WRITE setTimerType NOTIFY timerTypeChanged)

public:
    enum class Kind
    {
        Throttler,
        Debouncer,
    };
    Q_ENUM(Kind)

    enum class EmissionPolicy
    {
        Trailing,
        Leading,
    };
    Q_ENUM(EmissionPolicy)

    explicit KDGenericSignalThrottler(Kind kind, EmissionPolicy emissionPolicy, QObject *parent = nullptr);
    ~KDGenericSignalThrottler() override;

    Kind kind() const;
    EmissionPolicy emissionPolicy() const;

    int timeout() const;
    void setTimeout(int timeout);
    void setTimeout(std::chrono::milliseconds timeout);

    Qt::TimerType timerType() const;
    void setTimerType(Qt::TimerType timerType);

public Q_SLOTS:
    void throttle();

Q_SIGNALS:
    void triggered();
    void timeoutChanged(int timeout);
    void timerTypeChanged(Qt::TimerType timerType);

private:
    void maybeEmitTriggered();
    void emitTriggered();

    QTimer m_timer;
    Kind m_kind;
    EmissionPolicy m_emissionPolicy;
    bool m_hasPendingEmission;
};

// Convenience subclasses, e.g. for registering into QML

class KDSignalThrottler : public KDGenericSignalThrottler
{
    Q_OBJECT
public:
    explicit KDSignalThrottler(QObject *parent = nullptr);
    ~KDSignalThrottler() override;
};

class KDSignalLeadingThrottler : public KDGenericSignalThrottler
{
    Q_OBJECT
public:
    explicit KDSignalLeadingThrottler(QObject *parent = nullptr);
    ~KDSignalLeadingThrottler() override;
};

class KDSignalDebouncer : public KDGenericSignalThrottler
{
    Q_OBJECT
public:
    explicit KDSignalDebouncer(QObject *parent = nullptr);
    ~KDSignalDebouncer() override;
};

class KDSignalLeadingDebouncer : public KDGenericSignalThrottler
{
    Q_OBJECT
public:
    explicit KDSignalLeadingDebouncer(QObject *parent = nullptr);
    ~KDSignalLeadingDebouncer() override;
};

} // namespace KDToolBox

#endif // KDSIGNALTHROTTLER_H
