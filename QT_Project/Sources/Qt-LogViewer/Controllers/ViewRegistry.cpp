/**
 * @file ViewRegistry.cpp
 * @brief Implements the ViewRegistry which manages LogViewContext lifetimes and view state.
 */

#include "Qt-LogViewer/Controllers/ViewRegistry.h"

#include <QDebug>
#include <algorithm>

// Concrete includes for forward-declared and used types
#include "Qt-LogViewer/Controllers/FilterCoordinator.h"
#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Models/LogSortFilterProxyModel.h"
#include "Qt-LogViewer/Models/PagingProxyModel.h"

/**
 * @brief Construct a new ViewRegistry.
 * @param parent Optional QObject parent for ownership.
 */
ViewRegistry::ViewRegistry(QObject* parent): QObject(parent), m_contexts(), m_current_view_id() {}

/**
 * @brief Destroy the ViewRegistry.
 *
 * Clear the internal map to avoid leaving stale pointers; owned QObjects are deleted by Qt.
 */
ViewRegistry::~ViewRegistry()
{
    m_contexts.clear();
    m_current_view_id = QUuid();
}

/**
 * @brief Create a new view context with a generated id and return it.
 * @return QUuid Created view id.
 */
auto ViewRegistry::create_view() -> QUuid
{
    QUuid view_id = QUuid::createUuid();
    auto* ctx = new LogViewContext(this);
    m_contexts[view_id] = ctx;
    return view_id;
}

/**
 * @brief Create a new view context under the given fixed id.
 * @param view_id The explicit id to use.
 * @return True if created; false if it already existed.
 */
auto ViewRegistry::create_view(const QUuid& view_id) -> bool
{
    bool created = false;

    if (!view_id.isNull() && !m_contexts.contains(view_id))
    {
        auto* ctx = new LogViewContext(this);
        m_contexts.insert(view_id, ctx);
        created = true;
    }

    return created;
}

/**
 * @brief Ensures a view exists for the given id; creates it if missing.
 * @param view_id The id to ensure.
 */
auto ViewRegistry::ensure_view(const QUuid& view_id) -> void
{
    if (!view_id.isNull() && !m_contexts.contains(view_id))
    {
        create_view(view_id);
    }
}

/**
 * @brief Remove a view context and free resources.
 * @param view_id View id to remove.
 * @return True if removed, false if not found.
 */
auto ViewRegistry::remove_view(const QUuid& view_id) -> bool
{
    bool removed = false;

    if (m_contexts.contains(view_id))
    {
        delete m_contexts[view_id];
        m_contexts.remove(view_id);
        removed = true;

        if (m_current_view_id == view_id)
        {
            m_current_view_id = QUuid();
        }
    }

    return removed;
}

/**
 * @brief Set the current view id, emit `current_view_id_changed` on success.
 * @param view_id View id to set as current.
 * @return True if view exists and was set, false otherwise.
 */
auto ViewRegistry::set_current_view(const QUuid& view_id) -> bool
{
    bool success = false;

    if (m_contexts.contains(view_id))
    {
        m_current_view_id = view_id;
        success = true;
        emit current_view_id_changed(m_current_view_id);
    }

    return success;
}

/**
 * @brief Returns the current view id (may be null).
 * @return QUuid Current view id.
 */
auto ViewRegistry::get_current_view() const -> QUuid
{
    return m_current_view_id;
}

/**
 * @brief Return `LogViewContext*` for the given view id or nullptr.
 * @param view_id View id to lookup.
 * @return LogViewContext* or nullptr.
 */
auto ViewRegistry::get_context(const QUuid& view_id) const -> LogViewContext*
{
    LogViewContext* ctx = nullptr;

    if (m_contexts.contains(view_id))
    {
        ctx = m_contexts[view_id];
    }

    return ctx;
}

/**
 * @brief Return all view ids currently registered.
 * @return QVector<QUuid> of view ids.
 */
auto ViewRegistry::get_all_view_ids() const -> QVector<QUuid>
{
    QVector<QUuid> ids;
    ids.reserve(m_contexts.size());

    for (auto it = m_contexts.constBegin(); it != m_contexts.constEnd(); ++it)
    {
        ids.append(it.key());
    }

    return ids;
}

/**
 * @brief Return all entries for the given view.
 * @param view_id View id to query.
 * @return QVector<LogEntry> Entries (empty if none / view missing).
 */
auto ViewRegistry::get_entries(const QUuid& view_id) const -> QVector<LogEntry>
{
    QVector<LogEntry> result;
    auto* ctx = get_context(view_id);

    if (ctx != nullptr)
    {
        result = ctx->get_entries();
    }

    return result;
}

/**
 * @brief Return loaded absolute file paths for the given view.
 * @param view_id View id to query.
 * @return QVector<QString> File paths (empty if none / view missing).
 */
auto ViewRegistry::get_file_paths(const QUuid& view_id) const -> QVector<QString>
{
    QVector<QString> result;
    auto* ctx = get_context(view_id);

    if (ctx != nullptr)
    {
        result = ctx->get_file_paths();
    }

    return result;
}

/**
 * @brief Replace the loaded-files list for a view and notify listeners.
 * @param view_id Target view id.
 * @param files List of LogFileInfo to set.
 */
auto ViewRegistry::set_loaded_files(const QUuid& view_id, const QList<LogFileInfo>& files) -> void
{
    auto* ctx = get_context(view_id);

    if (ctx != nullptr)
    {
        ctx->set_loaded_files(files);
        emit view_file_paths_changed(view_id, ctx->get_file_paths());
    }
}

/**
 * @brief Add a single loaded file to a view and notify listeners.
 * @param view_id Target view id.
 * @param file File info to add.
 */
auto ViewRegistry::add_loaded_file(const QUuid& view_id, const LogFileInfo& file) -> void
{
    auto* ctx = get_context(view_id);

    if (ctx != nullptr)
    {
        ctx->add_loaded_file(file);
        emit view_file_paths_changed(view_id, ctx->get_file_paths());
    }
}

/**
 * @brief Remove all entries for `file_path` from the specified view.
 *        Also removes the file from the view's loaded-file list and notifies listeners.
 * @param view_id Target view id.
 * @param file_path Absolute file path to remove.
 */
auto ViewRegistry::remove_entries_by_file(const QUuid& view_id, const QString& file_path) -> void
{
    auto* ctx = get_context(view_id);

    if (ctx != nullptr)
    {
        QList<LogFileInfo> files = ctx->get_loaded_files();
        files.erase(std::remove_if(files.begin(), files.end(),
                                   [&file_path](const LogFileInfo& info) {
                                       return info.get_file_path() == file_path;
                                   }),
                    files.end());

        ctx->set_loaded_files(files);
        ctx->remove_entries_by_file_path(file_path);
        emit view_file_paths_changed(view_id, ctx->get_file_paths());
    }
}

/**
 * @brief Export a view's state including loaded files, filters, paging and sort.
 * @param view_id Target view id.
 * @param filters Coordinator used to read current filter/visibility on the view.
 * @return SessionViewState snapshot, including a suggested tab title.
 */
auto ViewRegistry::export_view_state(const QUuid& view_id,
                                     const FilterCoordinator& filters) const -> SessionViewState
{
    SessionViewState state;
    state.id = view_id;

    auto* ctx = get_context(view_id);
    if (ctx != nullptr)
    {
        // Loaded files
        state.loaded_files = ctx->get_loaded_files();

        // Filters via coordinator
        const FilterState fs = filters.export_filters(view_id);
        state.filters = fs;

        // Paging
        auto* paging = ctx->get_paging_proxy();
        if (paging != nullptr)
        {
            state.page_size = paging->get_page_size();
            state.current_page = paging->get_current_page();
        }

        // Sort
        auto* sort_proxy = ctx->get_sort_proxy();
        if (sort_proxy != nullptr)
        {
            state.sort_column = sort_proxy->get_sort_column();
            state.sort_order = sort_proxy->get_sort_order();
        }

        // Tab title: first file name or composed from loaded files
        if (!state.loaded_files.isEmpty())
        {
            const QString first_name = state.loaded_files.first().get_file_name();
            state.tab_title = first_name;

            if (state.loaded_files.size() > 1)
            {
                state.tab_title =
                    QStringLiteral("%1 (+%2)").arg(first_name).arg(state.loaded_files.size() - 1);
            }
        }
    }

    return state;
}

/**
 * @brief Import a view state: ensure/create view, set loaded files, reapply filters, and
 * paging/sort.
 * @param state Serialized view state to apply.
 * @param filters Coordinator used to apply filters.
 * @return QUuid The (ensured) view id of the imported state.
 *
 * Round-trip guarantee:
 * Applying an exported `SessionViewState` reconstructs the same logical view configuration,
 * assuming the referenced files exist and are accessible on the current machine.
 */
auto ViewRegistry::import_view_state(const SessionViewState& state,
                                     FilterCoordinator& filters) -> QUuid
{
    QUuid view_id = state.id.isNull() ? create_view() : state.id;
    ensure_view(view_id);

    // Loaded files
    set_loaded_files(view_id, state.loaded_files);

    // Filters
    filters.import_filters(view_id, state.filters);

    // Paging and sort
    auto* ctx = get_context(view_id);
    if (ctx != nullptr)
    {
        auto* paging = ctx->get_paging_proxy();
        if (paging != nullptr)
        {
            if (state.page_size > 0)
            {
                paging->set_page_size(state.page_size);
            }
            if (state.current_page > 0)
            {
                paging->set_current_page(state.current_page);
            }
        }

        auto* sort_proxy = ctx->get_sort_proxy();
        if (sort_proxy != nullptr)
        {
            sort_proxy->sort(state.sort_column, state.sort_order);
        }
    }

    return view_id;
}
