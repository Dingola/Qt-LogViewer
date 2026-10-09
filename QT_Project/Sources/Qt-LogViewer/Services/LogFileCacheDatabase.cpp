/**
 * @file LogFileCacheDatabase.cpp
 * @brief Implements the normalized per-file cache schema.
 */

#include "Qt-LogViewer/Services/LogFileCacheDatabase.h"

#include <QCborValue>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>
#include <memory>
#include <utility>

#include "Qt-LogViewer/Models/LogFieldDefinition.h"
#include "QtCommonLib/Sql/SqlTransaction.h"
#include "QtCommonLib/Sql/SqliteConnection.h"

using QtCommonLib::SqliteConnection;
using QtCommonLib::SqliteConnectionOptions;
using QtCommonLib::SqlTransaction;

/**
 * @brief Opens or creates a disposable cache database for one exact source
 * generation.
 * @param database_path Destination SQLite path.
 * @param identity Exact source and parser identity represented by the database.
 * @param parent Optional QObject parent.
 */
LogFileCacheDatabase::LogFileCacheDatabase(QString database_path, LogCacheIdentity identity,
                                           QObject* parent)
    : QObject(parent),
      m_database_path(QFileInfo(std::move(database_path)).absoluteFilePath()),
      m_identity(std::move(identity))
{
    m_is_available = initialize_database();
}

/** Rolls back unfinished writes and closes the private Qt SQL connection. */
LogFileCacheDatabase::~LogFileCacheDatabase() = default;

/**
 * @brief Reports whether schema initialization and identity validation
 * succeeded.
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
        QSqlQuery query(m_connection->database());
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
 * @brief Copies a finalized generation and retargets its identity for an appended source.
 * @param source_database_path Finalized prefix database to copy.
 * @param destination_database_path Building generation database to create.
 * @param destination_identity Identity of the current appended source snapshot.
 * @return True when the database was copied and its identity updated atomically.
 */
auto LogFileCacheDatabase::clone_generation(const QString& source_database_path,
                                            const QString& destination_database_path,
                                            const LogCacheIdentity& destination_identity) -> bool
{
    const QString source_path = QFileInfo(source_database_path).absoluteFilePath();
    const QString destination_path = QFileInfo(destination_database_path).absoluteFilePath();
    bool cloned = destination_identity.is_valid() && QFileInfo::exists(source_path) &&
                  source_path != destination_path &&
                  QDir().mkpath(QFileInfo(destination_path).absolutePath());

    if (cloned)
    {
        QFile::remove(destination_path);
        QFile::remove(destination_path + QStringLiteral("-wal"));
        QFile::remove(destination_path + QStringLiteral("-shm"));
        cloned = QFile::copy(source_path, destination_path);
    }

    if (cloned)
    {
        SqliteConnectionOptions options;
        options.connection_name_prefix = QStringLiteral("qt_log_viewer_cache_clone");
        SqliteConnection connection(destination_path, options);
        cloned = connection.is_open();
        if (cloned)
        {
            QSqlDatabase database = connection.database();
            SqlTransaction transaction(database);
            cloned = transaction.is_active();
            if (cloned)
            {
                QSqlQuery query(database);
                query.prepare(QStringLiteral(
                    "UPDATE cache_identity SET cache_key=?, canonical_path=?, file_size=?, "
                    "modified_utc_ms=?, sample_sha256=?, parser_sha256=? WHERE singleton=1"));
                query.addBindValue(destination_identity.cache_key);
                query.addBindValue(destination_identity.canonical_file_path);
                query.addBindValue(destination_identity.file_size);
                query.addBindValue(destination_identity.modified_utc_ms);
                query.addBindValue(destination_identity.sample_sha256);
                query.addBindValue(destination_identity.parser_sha256);
                cloned = query.exec() && query.numRowsAffected() == 1 && transaction.commit();
            }
        }
    }

    if (!cloned)
    {
        QFile::remove(destination_path);
    }
    return cloned;
}

/**
 * @brief Removes all indexed entries before a generation is rebuilt.
 * @return True when the old contents were cleared and a generation-wide
 * transaction started.
 */
auto LogFileCacheDatabase::reset_entries() -> bool
{
    bool reset = m_is_available;
    if (reset)
    {
        QSqlDatabase database = m_connection->database();
        SqlTransaction reset_transaction(database);
        reset = reset_transaction.is_active();
        QSqlQuery query(database);
        const QStringList statements{QStringLiteral("DELETE FROM log_entries"),
                                     QStringLiteral("DELETE FROM log_levels"),
                                     QStringLiteral("DELETE FROM applications"),
                                     QStringLiteral("INSERT INTO log_entries_fts(log_entries_fts) "
                                                    "VALUES('delete-all')")};

        for (qsizetype index = 0; index < statements.size() && reset; ++index)
        {
            reset = query.exec(statements.at(index));
        }
        reset = reset && reset_transaction.commit();
        if (reset)
        {
            m_write_transaction = std::make_unique<SqlTransaction>(database);
            reset = m_write_transaction->is_active();
            if (!reset)
            {
                m_write_transaction.reset();
            }
        }
        m_level_ids.clear();
        m_application_ids.clear();
    }
    return reset;
}

/**
 * @brief Starts a generation-wide transaction without clearing cloned entries.
 * @return True when the database is ready to append suffix entries.
 */
auto LogFileCacheDatabase::begin_append() -> bool
{
    bool started = m_is_available && m_write_transaction == nullptr;
    if (started)
    {
        m_write_transaction = std::make_unique<SqlTransaction>(m_connection->database());
        started = m_write_transaction->is_active();
        if (!started)
        {
            m_write_transaction.reset();
        }
    }
    return started;
}

/**
 * @brief Appends one parsed batch to the normalized index and contentless FTS
 * table.
 * @param entries Parsed entries carrying exact source byte ranges.
 * @return True when the complete batch was staged in the generation-wide
 * transaction.
 */
auto LogFileCacheDatabase::append_entries(const QVector<LogEntry>& entries) -> bool
{
    const bool can_store = m_is_available && m_write_transaction != nullptr &&
                           m_write_transaction->is_active() && !entries.isEmpty();
    bool stored = can_store;
    if (can_store)
    {
        QSqlDatabase database = m_connection->database();
        QSqlQuery entry_query(database);
        entry_query.prepare(
            QStringLiteral("INSERT INTO log_entries(source_line, byte_offset, byte_length, "
                           "timestamp_utc_ms, "
                           "level_id, app_id, parsed_fields_cbor) VALUES(?, ?, ?, ?, ?, ?, ?)"));
        QSqlQuery search_query(database);
        search_query.prepare(
            QStringLiteral("INSERT INTO log_entries_fts(rowid, message, level, "
                           "app_name) VALUES(?, ?, ?, ?)"));

        for (qsizetype index = 0; index < entries.size() && stored; ++index)
        {
            const LogEntry& entry = entries.at(index);
            const qint64 source_line = static_cast<qint64>(entry.get_source_line());
            const qint64 byte_offset = entry.get_byte_offset();
            const qint64 byte_length = entry.get_byte_length();
            const qint64 level_id = get_or_create_level_id(entry.get_level());
            const qint64 application_id = get_or_create_application_id(entry.get_app_name());
            stored = source_line >= 1 && byte_offset >= 0 && byte_length >= 0 && level_id >= 0 &&
                     application_id >= 0;

            QVariantMap custom_fields;
            if (stored)
            {
                const LogEntry::ParsedFields& parsed_fields = entry.get_parsed_fields();
                for (auto iterator = parsed_fields.cbegin(); iterator != parsed_fields.cend();
                     ++iterator)
                {
                    if (iterator.key() != LogField::Timestamp &&
                        iterator.key() != LogField::Level && iterator.key() != LogField::Message &&
                        iterator.key() != LogField::AppName)
                    {
                        custom_fields.insert(iterator.key(), iterator.value());
                    }
                }
            }

            if (stored)
            {
                const QByteArray parsed_fields_cbor =
                    QCborValue::fromVariant(custom_fields).toCbor();
                entry_query.addBindValue(source_line);
                entry_query.addBindValue(byte_offset);
                entry_query.addBindValue(byte_length);
                if (entry.get_timestamp().isValid())
                {
                    entry_query.addBindValue(entry.get_timestamp().toUTC().toMSecsSinceEpoch());
                }
                else
                {
                    entry_query.addBindValue(QVariant());
                }
                entry_query.addBindValue(level_id);
                entry_query.addBindValue(application_id);
                entry_query.addBindValue(parsed_fields_cbor);
                stored = entry_query.exec();
            }

            if (stored)
            {
                search_query.addBindValue(entry_query.lastInsertId());
                search_query.addBindValue(entry.get_message());
                search_query.addBindValue(entry.get_level());
                search_query.addBindValue(entry.get_app_name());
                stored = search_query.exec();
            }
        }

        if (!stored)
        {
            static_cast<void>(m_write_transaction->rollback());
            m_write_transaction.reset();
            m_level_ids.clear();
            m_application_ids.clear();
        }
    }
    return stored;
}

/**
 * @brief Flushes the completed index and truncates its WAL file.
 * @return True when optimization and the final checkpoint completed
 * successfully.
 */
auto LogFileCacheDatabase::finalize_writes() -> bool
{
    bool finalized =
        m_is_available && m_write_transaction != nullptr && m_write_transaction->is_active();
    if (finalized)
    {
        QSqlDatabase database = m_connection->database();
        finalized = m_write_transaction->commit();
        if (!finalized)
        {
            static_cast<void>(m_write_transaction->rollback());
        }
        m_write_transaction.reset();
        if (finalized)
        {
            QSqlQuery query(database);
            finalized = query.exec(QStringLiteral("PRAGMA optimize")) &&
                        query.exec(QStringLiteral("PRAGMA wal_checkpoint(TRUNCATE)"));
        }
    }
    return finalized;
}

/**
 * @brief Counts indexed records.
 * @return Number of rows in `log_entries`, or -1 when the query fails.
 */
auto LogFileCacheDatabase::get_entry_count() const -> qint64
{
    qint64 entry_count = -1;
    if (m_is_available)
    {
        QSqlQuery query(m_connection->database());
        if (query.exec(QStringLiteral("SELECT COUNT(*) FROM log_entries")) && query.next())
        {
            entry_count = query.value(0).toLongLong();
        }
    }
    return entry_count;
}

/**
 * @brief Measures the database and its SQLite sidecar files.
 * @return Combined size of the database, WAL and shared-memory files in bytes.
 */
auto LogFileCacheDatabase::get_storage_bytes() const -> qint64
{
    qint64 storage_bytes = 0;
    const QStringList paths{m_database_path, m_database_path + QStringLiteral("-wal"),
                            m_database_path + QStringLiteral("-shm")};
    for (const QString& path: paths)
    {
        const QFileInfo file_info(path);
        if (file_info.exists())
        {
            storage_bytes += file_info.size();
        }
    }
    return storage_bytes;
}

/**
 * @brief Opens SQLite, applies runtime pragmas and validates schema plus source
 * identity.
 * @return True when the schema exists and its stored identity matches the
 * expected one.
 */
auto LogFileCacheDatabase::initialize_database() -> bool
{
    const QFileInfo database_info(m_database_path);
    bool initialized = m_identity.is_valid() && !m_database_path.isEmpty() &&
                       QDir().mkpath(database_info.absolutePath());
    if (initialized)
    {
        SqliteConnectionOptions options;
        options.connection_name_prefix = QStringLiteral("qt_log_viewer_file_cache");
        options.connect_options = QStringLiteral("QSQLITE_BUSY_TIMEOUT=5000");
        options.connection_setup_statements = {
            QStringLiteral("PRAGMA foreign_keys=ON"), QStringLiteral("PRAGMA busy_timeout=5000"),
            QStringLiteral("PRAGMA journal_mode=WAL"), QStringLiteral("PRAGMA synchronous=NORMAL")};
        m_connection = std::make_unique<SqliteConnection>(m_database_path, options);
        initialized = m_connection->is_open();

        if (initialized)
        {
            QSqlDatabase database = m_connection->database();
            QSqlQuery query(database);
            initialized = query.exec(QStringLiteral("PRAGMA user_version")) && query.next();
            if (initialized)
            {
                const int version = query.value(0).toInt();
                query.finish();
                initialized = version == SchemaVersion ? create_schema() && identity_matches()
                                                       : rebuild_schema();
            }
        }
    }
    return initialized;
}

/**
 * @brief Replaces incompatible tables and records the identity of the new empty
 * cache.
 * @return True when the replacement schema and identity were committed
 * atomically.
 */
auto LogFileCacheDatabase::rebuild_schema() -> bool
{
    QSqlDatabase database = m_connection->database();
    SqlTransaction transaction(database);
    bool rebuilt = transaction.is_active();
    if (rebuilt)
    {
        QSqlQuery query(database);
        const QStringList drops{QStringLiteral("DROP TABLE IF EXISTS log_entries_fts"),
                                QStringLiteral("DROP TABLE IF EXISTS log_entries"),
                                QStringLiteral("DROP TABLE IF EXISTS log_levels"),
                                QStringLiteral("DROP TABLE IF EXISTS applications"),
                                QStringLiteral("DROP TABLE IF EXISTS cache_identity")};
        for (qsizetype index = 0; index < drops.size() && rebuilt; ++index)
        {
            rebuilt = query.exec(drops.at(index));
        }
        rebuilt = rebuilt && create_schema();
        rebuilt = rebuilt && store_identity();
        rebuilt = rebuilt && query.exec(QStringLiteral("PRAGMA user_version=2"));

        query.finish();
        rebuilt = rebuilt && transaction.commit();
    }
    return rebuilt;
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
    QSqlQuery query(m_connection->database());
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
    QSqlQuery query(m_connection->database());
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
 * @brief Compares every persisted identity component with the expected
 * identity.
 * @return True only when all components match exactly.
 */
auto LogFileCacheDatabase::identity_matches() const -> bool
{
    QSqlQuery query(m_connection->database());
    bool matches =
        query.exec(QStringLiteral("SELECT cache_key, canonical_path, file_size, modified_utc_ms, "
                                  "sample_sha256, "
                                  "parser_sha256 FROM cache_identity WHERE singleton=1")) &&
        query.next();
    if (matches)
    {
        matches = query.value(0).toString() == m_identity.cache_key &&
                  query.value(1).toString() == m_identity.canonical_file_path &&
                  query.value(2).toLongLong() == m_identity.file_size &&
                  query.value(3).toLongLong() == m_identity.modified_utc_ms &&
                  query.value(4).toByteArray() == m_identity.sample_sha256 &&
                  query.value(5).toByteArray() == m_identity.parser_sha256;
    }
    return matches;
}

/**
 * @brief Returns or creates a normalized log-level identifier.
 * @param value Original log-level text.
 * @return Positive row identifier, or -1 when lookup or insertion fails.
 */
auto LogFileCacheDatabase::get_or_create_level_id(const QString& value) -> qint64
{
    const QString stored_value = value.isNull() ? QStringLiteral("") : value;
    const QString normalized_value = stored_value.trimmed().toCaseFolded();
    const auto cached = m_level_ids.constFind(normalized_value);
    qint64 id = -1;
    if (cached != m_level_ids.cend())
    {
        id = cached.value();
    }
    else
    {
        QSqlDatabase database = m_connection->database();
        QSqlQuery insert_query(database);
        insert_query.prepare(
            QStringLiteral("INSERT OR IGNORE INTO log_levels(value, "
                           "normalized_value) VALUES(?, ?)"));
        insert_query.addBindValue(stored_value);
        insert_query.addBindValue(normalized_value);
        const bool inserted = insert_query.exec();

        QSqlQuery select_query(database);
        select_query.prepare(QStringLiteral("SELECT id FROM log_levels WHERE normalized_value=?"));
        select_query.addBindValue(normalized_value);
        if (inserted && select_query.exec() && select_query.next())
        {
            id = select_query.value(0).toLongLong();
            m_level_ids.insert(normalized_value, id);
        }
    }
    return id;
}

/**
 * @brief Returns or creates an application identifier.
 * @param value Original application name.
 * @return Positive row identifier, or -1 when lookup or insertion fails.
 */
auto LogFileCacheDatabase::get_or_create_application_id(const QString& value) -> qint64
{
    const QString stored_value = value.isNull() ? QStringLiteral("") : value;
    const auto cached = m_application_ids.constFind(stored_value);
    qint64 id = -1;
    if (cached != m_application_ids.cend())
    {
        id = cached.value();
    }
    else
    {
        QSqlDatabase database = m_connection->database();
        QSqlQuery insert_query(database);
        insert_query.prepare(QStringLiteral("INSERT OR IGNORE INTO applications(value) VALUES(?)"));
        insert_query.addBindValue(stored_value);
        const bool inserted = insert_query.exec();

        QSqlQuery select_query(database);
        select_query.prepare(QStringLiteral("SELECT id FROM applications WHERE value=?"));
        select_query.addBindValue(stored_value);
        if (inserted && select_query.exec() && select_query.next())
        {
            id = select_query.value(0).toLongLong();
            m_application_ids.insert(stored_value, id);
        }
    }
    return id;
}
