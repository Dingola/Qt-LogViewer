/**
 * @file LogPageCoordinator.cpp
 * @brief Implements the coordinator for database-backed log pages.
 */

#include "Qt-LogViewer/Controllers/LogPageCoordinator.h"

#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Services/LogHistoryService.h"

/**
 * @brief Constructs a page coordinator.
 * @param history_service Service used to query archived entries.
 * @param views Registry containing the target view contexts.
 * @param parent Optional QObject parent.
 */
LogPageCoordinator::LogPageCoordinator(LogHistoryService* history_service, ViewRegistry* views,
                                       QObject* parent)
    : QObject(parent), m_history_service(history_service), m_views(views)
{
    if (m_views != nullptr)
    {
        connect(m_views, &ViewRegistry::view_removed, this,
                [this](const QUuid& view_id) { m_page_states.remove(view_id); });
    }
}

/**
 * @brief Sets the query for a view and loads its first page.
 * @param view_id Target view.
 * @param query Query describing the result set.
 * @return True when the view and required services are available.
 */
auto LogPageCoordinator::set_query(const QUuid& view_id, const LogQuery& query) -> bool
{
    bool updated = false;

    if (m_history_service != nullptr && m_views != nullptr &&
        m_views->get_context(view_id) != nullptr)
    {
        LogQuery view_query = query;
        view_query.view_id = view_id;

        m_page_states[view_id].set_query(view_query);
        updated = reload(view_id);
    }

    return updated;
}

/**
 * @brief Sets the current page and reloads the view model.
 * @param view_id Target view.
 * @param page One-based page number.
 * @return True when page state exists for the view.
 */
auto LogPageCoordinator::set_current_page(const QUuid& view_id, qsizetype page) -> bool
{
    bool updated = false;
    auto state = m_page_states.find(view_id);

    if (state != m_page_states.end())
    {
        state->set_current_page(page);
        updated = reload(view_id);
    }

    return updated;
}

/**
 * @brief Sets the page size, selects page one, and reloads the view model.
 * @param view_id Target view.
 * @param page_size Positive number of entries per page.
 * @return True when page state exists and page_size is valid.
 */
auto LogPageCoordinator::set_page_size(const QUuid& view_id, qsizetype page_size) -> bool
{
    bool updated = false;
    auto state = m_page_states.find(view_id);

    if (state != m_page_states.end() && page_size > 0)
    {
        state->set_page_size(page_size);
        state->set_current_page(1);
        updated = reload(view_id);
    }

    return updated;
}

/**
 * @brief Reloads the current page for a view.
 * @param view_id Target view.
 * @return True when the view and its page state are available.
 */
auto LogPageCoordinator::reload(const QUuid& view_id) -> bool
{
    bool loaded = false;

    if (m_history_service != nullptr && m_views != nullptr)
    {
        auto state = m_page_states.find(view_id);
        LogViewContext* context = m_views->get_context(view_id);

        if (state != m_page_states.end() && context != nullptr)
        {
            const qsizetype total_entries = m_history_service->count_entries(state->get_query());

            state->set_total_entries(total_entries);

            const QVector<LogEntry> entries = m_history_service->load_entries_page(
                state->get_query(), state->get_offset(), state->get_page_size());

            context->replace_entries(entries);

            emit page_loaded(view_id, state->get_current_page(), state->get_total_pages(),
                             state->get_total_entries());

            loaded = true;
        }
    }

    return loaded;
}

/**
 * @brief Returns the page state belonging to a view.
 * @param view_id Target view.
 * @return Page state, or nullptr when no query has been assigned.
 */
auto LogPageCoordinator::get_page_state(const QUuid& view_id) const -> const LogPageState*
{
    const LogPageState* state = nullptr;
    auto iterator = m_page_states.constFind(view_id);

    if (iterator != m_page_states.cend())
    {
        state = &iterator.value();
    }

    return state;
}
