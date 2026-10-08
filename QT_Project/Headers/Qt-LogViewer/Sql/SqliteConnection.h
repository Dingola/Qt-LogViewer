#pragma once

#include <QSqlDatabase>
#include <QSqlError>
#include <QString>
#include <QStringList>

/**
 * @file SqliteConnection.h
 * @brief Declares an owning SQLite connection with deterministic Qt SQL cleanup.
 */

namespace QtCommonLib
{

/**
 * @struct SqliteConnectionOptions
 * @brief Defines optional Qt SQL and connection-initialization settings.
 */
struct SqliteConnectionOptions {
        /** @brief Prefix used when generating the unique Qt SQL connection name. */
        QString connection_name_prefix{QStringLiteral("sqlite")};

        /** @brief Driver-specific options passed to QSqlDatabase before opening. */
        QString connect_options;

        /** @brief SQL statements executed in order immediately after opening. */
        QStringList initialization_statements;
};

/**
 * @class SqliteConnection
 * @brief Owns one uniquely named Qt SQLite connection for its complete lifetime.
 *
 * The class opens the connection during construction and unregisters it during destruction.
 * Filesystem preparation and application-specific PRAGMA selection remain responsibilities of
 * the caller. The owner and all handles obtained from it must be used and destroyed in the thread
 * in which the owner was constructed.
 */
class SqliteConnection final
{
    public:
        /**
         * @brief Opens a uniquely named SQLite connection.
         * @param database_name SQLite database name such as a file path or `:memory:`.
         * @param options Optional connection and initialization settings.
         */
        explicit SqliteConnection(QString database_name, const SqliteConnectionOptions& options =
                                                             SqliteConnectionOptions());

        /** @brief Closes and unregisters the owned Qt SQL connection. */
        ~SqliteConnection();

        /**
         * @brief Prevents copying ownership of a registered Qt SQL connection.
         * @param other Connection owner that would otherwise be copied.
         */
        SqliteConnection(const SqliteConnection& other) = delete;

        /**
         * @brief Prevents copy assignment of a registered Qt SQL connection.
         * @param other Connection owner that would otherwise be assigned.
         * @return Reference to this connection owner.
         */
        auto operator=(const SqliteConnection& other) -> SqliteConnection& = delete;

        /**
         * @brief Prevents moving ownership of a registered Qt SQL connection.
         * @param other Connection owner that would otherwise be moved.
         */
        SqliteConnection(SqliteConnection&& other) = delete;

        /**
         * @brief Prevents move assignment of a registered Qt SQL connection.
         * @param other Connection owner that would otherwise be move-assigned.
         * @return Reference to this connection owner.
         */
        auto operator=(SqliteConnection&& other) -> SqliteConnection& = delete;

        /**
         * @brief Reports whether the owned database is currently open.
         * @return True when the registered SQLite connection is open.
         */
        [[nodiscard]] auto is_open() const -> bool;

        /**
         * @brief Returns a handle to the owned Qt SQL connection.
         *
         * All copies of the returned handle must be destroyed before this owner is destroyed.
         *
         * @return Registered database handle, or an invalid handle when registration was lost.
         */
        [[nodiscard]] auto database() const -> QSqlDatabase;

        /**
         * @brief Returns the generated Qt SQL connection name.
         * @return Unique name used in the Qt SQL connection registry.
         */
        [[nodiscard]] auto connection_name() const -> QString;

        /**
         * @brief Returns the error produced while opening or initializing the connection.
         * @return Initialization error, or a no-error value after successful construction.
         */
        [[nodiscard]] auto last_error() const -> QSqlError;

    private:
        /**
         * @brief Generates a unique Qt SQL connection name from a caller-provided prefix.
         * @param prefix Requested descriptive connection-name prefix.
         * @return Unique connection name with a non-empty prefix.
         */
        [[nodiscard]] static auto create_connection_name(const QString& prefix) -> QString;

    private:
        /** @brief Unique name of the owned entry in the Qt SQL connection registry. */
        QString m_connection_name;

        /** @brief Error captured while opening the database or running initialization SQL. */
        QSqlError m_last_error;
};

}  // namespace QtCommonLib
