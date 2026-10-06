#pragma once

#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>
#include <QUuid>
#include <QVector>
#include <optional>

#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Services/LogCacheCatalog.h"

class LogFileCacheDatabase;
class LogHistoryService;

/**
 * @file LogHistoryWriter.h
 * @brief Declares the background writer for asynchronously imported log history.
 */

/**
 * @class LogHistoryWriter
 * @brief Serializes history writes on a dedicated worker thread.
 *
 * The writer lazily creates its own LogHistoryService and SQLite connection in the thread that
 * invokes its methods. Batch and completion calls must be queued to the same writer instance so
 * their order is preserved.
 */
class LogHistoryWriter final: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a writer for the supplied SQLite database.
         * @param database_path Path shared with the read-side history service.
         * @param parent Optional QObject parent.
         */
        explicit LogHistoryWriter(QString database_path, QObject* parent = nullptr);

        /**
         * @brief Destroys the writer and its thread-owned history connection.
         */
        ~LogHistoryWriter() override;

        /**
         * @brief Prepares an optional building cache generation before its first batch arrives.
         * @param operation_id Unique identifier of the import attempt.
         * @param cache_generation Building generation to populate, or no value for legacy-only
         * writing and reuse of an already complete generation.
         */
        auto begin_import(const QUuid& operation_id,
                          const std::optional<LogCacheGeneration>& cache_generation) -> void;

        /**
         * @brief Stores one parsed batch for an asynchronous import.
         * @param operation_id Unique identifier of the import attempt.
         * @param view_id View that owns the imported entries.
         * @param file_path Source file used to identify one import operation.
         * @param entries Parsed entries to store.
         */
        auto store_batch(const QUuid& operation_id, const QUuid& view_id, const QString& file_path,
                         const QVector<LogEntry>& entries) -> void;

        /**
         * @brief Completes an import after all earlier queued batches were processed.
         * @param operation_id Unique identifier of the completed import attempt.
         * @param view_id View that owns the import.
         * @param file_path Imported source file.
         */
        auto finish_import(const QUuid& operation_id, const QUuid& view_id,
                           const QString& file_path) -> void;

        /**
         * @brief Closes and removes an incomplete cache generation for a cancelled import.
         * @param operation_id Unique identifier of the cancelled import attempt.
         */
        auto cancel_import(const QUuid& operation_id) -> void;

        /**
         * @brief Removes entries that may have been queued before a view was discarded.
         * @param view_id Discarded view.
         */
        auto discard_view(const QUuid& view_id) -> void;

        /**
         * @brief Removes entries that may have been queued before a file was discarded.
         * @param view_id View that owned the file.
         * @param file_path Discarded source file.
         */
        auto discard_file(const QUuid& view_id, const QString& file_path) -> void;

    signals:
        /**
         * @brief Emitted after the completion marker reaches the writer.
         * @param operation_id Unique identifier of the completed import attempt.
         * @param view_id View that owns the import.
         * @param file_path Imported source file.
         * @param succeeded True when every batch was committed.
         * @param error_message Storage error for a failed import.
         */
        auto import_write_finished(const QUuid& operation_id, const QUuid& view_id,
                                   const QString& file_path, bool succeeded,
                                   const QString& error_message) -> void;

    private:
        /**
         * @brief Creates the SQLite service in the writer thread when needed.
         * @return True when the writer connection is available.
         */
        auto ensure_history_service() -> bool;

        /**
         * @brief Closes one operation's cache connection and optionally removes its files.
         * @param operation_id Unique identifier of the import attempt.
         * @param remove_files Whether the SQLite database and sidecar files are deleted.
         */
        auto close_cache_import(const QUuid& operation_id, bool remove_files) -> void;

        /**
         * @brief Removes a SQLite database together with its WAL and shared-memory sidecars.
         * @param database_path Main SQLite database path.
         */
        static auto remove_cache_files(const QString& database_path) -> void;

    private:
        /** @brief Writer-thread state for one building per-file cache generation. */
        struct CacheImport {
                LogCacheGeneration generation;
                LogFileCacheDatabase* database{nullptr};
        };

        QString m_database_path;
        LogHistoryService* m_history_service{nullptr};
        QHash<QUuid, CacheImport> m_cache_imports;
        QSet<QUuid> m_failed_imports;
};
