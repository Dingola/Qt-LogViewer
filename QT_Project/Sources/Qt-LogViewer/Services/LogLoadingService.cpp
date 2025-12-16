/**
 * @file LogLoadingService.cpp
 * @brief Implements the `LogLoadingService` which wraps `LogLoader`, adding validation,
 * retry/backoff, and instrumentation while re-exposing its API and signals.
 */

#include "Qt-LogViewer/Services/LogLoadingService.h"

#include <QDebug>
#include <QEventLoop>
#include <QFile>
#include <QIODevice>
#include <QList>
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
 * @brief Destructor: cancel and wait for the worker to become idle to avoid QThread crash.
 */
LogLoadingService::~LogLoadingService()
{
    m_loader.cancel_async();
    wait_until_idle(5000);
}

/**
 * @brief Wires internal `LogLoader` signals to re-emit via this service, adding retry/backoff and
 *        instrumentation where appropriate.
 */
auto LogLoadingService::wire_loader_signals() -> void
{
    connect(&m_loader, &LogLoader::entry_batch_parsed, this,
            [this](const QString& file_path, const QVector<LogEntry>& batch) {
                qDebug().nospace()
                    << "[Service] batch file=\"" << file_path << "\" count=" << batch.size();
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
        reset_retry_state(file_path);
    });

    // Forward the real idle event from the loader to guarantee the worker is fully idle.
    connect(&m_loader, &LogLoader::streaming_idle, this, [this]() {
        qDebug().nospace() << "[Service] loader reported streaming_idle";
        emit streaming_idle();
    });
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
 *        Emits an error and returns an empty list if validation fails.
 * @param file_path Absolute file path to the log file.
 * @return QList<LogEntry> (empty on validation failure).
 */
auto LogLoadingService::load_log_file(const QString& file_path) -> QList<LogEntry>
{
    QList<LogEntry> result;

    if (validate_file(file_path))
    {
        m_timer.start();
        const QVector<LogEntry> entries_vec = m_loader.load_log_file(file_path);

        for (const auto& e: entries_vec)
        {
            result.append(e);
        }

        qDebug().nospace() << "Eager load finished: " << file_path << ", entries=" << result.size()
                           << ", elapsed=" << m_timer.elapsed() << " ms";
    }
    else
    {
        qWarning().nospace() << "Eager load validation failed for file: " << file_path;
        emit error(file_path, QStringLiteral("File not found or unreadable."));
    }

    return result;
}

/**
 * @brief Reads only the first log entry from the given file (lightweight peek).
 *        Returns default-constructed LogEntry if validation fails or no entry is found.
 * @param file_path Absolute file path to the log file.
 * @return The first LogEntry if available; otherwise default LogEntry.
 */
auto LogLoadingService::read_first_log_entry(const QString& file_path) const -> LogEntry
{
    LogEntry entry;

    if (validate_file(file_path))
    {
        entry = m_loader.read_first_log_entry(file_path);
    }
    else
    {
        qWarning().nospace() << "Peek read validation failed for file: " << file_path;
    }

    return entry;
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
        // No loader active in this branch, so we emit idle ourselves.
        emit streaming_idle();
    }
}

/**
 * @brief Cancels any ongoing asynchronous streaming operation.
 */
auto LogLoadingService::cancel_async() -> void
{
    qDebug().nospace() << "[Service] cancel_async() requested";
    m_loader.cancel_async();
}

/**
 * @brief Handles a streaming error and optionally schedules a retry with backoff.
 *        If retries are exhausted or the error is for a different file, re-emits `error` and
 *        lets the loader's `streaming_idle` indicate idle.
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
            qDebug().nospace() << "[Service] retry " << m_retry_count << " file=\"" << file_path
                               << "\" (batch=" << m_last_batch_size << ")";
            m_loader.load_log_file_async(file_path, m_last_batch_size);
        });
    }
    else
    {
        emit error(file_path, message);
        reset_retry_state(file_path);
        // Do not emit streaming_idle here; the loader will emit it when truly idle.
    }
}

/**
 * @brief Resets internal retry state when streaming finishes or gives up retrying.
 * @param file_path File path for which to reset state.
 */
auto LogLoadingService::reset_retry_state(const QString& file_path) -> void
{
    const bool same = (file_path == m_last_stream_file);
    if (same)
    {
        m_last_stream_file.clear();
        m_retry_count = 0;
        m_last_batch_size = 1000;
        qDebug().nospace() << "[Service] reset_retry_state for \"" << file_path << '"';
    }
}

/**
 * @brief Waits until the loader reports idle or timeout. Used at shutdown to avoid QThread crash.
 * @param timeout_ms Maximum time to wait in milliseconds.
 */
auto LogLoadingService::wait_until_idle(int timeout_ms) -> void
{
    const bool has_pending_stream = !m_last_stream_file.isEmpty();
    if (has_pending_stream)
    {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);

        auto c1 = connect(this, &LogLoadingService::streaming_idle, &loop, &QEventLoop::quit);
        auto c2 = connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

        timer.start(timeout_ms);
        loop.exec();

        disconnect(c1);
        disconnect(c2);
    }
}
