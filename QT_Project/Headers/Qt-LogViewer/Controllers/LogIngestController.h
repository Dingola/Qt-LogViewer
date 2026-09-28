#pragma once

#include <QObject>
#include <QString>
#include <QUuid>
#include <QVector>
#include <optional>

#include "Qt-LogViewer/Controllers/LogViewLoadQueue.h"
#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Services/LogLoadingService.h"
#include "Qt-LogViewer/Services/LogParseOutcome.h"
#include "Qt-LogViewer/Services/LogParsingProfile.h"

/**
 * @file LogIngestController.h
 * @brief Declares the LogIngestController class that encapsulates synchronous and asynchronous
 *        ingestion (parsing/streaming) of log files. It owns the loading service and queue,
 *        maps low-level loader signals to per-view signals, and provides helpers to enqueue,
 *        start-next and cancel streaming for specific views.
 */
class LogIngestController: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs the LogIngestController.
         * @param profile Parsing profile passed to the underlying loader.
         * @param parent Optional QObject parent for ownership.
         */
        explicit LogIngestController(const LogParsingProfile& profile, QObject* parent = nullptr);

        /**
         * @brief Destroys the LogIngestController. Cancels any ongoing async ingestion and
         *        disconnects loader signals to avoid late emissions during shutdown.
         */
        ~LogIngestController() override;

        /**
         * @brief Loads a file synchronously and returns parsed entries.
         *        Performs pre-flight validation through the service.
         * @param file_path Absolute path to the log file.
         * @return Parsed entries (may be empty on validation failure).
         */
        [[nodiscard]] auto load_file_sync(const QString& file_path) -> QVector<LogEntry>;

        /**
         * @brief Loads a file synchronously with an explicitly selected profile.
         * @param file_path Absolute path of the log file.
         * @param profile Parsing profile selected for this import.
         * @return Parsed entries, or an empty vector when loading fails.
         */
        [[nodiscard]] auto load_file_sync(const QString& file_path,
                                          const LogParsingProfile& profile) -> QVector<LogEntry>;

        /**
         * @brief Reads only the first log entry from the given file (lightweight peek).
         *
         * @param file_path Absolute path to the log file.
         * @return The first LogEntry if available; otherwise default-constructed LogEntry.
         */
        [[nodiscard]] auto read_first_log_entry(const QString& file_path) const -> LogEntry;

        /**
         * @brief Parses a bounded file sample without enqueuing or importing it.
         * @param file_path Absolute path of the file to preview.
         * @param profile Parsing profile to evaluate.
         * @param maximum_record_count Maximum number of non-empty records returned.
         * @return Parse outcomes in source order, including structured failures.
         */
        [[nodiscard]] auto preview_file(const QString& file_path, const LogParsingProfile& profile,
                                        qsizetype maximum_record_count) const
            -> QVector<LogParseOutcome>;

        /**
         * @brief Enqueues a file to be streamed for a specific view.
         *        Idempotent per `(view_id, file_path)`.
         * @param view_id Target view id.
         * @param file_path Absolute file path to stream.
         */
        auto enqueue_stream(const QUuid& view_id, const QString& file_path) -> void;

        /**
         * @brief Enqueues a file with the parsing profile selected for this import.
         * @param view_id Target view identifier.
         * @param file_path Absolute file path to stream.
         * @param profile Parsing profile used by the queued request.
         */
        auto enqueue_stream(const QUuid& view_id, const QString& file_path,
                            const LogParsingProfile& profile) -> void;

        /**
         * @brief Attempts to start the next asynchronous load if none is active.
         * @param batch_size Number of entries per batch appended to the model.
         */
        auto start_next_if_idle(qsizetype batch_size) -> void;

        /**
         * @brief Cancels any ongoing streaming for the specified view and clears its pending queue.
         * @param view_id The view to cancel streaming for.
         */
        auto cancel_for_view(const QUuid& view_id) -> void;

        /**
         * @brief Returns the active view id (empty if none).
         * @return The currently active view id, or a null QUuid if idle.
         */
        [[nodiscard]] auto get_active_view_id() const -> QUuid;

        /**
         * @brief Returns the active file path (empty if none).
         * @return The currently active file path, or empty if idle.
         */
        [[nodiscard]] auto get_active_file_path() const -> QString;

        /**
         * @brief Returns the number of pending items in the queue.
         * @return Count of pending `(view_id, file_path)` pairs.
         */
        [[nodiscard]] auto get_pending_count() const -> int;

        /**
         * @brief Returns the active batch size (only meaningful while active).
         * @return The batch size used for the active stream, or the last set value.
         */
        [[nodiscard]] auto get_active_batch_size() const -> qsizetype;

        /**
         * @brief Returns the parsing profile assigned to the active stream.
         * @return Active profile, or no value while idle.
         */
        [[nodiscard]] auto get_active_profile() const -> std::optional<LogParsingProfile>;

    signals:
        /**
         * @brief Emitted when a batch of entries is parsed during streaming for a view.
         * @param view_id View receiving streamed data.
         * @param file_path File being streamed.
         * @param batch Parsed entries batch.
         */
        void entry_batch_parsed(const QUuid& view_id, const QString& file_path,
                                const QVector<LogEntry>& batch);

        /**
         * @brief Emitted to report streaming progress for a specific view.
         * @param view_id The view receiving streamed data.
         * @param file_path File being streamed.
         * @param bytes_read Bytes read so far.
         * @param total_bytes Total file size.
         */
        void progress(const QUuid& view_id, const QString& file_path, qint64 bytes_read,
                      qint64 total_bytes);

        /**
         * @brief Emitted when an error occurs during streaming for a view.
         * @param view_id View that attempted to load the file.
         * @param file_path File that errored.
         * @param message Error message.
         */
        void error(const QUuid& view_id, const QString& file_path, const QString& message);

        /**
         * @brief Emitted when a file finishes streaming for a view.
         * @param view_id View that received the data.
         * @param file_path File that finished.
         */
        void finished(const QUuid& view_id, const QString& file_path);

        /**
         * @brief Emitted when the underlying loader reports idle (safe to start next task).
         *
         * This mirrors `LogLoadingService::streaming_idle` and indicates a definitive
         * worker-idle state. Controllers can use this to advance the queue.
         */
        void idle();

    private:
        /**
         * @brief Wires internal service signals and maps them to per-view signals using the queue's
         *        active view id so queued imports remain associated with their originating view.
         */
        auto wire_service_signals() -> void;

    private:
        /** Profile used by compatibility overloads without an explicit selection. */
        LogParsingProfile m_default_profile;
        LogLoadingService m_loader;
        LogViewLoadQueue m_queue;
        bool m_is_shutting_down{false};
};
