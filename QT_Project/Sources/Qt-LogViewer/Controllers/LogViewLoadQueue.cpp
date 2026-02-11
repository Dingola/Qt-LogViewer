/**
 * @file LogViewLoadQueue.cpp
 * @brief Implements the LogViewLoadQueue which coordinates per-view async log streaming.
 *
 * The queue stores (view_id, file_path) pairs and ensures only one stream is active at a time.
 */

#include "Qt-LogViewer/Controllers/LogViewLoadQueue.h"

#include <QDebug>
#include <QList>

// Concrete include for forward-declared type
#include "Qt-LogViewer/Services/LogLoadingService.h"

/**
 * @brief Enqueues a file to be streamed for a specific view.
 * @param view_id Target view id.
 * @param file_path Absolute file path.
 */
auto LogViewLoadQueue::enqueue(const QUuid& view_id, const QString& file_path) -> void
{
    bool already_pending = false;
    const bool already_active = (m_active_view_id == view_id) && (m_active_file_path == file_path);

    if (!already_active)
    {
        for (const auto& item: m_queue)
        {
            const bool same_view = (item.first == view_id);
            const bool same_path = (item.second == file_path);
            if (same_view && same_path)
            {
                already_pending = true;
            }
        }
    }

    const bool should_enqueue = !already_active && !already_pending;

    if (should_enqueue)
    {
        m_queue.append(qMakePair(view_id, file_path));
        qDebug().nospace() << "[Queue] enqueue view=" << view_id.toString() << " file=\""
                           << file_path << "\" size=" << m_queue.size();
    }
    else
    {
        qDebug().nospace() << "[Queue] skip enqueue (duplicate) view=" << view_id.toString()
                           << " file=\"" << file_path << "\" active=" << already_active
                           << " pending_dup=" << already_pending << " size=" << m_queue.size();
    }
}

/**
 * @brief Attempts to start the next async stream if none is active.
 * @param loader Loader service to start the stream.
 * @param batch_size Entries per emitted batch for the next stream.
 * @return True if a new stream started; false otherwise.
 */
auto LogViewLoadQueue::try_start_next(LogLoadingService* loader, qsizetype batch_size) -> bool
{
    bool started = false;

    const bool has_loader = (loader != nullptr);
    const bool is_idle = m_active_file_path.isEmpty();
    const bool has_pending = !m_queue.isEmpty();

    if (has_loader && is_idle && has_pending)
    {
        const QPair<QUuid, QString> next_item = m_queue.front();
        m_queue.pop_front();

        m_active_view_id = next_item.first;
        m_active_file_path = next_item.second;
        m_active_batch_size = batch_size;

        qDebug().nospace() << "[Queue] start_next view=" << m_active_view_id.toString()
                           << " file=\"" << m_active_file_path << "\" batch=" << m_active_batch_size
                           << " pending_left=" << m_queue.size();

        loader->load_log_file_async(m_active_file_path, m_active_batch_size);
        started = true;
    }
    else
    {
        qDebug().nospace() << "[Queue] start_next skipped has_loader=" << has_loader
                           << " is_idle=" << is_idle << " has_pending=" << has_pending
                           << " pending=" << m_queue.size()
                           << " active_view=" << m_active_view_id.toString() << " active_file=\""
                           << m_active_file_path << "\"";
    }

    return started;
}

/**
 * @brief Clears all pending items for the specified view.
 * @param view_id Target view id.
 */
auto LogViewLoadQueue::clear_pending_for_view(const QUuid& view_id) -> void
{
    QList<QPair<QUuid, QString>> kept;

    for (const auto& item: m_queue)
    {
        const bool keep_item = (item.first != view_id);
        if (keep_item)
        {
            kept.append(item);
        }
    }

    const int removed = m_queue.size() - kept.size();
    m_queue = kept;

    qDebug().nospace() << "[Queue] clear_pending_for_view view=" << view_id.toString()
                       << " removed=" << removed << " pending=" << m_queue.size();
}

/**
 * @brief Cancels the active stream if it belongs to the specified view and clears pendings.
 * @param loader Loader service used to cancel the active stream if needed.
 * @param view_id Target view id.
 */
auto LogViewLoadQueue::cancel_if_active(LogLoadingService* loader, const QUuid& view_id) -> void
{
    const bool same_view = (m_active_view_id == view_id);
    const bool can_cancel = same_view && (loader != nullptr);

    if (can_cancel)
    {
        qDebug().nospace() << "[Queue] cancel active view=" << view_id.toString() << " file=\""
                           << m_active_file_path << "\"";
        loader->cancel_async();
        m_active_view_id = QUuid();
        m_active_file_path = QString();
        m_active_batch_size = 1000;
    }

    clear_pending_for_view(view_id);
}

/**
 * @brief Clears the active state if the finished file matches the current active file.
 * @param file_path File path reported as finished.
 */
auto LogViewLoadQueue::clear_active_if(const QString& file_path) -> void
{
    const bool is_match = (file_path == m_active_file_path);

    if (is_match)
    {
        qDebug().nospace() << "[Queue] clear_active_if match file=\"" << file_path << "\"";
        m_active_view_id = QUuid();
        m_active_file_path = QString();
        m_active_batch_size = 1000;
    }
    else
    {
        qDebug().nospace() << "[Queue] clear_active_if no-match file=\"" << file_path
                           << "\" active_file=\"" << m_active_file_path << "\"";
    }
}

/**
 * @brief Unconditionally clears the active stream state.
 */
auto LogViewLoadQueue::clear_active() -> void
{
    qDebug().nospace() << "[Queue] clear_active force idle (was view="
                       << m_active_view_id.toString() << " file=\"" << m_active_file_path << "\")";
    m_active_view_id = QUuid();
    m_active_file_path = QString();
    m_active_batch_size = 1000;
}

/**
 * @brief Returns the active view id (empty if none).
 * @return The currently active view id, or a null QUuid if idle.
 */
auto LogViewLoadQueue::get_active_view_id() const -> QUuid
{
    auto result = m_active_view_id;
    return result;
}

/**
 * @brief Returns the active file path (empty if none).
 * @return The currently active file path, or empty if idle.
 */
auto LogViewLoadQueue::get_active_file_path() const -> QString
{
    auto result = m_active_file_path;
    return result;
}

/**
 * @brief Returns the number of pending items in the queue.
 * @return Count of pending (view_id, file_path) pairs.
 */
auto LogViewLoadQueue::get_pending_count() const -> int
{
    auto result = m_queue.size();
    return result;
}

/**
 * @brief Returns the active batch size (only meaningful while active).
 * @return The batch size used for the active stream, or the last set value.
 */
auto LogViewLoadQueue::get_active_batch_size() const -> qsizetype
{
    auto result = m_active_batch_size;
    return result;
}
