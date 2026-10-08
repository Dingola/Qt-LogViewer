/**
 * @file SqlTransaction.cpp
 * @brief Implements scoped Qt SQL transaction ownership.
 */

#include "Qt-LogViewer/Sql/SqlTransaction.h"

#include <utility>

namespace QtCommonLib
{

/**
 * @brief Starts a transaction on an existing database connection.
 * @param database Open database handle without an already active transaction.
 */
SqlTransaction::SqlTransaction(QSqlDatabase database): m_database(std::move(database))
{
    m_is_active = m_database.isValid() && m_database.transaction();
    if (!m_is_active)
    {
        m_last_error = m_database.lastError();
    }
}

/** @brief Rolls back the transaction when it is still active. */
SqlTransaction::~SqlTransaction()
{
    if (m_is_active)
    {
        static_cast<void>(rollback());
    }
}

/**
 * @brief Reports whether this owner still manages an active transaction.
 * @return True after a successful begin and before successful completion.
 */
auto SqlTransaction::is_active() const -> bool
{
    const bool active = m_is_active;
    return active;
}

/**
 * @brief Commits the active transaction.
 * @return True when the transaction was active and committed successfully.
 */
auto SqlTransaction::commit() -> bool
{
    bool committed = false;
    if (m_is_active)
    {
        committed = m_database.commit();
        if (committed)
        {
            m_is_active = false;
        }
        else
        {
            m_last_error = m_database.lastError();
        }
    }
    return committed;
}

/**
 * @brief Rolls back the active transaction immediately.
 * @return True when the transaction was active and rolled back successfully.
 */
auto SqlTransaction::rollback() -> bool
{
    bool rolled_back = false;
    if (m_is_active)
    {
        rolled_back = m_database.rollback();
        if (rolled_back)
        {
            m_is_active = false;
        }
        else
        {
            m_last_error = m_database.lastError();
        }
    }
    return rolled_back;
}

/**
 * @brief Returns the most recent begin, commit, or rollback error.
 * @return Transaction error, or a no-error value when all operations succeeded.
 */
auto SqlTransaction::last_error() const -> QSqlError
{
    const QSqlError error = m_last_error;
    return error;
}

}  // namespace QtCommonLib
