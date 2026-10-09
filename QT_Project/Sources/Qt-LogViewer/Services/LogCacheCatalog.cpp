/**
 * @file LogCacheCatalog.cpp
 * @brief Implements the versioned persistent cache metadata catalog.
 */

#include "Qt-LogViewer/Services/LogCacheCatalog.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QStringList>
#include <QVariant>
#include <memory>
#include <utility>

#include "QtCommonLib/Sql/SqlConnection.h"
#include "QtCommonLib/Sql/SqlTransaction.h"

using QtCommonLib::SqlConnection;
using QtCommonLib::SqlConnectionOptions;
using QtCommonLib::SqlTransaction;

namespace
{
/**
 * @brief Converts the constrained SQLite state text into its public representation.
 * @param state State value read from the catalog.
 * @return Matching generation state; unknown values conservatively map to Building.
 */
[[nodiscard]] auto state_from_string(const QString& state) -> LogCacheGenerationState
{
    LogCacheGenerationState generation_state = LogCacheGenerationState::Building;
    if (state == QStringLiteral("complete"))
    {
        generation_state = LogCacheGenerationState::Complete;
    }
    else if (state == QStringLiteral("failed"))
    {
        generation_state = LogCacheGenerationState::Failed;
    }
    return generation_state;
}
}  // namespace

/**
 * @brief Opens the catalog at the standard application cache location.
 * @param parent Optional QObject parent.
 */
LogCacheCatalog::LogCacheCatalog(QObject* parent): LogCacheCatalog(get_default_cache_root(), parent)
{}

/**
 * @brief Opens the catalog below an explicit root using a private Qt SQL connection.
 * @param cache_root Directory containing the catalog and per-file databases.
 * @param parent Optional QObject parent.
 */
LogCacheCatalog::LogCacheCatalog(QString cache_root, QObject* parent)
    : QObject(parent),
      m_cache_root(QFileInfo(std::move(cache_root)).absoluteFilePath()),
      m_database_path(QDir(m_cache_root).filePath(QStringLiteral("catalog.sqlite")))
{
    m_is_available = initialize_database();
}

/** @brief Closes and unregisters the private Qt SQL connection. */
LogCacheCatalog::~LogCacheCatalog() = default;

/**
 * @brief Reports whether catalog initialization succeeded.
 * @return True when the directory, SQLite connection and schema are usable.
 */
auto LogCacheCatalog::is_available() const -> bool
{
    return m_is_available;
}

/**
 * @brief Returns the managed cache root.
 * @return Absolute directory containing the catalog and per-file cache databases.
 */
auto LogCacheCatalog::get_cache_root() const -> QString
{
    return m_cache_root;
}

/**
 * @brief Returns the central catalog database path.
 * @return Absolute path of `catalog.sqlite`.
 */
auto LogCacheCatalog::get_database_path() const -> QString
{
    return m_database_path;
}

/**
 * @brief Reads the current SQLite user schema version.
 * @return `PRAGMA user_version`, or zero when the catalog is unavailable.
 */
auto LogCacheCatalog::get_schema_version() const -> int
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
 * @brief Registers or resumes a building generation for an exact cache identity.
 * @param identity Valid source and parser identity.
 * @return Generation metadata, or std::nullopt when registration fails.
 */
auto LogCacheCatalog::begin_generation(const LogCacheIdentity& identity)
    -> std::optional<LogCacheGeneration>
{
    std::optional<LogCacheGeneration> generation;
    if (m_is_available && identity.is_valid())
    {
        // Source registration and generation creation/resumption form one
        // transaction. Callers must never observe a generation whose source row was
        // not committed with it.
        QSqlDatabase database = m_connection->database();
        SqlTransaction transaction(database);
        bool succeeded = transaction.is_active();
        if (succeeded)
        {
            QSqlQuery source_query(database);
            source_query.prepare(
                QStringLiteral("INSERT INTO cache_sources(canonical_path, "
                               "last_seen_utc_ms) VALUES(?, ?) "
                               "ON CONFLICT(canonical_path) DO UPDATE SET "
                               "last_seen_utc_ms=excluded.last_seen_utc_ms"));
            source_query.addBindValue(identity.canonical_file_path);
            source_query.addBindValue(QDateTime::currentDateTimeUtc().toMSecsSinceEpoch());
            succeeded = source_query.exec();
        }

        qint64 source_id = -1;
        if (succeeded)
        {
            QSqlQuery id_query(database);
            id_query.prepare(QStringLiteral("SELECT id FROM cache_sources WHERE canonical_path=?"));
            id_query.addBindValue(identity.canonical_file_path);
            succeeded = id_query.exec() && id_query.next();
            if (succeeded)
            {
                source_id = id_query.value(0).toLongLong();
            }
        }

        qint64 generation_id = -1;
        if (succeeded)
        {
            QSqlQuery existing_query(database);
            existing_query.prepare(
                QStringLiteral("SELECT id FROM cache_generations WHERE cache_key=?"));
            existing_query.addBindValue(identity.cache_key);
            succeeded = existing_query.exec();

            if (succeeded && existing_query.next())
            {
                generation_id = existing_query.value(0).toLongLong();
                QSqlQuery resume_query(database);
                resume_query.prepare(
                    QStringLiteral("UPDATE cache_generations SET state='building', "
                                   "indexed_bytes=0, entry_count=0, "
                                   "storage_bytes=0, completed_utc_ms=NULL, "
                                   "error_message='' WHERE id=? AND "
                                   "state<>'complete'"));
                resume_query.addBindValue(generation_id);
                succeeded = resume_query.exec();
            }
            else if (succeeded)
            {
                const qint64 now = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
                QSqlQuery insert_query(database);
                insert_query.prepare(QStringLiteral(
                    "INSERT INTO cache_generations(source_id, cache_key, file_size, "
                    "modified_utc_ms, sample_sha256, parser_sha256, cache_file_name, "
                    "state, created_utc_ms, last_access_utc_ms) "
                    "VALUES(?, ?, ?, ?, ?, ?, ?, 'building', ?, ?)"));
                insert_query.addBindValue(source_id);
                insert_query.addBindValue(identity.cache_key);
                insert_query.addBindValue(identity.file_size);
                insert_query.addBindValue(identity.modified_utc_ms);
                insert_query.addBindValue(identity.sample_sha256);
                insert_query.addBindValue(identity.parser_sha256);
                insert_query.addBindValue(identity.cache_key + QStringLiteral(".sqlite"));
                insert_query.addBindValue(now);
                insert_query.addBindValue(now);
                succeeded = insert_query.exec();
                generation_id = insert_query.lastInsertId().toLongLong();
            }
        }

        succeeded = succeeded && transaction.commit();
        if (succeeded)
        {
            generation = load_generation(generation_id);
        }
    }

    return generation;
}

/**
 * @brief Finds a reusable complete generation for an exact identity.
 * @param identity Source and parser identity to find.
 * @return Complete generation metadata, or std::nullopt when no match exists.
 */
auto LogCacheCatalog::find_complete_generation(const LogCacheIdentity& identity) const
    -> std::optional<LogCacheGeneration>
{
    std::optional<LogCacheGeneration> generation;
    if (m_is_available && identity.is_valid())
    {
        QSqlQuery query(m_connection->database());
        query.prepare(
            QStringLiteral("SELECT id FROM cache_generations WHERE "
                           "cache_key=? AND state='complete'"));
        query.addBindValue(identity.cache_key);
        if (query.exec() && query.next())
        {
            generation = load_generation(query.value(0).toLongLong());
        }
    }
    return generation;
}

/**
 * @brief Finds the largest complete generation that remains an unchanged source prefix.
 * @param identity Current source and parser identity.
 * @return Largest reusable prefix generation, or std::nullopt when none is safe.
 */
auto LogCacheCatalog::find_complete_prefix(const LogCacheIdentity& identity) const
    -> std::optional<LogCacheGeneration>
{
    std::optional<LogCacheGeneration> generation;
    if (m_is_available && identity.is_valid())
    {
        QSqlQuery query(m_connection->database());
        query.prepare(QStringLiteral(
            "SELECT id FROM cache_generations WHERE source_id=(SELECT id FROM cache_sources "
            "WHERE canonical_path=?) AND parser_sha256=? AND state='complete' AND file_size<? "
            "AND indexed_bytes=file_size ORDER BY file_size DESC, last_access_utc_ms DESC"));
        query.addBindValue(identity.canonical_file_path);
        query.addBindValue(identity.parser_sha256);
        query.addBindValue(identity.file_size);
        const bool query_succeeded = query.exec();
        while (query_succeeded && query.next() && !generation.has_value())
        {
            const std::optional<LogCacheGeneration> candidate =
                load_generation(query.value(0).toLongLong());
            if (candidate.has_value() &&
                candidate->identity.matches_source_prefix(identity.canonical_file_path))
            {
                generation = candidate;
            }
        }
    }
    return generation;
}

/**
 * @brief Loads one catalog generation without restricting its lifecycle state.
 * @param generation_id Catalog generation primary key.
 * @return Reconstructed generation metadata, or std::nullopt when the catalog is unavailable or
 * the identifier does not exist.
 */
auto LogCacheCatalog::get_generation(qint64 generation_id) const
    -> std::optional<LogCacheGeneration>
{
    const std::optional<LogCacheGeneration> generation = load_generation(generation_id);
    return generation;
}

/**
 * @brief Finalizes a building generation and stores its measured sizes.
 * @param generation_id Catalog generation primary key.
 * @param indexed_bytes Number of source bytes represented by the cache.
 * @param entry_count Number of indexed log records.
 * @param storage_bytes Total measured cache storage in bytes.
 * @return True only when exactly one building generation was finalized.
 */
auto LogCacheCatalog::mark_complete(qint64 generation_id, qint64 indexed_bytes, qint64 entry_count,
                                    qint64 storage_bytes) -> bool
{
    const bool can_complete = m_is_available && generation_id >= 0 && indexed_bytes >= 0 &&
                              entry_count >= 0 && storage_bytes >= 0;
    bool completed = false;
    if (can_complete)
    {
        QSqlQuery query(m_connection->database());
        query.prepare(
            QStringLiteral("UPDATE cache_generations SET state='complete', "
                           "indexed_bytes=?, entry_count=?, "
                           "storage_bytes=?, completed_utc_ms=?, "
                           "last_access_utc_ms=?, error_message='' WHERE id=? "
                           "AND state='building'"));
        const qint64 now = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
        query.addBindValue(indexed_bytes);
        query.addBindValue(entry_count);
        query.addBindValue(storage_bytes);
        query.addBindValue(now);
        query.addBindValue(now);
        query.addBindValue(generation_id);
        completed = query.exec() && query.numRowsAffected() == 1;
    }
    return completed;
}

/**
 * @brief Marks a building generation failed without exposing it to views.
 * @param generation_id Catalog generation primary key.
 * @param error_message Diagnostic reason retained by the catalog.
 * @return True only when exactly one building generation was updated.
 */
auto LogCacheCatalog::mark_failed(qint64 generation_id, const QString& error_message) -> bool
{
    bool failed = false;
    if (m_is_available && generation_id >= 0)
    {
        QSqlQuery query(m_connection->database());
        query.prepare(
            QStringLiteral("UPDATE cache_generations SET state='failed', "
                           "completed_utc_ms=NULL, error_message=? "
                           "WHERE id=? AND state='building'"));
        query.addBindValue(error_message);
        query.addBindValue(generation_id);
        failed = query.exec() && query.numRowsAffected() == 1;
    }
    return failed;
}

/**
 * @brief Marks an incomplete generation failed and removes its disposable database.
 * @param generation_id Catalog generation primary key.
 * @param error_message Diagnostic reason for discarding the generation.
 * @return True when metadata was updated and all existing cache files were removed.
 */
auto LogCacheCatalog::discard_generation(qint64 generation_id, const QString& error_message) -> bool
{
    const std::optional<LogCacheGeneration> generation = load_generation(generation_id);
    bool discarded = false;
    if (generation.has_value() && generation->state != LogCacheGenerationState::Complete)
    {
        discarded = generation->state == LogCacheGenerationState::Failed ||
                    mark_failed(generation_id, error_message);
        const QStringList paths{generation->database_path,
                                generation->database_path + QStringLiteral("-wal"),
                                generation->database_path + QStringLiteral("-shm")};
        for (qsizetype index = 0; index < paths.size(); ++index)
        {
            const QString& path = paths.at(index);
            if (QFileInfo::exists(path))
            {
                discarded = QFile::remove(path) && discarded;
            }
        }
    }
    return discarded;
}

/**
 * @brief Refreshes the LRU timestamp of a generation.
 * @param generation_id Catalog generation primary key.
 * @return True when the generation exists and was touched.
 */
auto LogCacheCatalog::touch_generation(qint64 generation_id) -> bool
{
    bool touched = false;
    if (m_is_available && generation_id >= 0)
    {
        QSqlQuery query(m_connection->database());
        query.prepare(
            QStringLiteral("UPDATE cache_generations SET last_access_utc_ms=? WHERE id=?"));
        query.addBindValue(QDateTime::currentDateTimeUtc().toMSecsSinceEpoch());
        query.addBindValue(generation_id);
        touched = query.exec() && query.numRowsAffected() == 1;
    }
    return touched;
}

/**
 * @brief Replaces a view's ordered mapping with complete cache generations.
 * @param view_id Stable view identifier.
 * @param generation_ids Complete generation primary keys in display order.
 * @return True when the complete mapping was committed atomically.
 */
auto LogCacheCatalog::bind_view(const QUuid& view_id, const QVector<qint64>& generation_ids) -> bool
{
    bool bound = false;
    if (m_is_available && !view_id.isNull())
    {
        // Replacing the entire ordered mapping in one transaction prevents a restored
        // view from seeing a mixture of old and new source generations.
        QSqlDatabase database = m_connection->database();
        SqlTransaction transaction(database);
        bool succeeded = transaction.is_active();
        const QString view_text = view_id.toString(QUuid::WithoutBraces);

        if (succeeded)
        {
            QSqlQuery view_query(database);
            view_query.prepare(
                QStringLiteral("INSERT INTO cache_views(view_id, updated_utc_ms) VALUES(?, ?) "
                               "ON CONFLICT(view_id) DO UPDATE SET "
                               "updated_utc_ms=excluded.updated_utc_ms"));
            view_query.addBindValue(view_text);
            view_query.addBindValue(QDateTime::currentDateTimeUtc().toMSecsSinceEpoch());
            succeeded = view_query.exec();
        }

        if (succeeded)
        {
            QSqlQuery delete_query(database);
            delete_query.prepare(
                QStringLiteral("DELETE FROM cache_view_generations WHERE view_id=?"));
            delete_query.addBindValue(view_text);
            succeeded = delete_query.exec();
        }

        for (qsizetype position = 0; position < generation_ids.size() && succeeded; ++position)
        {
            QSqlQuery insert_query(database);
            insert_query.prepare(QStringLiteral(
                "INSERT INTO cache_view_generations(view_id, generation_id, position) "
                "SELECT ?, id, ? FROM cache_generations WHERE id=? AND state='complete'"));
            insert_query.addBindValue(view_text);
            insert_query.addBindValue(position);
            insert_query.addBindValue(generation_ids.at(position));
            succeeded = insert_query.exec() && insert_query.numRowsAffected() == 1;
        }

        bound = succeeded && transaction.commit();
    }
    return bound;
}

/**
 * @brief Reads the complete generations bound to a view.
 * @param view_id Stable view identifier.
 * @return Generation primary keys in display order, or an empty vector if none exist.
 */
auto LogCacheCatalog::get_view_generations(const QUuid& view_id) const -> QVector<qint64>
{
    QVector<qint64> generation_ids;
    if (m_is_available && !view_id.isNull())
    {
        QSqlQuery query(m_connection->database());
        query.prepare(
            QStringLiteral("SELECT generation_id FROM cache_view_generations WHERE "
                           "view_id=? ORDER BY position"));
        query.addBindValue(view_id.toString(QUuid::WithoutBraces));
        if (query.exec())
        {
            while (query.next())
            {
                generation_ids.append(query.value(0).toLongLong());
            }
        }
    }
    return generation_ids;
}

/**
 * @brief Removes a view mapping without deleting reusable cache generations.
 * @param view_id Stable view identifier.
 * @return True when the delete statement completed successfully.
 */
auto LogCacheCatalog::remove_view(const QUuid& view_id) -> bool
{
    bool removed = false;
    if (m_is_available && !view_id.isNull())
    {
        QSqlQuery query(m_connection->database());
        query.prepare(QStringLiteral("DELETE FROM cache_views WHERE view_id=?"));
        query.addBindValue(view_id.toString(QUuid::WithoutBraces));
        removed = query.exec();
    }
    return removed;
}

/**
 * @brief Resolves the standard application cache root.
 * @return Absolute `history/cache-v2` path below QStandardPaths::AppConfigLocation.
 */
auto LogCacheCatalog::get_default_cache_root() -> QString
{
    const QString config_path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return QFileInfo(QDir(config_path).filePath(QStringLiteral("history/cache-v2")))
        .absoluteFilePath();
}

/**
 * @brief Opens SQLite, applies runtime pragmas and restores a usable schema.
 * @return True when the catalog is ready for public operations.
 */
auto LogCacheCatalog::initialize_database() -> bool
{
    bool initialized = !m_cache_root.isEmpty() &&
                       QDir().mkpath(QDir(m_cache_root).filePath(QStringLiteral("files")));
    if (!initialized)
    {
        qWarning() << "Cache catalog directory initialization failed:" << m_cache_root;
    }
    else
    {
        SqlConnectionOptions options;
        options.connection_name_prefix = QStringLiteral("qt_log_viewer_cache_catalog");
        options.connect_options = QStringLiteral("QSQLITE_BUSY_TIMEOUT=5000");
        options.connection_setup_statements = {
            QStringLiteral("PRAGMA foreign_keys=ON"), QStringLiteral("PRAGMA busy_timeout=5000"),
            QStringLiteral("PRAGMA journal_mode=WAL"), QStringLiteral("PRAGMA synchronous=NORMAL")};
        m_connection = std::make_unique<SqlConnection>(m_database_path, options);
        initialized = m_connection->is_open();
        if (!initialized)
        {
            qWarning() << "Cache catalog database initialization failed:"
                       << m_connection->last_error();
        }

        QSqlDatabase database;
        if (initialized)
        {
            database = m_connection->database();
            QSqlQuery query(database);
            initialized = query.exec(QStringLiteral("PRAGMA user_version")) && query.next();
            if (!initialized)
            {
                qWarning() << "Cache catalog schema version query failed:" << query.lastError();
            }
            else
            {
                const int version = query.value(0).toInt();
                query.finish();
                initialized = version == SchemaVersion ? create_schema() : rebuild_schema();
            }
        }

        if (initialized)
        {
            // A process termination cannot leave a reusable partial generation.
            // Converting stale building rows to failed preserves diagnostics while
            // keeping them hidden from views.
            QSqlQuery interrupted_query(database);
            initialized = interrupted_query.exec(
                QStringLiteral("UPDATE cache_generations SET state='failed', "
                               "error_message='Interrupted cache build.' WHERE state='building'"));
            if (!initialized)
            {
                qWarning() << "Cache catalog interrupted-build recovery failed:"
                           << interrupted_query.lastError();
            }
        }
    }
    return initialized;
}

/**
 * @brief Rebuilds incompatible metadata and removes now-unreferenced cache databases.
 * @return True when the replacement schema was committed successfully.
 */
auto LogCacheCatalog::rebuild_schema() -> bool
{
    QSqlDatabase database = m_connection->database();
    SqlTransaction transaction(database);
    bool rebuilt = transaction.is_active();
    if (rebuilt)
    {
        QSqlQuery query(database);
        const QStringList drops{QStringLiteral("DROP TABLE IF EXISTS cache_view_generations"),
                                QStringLiteral("DROP TABLE IF EXISTS cache_views"),
                                QStringLiteral("DROP TABLE IF EXISTS cache_generations"),
                                QStringLiteral("DROP TABLE IF EXISTS cache_sources")};
        for (qsizetype index = 0; index < drops.size() && rebuilt; ++index)
        {
            const QString& statement = drops.at(index);
            rebuilt = query.exec(statement);
            if (!rebuilt)
            {
                qWarning() << "Cache catalog schema drop failed:" << statement << query.lastError();
            }
        }
        rebuilt = rebuilt && create_schema();
        if (rebuilt)
        {
            rebuilt = query.exec(QStringLiteral("PRAGMA user_version=2"));
            if (!rebuilt)
            {
                qWarning() << "Cache catalog schema version update failed:" << query.lastError();
            }
        }

        query.finish();
        const bool ready_to_commit = rebuilt;
        rebuilt = rebuilt && transaction.commit();
        if (!rebuilt)
        {
            if (ready_to_commit)
            {
                qWarning() << "Cache catalog schema commit failed:" << transaction.last_error();
            }
        }
        else
        {
            remove_stale_cache_files();
        }
    }
    return rebuilt;
}

/** Removes disposable per-file databases after their incompatible catalog was
 * replaced. */
auto LogCacheCatalog::remove_stale_cache_files() -> void
{
    QDir files_directory(QDir(m_cache_root).filePath(QStringLiteral("files")));
    const QStringList filters{QStringLiteral("*.sqlite"), QStringLiteral("*.sqlite-wal"),
                              QStringLiteral("*.sqlite-shm")};
    const QStringList stale_files = files_directory.entryList(filters, QDir::Files);
    for (const QString& file_name: stale_files)
    {
        if (!files_directory.remove(file_name))
        {
            qWarning() << "Removing incompatible cache file failed:"
                       << files_directory.absoluteFilePath(file_name);
        }
    }
}

/**
 * @brief Creates the idempotent catalog schema and lookup indexes.
 * @return True when every schema statement completed successfully.
 */
auto LogCacheCatalog::create_schema() -> bool
{
    QSqlQuery query(m_connection->database());
    const QStringList statements{
        QStringLiteral("CREATE TABLE IF NOT EXISTS cache_sources("
                       "id INTEGER PRIMARY KEY, canonical_path TEXT NOT NULL UNIQUE, "
                       "last_seen_utc_ms INTEGER NOT NULL)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS cache_generations("
                       "id INTEGER PRIMARY KEY, source_id INTEGER NOT NULL REFERENCES "
                       "cache_sources(id) ON DELETE CASCADE, cache_key TEXT NOT NULL "
                       "UNIQUE, "
                       "file_size INTEGER NOT NULL, modified_utc_ms INTEGER NOT NULL, "
                       "sample_sha256 BLOB NOT NULL, parser_sha256 BLOB NOT NULL, "
                       "cache_file_name TEXT NOT NULL UNIQUE, state TEXT NOT NULL "
                       "CHECK(state IN "
                       "('building','complete','failed')), indexed_bytes INTEGER NOT NULL "
                       "DEFAULT 0, "
                       "entry_count INTEGER NOT NULL DEFAULT 0, storage_bytes INTEGER NOT "
                       "NULL "
                       "DEFAULT 0, created_utc_ms INTEGER NOT NULL, last_access_utc_ms "
                       "INTEGER NOT "
                       "NULL, completed_utc_ms INTEGER, error_message TEXT NOT NULL DEFAULT "
                       "'')"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_cache_generations_lru ON "
                       "cache_generations(state, last_access_utc_ms)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_cache_generations_source ON "
                       "cache_generations(source_id, state)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS cache_views("
                       "view_id TEXT PRIMARY KEY, updated_utc_ms INTEGER NOT NULL)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS cache_view_generations("
                       "view_id TEXT NOT NULL REFERENCES cache_views(view_id) ON "
                       "DELETE CASCADE, "
                       "generation_id INTEGER NOT NULL REFERENCES "
                       "cache_generations(id) ON DELETE "
                       "CASCADE, position INTEGER NOT NULL, PRIMARY KEY(view_id, "
                       "generation_id), "
                       "UNIQUE(view_id, position))")};

    bool created = true;
    for (const QString& statement: statements)
    {
        if (created && !query.exec(statement))
        {
            qWarning() << "Cache catalog schema creation failed:" << statement << query.lastError();
            created = false;
        }
    }
    return created;
}

/**
 * @brief Reconstructs public generation metadata from normalized catalog tables.
 * @param generation_id Catalog generation primary key.
 * @return Generation metadata, or std::nullopt when unavailable or not found.
 */
auto LogCacheCatalog::load_generation(qint64 generation_id) const
    -> std::optional<LogCacheGeneration>
{
    std::optional<LogCacheGeneration> generation;
    if (m_is_available)
    {
        QSqlQuery query(m_connection->database());
        query.prepare(
            QStringLiteral("SELECT g.id, g.source_id, s.canonical_path, g.file_size, "
                           "g.modified_utc_ms, "
                           "g.sample_sha256, g.parser_sha256, g.cache_key, "
                           "g.cache_file_name, g.state, "
                           "g.indexed_bytes, g.entry_count, g.storage_bytes, "
                           "g.last_access_utc_ms, g.error_message "
                           "FROM cache_generations g JOIN cache_sources s ON "
                           "s.id=g.source_id WHERE g.id=?"));
        query.addBindValue(generation_id);
        if (query.exec() && query.next())
        {
            LogCacheGeneration value;
            value.id = query.value(0).toLongLong();
            value.source_id = query.value(1).toLongLong();
            value.identity.canonical_file_path = query.value(2).toString();
            value.identity.file_size = query.value(3).toLongLong();
            value.identity.modified_utc_ms = query.value(4).toLongLong();
            value.identity.sample_sha256 = query.value(5).toByteArray();
            value.identity.parser_sha256 = query.value(6).toByteArray();
            value.identity.cache_key = query.value(7).toString();
            value.database_path =
                get_cache_database_path(value.identity.cache_key + QStringLiteral(".sqlite"));
            value.state = state_from_string(query.value(9).toString());
            value.indexed_bytes = query.value(10).toLongLong();
            value.entry_count = query.value(11).toLongLong();
            value.storage_bytes = query.value(12).toLongLong();
            value.last_access_utc_ms = query.value(13).toLongLong();
            value.error_message = query.value(14).toString();
            generation = std::move(value);
        }
    }
    return generation;
}

/**
 * @brief Resolves a managed filename below the dedicated cache-file directory.
 * @param cache_file_name Cache filename to resolve.
 * @return Absolute path below the catalog's managed `files` directory.
 */
auto LogCacheCatalog::get_cache_database_path(const QString& cache_file_name) const -> QString
{
    return QFileInfo(
               QDir(QDir(m_cache_root).filePath(QStringLiteral("files"))).filePath(cache_file_name))
        .absoluteFilePath();
}
