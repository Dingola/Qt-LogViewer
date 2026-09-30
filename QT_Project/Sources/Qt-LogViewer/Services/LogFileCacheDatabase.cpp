/**
 * @file LogFileCacheDatabase.cpp
 * @brief Implements the normalized per-file cache schema.
 */

#include "Qt-LogViewer/Services/LogFileCacheDatabase.h"

#include <QDir>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>
#include <QUuid>
#include <QVariant>
#include <utility>

/**
 * @brief Opens or creates a disposable cache database for one exact source generation.
 * @param database_path Destination SQLite path.
 * @param identity Exact source and parser identity represented by the database.
 * @param parent Optional QObject parent.
 */
LogFileCacheDatabase::LogFileCacheDatabase(QString database_path, LogCacheIdentity identity,
                                           QObject* parent)
    : QObject(parent),
      m_connection_name(QStringLiteral("qt_log_viewer_file_cache_%1")
                            .arg(QUuid::createUuid().toString(QUuid::WithoutBraces))),
      m_database_path(QFileInfo(std::move(database_path)).absoluteFilePath()),
      m_identity(std::move(identity))
{
    m_is_available = initialize_database();
}

/** Closes and unregisters the private Qt SQL connection. */
LogFileCacheDatabase::~LogFileCacheDatabase()
{
    if (QSqlDatabase::contains(m_connection_name))
    {
        {
            QSqlDatabase database = QSqlDatabase::database(m_connection_name);
            database.close();
        }
        QSqlDatabase::removeDatabase(m_connection_name);
    }
}

/**
 * @brief Reports whether schema initialization and identity validation succeeded.
 * @return True when the database is usable for the requested cache identity.
 */
auto LogFileCacheDatabase::is_available() const -> bool
{
    return m_is_available;
}

/**
 * @brief Returns the per-file SQLite database path.
 * @return Absolute database path supplied to the constructor.
 */
auto LogFileCacheDatabase::get_database_path() const -> QString
{
    return m_database_path;
}

/**
 * @brief Reads the current SQLite user schema version.
 * @return `PRAGMA user_version`, or zero when the database is unavailable.
 */
auto LogFileCacheDatabase::get_schema_version() const -> int
{
    int version = 0;
    if (m_is_available)
    {
        QSqlQuery query(QSqlDatabase::database(m_connection_name));
        if (query.exec(QStringLiteral("PRAGMA user_version")) && query.next())
        {
            version = query.value(0).toInt();
        }
    }
    return version;
}

/**
 * @brief Returns the exact identity expected from this database.
 * @return Immutable source and parser identity owned by this instance.
 */
auto LogFileCacheDatabase::get_identity() const -> const LogCacheIdentity&
{
    return m_identity;
}

/**
 * @brief Opens SQLite, applies runtime pragmas and validates schema plus source identity.
 * @return True when the schema exists and its stored identity matches the expected one.
 */
auto LogFileCacheDatabase::initialize_database() -> bool
{
    const QFileInfo database_info(m_database_path);
    if (!m_identity.is_valid() || m_database_path.isEmpty() ||
        !QDir().mkpath(database_info.absolutePath()))
    {
        return false;
    }

    QSqlDatabase database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connection_name);
    database.setDatabaseName(m_database_path);
    database.setConnectOptions(QStringLiteral("QSQLITE_BUSY_TIMEOUT=5000"));
    if (!database.open())
    {
        return false;
    }

    QSqlQuery query(database);
    const bool configured = query.exec(QStringLiteral("PRAGMA foreign_keys=ON")) &&
                            query.exec(QStringLiteral("PRAGMA busy_timeout=5000")) &&
                            query.exec(QStringLiteral("PRAGMA journal_mode=WAL")) &&
                            query.exec(QStringLiteral("PRAGMA synchronous=NORMAL"));
    if (!configured || !query.exec(QStringLiteral("PRAGMA user_version")) || !query.next())
    {
        return false;
    }

    const int version = query.value(0).toInt();
    query.finish();
    if (version == SchemaVersion)
    {
        // A schema match alone is insufficient: the database must describe the
        // exact source and parser revision requested by the caller.
        return create_schema() && identity_matches();
    }
    return rebuild_schema();
}

/**
 * @brief Replaces incompatible tables and records the identity of the new empty cache.
 * @return True when the replacement schema and identity were committed atomically.
 */
auto LogFileCacheDatabase::rebuild_schema() -> bool
{
    QSqlDatabase database = QSqlDatabase::database(m_connection_name);
    if (!database.transaction())
    {
        return false;
    }

    QSqlQuery query(database);
    const QStringList drops{QStringLiteral("DROP TABLE IF EXISTS log_entries_fts"),
                            QStringLiteral("DROP TABLE IF EXISTS log_entries"),
                            QStringLiteral("DROP TABLE IF EXISTS log_levels"),
                            QStringLiteral("DROP TABLE IF EXISTS applications"),
                            QStringLiteral("DROP TABLE IF EXISTS cache_identity")};
    bool rebuilt = true;
    for (const QString& statement: drops)
    {
        rebuilt = rebuilt && query.exec(statement);
    }
    rebuilt = rebuilt && create_schema();
    rebuilt = rebuilt && store_identity();
    rebuilt = rebuilt && query.exec(QStringLiteral("PRAGMA user_version=2"));

    query.finish();
    if (rebuilt && database.commit())
    {
        return true;
    }
    database.rollback();
    return false;
}

/**
 * Creates the normalized entry schema and its contentless full-text search
 * companion.
 *
 * Raw records are intentionally not duplicated in `log_entries`; byte ranges
 * point back to the immutable source file. The FTS table is contentless so
 * indexed text is not repeated in a second ordinary content table.
 *
 * @return True when every schema statement completed successfully.
 */
auto LogFileCacheDatabase::create_schema() -> bool
{
    QSqlQuery query(QSqlDatabase::database(m_connection_name));
    const QStringList statements{
        QStringLiteral("CREATE TABLE IF NOT EXISTS cache_identity("
                       "singleton INTEGER PRIMARY KEY CHECK(singleton=1), "
                       "cache_key TEXT NOT NULL, "
                       "canonical_path TEXT NOT NULL, file_size INTEGER NOT "
                       "NULL, modified_utc_ms "
                       "INTEGER NOT NULL, sample_sha256 BLOB NOT NULL, "
                       "parser_sha256 BLOB NOT NULL)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS log_levels("
                       "id INTEGER PRIMARY KEY, value TEXT NOT NULL, "
                       "normalized_value TEXT NOT NULL "
                       "UNIQUE)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS applications("
                       "id INTEGER PRIMARY KEY, value TEXT NOT NULL UNIQUE)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS log_entries("
                       "id INTEGER PRIMARY KEY, source_line INTEGER NOT NULL, byte_offset "
                       "INTEGER "
                       "NOT NULL, byte_length INTEGER NOT NULL, timestamp_utc_ms INTEGER, "
                       "level_id "
                       "INTEGER REFERENCES log_levels(id), app_id INTEGER REFERENCES "
                       "applications(id), parsed_fields_cbor BLOB NOT NULL DEFAULT X'A0', "
                       "CHECK(byte_offset>=0), CHECK(byte_length>=0))"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_file_entries_timestamp ON "
                       "log_entries(timestamp_utc_ms, id)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_file_entries_level ON "
                       "log_entries(level_id, id)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_file_entries_app ON "
                       "log_entries(app_id, id)"),
        QStringLiteral("CREATE VIRTUAL TABLE IF NOT EXISTS log_entries_fts USING "
                       "fts5(message, level, app_name, content='')")};

    bool created = true;
    for (const QString& statement: statements)
    {
        created = created && query.exec(statement);
    }
    return created;
}

/**
 * @brief Stores all cache identity components in the singleton metadata row.
 * @return True when the identity row was written successfully.
 */
auto LogFileCacheDatabase::store_identity() -> bool
{
    QSqlQuery query(QSqlDatabase::database(m_connection_name));
    query.prepare(
        QStringLiteral("INSERT OR REPLACE INTO cache_identity(singleton, "
                       "cache_key, canonical_path, file_size, "
                       "modified_utc_ms, sample_sha256, parser_sha256) VALUES(1, "
                       "?, ?, ?, ?, ?, ?)"));
    query.addBindValue(m_identity.cache_key);
    query.addBindValue(m_identity.canonical_file_path);
    query.addBindValue(m_identity.file_size);
    query.addBindValue(m_identity.modified_utc_ms);
    query.addBindValue(m_identity.sample_sha256);
    query.addBindValue(m_identity.parser_sha256);
    return query.exec();
}

/**
 * @brief Compares every persisted identity component with the expected identity.
 * @return True only when all components match exactly.
 */
auto LogFileCacheDatabase::identity_matches() const -> bool
{
    QSqlQuery query(QSqlDatabase::database(m_connection_name));
    if (!query.exec(QStringLiteral("SELECT cache_key, canonical_path, file_size, modified_utc_ms, "
                                   "sample_sha256, "
                                   "parser_sha256 FROM cache_identity WHERE singleton=1")) ||
        !query.next())
    {
        return false;
    }

    return query.value(0).toString() == m_identity.cache_key &&
           query.value(1).toString() == m_identity.canonical_file_path &&
           query.value(2).toLongLong() == m_identity.file_size &&
           query.value(3).toLongLong() == m_identity.modified_utc_ms &&
           query.value(4).toByteArray() == m_identity.sample_sha256 &&
           query.value(5).toByteArray() == m_identity.parser_sha256;
}
