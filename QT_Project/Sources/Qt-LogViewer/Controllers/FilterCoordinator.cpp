/**
 * @file FilterCoordinator.cpp
 * @brief Implements FilterCoordinator which delegates per-view filtering and visibility operations.
 */

#include "Qt-LogViewer/Controllers/FilterCoordinator.h"

#include <algorithm>

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
