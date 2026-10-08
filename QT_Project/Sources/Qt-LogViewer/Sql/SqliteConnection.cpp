/**
 * @file SqliteConnection.cpp
 * @brief Implements deterministic ownership of a uniquely named SQLite connection.
 */

#include "Qt-LogViewer/Sql/SqliteConnection.h"

#include <QSqlQuery>
#include <QUuid>
#include <utility>

namespace QtCommonLib
{

/**
 * @brief Opens a uniquely named SQLite connection and applies caller-provided initialization SQL.
 * @param database_name SQLite database name such as a file path or `:memory:`.
 * @param options Optional connection and initialization settings.
 */
SqliteConnection::SqliteConnection(QString database_name, const SqliteConnectionOptions& options)
    : m_connection_name(create_connection_name(options.connection_name_prefix))
{
    QSqlDatabase sql_database =
        QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connection_name);
    sql_database.setDatabaseName(std::move(database_name));
    sql_database.setConnectOptions(options.connect_options);

    bool initialized = sql_database.open();
    if (!initialized)
    {
        m_last_error = sql_database.lastError();
    }

    for (qsizetype index = 0; index < options.connection_setup_statements.size() && initialized;
         ++index)
    {
        QSqlQuery query(sql_database);
        initialized = query.exec(options.connection_setup_statements.at(index));
        if (!initialized)
        {
            m_last_error = query.lastError();
        }
    }

    if (!initialized)
    {
        sql_database.close();
    }
}

/** @brief Closes and unregisters the owned Qt SQL connection. */
SqliteConnection::~SqliteConnection()
{
    if (QSqlDatabase::contains(m_connection_name))
    {
        {
            QSqlDatabase sql_database = QSqlDatabase::database(m_connection_name, false);
            sql_database.close();
        }
        QSqlDatabase::removeDatabase(m_connection_name);
    }
}

/**
 * @brief Reports whether the owned database is currently open.
 * @return True when the registered SQLite connection is open.
 */
auto SqliteConnection::is_open() const -> bool
{
    bool open = false;
    if (QSqlDatabase::contains(m_connection_name))
    {
        const QSqlDatabase sql_database = QSqlDatabase::database(m_connection_name, false);
        open = sql_database.isOpen();
    }
    return open;
}

/**
 * @brief Returns a handle to the owned Qt SQL connection.
 * @return Registered database handle, or an invalid handle when registration was lost.
 */
auto SqliteConnection::database() const -> QSqlDatabase
{
    QSqlDatabase sql_database;
    if (QSqlDatabase::contains(m_connection_name))
    {
        sql_database = QSqlDatabase::database(m_connection_name, false);
    }
    return sql_database;
}

/**
 * @brief Returns the generated Qt SQL connection name.
 * @return Unique name used in the Qt SQL connection registry.
 */
auto SqliteConnection::connection_name() const -> QString
{
    const QString connection_name = m_connection_name;
    return connection_name;
}

/**
 * @brief Returns the error produced while opening or initializing the connection.
 * @return Initialization error, or a no-error value after successful construction.
 */
auto SqliteConnection::last_error() const -> QSqlError
{
    const QSqlError error = m_last_error;
    return error;
}

/**
 * @brief Generates a unique Qt SQL connection name from a caller-provided prefix.
 * @param prefix Requested descriptive connection-name prefix.
 * @return Unique connection name with a non-empty prefix.
 */
auto SqliteConnection::create_connection_name(const QString& prefix) -> QString
{
    QString effective_prefix = prefix.trimmed();
    if (effective_prefix.isEmpty())
    {
        effective_prefix = QStringLiteral("sqlite");
    }
    const QString connection_name = QStringLiteral("%1_%2").arg(
        effective_prefix, QUuid::createUuid().toString(QUuid::WithoutBraces));
    return connection_name;
}

}  // namespace QtCommonLib
