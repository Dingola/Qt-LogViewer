/**
 * @file LogLoadingService.cpp
 * @brief Implements the `LogLoadingService` which wraps `LogLoader`, adding validation,
 * retry/backoff, and instrumentation while re-exposing its API and signals.
 */

#include "Qt-LogViewer/Services/LogLoadingService.h"

#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QTimer>

/**
 * @brief Constructs the LogLoadingService and wires loader signals.
 * @param log_format The log format string used by the loader for parsing.
 * @param parent Optional QObject parent.
 */
LogLoadingService::LogLoadingService(const QString& log_format, QObject* parent)
    : QObject(parent),
      m_loader(log_format, this),
      m_max_retries(0),
      m_retry_delay_ms(250),
      m_last_stream_file(),
      m_retry_count(0),
      m_last_batch_size(1000),
      m_timer()
{
    wire_loader_signals();
}

/**
 * @brief Default destructor.
 */
LogLoadingService::~LogLoadingService() = default;

/**
 * @brief Sets the maximum number of retries on streaming errors.
 * @param max_retries Number of retry attempts (negative values are clamped to 0).
 */
auto LogLoadingService::set_max_retries(int max_retries) -> void
{
    if (max_retries < 0)
    {
        m_max_retries = 0;
    }
    else
    {
        m_max_retries = max_retries;
    }
}

/**
 * @brief Returns the configured maximum number of retries.
 * @return Maximum retry attempts.
 */
auto LogLoadingService::get_max_retries() const -> int
{
    auto result = m_max_retries;
    return result;
}

/**
 * @brief Sets the delay between retries in milliseconds.
 * @param delay_ms Delay in milliseconds (negative values are clamped to 0).
 */
auto LogLoadingService::set_retry_delay_ms(int delay_ms) -> void
{
    if (delay_ms < 0)
    {
        m_retry_delay_ms = 0;
    }
    else
    {
        m_retry_delay_ms = delay_ms;
    }
}

/**
 * @brief Returns the configured retry delay in milliseconds.
 * @return Delay in milliseconds.
 */
auto LogLoadingService::get_retry_delay_ms() const -> int
{
    auto result = m_retry_delay_ms;
    return result;
}

/**
 * @brief Validates that the file exists and is readable.
 * @param file_path Absolute file path to validate.
 * @return True if the file exists and can be opened for reading; false otherwise.
 */
auto LogLoadingService::validate_file(const QString& file_path) const -> bool
{
    bool is_valid = false;

    QFile file_handle(file_path);
    const bool exists = file_handle.exists();
    const bool opened = exists ? file_handle.open(QIODevice::ReadOnly) : false;

    if (opened)
    {
        is_valid = true;
        file_handle.close();
    }

    return is_valid;
}

/**
 * @brief Loads a log file synchronously and returns parsed entries.
 *        Emits an error and returns an empty vector if validation fails.
 * @param file_path Absolute file path to the log file.
 * @return Vector of parsed entries (empty on validation failure).
 */
auto LogLoadingService::load_log_file(const QString& file_path) -> QVector<LogEntry>
{
    QVector<LogEntry> entries;

    if (validate_file(file_path))
    {
        m_timer.start();
        entries = m_loader.load_log_file(file_path);

        qDebug().nospace() << "Eager load finished: " << file_path << ", entries=" << entries.size()
                           << ", elapsed=" << m_timer.elapsed() << " ms";
    }
    else
    {
        qWarning().nospace() << "Eager load validation failed for file: " << file_path;
        emit error(file_path, QStringLiteral("File not found or unreadable."));
    }

    return entries;
}

/**
 * @brief Starts streaming load of a log file asynchronously.
 *        Emits an error and `streaming_idle` if validation fails.
 * @param file_path Absolute file path to the log file.
 * @param batch_size Number of entries per emitted batch.
 */
auto LogLoadingService::load_log_file_async(const QString& file_path, qsizetype batch_size) -> void
{
    if (validate_file(file_path))
    {
        m_last_stream_file = file_path;
        m_retry_count = 0;
        m_last_batch_size = batch_size;
        m_timer.start();

        qDebug().nospace() << "Streaming started: " << file_path << " (batch=" << batch_size << ")";

        m_loader.load_log_file_async(file_path, batch_size);
    }
    else
    {
        qWarning().nospace() << "Streaming validation failed for file: " << file_path;
        emit error(file_path, QStringLiteral("File not found or unreadable."));
        emit streaming_idle();
    }
}

/**
 * @brief Cancels any ongoing asynchronous streaming operation.
 */
auto LogLoadingService::cancel_async() -> void
{
    m_loader.cancel_async();
}

/**
 * @brief Wires internal `LogLoader` signals to re-emit via this service, adding retry/backoff and
 *        instrumentation where appropriate.
 */
auto LogLoadingService::wire_loader_signals() -> void
{
    connect(&m_loader, &LogLoader::entry_batch_parsed, this,
            [this](const QString& file_path, const QVector<LogEntry>& batch) {
                emit entry_batch_parsed(file_path, batch);
            });

    connect(&m_loader, &LogLoader::progress, this,
            [this](const QString& file_path, qint64 bytes_read, qint64 total_bytes) {
                emit progress(file_path, bytes_read, total_bytes);
            });

    connect(&m_loader, &LogLoader::error, this,
            [this](const QString& file_path, const QString& message) {
                handle_error_and_maybe_retry(file_path, message);
            });

    connect(&m_loader, &LogLoader::finished, this, [this](const QString& file_path) {
        qDebug().nospace() << "Streaming finished: " << file_path
                           << ", elapsed=" << m_timer.elapsed() << " ms";
        emit finished(file_path);
        emit streaming_idle();
        reset_retry_state(file_path);
    });

    connect(&m_loader, &LogLoader::streaming_idle, this, [this]() { emit streaming_idle(); });
}

/**
 * @brief Handles a streaming error and optionally schedules a retry with backoff.
 *        If retries are exhausted or the error is for a different file, re-emits `error` and
 *        `streaming_idle` to allow the queue to advance.
 * @param file_path File path that caused the error.
 * @param message Error message.
 */
auto LogLoadingService::handle_error_and_maybe_retry(const QString& file_path,
                                                     const QString& message) -> void
{
    const bool same_file = (file_path == m_last_stream_file);
    const bool can_retry = same_file && (m_retry_count < m_max_retries);

    qCritical().nospace() << "Streaming error: " << file_path << " (" << message
                          << ") retry_count=" << m_retry_count << " max_retries=" << m_max_retries;

    if (can_retry)
    {
        m_retry_count++;

        QTimer::singleShot(m_retry_delay_ms, this, [this, file_path]() {
            qDebug().nospace() << "Retry " << m_retry_count << " for file: " << file_path
                               << " (batch=" << m_last_batch_size << ")";
            m_loader.load_log_file_async(file_path, m_last_batch_size);
        });
    }
    else
    {
        emit error(file_path, message);
        emit streaming_idle();
        reset_retry_state(file_path);
    }
}

/**
 * @brief Resets internal retry state when streaming finishes or gives up retrying.
 * @param file_path File path for which to reset state.
 */
auto LogLoadingService::reset_retry_state(const QString& file_path) -> void
{
    if (file_path == m_last_stream_file)
    {
        m_last_stream_file.clear();
        m_retry_count = 0;
        m_last_batch_size = 1000;
    }
}
