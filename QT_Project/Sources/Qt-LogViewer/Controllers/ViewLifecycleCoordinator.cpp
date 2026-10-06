/**
 * @file ViewLifecycleCoordinator.cpp
 * @brief Implements ordered cleanup operations for log views and their files.
 */

#include "Qt-LogViewer/Controllers/ViewLifecycleCoordinator.h"

#include <QVector>

#include "Qt-LogViewer/Controllers/FileCatalogController.h"
#include "Qt-LogViewer/Controllers/FilterCoordinator.h"
#include "Qt-LogViewer/Controllers/LiveTailingCoordinator.h"
#include "Qt-LogViewer/Controllers/LogImportCoordinator.h"
#include "Qt-LogViewer/Controllers/LogPageCoordinator.h"
#include "Qt-LogViewer/Controllers/LogQueryController.h"
#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Services/HistoryWriteService.h"
#include "Qt-LogViewer/Services/LogHistoryService.h"

/**
 * @brief Constructs the lifecycle coordinator from the affected application components.
 * @param views Registry containing the views and file registrations.
 * @param filters Filter state repaired after file removal.
 * @param catalog File catalog updated by global removal.
 * @param imports Import workflow cancelled before view destruction.
 * @param live_tailing Live-tail registrations stopped before removal.
 * @param history Persisted history removed during cleanup.
 * @param history_writer Ordered writer receiving discard operations.
 * @param pages Page state used to detect views with active queries.
 * @param queries Query component used to refresh remaining files.
 * @param parent Optional QObject parent.
 */
ViewLifecycleCoordinator::ViewLifecycleCoordinator(
    ViewRegistry* views, FilterCoordinator* filters, FileCatalogController* catalog,
    LogImportCoordinator* imports, LiveTailingCoordinator* live_tailing, LogHistoryService* history,
    HistoryWriteService* history_writer, LogPageCoordinator* pages, LogQueryController* queries,
    QObject* parent)
    : QObject(parent),
      m_views(views),
      m_filters(filters),
      m_catalog(catalog),
      m_imports(imports),
      m_live_tailing(live_tailing),
      m_history(history),
      m_history_writer(history_writer),
      m_pages(pages),
      m_queries(queries)
{}

/**
 * @brief Closes one view after cancelling and cleaning all related work.
 * @param view_id View to close.
 * @return True when an existing view was removed.
 */
auto ViewLifecycleCoordinator::close_view(const QUuid& view_id) -> bool
{
    const bool view_exists =
        m_views != nullptr && !view_id.isNull() && m_views->get_context(view_id) != nullptr;
    bool removed = false;

    if (view_exists)
    {
        bool cleanup_queued = false;
        if (m_imports != nullptr)
        {
            m_imports->cancel(view_id);
            cleanup_queued = m_imports->discard_history(view_id);
        }

        if (m_live_tailing != nullptr)
        {
            m_live_tailing->remove_view(view_id);
        }

        if (!cleanup_queued && m_history_writer != nullptr)
        {
            cleanup_queued = m_history_writer->discard_view(view_id);
        }

        if (!cleanup_queued && m_history != nullptr)
        {
            m_history->remove_view_entries(view_id);
        }

        removed = m_views->remove_view(view_id);
    }

    return removed;
}

/**
 * @brief Closes every registered view through the same ordered cleanup path.
 */
auto ViewLifecycleCoordinator::close_all_views() -> void
{
    if (m_views != nullptr)
    {
        const QVector<QUuid> view_ids = m_views->get_all_view_ids();
        for (const QUuid& view_id: view_ids)
        {
            close_view(view_id);
        }
    }
}

/**
 * @brief Removes a file from every view and from the global file catalog.
 * @param file File metadata identifying the path to remove.
 */
auto ViewLifecycleCoordinator::remove_file(const LogFileInfo& file) -> void
{
    const QString file_path = file.get_file_path();
    QVector<QUuid> empty_view_ids;

    if (m_views != nullptr && !file_path.isEmpty())
    {
        const QVector<QUuid> view_ids = m_views->get_all_view_ids();
        for (const QUuid& view_id: view_ids)
        {
            const bool removed = remove_registered_file(view_id, file_path, false);
            if (removed && m_views->get_file_paths(view_id).isEmpty())
            {
                empty_view_ids.append(view_id);
            }
        }
    }

    if (m_catalog != nullptr)
    {
        m_catalog->remove_file(file);
    }

    for (const QUuid& view_id: empty_view_ids)
    {
        close_view(view_id);
    }
}

/**
 * @brief Removes one file registration from a specific view.
 * @param view_id View containing the file.
 * @param file_path Registered file path.
 * @return True when the file existed and was removed.
 */
auto ViewLifecycleCoordinator::remove_file(const QUuid& view_id, const QString& file_path) -> bool
{
    const bool removed = remove_registered_file(view_id, file_path, true);
    return removed;
}

/**
 * @brief Removes one registered file while optionally retaining an empty view temporarily.
 * @param view_id View containing the file.
 * @param file_path Registered file path.
 * @param close_empty_view Whether an emptied view is closed immediately.
 * @return True when the file existed and was removed.
 */
auto ViewLifecycleCoordinator::remove_registered_file(const QUuid& view_id,
                                                      const QString& file_path,
                                                      bool close_empty_view) -> bool
{
    const bool registered = m_views != nullptr && !view_id.isNull() && !file_path.isEmpty() &&
                            m_views->get_context(view_id) != nullptr &&
                            m_views->get_file_paths(view_id).contains(file_path);
    bool removed = false;

    if (registered)
    {
        if (m_live_tailing != nullptr)
        {
            m_live_tailing->stop_file(view_id, file_path);
        }

        const bool cleanup_queued =
            m_history_writer != nullptr && m_history_writer->discard_file(view_id, file_path);
        if (!cleanup_queued && m_history != nullptr)
        {
            m_history->remove_file_entries(view_id, file_path);
        }

        m_views->remove_entries_by_file(view_id, file_path);

        if (m_imports != nullptr)
        {
            m_imports->discard_file_cache_binding(view_id, file_path);
        }

        if (m_filters != nullptr)
        {
            m_filters->adjust_visibility_on_file_removed(view_id, file_path);
        }

        LogViewContext* context = m_views->get_context(view_id);
        if (context != nullptr)
        {
            context->remove_file_parsing_profile(file_path);
        }

        const bool view_is_empty = m_views->get_file_paths(view_id).isEmpty();
        if (view_is_empty && close_empty_view)
        {
            close_view(view_id);
        }
        else if (!view_is_empty && m_pages != nullptr && m_queries != nullptr &&
                 m_pages->get_page_state(view_id) != nullptr)
        {
            m_queries->reload_query(view_id);
        }

        removed = true;
    }

    return removed;
}
