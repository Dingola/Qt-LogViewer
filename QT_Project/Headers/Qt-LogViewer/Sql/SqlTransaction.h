#pragma once

#include <QSqlDatabase>
#include <QSqlError>

/**
 * @file SqlTransaction.h
 * @brief Declares a scoped Qt SQL transaction with automatic rollback.
 */

namespace QtCommonLib
{

/**
 * @class SqlTransaction
 * @brief Owns one transaction started on a caller-provided Qt SQL database handle.
 *
 * An active transaction is rolled back automatically unless `commit()` or `rollback()` completes
 * successfully. The transaction must be used and destroyed in the thread that owns the database
 * connection. Queries may use other handles for the same connection, but callers must not invoke
 * `transaction()`, `commit()`, or `rollback()` directly on that connection while this owner is
 * active.
 */
class SqlTransaction final
{
    public:
        /**
         * @brief Starts a transaction on an existing database connection.
         * @param database Open database handle without an already active transaction.
         */
        explicit SqlTransaction(QSqlDatabase database);

        /** @brief Rolls back the transaction when it is still active. */
        ~SqlTransaction();

        /**
         * @brief Prevents copying transaction ownership.
         * @param other Transaction owner that would otherwise be copied.
         */
        SqlTransaction(const SqlTransaction& other) = delete;

        /**
         * @brief Prevents copy assignment of transaction ownership.
         * @param other Transaction owner that would otherwise be assigned.
         * @return Reference to this transaction owner.
         */
        auto operator=(const SqlTransaction& other) -> SqlTransaction& = delete;

        /**
         * @brief Prevents moving transaction ownership.
         * @param other Transaction owner that would otherwise be moved.
         */
        SqlTransaction(SqlTransaction&& other) = delete;

        /**
         * @brief Prevents move assignment of transaction ownership.
         * @param other Transaction owner that would otherwise be move-assigned.
         * @return Reference to this transaction owner.
         */
        auto operator=(SqlTransaction&& other) -> SqlTransaction& = delete;

        /**
         * @brief Reports whether this owner still manages an active transaction.
         * @return True after a successful begin and before successful completion.
         */
        [[nodiscard]] auto is_active() const -> bool;

        /**
         * @brief Commits the active transaction.
         * @return True when the transaction was active and committed successfully.
         */
        [[nodiscard]] auto commit() -> bool;

        /**
         * @brief Rolls back the active transaction immediately.
         * @return True when the transaction was active and rolled back successfully.
         */
        [[nodiscard]] auto rollback() -> bool;

        /**
         * @brief Returns the most recent begin, commit, or rollback error.
         * @return Transaction error, or a no-error value when all operations succeeded.
         */
        [[nodiscard]] auto last_error() const -> QSqlError;

    private:
        /** @brief Database handle on which the transaction was started. */
        QSqlDatabase m_database;

        /** @brief Error captured from the most recent failed transaction operation. */
        QSqlError m_last_error;

        /** @brief Whether this owner still needs to complete or roll back the transaction. */
        bool m_is_active{false};
};

}  // namespace QtCommonLib
