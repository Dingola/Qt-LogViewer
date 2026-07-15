/**
 * @file LogViewerController.cpp
 * @brief Implements the LogViewerController which orchestrates log loading, view contexts,
 * filtering, and streaming.
 */

#include "Qt-LogViewer/Controllers/LogViewerController.h"

#include <QDebug>
#include <algorithm>

// Concrete includes for forward-declared types used in implementation
#include "Qt-LogViewer/Controllers/FileCatalogController.h"
#include "Qt-LogViewer/Controllers/FilterCoordinator.h"
#include "Qt-LogViewer/Controllers/LogIngestController.h"
#include "Qt-LogViewer/Controllers/LogPageCoordinator.h"
#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Models/LogFileTreeModel.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogSortFilterProxyModel.h"
#include "Qt-LogViewer/Models/PagingProxyModel.h"
#include "Qt-LogViewer/Services/LogHistoryService.h"
#include "Qt-LogViewer/Services/LogLoader.h"
#include "Qt-LogViewer/Services/LogLoadingService.h"
#include "Qt-LogViewer/Services/LogTailerService.h"

namespace
{
/**
 * @brief Maps the current search-field selection to log field identifiers.
 * @param search_field Current search-field selection.
 * @return Fields included in text searching.
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

/**
 * @brief Maps a LogModel column to a stable log field identifier.
 * @param column LogModel column index.
 * @return Stable field identifier.
 */
[[nodiscard]] auto get_query_sort_field(int column) -> QString
{
    QString field = LogField::Timestamp;

    switch (column)
    {
    case LogModel::Timestamp:
        field = LogField::Timestamp;
        break;

    case LogModel::Level:
        field = LogField::Level;
        break;

    case LogModel::Message:
        field = LogField::Message;
        break;

    case LogModel::AppName:
        field = LogField::AppName;
        break;

    case LogModel::Spacer:
    case LogModel::ColumnCount:
    default:
        break;
    }

    return field;
}
}  // namespace

/**
 * @brief Constructs a LogViewerController.
 * @param log_format The log format string for parsing.
 * @param parent The parent QObject.
 */
LogViewerController::LogViewerController(const QString& log_format, QObject* parent)
    : QObject(parent),
      m_is_shutting_down(false),
      m_ingest(new LogIngestController(log_format, this)),
      m_catalog(new FileCatalogController(m_ingest, this)),
      m_views(new ViewRegistry(this)),
      m_filters(new FilterCoordinator(m_views, this))
{
    // Initialize services
    m_history_service = new LogHistoryService(this);
    m_page_coordinator = new LogPageCoordinator(m_history_service, m_views, this);
    m_tailer_service = new LogTailerService(log_format, this);

    connect(m_page_coordinator, &LogPageCoordinator::page_loaded, this,
            [this](const QUuid& view_id, qsizetype current_page, qsizetype total_pages,
                   qsizetype total_entries) {
                emit page_loaded(view_id, current_page, total_pages, total_entries);
            });

    connect(m_tailer_service, &LogTailerService::entries_available, this,
            [this](const QUuid& view_id, const QString&, const QVector<LogEntry>& entries) {
                if (!m_is_shutting_down)
                {
                    LogViewContext* context = m_views->get_context(view_id);

                    if (context != nullptr)
                    {
                        m_history_service->add_entries(view_id, entries);
                        context->append_entries(entries);
                    }
                }
            });

    connect(m_views, &ViewRegistry::current_view_id_changed, this,
            [this](const QUuid& view_id) { emit current_view_id_changed(view_id); });
    connect(m_views, &ViewRegistry::view_removed, this,
            [this](const QUuid& view_id) { emit view_removed(view_id); });
    connect(m_views, &ViewRegistry::view_file_paths_changed, this,
            [this](const QUuid& view_id, const QVector<QString>& paths) {
                emit view_file_paths_changed(view_id, paths);
            });

    // Batch parsed: append to the active view context.
    connect(m_ingest, &LogIngestController::entry_batch_parsed, this,
            [this](const QUuid& view_id, const QString& file_path, const QVector<LogEntry>& batch) {
                if (!m_is_shutting_down)
                {
                    qDebug().nospace() << "[Controller] batch for view=" << view_id.toString()
                                       << " file=\"" << file_path << "\" count=" << batch.size();

                    if (!view_id.isNull())
                    {
                        auto* ctx = m_views->get_context(view_id);
                        if (ctx != nullptr)
                        {
                            m_history_service->add_entries(view_id, batch);
                            ctx->append_entries(batch);
                        }
                    }
                }
            });

    // Progress pass-through.
    connect(m_ingest, &LogIngestController::progress, this,
            [this](const QUuid& view_id, const QString& file_path, qint64 bytes_read,
                   qint64 total_bytes) {
                if (!m_is_shutting_down)
                {
                    qDebug().nospace()
                        << "[Controller] progress view=" << view_id.toString() << " file=\""
                        << file_path << "\" " << bytes_read << '/' << total_bytes;

                    if (!view_id.isNull())
                    {
                        emit loading_progress(view_id, bytes_read, total_bytes);
                    }
                }
            });

    // Error pass-through.
    connect(m_ingest, &LogIngestController::error, this,
            [this](const QUuid& view_id, const QString& file_path, const QString& message) {
                if (!m_is_shutting_down)
                {
                    qWarning().nospace()
                        << "[Controller] error view=" << view_id.toString() << " file=\""
                        << file_path << "\" msg=\"" << message << '"';

                    if (!view_id.isNull())
                    {
                        emit loading_error(view_id, file_path, message);
                    }
                    // IMPORTANT: Do not clear active state here; wait for idle to ensure
                    // late batches are still routed to the active view.
                }
            });

    // Finished pass-through.
    connect(m_ingest, &LogIngestController::finished, this,
            [this](const QUuid& view_id, const QString& file_path) {
                if (!m_is_shutting_down)
                {
                    qDebug().nospace() << "[Controller] finished view=" << view_id.toString()
                                       << " file=\"" << file_path << '"';

                    if (!view_id.isNull())
                    {
                        emit loading_finished(view_id, file_path);

                        if (m_live_tailing_views.contains(view_id))
                        {
                            m_tailer_service->start_tailing(view_id, file_path);
                        }
                    }
                    // IMPORTANT: Do not clear active state here; wait for idle to avoid
                    // dropping a late-arriving last batch for very small files.
                }
            });

    // Advance queue only after thread cleanup (safe to start next).
    connect(m_ingest, &LogIngestController::idle, this, [this]() {
        if (!m_is_shutting_down)
        {
            qDebug().nospace() << "[Controller] streaming_idle: force idle then try start next. "
                               << "pending=" << m_ingest->get_pending_count();

            // Start next first, then read new active view id (fix ordering).
            const bool started = (m_ingest->start_next_if_idle(m_ingest->get_active_batch_size()),
                                  !m_ingest->get_active_view_id().isNull());

            if (started)
            {
                const QUuid new_active = m_ingest->get_active_view_id();
                qDebug().nospace() << "[Controller] started next view=" << new_active.toString()
                                   << " file=\"" << m_ingest->get_active_file_path() << '"';

                if (!new_active.isNull() && (m_views->get_current_view() != new_active))
                {
                    m_views->set_current_view(new_active);
                }
            }
            else
            {
                qDebug().nospace() << "[Controller] no next item started (idle or empty queue).";

                // Queues are done; turn off ingestion mode mapped to any view
                const QVector<QUuid> ids = m_views->get_all_view_ids();
                for (const QUuid& id: ids)
                {
                    auto* proxy = get_sort_filter_proxy(id);
                    if (proxy != nullptr)
                    {
                        proxy->set_ingestion_mode(false);
                    }
                }
            }
        }
    });
}

/**
 * @brief Destroys the LogViewerController.
 *
 * Stops ingestion and live-tail callbacks before QObject-owned services are destroyed so no
 * background or watcher callback can target a removed view.
 */
LogViewerController::~LogViewerController()
{
    m_is_shutting_down = true;

    if (m_tailer_service != nullptr)
    {
        m_tailer_service->stop_all_tailing();
    }

    if (m_ingest != nullptr)
    {
        const QVector<QUuid> view_ids = m_views->get_all_view_ids();

        for (const QUuid& view_id: view_ids)
        {
            m_ingest->cancel_for_view(view_id);
        }
    }
}

/**
 * @brief Sets the current view to the given QUuid if it exists.
 * @param view_id The QUuid of the view to set as current.
 * @return True if the view was set successfully, false if the view_id does not exist.
 */
auto LogViewerController::set_current_view(const QUuid& view_id) -> bool
{
    bool success = m_views->set_current_view(view_id);
    return success;
}

/**
 * @brief Returns the QUuid of the current view.
 * @return The QUuid of the current view.
 */
auto LogViewerController::get_current_view() const -> QUuid
{
    QUuid id = m_views->get_current_view();
    return id;
}

/**
 * @brief Returns all registered view ids.
 * @return Vector of QUuid representing all views currently tracked.
 */
auto LogViewerController::get_all_view_ids() const -> QVector<QUuid>
{
    QVector<QUuid> ids;

    if (m_views != nullptr)
    {
        ids = m_views->get_all_view_ids();
    }

    return ids;
}

/**
 * @brief Removes a view and all resources associated with it.
 * @param view_id View identifier to remove.
 * @return True if the view existed and was removed.
 *
 * Tailing is stopped and archived history is deleted before the view context is destroyed.
 */
auto LogViewerController::remove_view(const QUuid& view_id) -> bool
{
    bool removed = false;

    if (!view_id.isNull() && m_views->get_context(view_id) != nullptr)
    {
        cancel_loading(view_id);

        if (m_tailer_service != nullptr)
        {
            m_tailer_service->stop_tailing_view(view_id);
        }

        if (m_history_service != nullptr)
        {
            m_history_service->remove_view_entries(view_id);
        }

        m_live_tailing_views.remove(view_id);
        removed = m_views->remove_view(view_id);
    }

    return removed;
}

/**
 * @brief Removes every view and releases associated loading, tailing, and history resources.
 *
 * Each view is removed through remove_view() so lifecycle cleanup is identical for normal removal,
 * session closing, and application shutdown.
 */
auto LogViewerController::clear_all_views() -> void
{
    const QVector<QUuid> view_ids = m_views->get_all_view_ids();

    for (const QUuid& view_id: view_ids)
    {
        remove_view(view_id);
    }
}

/**
 * @brief Adds a single log file to the LogFileTreeModel.
 * @param file_path The path to the log file.
 */
auto LogViewerController::add_log_file_to_tree(const QString& file_path) -> void
{
    if (m_catalog != nullptr)
    {
        m_catalog->add_file(file_path);
    }
}

/**
 * @brief Adds multiple log files to the LogFileTreeModel.
 * @param file_paths The vector of log file paths.
 */
auto LogViewerController::add_log_files_to_tree(const QVector<QString>& file_paths) -> void
{
    if (m_catalog != nullptr)
    {
        m_catalog->add_files(file_paths);
    }
}

/**
 * @brief Adds a single log file to a specific session in the LogFileTreeModel.
 * @param session_id The session identifier.
 * @param file_path The path to the log file.
 */
auto LogViewerController::add_log_file_to_session(const QString& session_id,
                                                  const QString& file_path) -> void
{
    if (m_catalog != nullptr)
    {
        m_catalog->add_file_to_session(session_id, file_path);
    }
}

/**
 * @brief Adds multiple log files to a specific session in the LogFileTreeModel.
 * @param session_id The session identifier.
 * @param file_paths The vector of log file paths.
 */
auto LogViewerController::add_log_files_to_session(const QString& session_id,
                                                   const QVector<QString>& file_paths) -> void
{
    if (m_catalog != nullptr)
    {
        m_catalog->add_files_to_session(session_id, file_paths);
    }
}

/**
 * @brief Loads a single log file into a new view.
 * @param file_path Path to the log file.
 * @return Identifier of the created view.
 */
auto LogViewerController::load_log_file(const QString& file_path) -> QUuid
{
    const QVector<LogEntry> entries = m_ingest->load_file_sync(file_path);
    const QString app_name =
        !entries.isEmpty() ? entries.first().get_app_name() : LogLoader::identify_app(file_path);
    const LogFileInfo loaded_log_file(file_path, app_name);
    const QUuid view_id = m_views->create_view();

    set_live_tailing_enabled(view_id, true);

    auto* context = m_views->get_context(view_id);
    auto* proxy = get_sort_filter_proxy(view_id);

    if (proxy != nullptr)
    {
        proxy->set_ingestion_mode(true);
    }

    if (context != nullptr)
    {
        m_history_service->add_entries(view_id, entries);
        context->append_entries(entries);
        m_views->set_loaded_files(view_id, QList<LogFileInfo>{loaded_log_file});
    }

    if (proxy != nullptr)
    {
        proxy->set_ingestion_mode(false);
    }

    m_tailer_service->start_tailing(view_id, file_path);

    return view_id;
}

/**
 * @brief Loads a single log file into an existing view.
 * @param view_id Target view identifier.
 * @param file_path Path to the log file.
 * @return True when the file was loaded.
 */
auto LogViewerController::load_log_file(const QUuid& view_id, const QString& file_path) -> bool
{
    bool success = false;

    ensure_view_models(view_id);

    auto* context = m_views->get_context(view_id);

    if (context != nullptr && !is_file_loaded(view_id, file_path))
    {
        auto* proxy = get_sort_filter_proxy(view_id);

        if (proxy != nullptr)
        {
            proxy->set_ingestion_mode(true);
        }

        const QVector<LogEntry> entries = m_ingest->load_file_sync(file_path);
        const QString app_name = !entries.isEmpty() ? entries.first().get_app_name()
                                                    : LogLoader::identify_app(file_path);

        m_history_service->add_entries(view_id, entries);
        context->append_entries(entries);
        m_views->add_loaded_file(view_id, LogFileInfo(file_path, app_name));

        if (proxy != nullptr)
        {
            proxy->set_ingestion_mode(false);
        }

        if (get_live_tailing_enabled(view_id))
        {
            m_tailer_service->start_tailing(view_id, file_path);
        }

        success = true;
    }

    return success;
}

/**
 * @brief Loads multiple log files into a new view.
 * @param file_paths Paths to load.
 * @return Identifier of the created view, or a null identifier for an empty request.
 */
auto LogViewerController::load_log_files(const QVector<QString>& file_paths) -> QUuid
{
    QUuid view_id;

    if (!file_paths.isEmpty())
    {
        QList<LogFileInfo> loaded_log_files;
        view_id = m_views->create_view();
        set_live_tailing_enabled(view_id, true);

        auto* proxy = get_sort_filter_proxy(view_id);

        if (proxy != nullptr)
        {
            proxy->set_ingestion_mode(true);
        }

        for (const QString& file_path: file_paths)
        {
            const QVector<LogEntry> entries = m_ingest->load_file_sync(file_path);
            const QString app_name = !entries.isEmpty() ? entries.first().get_app_name()
                                                        : LogLoader::identify_app(file_path);

            loaded_log_files.append(LogFileInfo(file_path, app_name));

            auto* context = m_views->get_context(view_id);

            if (context != nullptr)
            {
                m_history_service->add_entries(view_id, entries);
                context->append_entries(entries);
            }
        }

        m_views->set_loaded_files(view_id, loaded_log_files);

        if (proxy != nullptr)
        {
            proxy->set_ingestion_mode(false);
        }

        for (const QString& file_path: file_paths)
        {
            m_tailer_service->start_tailing(view_id, file_path);
        }
    }

    return view_id;
}

/**
 * @brief Starts streaming load of a single log file and creates a new view (model/proxy).
 * @param file_path The path to the log file to stream.
 * @param batch_size Number of entries per batch appended to the model.
 * @return QUuid of the created view.
 */
auto LogViewerController::load_log_file_async(const QString& file_path,
                                              qsizetype batch_size) -> QUuid
{
    QUuid view_id = m_views->create_view();
    set_live_tailing_enabled(view_id, true);

    const QString app_name = LogLoader::identify_app(file_path);
    LogFileInfo loaded_log_file(file_path, app_name);

    auto* ctx = m_views->get_context(view_id);
    if (ctx != nullptr)
    {
        m_views->set_loaded_files(view_id, QList<LogFileInfo>{loaded_log_file});
    }

    enqueue_async(view_id, file_path);
    try_start_next_async(batch_size);

    return view_id;
}

/**
 * @brief Starts streaming load of a single log file into an existing view (model/proxy).
 * @param view_id The target view to load the file into.
 * @param file_path The path to the log file to stream.
 * @param batch_size Number of entries per batch appended to the model.
 * @return True if the file was enqueued; false if already present or view missing.
 */
auto LogViewerController::load_log_file_async(const QUuid& view_id, const QString& file_path,
                                              qsizetype batch_size) -> bool
{
    bool success = false;

    ensure_view_models(view_id);

    if (!is_file_loaded(view_id, file_path))
    {
        const QString app_name = LogLoader::identify_app(file_path);
        LogFileInfo info(file_path, app_name);

        m_views->add_loaded_file(view_id, info);

        enqueue_async(view_id, file_path);
        try_start_next_async(batch_size);

        success = true;
    }

    return success;
}

/**
 * @brief Starts streaming load of multiple log files into a single new view (model/proxy).
 * @param file_paths Paths to stream.
 * @param batch_size Number of entries per batch appended to the model.
 * @return QUuid of the created view.
 */
auto LogViewerController::load_log_files_async(const QVector<QString>& file_paths,
                                               qsizetype batch_size) -> QUuid
{
    QUuid view_id;

    if (!file_paths.isEmpty())
    {
        view_id = m_views->create_view();
        set_live_tailing_enabled(view_id, true);
        QList<LogFileInfo> files_info;

        for (const QString& file_path: file_paths)
        {
            const QString app_name = LogLoader::identify_app(file_path);
            files_info.append(LogFileInfo(file_path, app_name));
            enqueue_async(view_id, file_path);
        }

        m_views->set_loaded_files(view_id, files_info);
        try_start_next_async(batch_size);
    }

    return view_id;
}

/**
 * @brief Cancels any ongoing streaming for the specified view and clears its pending queue.
 * @param view_id The view to cancel streaming for.
 */
auto LogViewerController::cancel_loading(const QUuid& view_id) -> void
{
    m_ingest->cancel_for_view(view_id);

    auto* proxy = get_sort_filter_proxy(view_id);
    if (proxy != nullptr)
    {
        proxy->set_ingestion_mode(false);
    }
}

/**
 * @brief Sets the application name filter for the current view.
 * @param app_name The application name to filter by.
 */
auto LogViewerController::set_app_name_filter(const QString& app_name) -> void
{
    set_app_name_filter(m_views->get_current_view(), app_name);
}

/**
 * @brief Sets the application name filter for the specified view.
 * @param view_id The QUuid of the view.
 * @param app_name The application name to filter by.
 */
auto LogViewerController::set_app_name_filter(const QUuid& view_id, const QString& app_name) -> void
{
    m_filters->set_app_name(view_id, app_name);
}

/**
 * @brief Sets the log level filter for the current view.
 * @param levels The set of log levels.
 */
auto LogViewerController::set_log_level_filters(const QSet<QString>& levels) -> void
{
    set_log_level_filters(m_views->get_current_view(), levels);
}

/**
 * @brief Sets the log level filter for the specified view.
 * @param view_id The QUuid of the view.
 * @param levels The set of log levels.
 */
auto LogViewerController::set_log_level_filters(const QUuid& view_id,
                                                const QSet<QString>& levels) -> void
{
    m_filters->set_log_levels(view_id, levels);
}

/**
 * @brief Sets the search filter for the current view.
 * @param search_text The search string or regex.
 * @param field The field to search in.
 * @param use_regex Whether to use regex.
 */
auto LogViewerController::set_search_filter(const QString& search_text, SearchField field,
                                            bool use_regex) -> void
{
    set_search_filter(m_views->get_current_view(), search_text, field, use_regex);
}

/**
 * @brief Sets the search filter for the specified view.
 * @param view_id The QUuid of the view.
 * @param search_text The search string or regex.
 * @param field The field to search in.
 * @param use_regex Whether to use regex.
 */
auto LogViewerController::set_search_filter(const QUuid& view_id, const QString& search_text,
                                            SearchField field, bool use_regex) -> void
{
    m_filters->set_search(view_id, search_text, field, use_regex);
}

/**
 * @brief Returns the LogModel for the current view.
 * @return Pointer to the LogModel.
 */
auto LogViewerController::get_log_model() -> LogModel*
{
    return get_log_model(m_views->get_current_view());
}

/**
 * @brief Returns the LogModel for the specified view.
 * @param view_id The QUuid of the view.
 * @return Pointer to the LogModel, or nullptr if not found.
 */
auto LogViewerController::get_log_model(const QUuid& view_id) -> LogModel*
{
    auto* ctx = m_views->get_context(view_id);
    LogModel* model = (ctx != nullptr) ? ctx->get_model() : nullptr;
    return model;
}

/**
 * @brief Returns the LogSortFilterProxyModel for the current view.
 * @return Pointer to the LogSortFilterProxyModel.
 */
auto LogViewerController::get_sort_filter_proxy() const -> LogSortFilterProxyModel*
{
    return get_sort_filter_proxy(m_views->get_current_view());
}

/**
 * @brief Returns the LogSortFilterProxyModel for the specified view.
 * @param view_id The QUuid of the view.
 * @return Pointer to the LogSortFilterProxyModel, or nullptr if not found.
 */
auto LogViewerController::get_sort_filter_proxy(const QUuid& view_id) const
    -> LogSortFilterProxyModel*
{
    auto* ctx = m_views->get_context(view_id);
    LogSortFilterProxyModel* proxy = (ctx != nullptr) ? ctx->get_sort_proxy() : nullptr;
    return proxy;
}

/**
 * @brief Returns the PagingProxyModel for the current view.
 * @return Pointer to the PagingProxyModel.
 */
auto LogViewerController::get_paging_proxy() -> PagingProxyModel*
{
    return get_paging_proxy(m_views->get_current_view());
}

/**
 * @brief Returns the PagingProxyModel for the specified view.
 * @param view_id The QUuid of the view.
 * @return Pointer to the PagingProxyModel, or nullptr if not found.
 */
auto LogViewerController::get_paging_proxy(const QUuid& view_id) -> PagingProxyModel*
{
    auto* ctx = m_views->get_context(view_id);
    PagingProxyModel* paging = (ctx != nullptr) ? ctx->get_paging_proxy() : nullptr;
    return paging;
}

/**
 * @brief Assigns a paged log query to a view and loads its first page.
 * @param view_id Target view.
 * @param query Query describing filtering and sorting.
 * @return True when the query was assigned and loaded.
 */
auto LogViewerController::set_page_query(const QUuid& view_id, const LogQuery& query) -> bool
{
    bool loaded = false;

    if (m_page_coordinator != nullptr)
    {
        loaded = m_page_coordinator->set_query(view_id, query);
    }

    return loaded;
}

/**
 * @brief Selects and loads a page for a view.
 * @param view_id Target view.
 * @param page One-based page number.
 * @return True when the page was loaded.
 */
auto LogViewerController::set_current_page(const QUuid& view_id, qsizetype page) -> bool
{
    bool loaded = false;

    if (m_page_coordinator != nullptr)
    {
        loaded = m_page_coordinator->set_current_page(view_id, page);
    }

    return loaded;
}

/**
 * @brief Changes the page size and loads the first page.
 * @param view_id Target view.
 * @param page_size Positive number of entries per page.
 * @return True when the page size was applied.
 */
auto LogViewerController::set_page_size(const QUuid& view_id, qsizetype page_size) -> bool
{
    bool loaded = false;

    if (m_page_coordinator != nullptr)
    {
        loaded = m_page_coordinator->set_page_size(view_id, page_size);
    }

    return loaded;
}

/**
 * @brief Reloads the current page for a view.
 * @param view_id Target view.
 * @return True when the page was loaded.
 */
auto LogViewerController::reload_page(const QUuid& view_id) -> bool
{
    bool loaded = false;

    if (m_page_coordinator != nullptr)
    {
        loaded = m_page_coordinator->reload(view_id);
    }

    return loaded;
}

/**
 * @brief Returns the paged query state for a view.
 * @param view_id Target view.
 * @return Page state, or nullptr when no query is assigned.
 */
auto LogViewerController::get_page_state(const QUuid& view_id) const -> const LogPageState*
{
    const LogPageState* state = nullptr;

    if (m_page_coordinator != nullptr)
    {
        state = m_page_coordinator->get_page_state(view_id);
    }

    return state;
}

/**
 * @brief Creates a log query from the filter and sorting state of a view.
 * @param view_id Source view.
 * @return Query representing the view state.
 */
auto LogViewerController::create_page_query(const QUuid& view_id) const -> LogQuery
{
    LogQuery query;
    LogViewContext* context = get_view_context(view_id);

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

        LogSortFilterProxyModel* sort_proxy = context->get_sort_proxy();

        if (sort_proxy != nullptr)
        {
            query.sort_field = get_query_sort_field(sort_proxy->get_sort_column());
            query.sort_order = sort_proxy->get_sort_order();
        }
    }

    return query;
}

/**
 * @brief Returns the set of unique application names from the loaded logs in the current view.
 * @return A set of application names.
 */
auto LogViewerController::get_app_names() const -> QSet<QString>
{
    return get_app_names(m_views->get_current_view());
}

/**
 * @brief Returns the set of unique application names from the loaded logs in the specified view.
 * @param view_id The QUuid of the view.
 * @return A set of application names.
 */
auto LogViewerController::get_app_names(const QUuid& view_id) const -> QSet<QString>
{
    QSet<QString> app_names;

    auto* ctx = m_views->get_context(view_id);
    if (ctx != nullptr)
    {
        for (const LogEntry& entry: ctx->get_entries())
        {
            app_names.insert(entry.get_app_name());
        }
    }

    return app_names;
}

/**
 * @brief Returns the current application name filter.
 * @return The application name filter string.
 */
auto LogViewerController::get_app_name_filter() const -> QString
{
    return get_app_name_filter(m_views->get_current_view());
}

/**
 * @brief Returns the application name filter for the specified view.
 * @param view_id The QUuid of the view.
 * @return The application name filter string.
 */
auto LogViewerController::get_app_name_filter(const QUuid& view_id) const -> QString
{
    QString filter = m_filters->get_app_name(view_id);
    return filter;
}

/**
 * @brief Returns the available log levels for the specified view.
 * @param view_id The QUuid of the view.
 * @return Vector of log level names (same list for all views).
 */
auto LogViewerController::get_available_log_levels(const QUuid& view_id) const -> QVector<QString>
{
    QVector<QString> log_levels = FilterCoordinator::get_available_log_levels();
    Q_UNUSED(view_id);
    return log_levels;
}

/**
 * @brief Returns the current set of log levels being filtered.
 * @return The set of log levels.
 */
auto LogViewerController::get_log_level_filters() const -> QSet<QString>
{
    return get_log_level_filters(m_views->get_current_view());
}

/**
 * @brief Returns a map of log level names to their counts in the specified view.
 * @param view_id The QUuid of the view.
 * @return QMap of log level names to counts.
 */
auto LogViewerController::get_log_level_counts(const QUuid& view_id) const -> QMap<QString, int>
{
    QMap<QString, int> counts = m_filters->get_log_level_counts(view_id);
    return counts;
}

/**
 * @brief Returns a map of log level names to their counts in the current view.
 * @return QMap of log level names to counts.
 */
auto LogViewerController::get_log_level_counts() const -> QMap<QString, int>
{
    QMap<QString, int> counts = get_log_level_counts(m_views->get_current_view());
    return counts;
}

/**
 * @brief Returns the set of log levels being filtered for the specified view.
 * @param view_id The QUuid of the view.
 * @return The set of log levels.
 */
auto LogViewerController::get_log_level_filters(const QUuid& view_id) const -> QSet<QString>
{
    QSet<QString> levels = m_filters->get_log_levels(view_id);
    return levels;
}

/**
 * @brief Returns the current search text.
 * @return The search text string.
 */
auto LogViewerController::get_search_text() const -> QString
{
    return get_search_text(m_views->get_current_view());
}

/**
 * @brief Returns the search text for the specified view.
 * @param view_id The QUuid of the view.
 * @return The search text string.
 */
auto LogViewerController::get_search_text(const QUuid& view_id) const -> QString
{
    QString text = m_filters->get_search_text(view_id);
    return text;
}

/**
 * @brief Returns the current search field.
 * @return The search field.
 */
auto LogViewerController::get_search_field() const -> SearchField
{
    SearchField field = get_search_field(m_views->get_current_view());
    return field;
}

/**
 * @brief Returns the search field for the specified view.
 * @param view_id The QUuid of the view.
 * @return The search field.
 */
auto LogViewerController::get_search_field(const QUuid& view_id) const -> SearchField
{
    SearchField field = m_filters->get_search_field(view_id);
    return field;
}

/**
 * @brief Returns whether the search text is treated as a regex.
 * @return True if using regex, false if plain text.
 */
auto LogViewerController::is_search_regex() const -> bool
{
    return is_search_regex(m_views->get_current_view());
}

/**
 * @brief Returns whether the search text is treated as a regex for the specified view.
 * @param view_id The QUuid of the view.
 * @return True if using regex, false if plain text.
 */
auto LogViewerController::is_search_regex(const QUuid& view_id) const -> bool
{
    bool regex = m_filters->is_search_regex(view_id);
    return regex;
}

/**
 * @brief Returns the LogFileTreeModel.
 *
 * This model provides a hierarchical view of log files and their applications.
 * @return Pointer to the LogFileTreeModel.
 */
auto LogViewerController::get_log_file_tree_model() -> LogFileTreeModel*
{
    LogFileTreeModel* model = (m_catalog != nullptr) ? m_catalog->get_model() : nullptr;
    return model;
}

/**
 * @brief Returns all log entries in the current view.
 * @return QVector<LogEntry> containing all entries.
 */
auto LogViewerController::get_log_entries() const -> QVector<LogEntry>
{
    return get_log_entries(m_views->get_current_view());
}

/**
 * @brief Returns all log entries in the specified view.
 * @param view_id The QUuid of the view.
 * @return QVector<LogEntry> containing all entries.
 */
auto LogViewerController::get_log_entries(const QUuid& view_id) const -> QVector<LogEntry>
{
    QVector<LogEntry> result = m_views->get_entries(view_id);
    return result;
}

/**
 * @brief Returns all log entries for a given file in the current view.
 * @param file_info The LogFileInfo for the file.
 * @return QVector<LogEntry> containing all entries for the file.
 */
auto LogViewerController::get_entries_for_file(const LogFileInfo& file_info) -> QVector<LogEntry>
{
    return get_entries_for_file(m_views->get_current_view(), file_info);
}

/**
 * @brief Returns all log entries for a given file in the specified view.
 * @param view_id The QUuid of the view.
 * @param file_info The LogFileInfo for the file.
 * @return QVector<LogEntry> containing all entries for the file.
 */
auto LogViewerController::get_entries_for_file(const QUuid& view_id,
                                               const LogFileInfo& file_info) -> QVector<LogEntry>
{
    QVector<LogEntry> result;
    QVector<LogEntry> entries = m_views->get_entries(view_id);

    for (const auto& entry: entries)
    {
        if (entry.get_file_info().get_file_path() == file_info.get_file_path())
        {
            result.append(entry);
        }
    }

    return result;
}

/**
 * @brief Checks if a log file with the given file path is already loaded.
 * @param file_path The file path to check.
 * @return True if the file is already loaded, false otherwise.
 */
auto LogViewerController::is_file_loaded(const QString& file_path) const -> bool
{
    bool found = false;

    QVector<QUuid> ids = m_views->get_all_view_ids();
    for (const QUuid& id: ids)
    {
        const auto paths = m_views->get_file_paths(id);
        const auto match = std::find_if(paths.begin(), paths.end(),
                                        [&file_path](const QString& p) { return p == file_path; });
        if (match != paths.end())
        {
            found = true;
        }
    }

    return found;
}

/**
 * @brief Checks if a log file with the given file path is loaded in the specified view.
 * @param view_id The QUuid of the view.
 * @param file_path The file path to check.
 * @return True if the file is loaded in the view, false otherwise.
 */
auto LogViewerController::is_file_loaded(const QUuid& view_id,
                                         const QString& file_path) const -> bool
{
    bool found = false;

    const auto paths = m_views->get_file_paths(view_id);
    const auto it = std::find_if(paths.begin(), paths.end(),
                                 [&file_path](const QString& p) { return p == file_path; });
    if (it != paths.end())
    {
        found = true;
    }

    return found;
}

/**
 * @brief Applies a "show only file" filter for the specified view.
 * @param view_id Target view id.
 * @param file_path File path to show exclusively, or empty to reset.
 */
auto LogViewerController::set_show_only_file(const QUuid& view_id, const QString& file_path) -> void
{
    m_filters->set_show_only(view_id, file_path);
}

/**
 * @brief Toggles a file's visibility (hide/show) in the specified view.
 * @param view_id Target view id.
 * @param file_path Absolute file path to toggle.
 */
auto LogViewerController::toggle_file_visibility(const QUuid& view_id,
                                                 const QString& file_path) -> void
{
    m_filters->toggle_visibility(view_id, file_path);
}

/**
 * @brief Hides (excludes) a specific file in the specified view.
 * @param view_id Target view id.
 * @param file_path File path to hide.
 */
auto LogViewerController::hide_file(const QUuid& view_id, const QString& file_path) -> void
{
    m_filters->hide_file(view_id, file_path);
}

/**
 * @brief Returns absolute file paths loaded in the specified view.
 * @param view_id The QUuid of the view.
 * @return Vector of file paths loaded in the view (empty if none).
 */
auto LogViewerController::get_view_file_paths(const QUuid& view_id) const -> QVector<QString>
{
    QVector<QString> result = m_views->get_file_paths(view_id);
    return result;
}

/**
 * @brief Exports serializable state for a view.
 * @param view_id Target view identifier.
 * @return Session snapshot including the per-view live-tail setting.
 */
auto LogViewerController::export_view_state(const QUuid& view_id) const -> SessionViewState
{
    SessionViewState state;

    if (!view_id.isNull())
    {
        state = m_views->export_view_state(view_id, *m_filters);
        state.filters.live_tailing_enabled = get_live_tailing_enabled(view_id);
    }

    return state;
}

/**
 * @brief Imports a single view state (files, filters, paging, sort) and returns the ensured view
 * id.
 * @param state The view state to apply.
 * @return QUuid of the imported/ensured view.
 */
auto LogViewerController::import_view_state(const SessionViewState& state) -> QUuid
{
    QUuid result;

    if (m_views != nullptr && m_filters != nullptr)
    {
        result = m_views->import_view_state(state, *m_filters);
        set_live_tailing_enabled(result, state.filters.live_tailing_enabled);

        // Update explorer tree
        if (m_catalog != nullptr && !state.loaded_files.isEmpty())
        {
            QVector<QString> paths;
            paths.reserve(state.loaded_files.size());
            for (const auto& lf: state.loaded_files)
            {
                const QString p = lf.get_file_path();
                if (!p.isEmpty())
                {
                    paths.append(p);
                }
            }

            if (!paths.isEmpty())
            {
                add_log_files_to_tree(paths);
            }
        }

        if (!result.isNull())
        {
            for (const auto& lf: state.loaded_files)
            {
                const QString path = lf.get_file_path();
                if (!path.isEmpty())
                {
                    ensure_view_models(result);
                    enqueue_async(result, path);
                    try_start_next_async(1000);
                }
            }
        }
    }

    return result;
}

/**
 * @brief Imports a single view state for a specific session.
 * @param session_id The session identifier for the tree model.
 * @param state The view state to apply.
 * @return QUuid of the imported/ensured view.
 */
auto LogViewerController::import_view_state_for_session(const QString& session_id,
                                                        const SessionViewState& state) -> QUuid
{
    QUuid result;

    if (m_views != nullptr && m_filters != nullptr)
    {
        result = m_views->import_view_state(state, *m_filters);
        set_live_tailing_enabled(result, state.filters.live_tailing_enabled);

        // Update explorer tree with session context
        if (m_catalog != nullptr && !state.loaded_files.isEmpty() && !session_id.isEmpty())
        {
            QVector<QString> paths;
            paths.reserve(state.loaded_files.size());
            for (const auto& lf: state.loaded_files)
            {
                const QString p = lf.get_file_path();
                if (!p.isEmpty())
                {
                    paths.append(p);
                }
            }

            if (!paths.isEmpty())
            {
                add_log_files_to_session(session_id, paths);
            }
        }

        if (!result.isNull())
        {
            for (const auto& lf: state.loaded_files)
            {
                const QString path = lf.get_file_path();
                if (!path.isEmpty())
                {
                    ensure_view_models(result);
                    enqueue_async(result, path);
                    try_start_next_async(1000);
                }
            }
        }
    }

    return result;
}

/**
 * @brief Enables or disables live tailing for a view.
 * @param view_id Target view.
 * @param enabled True to start tailing loaded files.
 */
auto LogViewerController::set_live_tailing_enabled(const QUuid& view_id, bool enabled) -> void
{
    if (enabled)
    {
        m_live_tailing_views.insert(view_id);

        const QVector<QString> file_paths = get_view_file_paths(view_id);

        for (const QString& file_path: file_paths)
        {
            m_tailer_service->start_tailing(view_id, file_path);
        }
    }
    else
    {
        m_live_tailing_views.remove(view_id);
        m_tailer_service->stop_tailing_view(view_id);
    }
}

/**
 * @brief Returns whether live tailing is enabled for a view.
 * @param view_id Target view.
 * @return True when enabled.
 */
auto LogViewerController::get_live_tailing_enabled(const QUuid& view_id) const -> bool
{
    const bool enabled = m_live_tailing_views.contains(view_id);
    return enabled;
}

/**
 * @brief Searches every SQLite-archived entry for a view.
 * @param view_id Target view.
 * @param search_text Plain-text FTS query.
 * @param search_field Field to search.
 * @param limit Maximum result count.
 * @return Matching archive entries.
 */
auto LogViewerController::search_history(const QUuid& view_id, const QString& search_text,
                                         SearchField search_field,
                                         int limit) const -> QVector<LogEntry>
{
    QVector<LogEntry> entries;

    if (m_history_service != nullptr)
    {
        entries = m_history_service->search_entries(view_id, search_text, search_field, limit);
    }

    return entries;
}

/**
 * @brief Removes a file from every view and from the file tree.
 * @param file File metadata identifying the path to remove.
 *
 * Tailing is stopped and the corresponding archived entries are deleted before the file is
 * removed from each view model.
 */
auto LogViewerController::remove_log_file(const LogFileInfo& file) -> void
{
    QList<QUuid> views_to_remove;
    const QString file_path = file.get_file_path();
    const QVector<QUuid> view_ids = m_views->get_all_view_ids();

    for (const QUuid& view_id: view_ids)
    {
        auto* context = m_views->get_context(view_id);

        if (context != nullptr && is_file_loaded(view_id, file_path))
        {
            m_tailer_service->stop_tailing(view_id, file_path);
            m_history_service->remove_file_entries(view_id, file_path);

            QList<LogFileInfo> files = context->get_loaded_files();
            files.erase(std::remove_if(files.begin(), files.end(),
                                       [&file_path](const LogFileInfo& info) {
                                           return info.get_file_path() == file_path;
                                       }),
                        files.end());

            context->set_loaded_files(files);
            context->remove_entries_by_file_path(file_path);
            m_filters->adjust_visibility_on_file_removed(view_id, file_path);

            if (context->get_entries().isEmpty())
            {
                views_to_remove.append(view_id);
            }

            emit view_file_paths_changed(view_id, context->get_file_paths());
        }
    }

    if (m_catalog != nullptr)
    {
        m_catalog->remove_file(file);
    }

    for (const QUuid& view_id: views_to_remove)
    {
        remove_view(view_id);
    }
}

/**
 * @brief Removes a file from one view.
 * @param view_id Target view identifier.
 * @param file_path Absolute file path to remove.
 *
 * Tailing is stopped and file-specific archive history is deleted before the live model is
 * changed. An empty view is then removed through remove_view().
 */
auto LogViewerController::remove_log_file(const QUuid& view_id, const QString& file_path) -> void
{
    const bool has_valid_args =
        !view_id.isNull() && !file_path.isEmpty() && is_file_loaded(view_id, file_path);
    bool view_became_empty = false;

    if (has_valid_args)
    {
        m_tailer_service->stop_tailing(view_id, file_path);
        m_history_service->remove_file_entries(view_id, file_path);

        m_views->remove_entries_by_file(view_id, file_path);
        m_filters->adjust_visibility_on_file_removed(view_id, file_path);

        const QVector<LogEntry> entries = m_views->get_entries(view_id);
        view_became_empty = entries.isEmpty();

        emit view_file_paths_changed(view_id, get_view_file_paths(view_id));
    }

    if (has_valid_args && view_became_empty)
    {
        remove_view(view_id);
    }
}

/**
 * @brief Enqueues an asynchronous load request for a log file.
 * @param view_id The QUuid of the view to load into.
 * @param file_path The path to the log file.
 */
auto LogViewerController::enqueue_async(const QUuid& view_id, const QString& file_path) -> void
{
    auto* proxy = get_sort_filter_proxy(view_id);
    if (proxy != nullptr)
    {
        proxy->set_ingestion_mode(true);
    }

    m_ingest->enqueue_stream(view_id, file_path);
}

/**
 * @brief Attempts to start the next asynchronous load if none is active.
 * @param batch_size Number of entries per batch.
 */
auto LogViewerController::try_start_next_async(qsizetype batch_size) -> void
{
    m_ingest->start_next_if_idle(batch_size);

    const QUuid active_view_id = m_ingest->get_active_view_id();
    if (!active_view_id.isNull() && (m_views->get_current_view() != active_view_id))
    {
        m_views->set_current_view(active_view_id);
    }
}

/**
 * @brief Clears pending async requests for a specific view.
 * @param view_id The QUuid of the view.
 */
auto LogViewerController::clear_pending_for_view(const QUuid& view_id) -> void
{
    m_ingest->cancel_for_view(view_id);
}

/**
 * @brief Ensures that models and proxies exist for the specified view ID.
 * @param view_id The QUuid of the view.
 */
auto LogViewerController::ensure_view_models(const QUuid& view_id) -> void
{
    m_views->ensure_view(view_id);
}

/**
 * @brief Returns the context for a view or nullptr if not present.
 * @param view_id The QUuid of the view.
 * @return Pointer to LogViewContext.
 */
auto LogViewerController::get_view_context(const QUuid& view_id) const -> LogViewContext*
{
    LogViewContext* ctx = m_views->get_context(view_id);
    return ctx;
}
