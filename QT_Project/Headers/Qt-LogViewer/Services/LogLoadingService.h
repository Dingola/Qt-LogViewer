#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QString>
#include <QVector>

/**
 * @file LogLoadingService.h
 * @brief Declares the `LogLoadingService` which wraps `LogLoader`, adding validation,
 * retry/backoff, and instrumentation while re-exposing its API and signals.
 */

#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Services/LogLoader.h"

/**
 * @class LogLoadingService
 * @brief Service that encapsulates `LogLoader` and provides synchronous and asynchronous
 *        log file loading with additional policies:
 *        - Pre-flight validation (file existence and readability)
 *        - Bounded retry/backoff for transient streaming errors
 *        - Simple instrumentation (timings and logging hooks)
 *
 * Responsibilities:
 * - Own a `LogLoader` instance and forward calls:
 *   - `load_log_file(file_path)` for eager (synchronous) loading.
 *   - `load_log_file_async(file_path, batch_size)` for streaming (asynchronous) loading.
 *   - `cancel_async()` to stop any ongoing streaming task.
 * - Re-emit `LogLoader` signals so that controllers can subscribe without depending on `LogLoader`
 *   directly.
 *
 * Thread-safety:
 * - The service itself is a `QObject` and should be used from the GUI thread.
 * - The underlying `LogLoader` manages its own worker thread mechanics for streaming.
 */
class LogLoadingService final: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs the LogLoadingService.
         * @param log_format The log format string used by the loader for parsing.
         * @param parent Optional QObject parent for ownership.
         */
        explicit LogLoadingService(const QString& log_format, QObject* parent = nullptr);

        /**
         * @brief Destructor. Ensures any ongoing async operation is cancelled.
         */
        ~LogLoadingService() override;

        /**
         * @brief Loads a log file synchronously and returns all parsed entries.
         *        Performs pre-flight validation before delegating to the loader.
         * @param file_path Absolute file path to the log file.
         * @return Vector of parsed entries (empty on validation failure).
         */
        [[nodiscard]] auto load_log_file(const QString& file_path) -> QVector<LogEntry>;

        /**
         * @brief Reads only the first log entry from the given file (lightweight peek).
         *        Performs pre-flight validation; returns default entry if invalid/unreadable.
         * @param file_path Absolute file path to the log file.
         * @return The first LogEntry if available; otherwise default-constructed LogEntry.
         */
        [[nodiscard]] auto read_first_log_entry(const QString& file_path) const -> LogEntry;

        /**
         * @brief Starts streaming load of a log file asynchronously.
         *        Performs pre-flight validation, initializes retry state and instrumentation.
         * @param file_path Absolute file path to the log file.
         * @param batch_size Number of entries per emitted batch.
         */
        auto load_log_file_async(const QString& file_path, qsizetype batch_size = 1000) -> void;

        /**
         * @brief Cancels any ongoing asynchronous streaming operation.
         */
        auto cancel_async() -> void;

        /**
         * @brief Sets the maximum number of retries on streaming errors for the same file.
         * @param max_retries Number of retry attempts (0 disables retry).
         */
        auto set_max_retries(int max_retries) -> void;

        /**
         * @brief Returns the currently configured maximum number of retries.
         * @return Maximum retry attempts.
         */
        [[nodiscard]] auto get_max_retries() const -> int;

        /**
         * @brief Sets the delay between retries in milliseconds.
         * @param delay_ms Delay in milliseconds (0 performs immediate retry).
         */
        auto set_retry_delay_ms(int delay_ms) -> void;

        /**
         * @brief Returns the currently configured retry delay in milliseconds.
         * @return Delay in milliseconds.
         */
        [[nodiscard]] auto get_retry_delay_ms() const -> int;

    signals:
        /**
         * @brief Emitted when a batch of entries is parsed during streaming.
         * @param file_path File being streamed.
         * @param batch Parsed entries batch.
         */
        void entry_batch_parsed(const QString& file_path, const QVector<LogEntry>& batch);

        /**
         * @brief Emitted to report streaming progress.
         * @param file_path File being streamed.
         * @param bytes_read Bytes read so far.
         * @param total_bytes Total file size.
         */
        void progress(const QString& file_path, qint64 bytes_read, qint64 total_bytes);

        /**
         * @brief Emitted when an error occurs during streaming.
         * @param file_path File that errored.
         * @param message Error message.
         */
        void error(const QString& file_path, const QString& message);

        /**
         * @brief Emitted when streaming finishes for a file.
         * @param file_path File that finished.
         */
        void finished(const QString& file_path);

        /**
         * @brief Emitted when the underlying loader reports idle (safe to start next task).
         *
         * This is forwarded from `LogLoader::streaming_idle` to guarantee real worker-idle state.
         * On validation failure (no loader run), this service may emit it itself.
         */
        void streaming_idle();

    private:
        /**
         * @brief Wires internal `LogLoader` signals to re-emit via this service.
         */
        auto wire_loader_signals() -> void;

        /**
         * @brief Validates that the file exists and is readable.
         * @param file_path Absolute file path to validate.
         * @return True if the file exists and can be opened for reading; false otherwise.
         */
        [[nodiscard]] auto validate_file(const QString& file_path) const -> bool;

        /**
         * @brief Handles a streaming error and optionally schedules a retry.
         *        When retries are exhausted or not applicable, only emits `error` and resets
         *        the retry state. The idle signal will be forwarded from the loader.
         * @param file_path File path that caused the error.
         * @param message Error message.
         */
        auto handle_error_and_maybe_retry(const QString& file_path, const QString& message) -> void;

        /**
         * @brief Resets internal retry state when a stream finishes or gives up.
         * @param file_path File path for which to reset state.
         */
        auto reset_retry_state(const QString& file_path) -> void;

        /**
         * @brief Blocks the caller until the streaming worker is idle or timeout occurs.
         * @param timeout_ms Maximum time to wait (ms).
         */
        auto wait_until_idle(int timeout_ms) -> void;

    private:
        LogLoader m_loader;

        // Retry/backoff configuration
        int m_max_retries{0};
        int m_retry_delay_ms{250};

        // Retry/backoff state
        QString m_last_stream_file;
        int m_retry_count{0};
        qsizetype m_last_batch_size{1000};

        // Instrumentation
        QElapsedTimer m_timer;
};
