/**
 * @file LogPageCoordinator.cpp
 * @brief Implements the coordinator for database-backed log pages.
 */

#include "Qt-LogViewer/Controllers/LogPageCoordinator.h"

#include <QThread>
#include <functional>
#include <utility>

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
        connect(m_views, &ViewRegistry::view_removed, this, [this](const QUuid& view_id) {
            cancel_regex_query(view_id);
            m_page_states.remove(view_id);
            m_request_revisions.remove(view_id);
        });
    }
}

/** @brief Cancels and joins active regular-expression queries. */
LogPageCoordinator::~LogPageCoordinator()
{
    for (auto iterator = m_regex_cancellations.cbegin(); iterator != m_regex_cancellations.cend();
         ++iterator)
    {
        iterator.value()->store(true);
    }
    const QSet<QThread*> workers = m_regex_workers;
    m_regex_workers.clear();
    for (QThread* worker: workers)
    {
        worker->wait();
        delete worker;
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
 * @brief Changes result ordering and reloads page one without recounting unchanged rows.
 * @param view_id Target view.
 * @param sort_field Stable identifier of the field used for sorting.
 * @param sort_order Requested sort direction.
 * @return True when page state exists for the view.
 */
auto LogPageCoordinator::set_sorting(const QUuid& view_id, const QString& sort_field,
                                     Qt::SortOrder sort_order) -> bool
{
    bool updated = false;
    auto state = m_page_states.find(view_id);

    if (state != m_page_states.end())
    {
        state->set_sorting(sort_field, sort_order);
        updated = load_current_page(view_id);
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
        updated = load_current_page(view_id);
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
        updated = load_current_page(view_id);
    }

    return updated;
}

/**
 * @brief Applies query, page size, and requested page with one model reload.
 * @param view_id Target view.
 * @param query Query describing filtering and sorting.
 * @param page_size Positive number of entries per page.
 * @param current_page Requested one-based page number.
 * @return True when the complete state was applied and loaded.
 */
auto LogPageCoordinator::apply_state(const QUuid& view_id, const LogQuery& query,
                                     qsizetype page_size, qsizetype current_page) -> bool
{
    bool loaded = false;
    LogViewContext* context = m_views != nullptr ? m_views->get_context(view_id) : nullptr;

    if (m_history_service != nullptr && context != nullptr && page_size > 0 && current_page > 0)
    {
        LogQuery view_query = query;
        view_query.view_id = view_id;

        LogPageState& state = m_page_states[view_id];
        state.set_query(view_query);
        state.set_page_size(page_size);
        const bool regex_query =
            state.get_query().use_regex && !state.get_query().search_text.trimmed().isEmpty();
        if (regex_query)
        {
            state.set_total_entries(current_page * page_size);
            state.set_current_page(current_page);
            loaded = load_regex_page_async(view_id, true, false);
        }
        else
        {
            cancel_regex_query(view_id);
            const qsizetype requested_page = std::max<qsizetype>(1, current_page);
            const qsizetype requested_offset = (requested_page - 1) * page_size;
            LogPageResult result = m_history_service->load_entries_page_result(
                state.get_query(), requested_offset, state.get_page_size());
            state.set_total_entries(result.total_entries);
            state.set_current_page(requested_page);
            if (state.get_offset() != requested_offset)
            {
                result.entries = m_history_service->load_entries_page(
                    state.get_query(), state.get_offset(), state.get_page_size());
            }
            m_history_service->remember_page_result(state.get_query(), result);
            context->replace_entries(result.entries);

            emit page_loaded(view_id, state.get_current_page(), state.get_total_pages(),
                             state.get_total_entries());
            emit result_set_changed(view_id);
            loaded = true;
        }
    }

    return loaded;
}

/**
 * @brief Installs page state without querying storage or populating the model.
 * @param view_id Target view.
 * @param query Query describing filtering and sorting.
 * @param page_size Positive number of entries per page.
 * @param current_page Requested one-based page number.
 * @return True when empty deferred state was installed for the view.
 */
auto LogPageCoordinator::prepare_state(const QUuid& view_id, const LogQuery& query,
                                       qsizetype page_size, qsizetype current_page) -> bool
{
    LogViewContext* context = m_views != nullptr ? m_views->get_context(view_id) : nullptr;
    const bool prepared = context != nullptr && page_size > 0 && current_page > 0;
    if (prepared)
    {
        LogQuery view_query = query;
        view_query.view_id = view_id;

        LogPageState& state = m_page_states[view_id];
        state.set_query(view_query);
        state.set_page_size(page_size);
        state.set_total_entries(0);
        state.set_current_page(current_page);
        context->replace_entries(QVector<LogEntry>());
    }
    return prepared;
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
            const bool regex_query =
                state->get_query().use_regex && !state->get_query().search_text.trimmed().isEmpty();
            if (regex_query)
            {
                loaded = load_regex_page_async(view_id, true, false);
            }
            else
            {
                cancel_regex_query(view_id);
                const qsizetype requested_offset = state->get_offset();
                LogPageResult result = m_history_service->load_entries_page_result(
                    state->get_query(), requested_offset, state->get_page_size());
                state->set_total_entries(result.total_entries);
                if (state->get_offset() != requested_offset)
                {
                    result.entries = m_history_service->load_entries_page(
                        state->get_query(), state->get_offset(), state->get_page_size());
                }
                m_history_service->remember_page_result(state->get_query(), result);
                context->replace_entries(result.entries);
                emit page_loaded(view_id, state->get_current_page(), state->get_total_pages(),
                                 state->get_total_entries());
                emit result_set_changed(view_id);
                loaded = true;
            }
        }
    }

    return loaded;
}

/**
 * @brief Loads the selected page without recounting an unchanged query.
 * @param view_id Target view.
 * @return True when the view and its page state are available.
 */
auto LogPageCoordinator::load_current_page(const QUuid& view_id) -> bool
{
    bool loaded = false;
    if (m_history_service != nullptr && m_views != nullptr)
    {
        auto state = m_page_states.find(view_id);
        LogViewContext* context = m_views->get_context(view_id);
        if (state != m_page_states.end() && context != nullptr)
        {
            const bool regex_query =
                state->get_query().use_regex && !state->get_query().search_text.trimmed().isEmpty();
            if (regex_query)
            {
                loaded = load_regex_page_async(view_id, false, false);
            }
            else
            {
                cancel_regex_query(view_id);
                const QVector<LogEntry> entries = m_history_service->load_entries_page(
                    state->get_query(), state->get_offset(), state->get_page_size());
                context->replace_entries(entries);
                emit page_loaded(view_id, state->get_current_page(), state->get_total_pages(),
                                 state->get_total_entries());
                loaded = true;
            }
        }
    }
    return loaded;
}

/**
 * @brief Refreshes the result count without replacing the visible page.
 * @param view_id Target view.
 * @return True when the view and its page state are available.
 */
auto LogPageCoordinator::refresh_total_entries(const QUuid& view_id) -> bool
{
    bool updated = false;

    if (m_history_service != nullptr && m_views != nullptr)
    {
        auto state = m_page_states.find(view_id);
        LogViewContext* context = m_views->get_context(view_id);

        if (state != m_page_states.end() && context != nullptr)
        {
            const bool regex_query =
                state->get_query().use_regex && !state->get_query().search_text.trimmed().isEmpty();
            if (regex_query)
            {
                updated = load_regex_page_async(view_id, true, true);
            }
            else
            {
                cancel_regex_query(view_id);
                const qsizetype total_entries =
                    m_history_service->count_entries(state->get_query());
                state->set_total_entries(total_entries);

                emit page_state_updated(view_id, state->get_current_page(),
                                        state->get_total_pages(), state->get_total_entries());
                emit result_set_changed(view_id);
                updated = true;
            }
        }
    }

    return updated;
}

/**
 * @brief Starts a replaceable background regular-expression page query.
 * @param view_id Target view.
 * @param notify_result_set_changed True when aggregates changed with the query.
 * @param preserve_visible_entries True when only pagination totals may be updated.
 * @return True when a background operation was started.
 */
auto LogPageCoordinator::load_regex_page_async(const QUuid& view_id, bool notify_result_set_changed,
                                               bool preserve_visible_entries) -> bool
{
    bool started = false;
    auto state = m_page_states.find(view_id);
    LogViewContext* context = m_views != nullptr ? m_views->get_context(view_id) : nullptr;
    if (state != m_page_states.end() && context != nullptr && m_history_service != nullptr)
    {
        cancel_regex_query(view_id);
        const auto cancelled = std::make_shared<std::atomic_bool>(false);
        const LogQuery query = state->get_query();
        const qsizetype requested_offset = state->get_offset();
        const qsizetype page_size = state->get_page_size();
        std::function<LogPageResult()> task = m_history_service->create_regex_page_task(
            query, requested_offset, page_size, cancelled);
        if (task)
        {
            const quint64 revision = m_request_revisions.value(view_id, 0) + 1;
            m_request_revisions.insert(view_id, revision);
            m_regex_cancellations.insert(view_id, cancelled);
            const auto result = std::make_shared<LogPageResult>();
            QThread* worker =
                QThread::create([task = std::move(task), result] { *result = task(); });
            m_regex_workers.insert(worker);
            connect(worker, &QThread::finished, this,
                    [this, worker, view_id, query, requested_offset, revision, cancelled, result,
                     notify_result_set_changed, preserve_visible_entries] {
                        m_regex_workers.remove(worker);
                        const bool current = !cancelled->load() &&
                                             m_request_revisions.value(view_id, 0) == revision &&
                                             m_regex_cancellations.value(view_id) == cancelled;
                        if (current)
                        {
                            m_regex_cancellations.remove(view_id);
                            auto page_state = m_page_states.find(view_id);
                            LogViewContext* view_context =
                                m_views != nullptr ? m_views->get_context(view_id) : nullptr;
                            if (page_state != m_page_states.end() && view_context != nullptr)
                            {
                                page_state->set_total_entries(result->total_entries);
                                const bool same_page = page_state->get_offset() == requested_offset;
                                if (same_page)
                                {
                                    if (preserve_visible_entries)
                                    {
                                        emit page_state_updated(view_id,
                                                                page_state->get_current_page(),
                                                                page_state->get_total_pages(),
                                                                page_state->get_total_entries());
                                        emit result_set_changed(view_id);
                                    }
                                    else
                                    {
                                        m_history_service->remember_page_result(query, *result);
                                        view_context->replace_entries(result->entries);
                                        emit page_loaded(view_id, page_state->get_current_page(),
                                                         page_state->get_total_pages(),
                                                         page_state->get_total_entries());
                                        if (notify_result_set_changed)
                                        {
                                            emit result_set_changed(view_id);
                                        }
                                    }
                                }
                                else
                                {
                                    load_regex_page_async(view_id, notify_result_set_changed,
                                                          preserve_visible_entries);
                                }
                            }
                        }
                        worker->deleteLater();
                    });
            worker->start();
            started = true;
        }
    }
    return started;
}

/**
 * @brief Cooperatively cancels the active regular-expression query for a view.
 * @param view_id Target view.
 */
auto LogPageCoordinator::cancel_regex_query(const QUuid& view_id) -> void
{
    const auto cancellation = m_regex_cancellations.find(view_id);
    if (cancellation != m_regex_cancellations.end())
    {
        cancellation.value()->store(true);
        m_regex_cancellations.erase(cancellation);
    }
    m_request_revisions.insert(view_id, m_request_revisions.value(view_id, 0) + 1);
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
