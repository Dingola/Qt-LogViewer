/**
 * @file LogIngestController.cpp
 * @brief Implements the LogIngestController which encapsulates synchronous and asynchronous
 *        ingestion (parsing/streaming) of log files. It owns the loading service and queue,
 *        maps low-level loader signals to per-view signals, and provides helpers to enqueue,
 *        start-next and cancel streaming for specific views.
 */

#include "Qt-LogViewer/Controllers/LogIngestController.h"

#include <QDebug>

/**
 * @brief Constructs the LogIngestController.
 * @param log_format Format string passed to the underlying loader for parsing.
 * @param parent Optional QObject parent for ownership.
 */
LogIngestController::LogIngestController(const QString& log_format, QObject* parent)
    : QObject(parent), m_loader(log_format, this), m_queue(), m_is_shutting_down(false)
{
    wire_service_signals();
}

/**
 * @brief Destroys the LogIngestController. Cancels any ongoing async ingestion and
 *        disconnects loader signals to avoid late emissions during shutdown.
 */
LogIngestController::~LogIngestController()
{
    m_is_shutting_down = true;
    QObject::disconnect(&m_loader, nullptr, this, nullptr);
    m_loader.cancel_async();
}

/**
 * @brief Loads a file synchronously and returns parsed entries.
 *        Performs pre-flight validation through the service.
 * @param file_path Absolute path to the log file.
 * @return Parsed entries (may be empty on validation failure).
 */
auto LogIngestController::load_file_sync(const QString& file_path) -> QVector<LogEntry>
{
    QVector<LogEntry> entries = m_loader.load_log_file(file_path);
    return entries;
}

/**
 * @brief Reads only the first log entry from the given file (lightweight peek).
 *
 * @param file_path Absolute path to the log file.
 * @return The first LogEntry if available; otherwise default-constructed LogEntry.
 */
auto LogIngestController::read_first_log_entry(const QString& file_path) const -> LogEntry
{
    LogEntry entry = m_loader.read_first_log_entry(file_path);
    return entry;
}

/**
 * @brief Enqueues a file to be streamed for a specific view.
 *        Idempotent per `(view_id, file_path)`.
 * @param view_id Target view id.
 * @param file_path Absolute file path to stream.
 */
auto LogIngestController::enqueue_stream(const QUuid& view_id, const QString& file_path) -> void
{
    m_queue.enqueue(view_id, file_path);
}

/**
 * @brief Attempts to start the next asynchronous load if none is active.
 * @param batch_size Number of entries per batch appended to the model.
 */
auto LogIngestController::start_next_if_idle(qsizetype batch_size) -> void
{
    const bool started = m_queue.try_start_next(&m_loader, batch_size);
    if (started)
    {
        qDebug().nospace() << "[Ingest] started next view="
                           << m_queue.get_active_view_id().toString() << " file=\""
                           << m_queue.get_active_file_path() << '"';
    }
}

/**
 * @brief Cancels any ongoing streaming for the specified view and clears its pending queue.
 * @param view_id The view to cancel streaming for.
 */
auto LogIngestController::cancel_for_view(const QUuid& view_id) -> void
{
    m_queue.cancel_if_active(&m_loader, view_id);
}

/**
 * @brief Returns the active view id (empty if none).
 * @return The currently active view id, or a null QUuid if idle.
 */
auto LogIngestController::get_active_view_id() const -> QUuid
{
    QUuid id = m_queue.get_active_view_id();
    return id;
}

/**
 * @brief Returns the active file path (empty if none).
 * @return The currently active file path, or empty if idle.
 */
auto LogIngestController::get_active_file_path() const -> QString
{
    QString path = m_queue.get_active_file_path();
    return path;
}

/**
 * @brief Returns the number of pending items in the queue.
 * @return Count of pending `(view_id, file_path)` pairs.
 */
auto LogIngestController::get_pending_count() const -> int
{
    int count = m_queue.get_pending_count();
    return count;
}

/**
 * @brief Returns the active batch size (only meaningful while active).
 * @return The batch size used for the active stream, or the last set value.
 */
auto LogIngestController::get_active_batch_size() const -> qsizetype
{
    qsizetype size = m_queue.get_active_batch_size();
    return size;
}

/**
 * @brief Wires internal service signals and maps them to per-view signals using the queue's
 *        active view id. Keeps logic aligned with previous `LogViewerController` behavior.
 */
auto LogIngestController::wire_service_signals() -> void
{
    connect(&m_loader, &LogLoadingService::entry_batch_parsed, this,
            [this](const QString& file_path, const QVector<LogEntry>& batch) {
                if (!m_is_shutting_down)
                {
                    const QUuid view_id = m_queue.get_active_view_id();
                    qDebug().nospace() << "[Ingest] batch for view=" << view_id.toString()
                                       << " file=\"" << file_path << "\" count=" << batch.size();

                    if (!view_id.isNull())
                    {
                        emit entry_batch_parsed(view_id, file_path, batch);
                    }
                }
            });

    connect(&m_loader, &LogLoadingService::progress, this,
            [this](const QString& file_path, qint64 bytes_read, qint64 total_bytes) {
                if (!m_is_shutting_down)
                {
                    const QUuid view_id = m_queue.get_active_view_id();
                    qDebug().nospace()
                        << "[Ingest] progress view=" << view_id.toString() << " file=\""
                        << file_path << "\" " << bytes_read << '/' << total_bytes;

                    if (!view_id.isNull())
                    {
                        emit progress(view_id, file_path, bytes_read, total_bytes);
                    }
                }
            });

    connect(&m_loader, &LogLoadingService::error, this,
            [this](const QString& file_path, const QString& message) {
                if (!m_is_shutting_down)
                {
                    const QUuid view_id = m_queue.get_active_view_id();
                    qWarning().nospace()
                        << "[Ingest] error view=" << view_id.toString() << " file=\"" << file_path
                        << "\" msg=\"" << message << '"';

                    if (!view_id.isNull())
                    {
                        emit error(view_id, file_path, message);
                    }
                    // IMPORTANT: Do not clear active state here; wait for streaming_idle to ensure
                    // any late batches are still routed to the active view.
                }
            });

    connect(&m_loader, &LogLoadingService::finished, this, [this](const QString& file_path) {
        if (!m_is_shutting_down)
        {
            const QUuid view_id = m_queue.get_active_view_id();
            qDebug().nospace() << "[Ingest] finished view=" << view_id.toString() << " file=\""
                               << file_path << '"';

            if (!view_id.isNull())
            {
                emit finished(view_id, file_path);
            }
            // IMPORTANT: Do not clear active state here; wait for streaming_idle to avoid
            // dropping a late-arriving last batch for very small files.
        }
    });

    // Advance queue only after thread cleanup (safe to start next).
    connect(&m_loader, &LogLoadingService::streaming_idle, this, [this]() {
        if (!m_is_shutting_down)
        {
            qDebug().nospace() << "[Ingest] streaming_idle: force idle then try start next. "
                               << "pending=" << m_queue.get_pending_count();

            // Clear active only when the loader reports true idle.
            m_queue.clear_active();

            // Start next first, then read new active view id (fix ordering).
            const bool started = m_queue.try_start_next(&m_loader, m_queue.get_active_batch_size());
            if (started)
            {
                const QUuid new_active = m_queue.get_active_view_id();
                qDebug().nospace() << "[Ingest] started next view=" << new_active.toString()
                                   << " file=\"" << m_queue.get_active_file_path() << '"';
            }
            else
            {
                qDebug().nospace() << "[Ingest] no next item started (idle or empty queue).";
            }

            emit idle();
        }
    });
}
