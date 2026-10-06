#pragma once

#include <QHash>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QUuid>
#include <QVector>
#include <atomic>
#include <memory>
#include <optional>

#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Services/LogCacheCatalog.h"

class LogHistoryWriter;
class QThread;

/**
 * @file HistoryWriteService.h
 * @brief Declares the lifecycle boundary for ordered, cancellation-aware history writes.
 */

/**
 * @class HistoryWriteService
 * @brief Owns the history writer thread and coalesces ordered write operations.
 *
 * Each import owns independent cancellation state. Parsed batches are collected per import and
 * drained by one queued writer task, preventing a cancelled import from leaving thousands of
 * obsolete events in the writer thread.
 */
class HistoryWriteService final: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Creates and starts a background writer for the supplied database.
         * @param database_path SQLite database path shared with the read-side history service.
         * @param parent Optional QObject parent.
         */
        explicit HistoryWriteService(QString database_path, QObject* parent = nullptr);

        /** @brief Waits for queued work and stops the writer thread. */
        ~HistoryWriteService() override;

        /**
         * @brief Registers one uniquely identified asynchronous import.
         * @param operation_id Unique identifier of this import attempt.
         * @param view_id View receiving the imported entries.
         * @param file_path Source file belonging to the import attempt.
         * @param cache_generation Optional building cache generation populated by the writer.
         * @return True when the operation was registered.
         */
        auto begin_import(const QUuid& operation_id, const QUuid& view_id, const QString& file_path,
                          std::optional<LogCacheGeneration> cache_generation = std::nullopt)
            -> bool;

        /**
         * @brief Adds one parsed batch to the operation's coalesced writer buffer.
         * @param operation_id Unique identifier of the import attempt.
         * @param view_id View that owns the entries.
         * @param file_path Source file identifying the import operation.
         * @param entries Parsed entries to store.
         * @return True when the operation accepted the batch.
         */
        auto store_batch(const QUuid& operation_id, const QUuid& view_id, const QString& file_path,
                         const QVector<LogEntry>& entries) -> bool;

        /**
         * @brief Requests completion after every buffered batch has been stored.
         * @param operation_id Unique identifier of the import attempt.
         * @param view_id View that owns the import.
         * @param file_path Imported source file.
         * @return True when the completion request was accepted.
         */
        auto finish_import(const QUuid& operation_id, const QUuid& view_id,
                           const QString& file_path) -> bool;

        /**
         * @brief Immediately marks every import belonging to a view as cancelled.
         * @param view_id View whose buffered and queued writer work must be skipped.
         */
        auto cancel_view(const QUuid& view_id) -> void;

        /**
         * @brief Queues cleanup for a discarded view behind an in-progress writer call.
         * @param view_id Discarded view.
         * @return True when the cleanup operation was accepted.
         */
        auto discard_view(const QUuid& view_id) -> bool;

        /**
         * @brief Queues cleanup for a discarded file behind an in-progress writer call.
         * @param view_id View that owned the file.
         * @param file_path Discarded source file.
         * @return True when the cleanup operation was accepted.
         */
        auto discard_file(const QUuid& view_id, const QString& file_path) -> bool;

        /**
         * @brief Stops accepting work, waits for queued operations, and ends the worker thread.
         *
         * This operation is idempotent and is also performed by the destructor.
         */
        auto shutdown() -> void;

        /**
         * @brief Reports whether the service can currently accept operations.
         * @return True while the worker thread is running and shutdown has not begun.
         */
        [[nodiscard]] auto is_running() const -> bool;

    signals:
        /**
         * @brief Emitted after coalesced batches have left the writer backlog.
         * @param operation_id Import operation owning the batches.
         * @param batch_count Number of processed parser batches.
         */
        auto batches_processed(const QUuid& operation_id, qsizetype batch_count) -> void;

        /**
         * @brief Emitted when an active import completion request has been processed.
         * @param operation_id Unique identifier of the completed import attempt.
         * @param view_id View that owns the import.
         * @param file_path Imported source file.
         * @param succeeded True when every preceding batch was stored.
         * @param error_message Storage error for a failed import.
         */
        auto import_write_finished(const QUuid& operation_id, const QUuid& view_id,
                                   const QString& file_path, bool succeeded,
                                   const QString& error_message) -> void;

    private:
        /** @brief Shared state used by the producer thread and its single writer drain. */
        struct ImportOperation {
                QUuid view_id;
                QString file_path;
                std::optional<LogCacheGeneration> cache_generation;
                std::atomic_bool cancelled{false};
                QMutex mutex;
                QVector<LogEntry> pending_entries;
                qsizetype pending_batch_count{0};
                bool drain_scheduled{false};
                bool finish_requested{false};
        };

        /**
         * @brief Queues the sole drain task for one import operation.
         * @param operation_id Unique identifier forwarded with writer completion.
         * @param operation Shared operation state retained by the queued task.
         */
        auto queue_drain(const QUuid& operation_id,
                         const std::shared_ptr<ImportOperation>& operation) -> void;

        QThread* m_thread{nullptr};
        LogHistoryWriter* m_writer{nullptr};
        QHash<QUuid, std::shared_ptr<ImportOperation>> m_operations;
        bool m_accepting_work{false};
};
