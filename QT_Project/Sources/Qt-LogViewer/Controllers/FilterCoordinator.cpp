/**
 * @file FilterCoordinator.cpp
 * @brief Implements per-view filter and file-visibility state management.
 */

#include "Qt-LogViewer/Controllers/FilterCoordinator.h"

#include <algorithm>

#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Models/LogEntry.h"

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
    LogViewContext* context = get_context(view_id);

    if (context != nullptr)
    {
        FilterState state = context->get_filter_state();
        state.app_name = app_name;
        context->set_filter_state(state);
    }
}

/**
 * @brief Set log level filters for a specific view.
 * @param view_id Target view id.
 * @param levels Set of log level names to filter by.
 */
auto FilterCoordinator::set_log_levels(const QUuid& view_id, const QSet<QString>& levels) -> void
{
    LogViewContext* context = get_context(view_id);

    if (context != nullptr)
    {
        FilterState state = context->get_filter_state();
        state.log_levels = normalize_log_levels(levels);
        context->set_filter_state(state);
    }
}

/**
 * @brief Set search filter for a specific view.
 * @param view_id Target view id.
 * @param text Search text or regex.
 * @param field Field to search in.
 * @param use_regex Whether to treat text as a regex.
 */
auto FilterCoordinator::set_search(const QUuid& view_id, const QString& text, SearchField field,
                                   bool use_regex) -> void
{
    LogViewContext* context = get_context(view_id);

    if (context != nullptr)
    {
        FilterState state = context->get_filter_state();
        state.search_text = text;
        state.search_field = field;
        state.use_regex = use_regex;
        context->set_filter_state(state);
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
    LogViewContext* context = get_context(view_id);

    if (context != nullptr)
    {
        FilterState state = context->get_filter_state();
        state.show_only_file = file_path;

        if (file_path.isEmpty())
        {
            state.hidden_files.clear();
        }
        else
        {
            state.hidden_files.remove(file_path);
        }

        context->set_filter_state(state);
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
    LogViewContext* context = get_context(view_id);
    const bool can_toggle = context != nullptr && !file_path.isEmpty();

    if (can_toggle)
    {
        FilterState state = context->get_filter_state();

        const QString show_only = state.show_only_file;
        const QSet<QString> hidden = state.hidden_files;
        const bool is_hidden = hidden.contains(file_path);
        const bool show_only_active = !show_only.isEmpty();
        const bool toggling_show_only_target = show_only == file_path;

        if (!show_only_active)
        {
            if (is_hidden)
            {
                state.hidden_files.remove(file_path);
            }
            else
            {
                state.hidden_files.insert(file_path);
            }
        }
        else if (toggling_show_only_target)
        {
            state.show_only_file.clear();
            state.hidden_files.clear();

            const QVector<QString> files = m_views->get_file_paths(view_id);

            for (const QString& path: files)
            {
                state.hidden_files.insert(path);
            }
        }
        else
        {
            state.show_only_file.clear();
            state.hidden_files.clear();

            const QVector<QString> files = m_views->get_file_paths(view_id);

            for (const QString& path: files)
            {
                const bool keep_visible = path == show_only || path == file_path;

                if (!keep_visible)
                {
                    state.hidden_files.insert(path);
                }
            }

            for (const QString& hidden_path: hidden)
            {
                const bool keep_visible = hidden_path == show_only || hidden_path == file_path;

                if (!keep_visible)
                {
                    state.hidden_files.insert(hidden_path);
                }
            }
        }

        context->set_filter_state(state);
    }
}

/**
 * @brief Hide a specific file in the specified view.
 * @param view_id Target view id.
 * @param file_path File path to hide.
 */
auto FilterCoordinator::hide_file(const QUuid& view_id, const QString& file_path) -> void
{
    LogViewContext* context = get_context(view_id);

    if (context != nullptr && !file_path.isEmpty())
    {
        FilterState state = context->get_filter_state();
        state.hidden_files.insert(file_path);
        context->set_filter_state(state);
    }
}

/**
 * @brief Get current application name filter for a view.
 * @param view_id Target view id.
 * @return Application name filter string (empty if none).
 */
auto FilterCoordinator::get_app_name(const QUuid& view_id) const -> QString
{
    const LogViewContext* context = get_context(view_id);
    const QString result = context != nullptr ? context->get_filter_state().app_name : QString();

    return result;
}

/**
 * @brief Get current log level filters for a view.
 * @param view_id Target view id.
 * @return Set of log level names.
 */
auto FilterCoordinator::get_log_levels(const QUuid& view_id) const -> QSet<QString>
{
    const LogViewContext* context = m_views != nullptr ? m_views->get_context(view_id) : nullptr;
    const QSet<QString> result =
        context != nullptr ? context->get_filter_state().log_levels : QSet<QString>();

    return result;
}

/**
 * @brief Get current search text for a view.
 * @param view_id Target view id.
 * @return Search text string.
 */
auto FilterCoordinator::get_search_text(const QUuid& view_id) const -> QString
{
    const LogViewContext* context = m_views != nullptr ? m_views->get_context(view_id) : nullptr;
    const QString result = context != nullptr ? context->get_filter_state().search_text : QString();

    return result;
}

/**
 * @brief Get current search field for a view.
 * @param view_id Target view id.
 * @return Search field.
 */
auto FilterCoordinator::get_search_field(const QUuid& view_id) const -> SearchField
{
    const LogViewContext* context = m_views != nullptr ? m_views->get_context(view_id) : nullptr;
    const SearchField result =
        context != nullptr ? context->get_filter_state().search_field : SearchField::AllFields;

    return result;
}

/**
 * @brief Get whether the current search uses regex for a view.
 * @param view_id Target view id.
 * @return True if using regex, false otherwise.
 */
auto FilterCoordinator::is_search_regex(const QUuid& view_id) const -> bool
{
    const LogViewContext* context = m_views != nullptr ? m_views->get_context(view_id) : nullptr;
    const bool result = context != nullptr && context->get_filter_state().use_regex;

    return result;
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
    LogViewContext* context = get_context(view_id);

    if (context != nullptr)
    {
        FilterState state = context->get_filter_state();

        if (state.show_only_file == file_path)
        {
            state.show_only_file.clear();
            state.hidden_files.clear();

            const QVector<QString> remaining_files = m_views->get_file_paths(view_id);

            for (const QString& remaining_file: remaining_files)
            {
                state.hidden_files.insert(remaining_file);
            }
        }
        else
        {
            state.hidden_files.remove(file_path);
        }

        context->set_filter_state(state);
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
 * @brief Export the current filter and visibility state for `view_id`.
 * @param view_id Target view id.
 * @return FilterState snapshot that can be round-tripped via `import_filters()`.
 */
auto FilterCoordinator::export_filters(const QUuid& view_id) const -> FilterState
{
    const LogViewContext* context = m_views != nullptr ? m_views->get_context(view_id) : nullptr;
    const FilterState state = context != nullptr ? context->get_filter_state() : FilterState();

    return state;
}

/**
 * @brief Returns the context of the requested view.
 * @param view_id Target view.
 * @return View context or nullptr if the view does not exist.
 */
auto FilterCoordinator::get_context(const QUuid& view_id) const -> LogViewContext*
{
    LogViewContext* context = m_views != nullptr ? m_views->get_context(view_id) : nullptr;

    return context;
}

/**
 * @brief Normalizes log-level names for query comparisons.
 * @param levels Log-level names to normalize.
 * @return Trimmed, lowercase log-level names.
 */
auto FilterCoordinator::normalize_log_levels(const QSet<QString>& levels) -> QSet<QString>
{
    QSet<QString> normalized_levels;

    for (const QString& level: levels)
    {
        normalized_levels.insert(level.trimmed().toLower());
    }

    return normalized_levels;
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
    LogViewContext* context = get_context(view_id);

    if (context != nullptr)
    {
        FilterState imported_state = state;
        imported_state.log_levels = normalize_log_levels(imported_state.log_levels);

        if (!imported_state.show_only_file.isEmpty())
        {
            imported_state.hidden_files.remove(imported_state.show_only_file);
        }

        context->set_filter_state(imported_state);
    }
}
