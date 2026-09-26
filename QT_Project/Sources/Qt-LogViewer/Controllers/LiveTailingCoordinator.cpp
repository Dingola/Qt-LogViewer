/**
 * @file LiveTailingCoordinator.cpp
 * @brief Implements live-tail state, persistence, and batched visible updates.
 */

#include "Qt-LogViewer/Controllers/LiveTailingCoordinator.h"

#include <QTimer>
#include <algorithm>

#include "Qt-LogViewer/Controllers/LogQueryController.h"
#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Services/LogHistoryService.h"
#include "Qt-LogViewer/Services/LogTailerService.h"

/**
 * @brief Constructs a live-tailing coordinator and its low-level reader.
 * @param default_profile Fallback profile for files without a retained profile.
 * @param history_service Storage receiving appended entries.
 * @param views Registry containing live view state and file profiles.
 * @param queries Query component used for visible page updates.
 * @param parent Optional QObject parent.
 */
LiveTailingCoordinator::LiveTailingCoordinator(const LogParsingProfile& default_profile,
                                               LogHistoryService* history_service,
                                               ViewRegistry* views, LogQueryController* queries,
                                               QObject* parent)
    : QObject(parent),
      m_default_profile(default_profile),
      m_history_service(history_service),
      m_views(views),
      m_queries(queries),
      m_tailer_service(new LogTailerService(default_profile, this)),
      m_refresh_timer(new QTimer(this))
{
    m_refresh_timer->setSingleShot(true);
    m_refresh_timer->setInterval(150);

    connect(m_tailer_service, &LogTailerService::entries_available, this,
            [this](const QUuid& view_id, const QString&, const QVector<LogEntry>& entries) {
                handle_entries(view_id, entries);
            });
    connect(m_refresh_timer, &QTimer::timeout, this,
            &LiveTailingCoordinator::refresh_pending_views);
}

/**
 * @brief Stops all registrations before owned objects are destroyed.
 */
LiveTailingCoordinator::~LiveTailingCoordinator()
{
    shutdown();
}

/**
 * @brief Enables or disables live tailing for a view.
 * @param view_id Target view.
 * @param enabled Whether live tailing should be active.
 */
auto LiveTailingCoordinator::set_enabled(const QUuid& view_id, bool enabled) -> void
{
    LogViewContext* context =
        m_views != nullptr && !view_id.isNull() ? m_views->get_context(view_id) : nullptr;

    if (enabled && !m_shutting_down && context != nullptr)
    {
        m_enabled_views.insert(view_id);

        for (const LogFileInfo& file_info: context->get_loaded_files())
        {
            const std::optional<LogParsingProfile> profile =
                context->get_file_parsing_profile(file_info.get_file_path());
            start_file(view_id, file_info.get_file_path(), profile.value_or(m_default_profile));
        }
    }
    else
    {
        m_enabled_views.remove(view_id);
        m_pending_refresh_views.remove(view_id);

        if (m_tailer_service != nullptr)
        {
            m_tailer_service->stop_tailing_view(view_id);
        }
    }
}

/**
 * @brief Returns whether live tailing is enabled for a view.
 * @param view_id Target view.
 * @return True when the view is enabled.
 */
auto LiveTailingCoordinator::is_enabled(const QUuid& view_id) const -> bool
{
    return m_enabled_views.contains(view_id);
}

/**
 * @brief Resets registrations while retaining a requested restore-time setting.
 * @param view_id Restored view.
 * @param enabled Desired live-tailing state.
 */
auto LiveTailingCoordinator::reset_view(const QUuid& view_id, bool enabled) -> void
{
    m_pending_refresh_views.remove(view_id);

    if (m_tailer_service != nullptr)
    {
        m_tailer_service->stop_tailing_view(view_id);
    }

    const bool valid_enabled_view = enabled && !m_shutting_down && m_views != nullptr &&
                                    m_views->get_context(view_id) != nullptr;

    if (valid_enabled_view)
    {
        m_enabled_views.insert(view_id);
    }
    else
    {
        m_enabled_views.remove(view_id);
    }
}

/**
 * @brief Starts one file when its view is enabled.
 * @param view_id Owning view.
 * @param file_path Imported file path.
 * @param profile Profile selected for the file.
 */
auto LiveTailingCoordinator::start_file(const QUuid& view_id, const QString& file_path,
                                        const LogParsingProfile& profile) -> void
{
    const bool can_start = !m_shutting_down && m_enabled_views.contains(view_id) &&
                           m_views != nullptr && m_views->get_context(view_id) != nullptr;

    if (can_start && m_tailer_service != nullptr)
    {
        m_tailer_service->start_tailing(view_id, file_path, profile);
    }
}

/**
 * @brief Stops one file registration and drops no view-level setting.
 * @param view_id Owning view.
 * @param file_path Removed file path.
 */
auto LiveTailingCoordinator::stop_file(const QUuid& view_id, const QString& file_path) -> void
{
    if (m_tailer_service != nullptr)
    {
        m_tailer_service->stop_tailing(view_id, file_path);
    }
}

/**
 * @brief Removes all live-tail state belonging to a closing view.
 * @param view_id Closing view.
 */
auto LiveTailingCoordinator::remove_view(const QUuid& view_id) -> void
{
    m_enabled_views.remove(view_id);
    m_pending_refresh_views.remove(view_id);

    if (m_tailer_service != nullptr)
    {
        m_tailer_service->stop_tailing_view(view_id);
    }
}

/**
 * @brief Stops timers and every low-level file registration.
 */
auto LiveTailingCoordinator::shutdown() -> void
{
    if (!m_shutting_down)
    {
        m_shutting_down = true;
        m_refresh_timer->stop();
        m_pending_refresh_views.clear();
        m_enabled_views.clear();
        m_tailer_service->stop_all_tailing();
    }
}

/**
 * @brief Sets the interval used to combine visible updates.
 * @param interval_ms Non-negative batching interval in milliseconds.
 */
auto LiveTailingCoordinator::set_refresh_interval_ms(int interval_ms) -> void
{
    m_refresh_timer->setInterval(std::max(0, interval_ms));
}

/**
 * @brief Persists appended entries and schedules one visible update for their view.
 * @param view_id Receiving view.
 * @param entries Newly parsed entries.
 */
auto LiveTailingCoordinator::handle_entries(const QUuid& view_id,
                                            const QVector<LogEntry>& entries) -> void
{
    const bool can_process = !m_shutting_down && m_enabled_views.contains(view_id) &&
                             !entries.isEmpty() && m_history_service != nullptr &&
                             m_views != nullptr && m_views->get_context(view_id) != nullptr;

    if (can_process && m_history_service->add_entries(view_id, entries))
    {
        m_pending_refresh_views.insert(view_id);
        m_refresh_timer->start();
    }
}

/**
 * @brief Applies the accumulated visible updates through the query component.
 */
auto LiveTailingCoordinator::refresh_pending_views() -> void
{
    const QSet<QUuid> views_to_refresh = m_pending_refresh_views;
    m_pending_refresh_views.clear();

    for (const QUuid& view_id: views_to_refresh)
    {
        const bool can_refresh = !m_shutting_down && m_enabled_views.contains(view_id) &&
                                 m_views != nullptr && m_views->get_context(view_id) != nullptr &&
                                 m_queries != nullptr;

        if (can_refresh)
        {
            m_queries->refresh_after_entries_appended(view_id);
        }
    }
}
