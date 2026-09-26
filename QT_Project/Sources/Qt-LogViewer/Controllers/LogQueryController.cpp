/**
 * @file LogQueryController.cpp
 * @brief Implements complete filter, sorting, and paging update operations.
 */

#include "Qt-LogViewer/Controllers/LogQueryController.h"

#include <algorithm>

#include "Qt-LogViewer/Controllers/FilterCoordinator.h"
#include "Qt-LogViewer/Controllers/LogPageCoordinator.h"
#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogPageState.h"

namespace
{
/**
 * @brief Maps a search-field selection to stable log field identifiers.
 * @param search_field Search field selected by the user.
 * @return Fields included in text searching; empty means all searchable fields.
 */
[[nodiscard]] auto get_query_search_fields(SearchField search_field) -> QSet<QString>
{
    QSet<QString> fields;

    switch (search_field)
    {
    case SearchField::Message:
        fields.insert(LogField::Message);
        break;

    case SearchField::Level:
        fields.insert(LogField::Level);
        break;

    case SearchField::AppName:
        fields.insert(LogField::AppName);
        break;

    case SearchField::AllFields:
    case SearchField::Count:
        break;
    }

    return fields;
}
}  // namespace

/**
 * @brief Constructs a query controller from the existing state coordinators.
 * @param filters Coordinator that owns per-view filter state.
 * @param pages Coordinator that owns per-view paging state.
 * @param views Registry used to access view contexts and column definitions.
 * @param parent Optional QObject parent.
 */
LogQueryController::LogQueryController(FilterCoordinator* filters, LogPageCoordinator* pages,
                                       ViewRegistry* views, QObject* parent)
    : QObject(parent), m_filters(filters), m_pages(pages), m_views(views)
{}

/**
 * @brief Sets the application-name filter and reloads page one.
 * @param view_id Target view.
 * @param app_name Application name, or an empty string for all applications.
 * @return True when the updated query was loaded.
 */
auto LogQueryController::set_app_name(const QUuid& view_id, const QString& app_name) -> bool
{
    if (m_filters != nullptr)
    {
        m_filters->set_app_name(view_id, app_name);
    }

    return reload_query(view_id);
}

/**
 * @brief Sets the log-level filter and reloads page one.
 * @param view_id Target view.
 * @param levels Included log levels, or an empty set for all levels.
 * @return True when the updated query was loaded.
 */
auto LogQueryController::set_log_levels(const QUuid& view_id, const QSet<QString>& levels) -> bool
{
    if (m_filters != nullptr)
    {
        m_filters->set_log_levels(view_id, levels);
    }

    return reload_query(view_id);
}

/**
 * @brief Sets the text-search filter and reloads page one.
 * @param view_id Target view.
 * @param text Search text or regular expression.
 * @param field Field selected for searching.
 * @param use_regex Whether text is interpreted as a regular expression.
 * @return True when the updated query was loaded.
 */
auto LogQueryController::set_search(const QUuid& view_id, const QString& text, SearchField field,
                                    bool use_regex) -> bool
{
    if (m_filters != nullptr)
    {
        m_filters->set_search(view_id, text, field, use_regex);
    }

    return reload_query(view_id);
}

/**
 * @brief Applies a show-only-file filter and reloads page one.
 * @param view_id Target view.
 * @param file_path File to show exclusively, or an empty path to reset the filter.
 * @return True when the updated query was loaded.
 */
auto LogQueryController::set_show_only_file(const QUuid& view_id, const QString& file_path) -> bool
{
    if (m_filters != nullptr)
    {
        m_filters->set_show_only(view_id, file_path);
    }

    return reload_query(view_id);
}

/**
 * @brief Toggles one file's visibility and reloads page one.
 * @param view_id Target view.
 * @param file_path File whose visibility changes.
 * @return True when the updated query was loaded.
 */
auto LogQueryController::toggle_file_visibility(const QUuid& view_id,
                                                const QString& file_path) -> bool
{
    if (m_filters != nullptr)
    {
        m_filters->toggle_visibility(view_id, file_path);
    }

    return reload_query(view_id);
}

/**
 * @brief Hides one file and reloads page one.
 * @param view_id Target view.
 * @param file_path File to hide.
 * @return True when the updated query was loaded.
 */
auto LogQueryController::hide_file(const QUuid& view_id, const QString& file_path) -> bool
{
    if (m_filters != nullptr)
    {
        m_filters->hide_file(view_id, file_path);
    }

    return reload_query(view_id);
}

/**
 * @brief Changes sorting and reloads page one.
 * @param view_id Target view.
 * @param column Sortable model column.
 * @param order Requested sort direction.
 * @return True when the column is sortable and the query was loaded.
 */
auto LogQueryController::set_sort(const QUuid& view_id, int column, Qt::SortOrder order) -> bool
{
    bool sorted = false;
    LogViewContext* context = m_views != nullptr ? m_views->get_context(view_id) : nullptr;
    LogModel* model = context != nullptr ? context->get_model() : nullptr;

    if (model != nullptr && model->is_column_sortable(column) && m_pages != nullptr)
    {
        LogQuery query = create_query(view_id);
        query.sort_field = model->get_column_field_id(column);
        query.sort_order = order;
        sorted = m_pages->set_query(view_id, query);
    }

    return sorted;
}

/**
 * @brief Rebuilds the query from current filter state and reloads page one.
 * @param view_id Target view.
 * @return True when the query was loaded.
 */
auto LogQueryController::reload_query(const QUuid& view_id) -> bool
{
    return m_pages != nullptr && m_pages->set_query(view_id, create_query(view_id));
}

/**
 * @brief Refreshes a view after new entries were appended to its stored history.
 * @param view_id Target view.
 * @return True when the current page or its totals were refreshed.
 */
auto LogQueryController::refresh_after_entries_appended(const QUuid& view_id) -> bool
{
    const LogPageState* page_state =
        m_pages != nullptr ? m_pages->get_page_state(view_id) : nullptr;
    bool refreshed = false;

    if (page_state != nullptr)
    {
        if (page_state->get_current_page() == 1)
        {
            refreshed = m_pages->reload(view_id);
        }
        else
        {
            refreshed = m_pages->refresh_total_entries(view_id);
        }
    }

    return refreshed;
}

/**
 * @brief Applies filters, sorting, page size, and page number with one reload.
 * @param view_id Target view.
 * @param state Complete saved state of the view.
 * @return True when the complete state was applied and loaded.
 */
auto LogQueryController::apply_view_state(const QUuid& view_id,
                                          const SessionViewState& state) -> bool
{
    bool applied = false;
    LogViewContext* context = m_views != nullptr ? m_views->get_context(view_id) : nullptr;
    LogModel* model = context != nullptr ? context->get_model() : nullptr;

    if (m_filters != nullptr && m_pages != nullptr && model != nullptr)
    {
        m_filters->import_filters(view_id, state.filters);

        LogQuery query = create_query(view_id);
        if (model->is_column_sortable(state.sort_column))
        {
            query.sort_field = model->get_column_field_id(state.sort_column);
            query.sort_order = state.sort_order;
        }

        const qsizetype page_size = state.page_size > 0 ? state.page_size : 25;
        const qsizetype current_page = std::max(1, state.current_page);
        applied = m_pages->apply_state(view_id, query, page_size, current_page);
    }

    return applied;
}

/**
 * @brief Builds a query from the current filter and sorting state.
 * @param view_id Source view.
 * @return Query representing the view state.
 */
auto LogQueryController::create_query(const QUuid& view_id) const -> LogQuery
{
    LogQuery query;
    const LogViewContext* context = m_views != nullptr ? m_views->get_context(view_id) : nullptr;

    if (context != nullptr && m_filters != nullptr)
    {
        const FilterState filters = m_filters->export_filters(view_id);

        query.view_id = view_id;
        query.app_name = filters.app_name;
        query.log_levels = filters.log_levels;
        query.search_text = filters.search_text;
        query.search_fields = get_query_search_fields(filters.search_field);
        query.use_regex = filters.use_regex;
        query.show_only_file = filters.show_only_file;
        query.hidden_files = filters.hidden_files;

        const LogPageState* page_state =
            m_pages != nullptr ? m_pages->get_page_state(view_id) : nullptr;
        if (page_state != nullptr)
        {
            query.sort_field = page_state->get_query().sort_field;
            query.sort_order = page_state->get_query().sort_order;
        }
    }

    return query;
}
