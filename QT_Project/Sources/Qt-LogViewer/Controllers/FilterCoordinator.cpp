/**
 * @file FilterCoordinator.cpp
 * @brief Implements `FilterCoordinator` which delegates per-view filtering and file visibility
 * operations.
 *
 * This class forwards filter and visibility requests to the `LogSortFilterProxyModel` of a given
 * view and uses `ViewRegistry` to query current entries and file paths when computing counts or
 * performing visibility updates that depend on the view's loaded files.
 */

#include "Qt-LogViewer/Controllers/FilterCoordinator.h"

#include <algorithm>

#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Models/LogSortFilterProxyModel.h"

/**
 * @brief Construct a new FilterCoordinator.
 * @param views Non-owning pointer to the `ViewRegistry` used for context/proxy lookups.
 * @param parent Optional QObject parent.
 */
FilterCoordinator::FilterCoordinator(ViewRegistry* views, QObject* parent)
    : QObject(parent), m_views(views)
{}

/**
 * @brief Set application name filter for a specific view.
 * @param view_id Target view id.
 * @param app_name Application name to filter by.
 */
auto FilterCoordinator::set_app_name(const QUuid& view_id, const QString& app_name) -> void
{
    auto* proxy = get_sort_filter_proxy(view_id);
    if (proxy != nullptr)
    {
        proxy->set_app_name_filter(app_name);
    }
}

/**
 * @brief Set log level filters for a specific view.
 * @param view_id Target view id.
 * @param levels Set of log level names to filter by.
 */
auto FilterCoordinator::set_log_levels(const QUuid& view_id, const QSet<QString>& levels) -> void
{
    auto* proxy = get_sort_filter_proxy(view_id);
    if (proxy != nullptr)
    {
        proxy->set_log_level_filters(levels);
    }
}

/**
 * @brief Set search filter for a specific view.
 * @param view_id Target view id.
 * @param text Search text or regex.
 * @param field Field to search in.
 * @param use_regex Whether to treat text as a regex.
 */
auto FilterCoordinator::set_search(const QUuid& view_id, const QString& text, const QString& field,
                                   bool use_regex) -> void
{
    auto* proxy = get_sort_filter_proxy(view_id);
    if (proxy != nullptr)
    {
        proxy->set_search_filter(text, field, use_regex);
    }
}

/**
 * @brief Apply a "show only file" visibility filter for the specified view.
 *        Pass empty string to reset (show all).
 * @param view_id Target view id.
 * @param file_path File path to show exclusively, or empty to reset.
 */
auto FilterCoordinator::set_show_only(const QUuid& view_id, const QString& file_path) -> void
{
    auto* proxy = get_sort_filter_proxy(view_id);

    if (proxy != nullptr)
    {
        proxy->set_show_only_file_path(file_path);

        if (file_path.isEmpty())
        {
            proxy->clear_hidden_files();
        }
        else
        {
            proxy->unhide_file(file_path);
        }
    }
}

/**
 * @brief Toggle visibility of a file in the specified view (hide/unhide).
 *        Honors current "show only" rules exactly as in the facade logic.
 * @param view_id Target view id.
 * @param file_path Absolute file path to toggle.
 */
auto FilterCoordinator::toggle_visibility(const QUuid& view_id, const QString& file_path) -> void
{
    auto* proxy = get_sort_filter_proxy(view_id);

    const bool can_toggle = (proxy != nullptr) && !file_path.isEmpty();

    if (can_toggle)
    {
        const QString show_only = proxy->get_show_only_file_path();
        const QSet<QString> hidden = proxy->get_hidden_file_paths();
        const bool is_hidden = hidden.contains(file_path);
        const bool show_only_active = !show_only.isEmpty();
        const bool toggling_show_only_target = (show_only == file_path);

        if (!show_only_active)
        {
            // No show-only active: standard toggle for the requested file.
            if (is_hidden)
            {
                proxy->unhide_file(file_path);
            }
            else
            {
                proxy->hide_file(file_path);
            }
        }
        else
        {
            if (toggling_show_only_target)
            {
                // Toggling the current show-only file: clear show-only and hide all files (empty
                // view).
                proxy->set_show_only_file_path(QString());

                const QVector<QString> files = m_views->get_file_paths(view_id);
                QSet<QString> all_hidden;
                for (const auto& p: files)
                {
                    all_hidden.insert(p);
                }
                proxy->set_hidden_file_paths(all_hidden);
            }
            else
            {
                // Clear show-only, make requested file visible and convert effective-hidden to
                // explicit.
                proxy->set_show_only_file_path(QString());

                if (is_hidden)
                {
                    proxy->unhide_file(file_path);
                }

                const QVector<QString> files = m_views->get_file_paths(view_id);
                QSet<QString> new_hidden;

                for (const auto& p: files)
                {
                    const bool keep_visible = (p == show_only) || (p == file_path);
                    if (!keep_visible)
                    {
                        new_hidden.insert(p);
                    }
                }

                for (const auto& h: hidden)
                {
                    const bool exclude = (h == show_only) || (h == file_path);
                    if (!exclude)
                    {
                        new_hidden.insert(h);
                    }
                }

                proxy->set_hidden_file_paths(new_hidden);
            }
        }
    }
}

/**
 * @brief Hide a specific file in the specified view.
 * @param view_id Target view id.
 * @param file_path File path to hide.
 */
auto FilterCoordinator::hide_file(const QUuid& view_id, const QString& file_path) -> void
{
    auto* proxy = get_sort_filter_proxy(view_id);
    if (proxy != nullptr)
    {
        proxy->hide_file(file_path);
    }
}

/**
 * @brief Get current application name filter for a view.
 * @param view_id Target view id.
 * @return Application name filter string (empty if none).
 */
auto FilterCoordinator::get_app_name(const QUuid& view_id) const -> QString
{
    auto* proxy = get_sort_filter_proxy(view_id);
    QString filter = (proxy != nullptr) ? proxy->get_app_name_filter() : QString();
    return filter;
}

/**
 * @brief Get current log level filters for a view.
 * @param view_id Target view id.
 * @return Set of log level names.
 */
auto FilterCoordinator::get_log_levels(const QUuid& view_id) const -> QSet<QString>
{
    auto* proxy = get_sort_filter_proxy(view_id);
    QSet<QString> levels = (proxy != nullptr) ? proxy->get_log_level_filters() : QSet<QString>();
    return levels;
}

/**
 * @brief Get current search text for a view.
 * @param view_id Target view id.
 * @return Search text string.
 */
auto FilterCoordinator::get_search_text(const QUuid& view_id) const -> QString
{
    auto* proxy = get_sort_filter_proxy(view_id);
    QString text = (proxy != nullptr) ? proxy->get_search_text() : QString();
    return text;
}

/**
 * @brief Get current search field for a view.
 * @param view_id Target view id.
 * @return Search field string.
 */
auto FilterCoordinator::get_search_field(const QUuid& view_id) const -> QString
{
    auto* proxy = get_sort_filter_proxy(view_id);
    QString field = (proxy != nullptr) ? proxy->get_search_field() : QString();
    return field;
}

/**
 * @brief Get whether the current search uses regex for a view.
 * @param view_id Target view id.
 * @return True if using regex, false otherwise.
 */
auto FilterCoordinator::is_search_regex(const QUuid& view_id) const -> bool
{
    auto* proxy = get_sort_filter_proxy(view_id);
    bool regex = (proxy != nullptr) ? proxy->is_search_regex() : false;
    return regex;
}

/**
 * @brief Compute per-view log level counts using entries from `ViewRegistry`.
 * @param view_id Target view id.
 * @return Map of level name -> count.
 */
auto FilterCoordinator::get_log_level_counts(const QUuid& view_id) const -> QMap<QString, int>
{
    QVector<LogEntry> entries = m_views->get_entries(view_id);
    QMap<QString, int> level_counts;

    for (const auto& entry: entries)
    {
        level_counts[entry.get_level()]++;
    }

    return level_counts;
}

/**
 * @brief Static list of available log levels (same across all views).
 * @return Vector of log level names.
 */
auto FilterCoordinator::get_available_log_levels() -> QVector<QString>
{
    QVector<QString> log_levels;

    log_levels << "Trace"
               << "Debug"
               << "Info"
               << "Warning"
               << "Error"
               << "Fatal";

    return log_levels;
}

/**
 * @brief Adjust visibility state for a view when a file is removed.
 * @param view_id Target view id.
 * @param file_path Removed absolute file path.
 */
auto FilterCoordinator::adjust_visibility_on_file_removed(const QUuid& view_id,
                                                          const QString& file_path) -> void
{
    auto* proxy = get_sort_filter_proxy(view_id);
    if (proxy != nullptr)
    {
        const QString show_only_path = proxy->get_show_only_file_path();
        const bool removed_was_show_only = (show_only_path == file_path);

        if (removed_was_show_only)
        {
            proxy->set_show_only_file_path(QString());

            const QVector<QString> remaining = m_views->get_file_paths(view_id);
            QSet<QString> all_hidden;
            for (const auto& p: remaining)
            {
                all_hidden.insert(p);
            }
            proxy->set_hidden_file_paths(all_hidden);
        }
        else
        {
            QSet<QString> hidden = proxy->get_hidden_file_paths();
            if (hidden.contains(file_path))
            {
                hidden.remove(file_path);
                proxy->set_hidden_file_paths(hidden);
            }
        }
    }
}

/**
 * @brief Adjust visibility across all views when a file is removed globally.
 * @param file_path Removed absolute file path.
 */
auto FilterCoordinator::adjust_visibility_on_global_file_removed(const QString& file_path) -> void
{
    const QVector<QUuid> ids = m_views->get_all_view_ids();
    for (const QUuid& view_id: ids)
    {
        adjust_visibility_on_file_removed(view_id, file_path);
    }
}

/**
 * @brief Helper to access the sort/filter proxy for a given view.
 * @param view_id Target view id.
 * @return Pointer to LogSortFilterProxyModel or nullptr if the context is missing.
 */
auto FilterCoordinator::get_sort_filter_proxy(const QUuid& view_id) const
    -> LogSortFilterProxyModel*
{
    auto* ctx = m_views != nullptr ? m_views->get_context(view_id) : nullptr;
    LogSortFilterProxyModel* proxy = (ctx != nullptr) ? ctx->get_sort_proxy() : nullptr;
    return proxy;
}

/**
 * @brief Export the current filter and visibility state for `view_id`.
 * @param view_id Target view id.
 * @return FilterState snapshot that can be round-tripped via `import_filters()`.
 */
auto FilterCoordinator::export_filters(const QUuid& view_id) const -> FilterState
{
    FilterState state;

    auto* proxy = get_sort_filter_proxy(view_id);
    if (proxy != nullptr)
    {
        state.app_name = proxy->get_app_name_filter();
        state.log_levels = proxy->get_log_level_filters();
        state.search_text = proxy->get_search_text();
        state.search_field = proxy->get_search_field();
        state.use_regex = proxy->is_search_regex();
        state.show_only_file = proxy->get_show_only_file_path();
        state.hidden_files = proxy->get_hidden_file_paths();
    }

    return state;
}

/**
 * @brief Import (apply) a previously exported filter state onto `view_id`.
 * @param view_id Target view id.
 * @param state FilterState previously obtained via `export_filters()`.
 *
 * Round-trip guarantee:
 * Applying the exported state reconstructs the same effective filter/visibility
 * configuration, assuming the view's file set has not changed in-between.
 */
auto FilterCoordinator::import_filters(const QUuid& view_id, const FilterState& state) -> void
{
    auto* proxy = get_sort_filter_proxy(view_id);

    if (proxy != nullptr)
    {
        proxy->set_app_name_filter(state.app_name);
        proxy->set_log_level_filters(state.log_levels);
        proxy->set_search_filter(state.search_text, state.search_field, state.use_regex);

        // Visibility: show-only and hidden set
        proxy->set_show_only_file_path(state.show_only_file);
        proxy->set_hidden_file_paths(state.hidden_files);

        // If show-only is active, ensure the target is not hidden.
        if (!state.show_only_file.isEmpty())
        {
            proxy->unhide_file(state.show_only_file);
        }
    }
}
