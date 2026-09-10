#pragma once

#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>
#include <QUuid>
#include <QVector>

#include "Qt-LogViewer/Models/LogEntry.h"

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
         * @brief Stores one parsed batch for an asynchronous import.
         * @param view_id View that owns the imported entries.
         * @param file_path Source file used to identify one import operation.
         * @param entries Parsed entries to store.
         */
        auto store_batch(const QUuid& view_id, const QString& file_path,
                         const QVector<LogEntry>& entries) -> void;

        /**
         * @brief Completes an import after all earlier queued batches were processed.
         * @param view_id View that owns the import.
         * @param file_path Imported source file.
         */
        auto finish_import(const QUuid& view_id, const QString& file_path) -> void;

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
         * @param view_id View that owns the import.
         * @param file_path Imported source file.
         * @param succeeded True when every batch was committed.
         * @param error_message Storage error for a failed import.
         */
        auto import_write_finished(const QUuid& view_id, const QString& file_path, bool succeeded,
                                   const QString& error_message) -> void;

    private:
        /**
         * @brief Creates the SQLite service in the writer thread when needed.
         * @return True when the writer connection is available.
         */
        auto ensure_history_service() -> bool;

        /**
         * @brief Clears the failed state for one import operation.
         * @param view_id View that owns the import.
         * @param file_path Imported source file.
         */
        auto clear_failed_import(const QUuid& view_id, const QString& file_path) -> void;

    private:
        QString m_database_path;
        LogHistoryService* m_history_service{nullptr};
        QHash<QUuid, QSet<QString>> m_failed_imports;
};
