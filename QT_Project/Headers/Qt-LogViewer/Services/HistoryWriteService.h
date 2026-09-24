#pragma once

#include <QObject>
#include <QString>
#include <QUuid>
#include <QVector>

#include "Qt-LogViewer/Models/LogEntry.h"

class LogHistoryWriter;
class QThread;

/**
 * @file HistoryWriteService.h
 * @brief Declares the lifecycle boundary for ordered background history writes.
 */

/**
 * @class HistoryWriteService
 * @brief Owns the history writer thread and queues ordered write operations.
 *
 * Calls accepted by this service are delivered to one worker in submission order. Shutdown waits
 * until all previously queued operations have completed before stopping the worker thread.
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

        /**
         * @brief Waits for queued work and stops the writer thread.
         */
        ~HistoryWriteService() override;

        /**
         * @brief Queues one parsed batch for ordered background storage.
         * @param view_id View that owns the entries.
         * @param file_path Source file identifying the import operation.
         * @param entries Parsed entries to store.
         * @return True when the operation was accepted.
         */
        auto store_batch(const QUuid& view_id, const QString& file_path,
                         const QVector<LogEntry>& entries) -> bool;

        /**
         * @brief Queues an import completion marker behind earlier batches.
         * @param view_id View that owns the import.
         * @param file_path Imported source file.
         * @return True when the completion marker was accepted.
         */
        auto finish_import(const QUuid& view_id, const QString& file_path) -> bool;

        /**
         * @brief Queues cleanup for a discarded view behind earlier writes.
         * @param view_id Discarded view.
         * @return True when the cleanup operation was accepted.
         */
        auto discard_view(const QUuid& view_id) -> bool;

        /**
         * @brief Queues cleanup for a discarded file behind earlier writes.
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
         * @brief Emitted when an import completion marker has been processed.
         * @param view_id View that owns the import.
         * @param file_path Imported source file.
         * @param succeeded True when every preceding batch was stored.
         * @param error_message Storage error for a failed import.
         */
        auto import_write_finished(const QUuid& view_id, const QString& file_path, bool succeeded,
                                   const QString& error_message) -> void;

    private:
        QThread* m_thread{nullptr};
        LogHistoryWriter* m_writer{nullptr};
        bool m_accepting_work{false};
};
