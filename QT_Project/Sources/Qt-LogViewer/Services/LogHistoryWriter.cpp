/**
 * @file LogHistoryWriter.cpp
 * @brief Implements the background writer for asynchronously imported log
 * history.
 */

#include "Qt-LogViewer/Services/LogHistoryWriter.h"

#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QStringList>
#include <utility>

#include "Qt-LogViewer/Services/LogFileCacheDatabase.h"
#include "Qt-LogViewer/Services/LogHistoryService.h"

namespace
{
constexpr auto k_history_write_error = "Could not store parsed entries in persistent storage.";
}

/**
 * @brief Constructs a writer for the supplied SQLite database.
 * @param database_path Path shared with the read-side history service.
 * @param parent Optional QObject parent.
 */
LogHistoryWriter::LogHistoryWriter(QString database_path, QObject* parent)
    : QObject(parent), m_database_path(std::move(database_path))
{}

/**
 * @brief Destroys the writer and its thread-owned history connection.
 */
LogHistoryWriter::~LogHistoryWriter()
{
    const QList<QUuid> operation_ids = m_cache_imports.keys();
    for (const QUuid& operation_id: operation_ids)
    {
        close_cache_import(operation_id, true);
    }
}

/**
 * @brief Prepares an optional building cache generation before its first batch
 * arrives.
 * @param operation_id Unique identifier of the import attempt.
 * @param cache_generation Building generation to populate, or no value for
 * legacy-only writing and reuse of an already complete generation.
 * @param prefix_generation Complete generation cloned before suffix batches are appended.
 */
auto LogHistoryWriter::begin_import(
    const QUuid& operation_id, const std::optional<LogCacheGeneration>& cache_generation,
    const std::optional<LogCacheGeneration>& prefix_generation) -> void
{
    const bool should_begin = !operation_id.isNull() && cache_generation.has_value() &&
                              cache_generation->state == LogCacheGenerationState::Building;
    if (should_begin)
    {
        remove_cache_files(cache_generation->database_path);
        const bool uses_prefix = prefix_generation.has_value() &&
                                 prefix_generation->state == LogCacheGenerationState::Complete;
        const bool prepared =
            !uses_prefix || LogFileCacheDatabase::clone_generation(prefix_generation->database_path,
                                                                   cache_generation->database_path,
                                                                   cache_generation->identity);
        auto* database = new LogFileCacheDatabase(cache_generation->database_path,
                                                  cache_generation->identity, this);
        const bool transaction_started =
            prepared && database->is_available() &&
            (uses_prefix ? database->begin_append() : database->reset_entries());
        if (transaction_started)
        {
            m_cache_imports.insert(operation_id, CacheImport{cache_generation.value(), database});
        }
        else
        {
            delete database;
            remove_cache_files(cache_generation->database_path);
            m_failed_imports.insert(operation_id);
        }
    }
}

/**
 * @brief Stores one parsed batch in its per-file cache or legacy history fallback.
 * @param operation_id Unique identifier of the import attempt.
 * @param view_id View that owns the imported entries.
 * @param file_path Source file used to identify one import operation.
 * @param entries Parsed entries to store.
 */
auto LogHistoryWriter::store_batch(const QUuid& operation_id, const QUuid& view_id,
                                   const QString& file_path,
                                   const QVector<LogEntry>& entries) -> void
{
    const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
    const bool already_failed = m_failed_imports.contains(operation_id);
    const auto cache_iterator = m_cache_imports.find(operation_id);
    const bool writes_cache = cache_iterator != m_cache_imports.end();
    const bool can_store = !operation_id.isNull() && !view_id.isNull() &&
                           !absolute_file_path.isEmpty() && !entries.isEmpty() && !already_failed;
    bool stored = false;

    if (can_store && writes_cache)
    {
        stored = cache_iterator->database != nullptr &&
                 cache_iterator->database->append_entries(entries);
    }
    else if (can_store && ensure_history_service())
    {
        stored = m_history_service->add_entries(view_id, entries);
    }

    if (!already_failed && !stored)
    {
        m_failed_imports.insert(operation_id);
        if (!writes_cache && m_history_service != nullptr)
        {
            m_history_service->remove_file_entries(view_id, absolute_file_path);
        }
    }
}

/**
 * @brief Completes an import after all earlier queued batches were processed.
 * @param operation_id Unique identifier of the completed import attempt.
 * @param view_id View that owns the import.
 * @param file_path Imported source file.
 */
auto LogHistoryWriter::finish_import(const QUuid& operation_id, const QUuid& view_id,
                                     const QString& file_path) -> void
{
    const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
    const auto cache_iterator = m_cache_imports.find(operation_id);
    const bool storage_available =
        cache_iterator != m_cache_imports.end() || ensure_history_service();
    bool write_failed = !storage_available || m_failed_imports.contains(operation_id);
    if (!write_failed && cache_iterator != m_cache_imports.end())
    {
        write_failed =
            cache_iterator->database == nullptr || !cache_iterator->database->finalize_writes();
    }
    const QString error_message =
        write_failed ? QString::fromLatin1(k_history_write_error) : QString();

    close_cache_import(operation_id, write_failed);
    m_failed_imports.remove(operation_id);
    emit import_write_finished(operation_id, view_id, absolute_file_path, !write_failed,
                               error_message);
}

/**
 * @brief Closes and removes an incomplete cache generation for a cancelled
 * import.
 * @param operation_id Unique identifier of the cancelled import attempt.
 */
auto LogHistoryWriter::cancel_import(const QUuid& operation_id) -> void
{
    close_cache_import(operation_id, true);
    m_failed_imports.remove(operation_id);
}

/**
 * @brief Removes entries that may have been queued before a view was discarded.
 * @param view_id Discarded view.
 */
auto LogHistoryWriter::discard_view(const QUuid& view_id) -> void
{
    if (ensure_history_service())
    {
        m_history_service->remove_view_entries(view_id);
    }
}

/**
 * @brief Removes entries that may have been queued before a file was discarded.
 * @param view_id View that owned the file.
 * @param file_path Discarded source file.
 */
auto LogHistoryWriter::discard_file(const QUuid& view_id, const QString& file_path) -> void
{
    const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();

    if (ensure_history_service())
    {
        m_history_service->remove_file_entries(view_id, absolute_file_path);
    }
}

/**
 * @brief Creates the SQLite service in the writer thread when needed.
 * @return True when the writer connection is available.
 */
auto LogHistoryWriter::ensure_history_service() -> bool
{
    if (m_history_service == nullptr)
    {
        m_history_service = new LogHistoryService(m_database_path, this);
    }

    return m_history_service->is_available();
}

/**
 * @brief Closes one operation's cache connection and optionally removes its
 * files.
 * @param operation_id Unique identifier of the import attempt.
 * @param remove_files Whether the SQLite database and sidecar files are
 * deleted.
 */
auto LogHistoryWriter::close_cache_import(const QUuid& operation_id, bool remove_files) -> void
{
    const auto iterator = m_cache_imports.find(operation_id);
    if (iterator != m_cache_imports.end())
    {
        const QString database_path = iterator->generation.database_path;
        delete iterator->database;
        m_cache_imports.erase(iterator);

        if (remove_files)
        {
            remove_cache_files(database_path);
        }
    }
}

/**
 * @brief Removes a SQLite database together with its WAL and shared-memory
 * sidecars.
 * @param database_path Main SQLite database path.
 */
auto LogHistoryWriter::remove_cache_files(const QString& database_path) -> void
{
    const QStringList paths{database_path, database_path + QStringLiteral("-wal"),
                            database_path + QStringLiteral("-shm")};
    for (const QString& path: paths)
    {
        if (QFileInfo::exists(path))
        {
            QFile::remove(path);
        }
    }
}
