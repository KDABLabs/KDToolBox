/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#ifndef KDTOOLBOX_KDSQLDATABASETRANSACTION_H
#define KDTOOLBOX_KDSQLDATABASETRANSACTION_H

#include <QLoggingCategory>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QtDebug>

namespace KDToolBox
{

namespace Private
{
// `inline` logging category; Qt doesn't support them.
inline const QLoggingCategory &kdsqllc()
{
    static const QLoggingCategory category("KDToolBox.KDSqlDatabaseTransaction", QtWarningMsg);
    return category;
}
} // namespace Private

class Q_REQUIRED_RESULT KDSqlDatabaseTransaction
{
    Q_DISABLE_COPY(KDSqlDatabaseTransaction)

public:
    /*!
     * \brief Begins a new transaction. The transaction will be
     * committed when commit() is called (and returns true), otherwise
     * it will be automatically rolled back.
     *
     * \param database is the database connection to use. The connection
     * must be opened before use.
     */
    explicit KDSqlDatabaseTransaction(const QSqlDatabase &database = QSqlDatabase::database())
        : m_db(database)
        , m_shouldRollback(true)
    {
        if (!m_db.isOpen())
        {
            qCWarning(Private::kdsqllc) << "The database" << m_db << "is not open";
            m_shouldRollback = false;
        }
        else if (!m_db.driver()->hasFeature(QSqlDriver::Transactions))
        {
            qCWarning(Private::kdsqllc) << "The database" << m_db << "does not support transactions";
            m_shouldRollback = false;
        }
        else if (!m_db.transaction())
        {
            qCWarning(Private::kdsqllc) << "Could not begin a new transaction";
            m_shouldRollback = false;
        }
    }

    /*!
     * \brief Destroys the transaction object. If it has not been committed
     * by calling commit(), the transaction will be rolled back.
     */
    ~KDSqlDatabaseTransaction()
    {
        if (m_shouldRollback)
        {
            m_db.rollback();
        }
    }

    KDSqlDatabaseTransaction(KDSqlDatabaseTransaction &&other) noexcept
        : m_db(std::move(other.m_db))
        , m_shouldRollback(std::exchange(other.m_shouldRollback, false))
    {
    }

    KDSqlDatabaseTransaction &operator=(KDSqlDatabaseTransaction &&other) noexcept
    {
        KDSqlDatabaseTransaction moved(std::move(other));
        swap(moved);
        return *this;
    }

    void swap(KDSqlDatabaseTransaction &other) noexcept
    {
        qSwap(m_db, other.m_db);
        qSwap(m_shouldRollback, other.m_shouldRollback);
    }

    /*!
     * \brief Commits the transaction. If the commit fails, the
     * KDSqlDatabaseTransaction object will roll back the transaction
     * when it gets destroyed.
     *
     * \return True if committing succeeded, false if it failed.
     */
    bool commit()
    {
        if (!m_shouldRollback)
        {
            qCWarning(Private::kdsqllc) << "commit() / rollback() already called on this object";
            return false;
        }
        const bool result = m_db.commit();
        if (result)
        {
            m_shouldRollback = false;
        }
        return result;
    }

    /*!
     * \brief Rolls back the transaction. If the rollback fails, the
     * KDSqlDatabaseTransaction object will NOT roll back the transaction
     * again when it gets destroyed.
     *
     * \return True if rollback succeeded, false if it failed.
     */
    bool rollback()
    {
        if (!m_shouldRollback)
        {
            qCWarning(Private::kdsqllc) << "commit() / rollback() already called on this object";
            return false;
        }
        // in case rollback fails, what can we do? should we try again?
        m_shouldRollback = false;
        return m_db.rollback();
    }

    /*!
     * \brief Returns the database connection used by this object.
     * \return The database connection.
     */
    const QSqlDatabase &database() const { return m_db; }

private:
    QSqlDatabase m_db;
    bool m_shouldRollback;
};

} // namespace KDToolBox

#endif // KDTOOLBOX_KDSQLDATABASETRANSACTION_H
