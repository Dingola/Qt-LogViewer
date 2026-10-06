/**
 * @file LogViewLoadQueue.cpp
 * @brief Implements the LogViewLoadQueue which coordinates per-view async log streaming.
 *
 * The queue stores (view_id, file_path) pairs and ensures only one stream is active at a time.
 */

#include "Qt-LogViewer/Controllers/LogViewLoadQueue.h"

#include <QDebug>
#include <QList>
#include <utility>

// Concrete include for forward-declared type
#include "Qt-LogViewer/Services/LogLoadingService.h"

/**
 * @brief Enqueues a file to be streamed for a specific view.
 * @param view_id Target view id.
 * @param file_path Absolute file path.
 * @return Unique operation identifier, or a null identifier for a duplicate request.
 */
auto LogViewLoadQueue::enqueue(const QUuid& view_id, const QString& file_path) -> QUuid
{
    return enqueue_request(view_id, file_path, std::nullopt, 0, -1);
}

/**
 * @brief Enqueues a file together with the parsing profile selected for its import.
 * @param view_id Target view identifier.
 * @param file_path Absolute file path.
 * @param profile Parsing profile used for this request.
 * @return Unique operation identifier, or a null identifier for a duplicate request.
 */
auto LogViewLoadQueue::enqueue(const QUuid& view_id, const QString& file_path,
                               const LogParsingProfile& profile) -> QUuid
{
    return enqueue_request(view_id, file_path, profile, 0, -1);
}

/**
 * @brief Enqueues a bounded source range with its parsing profile.
 * @param view_id Target view identifier.
 * @param file_path Absolute file path.
 * @param profile Parsing profile used for this request.
 * @param start_offset First source byte to parse.
 * @param end_offset Exclusive source byte boundary.
 * @return Unique operation identifier, or a null identifier for a duplicate request.
 */
auto LogViewLoadQueue::enqueue(const QUuid& view_id, const QString& file_path,
                               const LogParsingProfile& profile, qint64 start_offset,
                               qint64 end_offset) -> QUuid
{
    return enqueue_request(view_id, file_path, profile, start_offset, end_offset);
}

/**
 * @brief Enqueues one request after applying duplicate suppression.
 * @param view_id Target view identifier.
 * @param file_path Absolute file path.
 * @param profile Optional profile overriding the loader default.
 * @param start_offset First source byte to parse.
 * @param end_offset Exclusive source byte boundary, or -1 for the open-time size.
 * @return Unique operation identifier, or a null identifier for a duplicate request.
 */
auto LogViewLoadQueue::enqueue_request(const QUuid& view_id, const QString& file_path,
                                       std::optional<LogParsingProfile> profile,
                                       qint64 start_offset, qint64 end_offset) -> QUuid
{
    QUuid operation_id;
    bool already_pending = false;
    const bool already_active = !m_active_cancel_requested && (m_active_view_id == view_id) &&
                                (m_active_file_path == file_path);

    if (!already_active)
    {
        for (const LoadRequest& item: m_queue)
        {
            const bool same_view = (item.view_id == view_id);
            const bool same_path = (item.file_path == file_path);
            if (same_view && same_path)
            {
                already_pending = true;
            }
        }
    }

    const bool should_enqueue = !already_active && !already_pending;

    if (should_enqueue)
    {
        operation_id = QUuid::createUuid();
        m_queue.append(
            {view_id, operation_id, file_path, std::move(profile), start_offset, end_offset});
        qDebug().nospace() << "[Queue] enqueue operation=" << operation_id.toString()
                           << " view=" << view_id.toString() << " file=\"" << file_path
                           << "\" size=" << m_queue.size();
    }
    else
    {
        qDebug().nospace() << "[Queue] skip enqueue (duplicate) view=" << view_id.toString()
                           << " file=\"" << file_path << "\" active=" << already_active
                           << " pending_dup=" << already_pending << " size=" << m_queue.size();
    }

    return operation_id;
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
        const LoadRequest next_item = m_queue.front();
        m_queue.pop_front();

        m_active_view_id = next_item.view_id;
        m_active_operation_id = next_item.operation_id;
        m_active_file_path = next_item.file_path;
        m_active_profile = next_item.profile;
        m_active_batch_size = batch_size;
        m_active_cancel_requested = false;

        qDebug().nospace() << "[Queue] start_next view=" << m_active_view_id.toString()
                           << " file=\"" << m_active_file_path << "\" batch=" << m_active_batch_size
                           << " pending_left=" << m_queue.size();

        if (m_active_profile.has_value())
        {
            loader->load_log_file_async(m_active_file_path, m_active_batch_size,
                                        m_active_profile.value(), next_item.start_offset,
                                        next_item.end_offset);
        }
        else
        {
            loader->load_log_file_async(m_active_file_path, m_active_batch_size);
        }
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
    QList<LoadRequest> kept;

    for (const LoadRequest& item: m_queue)
    {
        const bool keep_item = (item.view_id != view_id);
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
 * @brief Requests cancellation for the active stream and removes pending items for the view.
 * @param loader Loader service used to cancel the active stream.
 * @param view_id Target view id.
 *
 * The active registration remains assigned until the loader reports streaming_idle.
 * This keeps late signals associated with the stream that produced them.
 */
auto LogViewLoadQueue::cancel_if_active(LogLoadingService* loader, const QUuid& view_id) -> void
{
    const bool same_view = m_active_view_id == view_id;
    const bool can_cancel = same_view && loader != nullptr;

    if (can_cancel)
    {
        m_active_cancel_requested = true;
        qDebug().nospace() << "[Queue] cancel active view=" << view_id.toString() << " file=\""
                           << m_active_file_path << '"';

        loader->cancel_async();
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
        m_active_operation_id = QUuid();
        m_active_file_path = QString();
        m_active_profile.reset();
        m_active_batch_size = 1000;
        m_active_cancel_requested = false;
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
    m_active_operation_id = QUuid();
    m_active_file_path = QString();
    m_active_profile.reset();
    m_active_batch_size = 1000;
    m_active_cancel_requested = false;
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
 * @brief Returns the active import operation identifier.
 * @return Unique operation identifier, or a null identifier while idle.
 */
auto LogViewLoadQueue::get_active_operation_id() const -> QUuid
{
    return m_active_operation_id;
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

/**
 * @brief Returns the parsing profile assigned to the active stream.
 * @return Active profile, or no value when idle or using the loader default.
 */
auto LogViewLoadQueue::get_active_profile() const -> std::optional<LogParsingProfile>
{
    return m_active_profile;
}
