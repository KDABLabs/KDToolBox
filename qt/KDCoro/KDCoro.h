/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#pragma once

#include <QPointer>
#include <QLoggingCategory>
#include <QtDebug>
#include <qtestcase.h>

#include <coroutine>
#include <expected>

namespace KDToolBox
{

template <typename T>
class KDCoroExpected
{
    Q_DISABLE_COPY(KDCoroExpected)
public:
    bool await_ready() const noexcept
    {
        return m_result.has_value() || !m_result.error().isEmpty();
    }

    bool await_suspend(std::coroutine_handle<> h) noexcept
    {
        m_handle = h;
        if (m_sender) {
            m_receiverConn = QObject::connect(m_sender, &QObject::destroyed, [h, this] {
                m_result = std::unexpected(QStringLiteral("KDCoroExpected: QObject receiver* destroyed"));
                h.resume();
            });
        }
        return !await_ready();
    }

    std::expected<T, QString> await_resume() { return m_result; }

    KDCoroExpected(QObject *sender, QObject *receiver = nullptr)
        : m_sender(sender)
        , m_result{std::unexpected(QString{})}
    {
        callbackExpected = [this](T result) {
            m_result = result;
            QObject::disconnect(m_lambdaConn);

            if (m_handle) {
                m_handle.resume();
            }
        };

        callbackUnexpected = [this](const QString &error) {
            m_result = std::unexpected(error);
            QObject::disconnect(m_lambdaConn);

            if (m_handle) {
                m_handle.resume();
            }
        };

        if (receiver) {
            m_receiverConn = QObject::connect(receiver, &QObject::destroyed, [this] {
                m_result = std::unexpected(QStringLiteral("KDCoroExpected: QObject receiver* destroyed"));
                m_handle.resume();
            });
        }
    }

    std::suspend_never initial_suspend() const noexcept { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }

    KDCoroExpected(KDCoroExpected &&other) {
        callbackExpected = std::move(other.callbackExpected);
        callbackUnexpected = std::move(other.callbackUnexpected);

        m_senderConn = std::move(other.m_senderConn);
        m_receiverConn = std::move(other.m_receiverConn);
        m_lambdaConn = std::move(other.m_lambdaConn);
        m_sender = std::move(other.m_sender);
        m_result = std::move(other.m_result);
        m_handle = std::move(other.m_handle);
    }
    ~KDCoroExpected() {
        QObject::disconnect(m_senderConn);
        QObject::disconnect(m_receiverConn);
    }

    std::function<void(T)> callbackExpected;
    std::function<void(const QString &error)> callbackUnexpected;

    QMetaObject::Connection m_senderConn;
    QMetaObject::Connection m_receiverConn;
    QMetaObject::Connection m_lambdaConn;
    QPointer<QObject> m_sender;
    std::expected<T, QString> m_result;
    std::coroutine_handle<> m_handle;
};

template<typename T, typename Func1>
inline auto kdCoroSignal(typename QtPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal)
    -> KDCoroExpected<T> {

    KDCoroExpected<T> awaiter(sender);

    auto cb = awaiter.callbackExpected;
    awaiter.m_lambdaConn = QObject::connect(sender, std::forward<Func1>(signal), [cb](const T &args) mutable {
        if (cb) {
            cb(std::move(args));
        }
    });

    return awaiter;
}

template<typename T, typename Func1>
inline auto kdCoroSignal(typename QtPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal, QObject *receiver)
    -> KDCoroExpected<T> {

    KDCoroExpected<T> awaiter(sender);

    auto cb = awaiter.callbackExpected;
    awaiter.m_lambdaConn = QObject::connect(sender, std::forward<Func1>(signal), receiver, [cb](const T &args) mutable {
        if (cb) {
            cb(std::move(args));
        }
    });

    return awaiter;
}

class KDCoroTerminator
{
public:
    struct promise_type {
        std::coroutine_handle<promise_type> handle;
        std::vector<QMetaObject::Connection> connections;

        void clean()
        {
            for (auto &conn : connections) {
                QObject::disconnect(conn);
            }
            connections.clear();
        }

        void return_void() noexcept {}

        KDCoroTerminator get_return_object()
        {
            handle = std::coroutine_handle<promise_type>::from_promise(*this);
            return {};
        }

        std::suspend_never initial_suspend() const noexcept { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void unhandled_exception() {}

        bool await_ready() const noexcept { return false; }

        std::suspend_never yield_value(QObject *obj)
        {
            auto conn = QObject::connect(obj, &QObject::destroyed, [this] {
                clean();
                if (handle) {
                    handle.destroy();
                }
            });
            connections.emplace_back(std::move(conn));
            return {};
        }

        void await_suspend(std::coroutine_handle<> h) noexcept {}
        void await_resume() const noexcept {}

        ~promise_type() { clean(); }
    };
};

// A wrapper lambda is introduced to extend the lifetime of lhs and rhs in
// case they are temporary objects.
// We also use IILE to prevent potential name clashes and shadowing of variables
// from user code. A drawback of the approach is that it looks ugly :(
#define KDCOROCOMPARE_OP_IMPL(lhs, rhs, op, opId) \
do { \
        if (![](auto &&qt_lhs_arg, auto &&qt_rhs_arg) { \
                    /* assumes that op does not actually move from qt_{lhs, rhs}_arg */ \
                    return QTest::reportResult(std::forward<decltype(qt_lhs_arg)>(qt_lhs_arg) \
                                               op \
                                               std::forward<decltype(qt_rhs_arg)>(qt_rhs_arg), \
                                               [&qt_lhs_arg] { return QTest::toString(qt_lhs_arg); }, \
                                               [&qt_rhs_arg] { return QTest::toString(qt_rhs_arg); }, \
#lhs, #rhs, QTest::ComparisonOperation::opId, \
                                               __FILE__, __LINE__); \
            }(lhs, rhs)) { \
            co_return; \
    } \
} while (false)

#define KDCOROCOMPARE_EQ(computed, baseline) KDCOROCOMPARE_OP_IMPL(computed, baseline, ==, Equal)
#define KDCOROCOMPARE_NE(computed, baseline) KDCOROCOMPARE_OP_IMPL(computed, baseline, !=, NotEqual)
#define KDCOROCOMPARE_LT(computed, baseline) KDCOROCOMPARE_OP_IMPL(computed, baseline, <, LessThan)
#define KDCOROCOMPARE_LE(computed, baseline) KDCOROCOMPARE_OP_IMPL(computed, baseline, <=, LessThanOrEqual)
#define KDCOROCOMPARE_GT(computed, baseline) KDCOROCOMPARE_OP_IMPL(computed, baseline, >, GreaterThan)
#define KDCOROCOMPARE_GE(computed, baseline) KDCOROCOMPARE_OP_IMPL(computed, baseline, >=, GreaterThanOrEqual)

} // namespace KDToolBox
