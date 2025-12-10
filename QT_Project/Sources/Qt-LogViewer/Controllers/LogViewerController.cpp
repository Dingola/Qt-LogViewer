#include "Qt-LogViewer/Controllers/LogViewerController.h"

#include <algorithm>

/**
 * @brief Constructs a LogViewerController.
 * @param log_format The log format string for parsing.
 * @param parent The parent QObject.
 */
LogViewerController::LogViewerController(const QString& log_format, QObject* parent)
    : QObject(parent),
      m_file_tree_model(new LogFileTreeModel(this)),
      m_loader(log_format, this),
      m_current_view_id(),
      m_async_queue(),
      m_active_view_id(),
      m_active_file_path(),
      m_active_batch_size(1000)
{
    connect(&m_loader, &LogLoader::entry_batch_parsed, this,
            [this](const QString& file_path, const QVector<LogEntry>& batch) {
                QUuid view_id;
                if (file_path == m_active_file_path)
                {
                    view_id = m_active_view_id;
                }
                if (!view_id.isNull() && m_view_models.contains(view_id))
                {
                    m_view_models[view_id]->add_entries(batch);
                }
            });

    connect(&m_loader, &LogLoader::progress, this,
            [this](const QString& file_path, qint64 bytes_read, qint64 total_bytes) {
                QUuid view_id;
                if (file_path == m_active_file_path)
                {
                    view_id = m_active_view_id;
                }
                if (!view_id.isNull())
                {
                    emit loading_progress(view_id, bytes_read, total_bytes);
                }
            });

    connect(&m_loader, &LogLoader::error, this,
            [this](const QString& file_path, const QString& message) {
                QUuid view_id;
                if (file_path == m_active_file_path)
                {
                    view_id = m_active_view_id;
                }
                if (!view_id.isNull())
                {
                    emit loading_error(view_id, file_path, message);
                }
            });

    connect(&m_loader, &LogLoader::finished, this, [this](const QString& file_path) {
        QUuid view_id;
        if (file_path == m_active_file_path)
        {
            view_id = m_active_view_id;
        }
        if (!view_id.isNull())
        {
            emit loading_finished(view_id, file_path);
        }
        m_active_view_id = QUuid();
        m_active_file_path = QString();
    });

    // Advance queue only after thread cleanup (safe to start next).
    connect(&m_loader, &LogLoader::streaming_idle, this,
            [this]() { try_start_next_async(m_active_batch_size); });
}

/**
 * @brief Sets the current view to the given QUuid if it exists.
 * @param view_id The QUuid of the view to set as current.
 * @return True if the view was set successfully, false if the view_id does not exist.
 */
auto LogViewerController::set_current_view(const QUuid& view_id) -> bool
{
    bool success = false;

    if (m_view_models.contains(view_id))
    {
        m_current_view_id = view_id;
        success = true;
        emit current_view_id_changed(m_current_view_id);
    }

    return success;
}

/**
 * @brief Returns the QUuid of the current view.
 * @return The QUuid of the current view.
 */
auto LogViewerController::get_current_view() const -> QUuid
{
    return m_current_view_id;
}

/**
 * @brief Removes a view and all associated models and proxies.
 * @param view_id The QUuid of the view to remove.
 * @return True if the view was removed, false if not found.
 */
auto LogViewerController::remove_view(const QUuid& view_id) -> bool
{
    bool removed = false;

    if (m_view_models.contains(view_id))
    {
        delete m_view_models[view_id];
        m_view_models.remove(view_id);
        removed = true;
    }
    if (m_sort_filter_models.contains(view_id))
    {
        delete m_sort_filter_models[view_id];
        m_sort_filter_models.remove(view_id);
    }
    if (m_paging_models.contains(view_id))
    {
        delete m_paging_models[view_id];
        m_paging_models.remove(view_id);
    }
    if (m_loaded_log_files.contains(view_id))
    {
        m_loaded_log_files.remove(view_id);
    }
    if (m_current_view_id == view_id)
    {
        m_current_view_id = QUuid();  // Reset current view if it was removed
    }

    // Also clear queued/active streaming work for this view
    cancel_loading(view_id);

    return removed;
}

/**
 * @brief Adds a single log file to the LogFileTreeModel.
 * @param file_path The path to the log file.
 */
auto LogViewerController::add_log_file_to_tree(const QString& file_path) -> void
{
    LogEntry entry = m_loader.read_first_log_entry(file_path);
    LogFileInfo info;

    if (!entry.get_app_name().isEmpty())
    {
        info = entry.get_file_info();
    }
    else
    {
        // Fallback: derive app name from file if the log line did not provide one
        const QString app_name = LogLoader::identify_app(file_path);
        info = LogFileInfo(file_path, app_name);
    }

    m_file_tree_model->add_log_file(info);
}

/**
 * @brief Adds multiple log files to the LogFileTreeModel.
 * @param file_paths The vector of log file paths.
 */
auto LogViewerController::add_log_files_to_tree(const QVector<QString>& file_paths) -> void
{
    for (const auto& file_path: file_paths)
    {
        add_log_file_to_tree(file_path);
    }
}

/**
 * @brief Loads a single log file and creates a new view (model/proxy) for it.
 * @param log_file The LogFileInfo to load and display.
 * @return QUuid of the created view, or an empty QUuid if the file is already loaded.
 */
auto LogViewerController::load_log_file(const QString& file_path) -> QUuid
{
    auto entries = m_loader.load_log_file(file_path);
    QString app_name =
        (!entries.isEmpty()) ? entries.first().get_app_name() : LogLoader::identify_app(file_path);
    LogFileInfo loaded_log_file(file_path, app_name);
    auto view_id = QUuid::createUuid();

    ensure_view_models(view_id);

    if (m_view_models.contains(view_id))
    {
        m_view_models[view_id]->add_entries(entries);
    }

    m_loaded_log_files[view_id] = QList<LogFileInfo>{loaded_log_file};
    emit view_file_paths_changed(view_id, get_view_file_paths(view_id));

    return view_id;
}

/**
 * @brief Loads a single log file into an existing view (model/proxy).
 * @param view_id The target view to load the file into.
 * @param file_path The path to the log file.
 * @return True if the file was loaded and entries appended; false if the file was already present
 * or the view does not exist.
 */
auto LogViewerController::load_log_file(const QUuid& view_id, const QString& file_path) -> bool
{
    bool success = false;

    ensure_view_models(view_id);

    if (m_view_models.contains(view_id) && !is_file_loaded(view_id, file_path))
    {
        auto entries = m_loader.load_log_file(file_path);
        const QString app_name = (!entries.isEmpty()) ? entries.first().get_app_name()
                                                      : LogLoader::identify_app(file_path);

        m_view_models[view_id]->add_entries(entries);

        LogFileInfo info(file_path, app_name);
        if (!m_loaded_log_files.contains(view_id))
        {
            m_loaded_log_files[view_id] = QList<LogFileInfo>();
        }
        m_loaded_log_files[view_id].append(info);

        success = true;
        emit view_file_paths_changed(view_id, get_view_file_paths(view_id));
    }

    return success;
}

/**
 * @brief Loads log files from the specified file paths and creates a new view (model/proxy) for
 * them.
 * @param file_paths A vector of file paths to load logs from.
 * @return QUuid of the created view, or an empty QUuid if no files were loaded.
 *
 * This method uses the LogLoader service to parse log files and populate a new LogModel.
 * It also updates the LogFileTreeModel with information about the loaded log files.
 */
auto LogViewerController::load_log_files(const QVector<QString>& file_paths) -> QUuid
{
    QList<LogFileInfo> loaded_log_files;
    auto view_id = QUuid::createUuid();

    ensure_view_models(view_id);

    for (const QString& file_path: file_paths)
    {
        auto entries = m_loader.load_log_file(file_path);
        QString app_name = (!entries.isEmpty()) ? entries.first().get_app_name()
                                                : LogLoader::identify_app(file_path);

        LogFileInfo log_file_info(file_path, app_name);
        loaded_log_files.append(log_file_info);

        if (m_view_models.contains(view_id))
        {
            m_view_models[view_id]->add_entries(entries);
        }
    }

    m_loaded_log_files[view_id] = loaded_log_files;
    emit view_file_paths_changed(view_id, get_view_file_paths(view_id));

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
    auto view_id = QUuid::createUuid();
    ensure_view_models(view_id);

    // Track file in the view (no tree insert here to avoid duplicates)
    const QString app_name = LogLoader::identify_app(file_path);
    LogFileInfo loaded_log_file(file_path, app_name);
    m_loaded_log_files[view_id] = QList<LogFileInfo>{loaded_log_file};
    emit view_file_paths_changed(view_id, get_view_file_paths(view_id));

    enqueue_async(view_id, file_path);
    m_active_batch_size = batch_size;
    try_start_next_async(m_active_batch_size);

    return view_id;
}

/**
 * @brief Starts streaming load of a single log file into an existing view (model/proxy).
 *        The file will be streamed in the background according to the controller queue.
 * @param view_id The target view to load the file into.
 * @param file_path The path to the log file to stream.
 * @param batch_size Number of entries per batch appended to the model.
 * @return True if the file was enqueued for streaming; false if the file was already present or the
 * view does not exist.
 */
auto LogViewerController::load_log_file_async(const QUuid& view_id, const QString& file_path,
                                              qsizetype batch_size) -> bool
{
    bool success = false;

    ensure_view_models(view_id);

    if (m_view_models.contains(view_id) && !is_file_loaded(view_id, file_path))
    {
        const QString app_name = LogLoader::identify_app(file_path);
        LogFileInfo info(file_path, app_name);

        if (!m_loaded_log_files.contains(view_id))
        {
            m_loaded_log_files[view_id] = QList<LogFileInfo>();
        }
        m_loaded_log_files[view_id].append(info);
        emit view_file_paths_changed(view_id, get_view_file_paths(view_id));

        enqueue_async(view_id, file_path);
        m_active_batch_size = batch_size;
        try_start_next_async(m_active_batch_size);

        success = true;
    }

    return success;
}

/**
 * @brief Starts streaming load of multiple log files into a single new view (model/proxy).
 *        Files are streamed sequentially in the background.
 * @param file_paths Paths to stream.
 * @param batch_size Number of entries per batch appended to the model.
 * @return QUuid of the created view.
 */
auto LogViewerController::load_log_files_async(const QVector<QString>& file_paths,
                                               qsizetype batch_size) -> QUuid
{
    auto view_id = QUuid::createUuid();
    ensure_view_models(view_id);

    QList<LogFileInfo> files_info;

    for (const QString& file_path: file_paths)
    {
        const QString app_name = LogLoader::identify_app(file_path);
        files_info.append(LogFileInfo(file_path, app_name));
        enqueue_async(view_id, file_path);
    }

    m_loaded_log_files[view_id] = files_info;
    emit view_file_paths_changed(view_id, get_view_file_paths(view_id));

    m_active_batch_size = batch_size;
    try_start_next_async(m_active_batch_size);

    return view_id;
}

/**
 * @brief Cancels any ongoing streaming for the specified view and clears its pending queue.
 * @param view_id The view to cancel streaming for.
 */
auto LogViewerController::cancel_loading(const QUuid& view_id) -> void
{
    // Cancel active if it belongs to the view
    if (m_active_view_id == view_id)
    {
        m_loader.cancel_async();
    }

    // Clear any queued items for the view
    clear_pending_for_view(view_id);
}

/**
 * @brief Sets the application name filter for the current view.
 * @param app_name The application name to filter by.
 */
auto LogViewerController::set_app_name_filter(const QString& app_name) -> void
{
    set_app_name_filter(m_current_view_id, app_name);
}

/**
 * @brief Sets the application name filter for the specified view.
 * @param view_id The QUuid of the view.
 * @param app_name The application name to filter by.
 */
auto LogViewerController::set_app_name_filter(const QUuid& view_id, const QString& app_name) -> void
{
    if (m_sort_filter_models.contains(view_id))
    {
        m_sort_filter_models[view_id]->set_app_name_filter(app_name);
    }
}

/**
 * @brief Sets the log level filter for the current view.
 * @param levels The set of log levels.
 */
auto LogViewerController::set_log_level_filters(const QSet<QString>& levels) -> void
{
    set_log_level_filters(m_current_view_id, levels);
}

/**
 * @brief Sets the log level filter for the specified view.
 * @param view_id The QUuid of the view.
 * @param levels The set of log levels.
 */
auto LogViewerController::set_log_level_filters(const QUuid& view_id,
                                                const QSet<QString>& levels) -> void
{
    if (m_sort_filter_models.contains(view_id))
    {
        m_sort_filter_models[view_id]->set_log_level_filters(levels);
    }
}

/**
 * @brief Sets the search filter for the current view.
 * @param search_text The search string or regex.
 * @param field The field to search in.
 * @param use_regex Whether to use regex.
 */
auto LogViewerController::set_search_filter(const QString& search_text, const QString& field,
                                            bool use_regex) -> void
{
    set_search_filter(m_current_view_id, search_text, field, use_regex);
}

/**
 * @brief Sets the search filter for the specified view.
 * @param view_id The QUuid of the view.
 * @param search_text The search string or regex.
 * @param field The field to search in.
 * @param use_regex Whether to use regex.
 */
auto LogViewerController::set_search_filter(const QUuid& view_id, const QString& search_text,
                                            const QString& field, bool use_regex) -> void
{
    if (m_sort_filter_models.contains(view_id))
    {
        m_sort_filter_models[view_id]->set_search_filter(search_text, field, use_regex);
    }
}

/**
 * @brief Returns the LogModel for the current view.
 * @return Pointer to the LogModel.
 */
auto LogViewerController::get_log_model() -> LogModel*
{
    return get_log_model(m_current_view_id);
}

/**
 * @brief Returns the LogModel for the specified view.
 * @param view_id The QUuid of the view.
 * @return Pointer to the LogModel, or nullptr if not found.
 */
auto LogViewerController::get_log_model(const QUuid& view_id) -> LogModel*
{
    return m_view_models.contains(view_id) ? m_view_models[view_id] : nullptr;
}

/**
 * @brief Returns the LogSortFilterProxyModel for the current view.
 * @return Pointer to the LogSortFilterProxyModel.
 */
auto LogViewerController::get_sort_filter_proxy() -> LogSortFilterProxyModel*
{
    return get_sort_filter_proxy(m_current_view_id);
}

/**
 * @brief Returns the LogSortFilterProxyModel for the specified view.
 * @param view_id The QUuid of the view.
 * @return Pointer to the LogSortFilterProxyModel, or nullptr if not found.
 */
auto LogViewerController::get_sort_filter_proxy(const QUuid& view_id) -> LogSortFilterProxyModel*
{
    return m_sort_filter_models.contains(view_id) ? m_sort_filter_models[view_id] : nullptr;
}

/**
 * @brief Returns the PagingProxyModel for the current view.
 * @return Pointer to the PagingProxyModel.
 */
auto LogViewerController::get_paging_proxy() -> PagingProxyModel*
{
    return get_paging_proxy(m_current_view_id);
}

/**
 * @brief Returns the PagingProxyModel for the specified view.
 * @param view_id The QUuid of the view.
 * @return Pointer to the PagingProxyModel, or nullptr if not found.
 */
auto LogViewerController::get_paging_proxy(const QUuid& view_id) -> PagingProxyModel*
{
    return m_paging_models.contains(view_id) ? m_paging_models[view_id] : nullptr;
}

/**
 * @brief Returns the set of unique application names from the loaded logs in the current view.
 * @return A set of application names.
 */
auto LogViewerController::get_app_names() const -> QSet<QString>
{
    return get_app_names(m_current_view_id);
}

/**
 * @brief Returns the set of unique application names from the loaded logs in the specified view.
 * @param view_id The QUuid of the view.
 * @return A set of application names.
 */
auto LogViewerController::get_app_names(const QUuid& view_id) const -> QSet<QString>
{
    QSet<QString> app_names;

    if (m_view_models.contains(view_id))
    {
        const auto* model = m_view_models[view_id];

        for (const LogEntry& entry: model->get_entries())
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
    return get_app_name_filter(m_current_view_id);
}

/**
 * @brief Returns the application name filter for the specified view.
 * @param view_id The QUuid of the view.
 * @return The application name filter string.
 */
auto LogViewerController::get_app_name_filter(const QUuid& view_id) const -> QString
{
    return m_sort_filter_models.contains(view_id)
               ? m_sort_filter_models[view_id]->get_app_name_filter()
               : QString();
}

/**
 * @brief Returns the available log levels for the specified view.
 *        Currently returns the same static set for all views.
 * @param view_id The QUuid of the view.
 * @return QVector of log level names (e.g., "Trace", "Debug", ...).
 */
auto LogViewerController::get_available_log_levels(const QUuid& view_id) const -> QVector<QString>
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
 * @brief Returns the current set of log levels being filtered.
 * @return The set of log levels.
 */
auto LogViewerController::get_log_level_filters() const -> QSet<QString>
{
    return get_log_level_filters(m_current_view_id);
}

/**
 * @brief Returns a map of log level names to their counts in the specified view.
 * @param view_id The QUuid of the view.
 * @return QMap of log level names to counts.
 */
auto LogViewerController::get_log_level_counts(const QUuid& view_id) const -> QMap<QString, int>
{
    QVector<LogEntry> log_entries = get_log_entries(view_id);
    QMap<QString, int> level_counts;

    for (const auto& entry: log_entries)
    {
        level_counts[entry.get_level()]++;
    }

    return level_counts;
}

/**
 * @brief Returns a map of log level names to their counts in the current view.
 * @return QMap of log level names to counts.
 */
auto LogViewerController::get_log_level_counts() const -> QMap<QString, int>
{
    return get_log_level_counts(m_current_view_id);
}

/**
 * @brief Returns the set of log levels being filtered for the specified view.
 * @param view_id The QUuid of the view.
 * @return The set of log levels.
 */
auto LogViewerController::get_log_level_filters(const QUuid& view_id) const -> QSet<QString>
{
    return m_sort_filter_models.contains(view_id)
               ? m_sort_filter_models[view_id]->get_log_level_filters()
               : QSet<QString>();
}

/**
 * @brief Returns the current search text.
 * @return The search text string.
 */
auto LogViewerController::get_search_text() const -> QString
{
    return get_search_text(m_current_view_id);
}

/**
 * @brief Returns the search text for the specified view.
 * @param view_id The QUuid of the view.
 * @return The search text string.
 */
auto LogViewerController::get_search_text(const QUuid& view_id) const -> QString
{
    return m_sort_filter_models.contains(view_id) ? m_sort_filter_models[view_id]->get_search_text()
                                                  : QString();
}

/**
 * @brief Returns the current search field.
 * @return The search field string.
 */
auto LogViewerController::get_search_field() const -> QString
{
    return get_search_field(m_current_view_id);
}

/**
 * @brief Returns the search field for the specified view.
 * @param view_id The QUuid of the view.
 * @return The search field string.
 */
auto LogViewerController::get_search_field(const QUuid& view_id) const -> QString
{
    return m_sort_filter_models.contains(view_id)
               ? m_sort_filter_models[view_id]->get_search_field()
               : QString();
}

/**
 * @brief Returns whether the search text is treated as a regex.
 * @return True if using regex, false if plain text.
 */
auto LogViewerController::is_search_regex() const -> bool
{
    return is_search_regex(m_current_view_id);
}

/**
 * @brief Returns whether the search text is treated as a regex for the specified view.
 * @param view_id The QUuid of the view.
 * @return True if using regex, false if plain text.
 */
auto LogViewerController::is_search_regex(const QUuid& view_id) const -> bool
{
    return m_sort_filter_models.contains(view_id) ? m_sort_filter_models[view_id]->is_search_regex()
                                                  : false;
}

/**
 * @brief Returns the LogFileTreeModel.
 *
 * This model provides a hierarchical view of log files and their applications.
 * @return Pointer to the LogFileTreeModel.
 */
auto LogViewerController::get_log_file_tree_model() -> LogFileTreeModel*
{
    return m_file_tree_model;
}

/**
 * @brief Returns all log entries in the current view.
 * @return QVector<LogEntry> containing all entries.
 */
auto LogViewerController::get_log_entries() const -> QVector<LogEntry>
{
    return get_log_entries(m_current_view_id);
}

/**
 * @brief Returns all log entries in the specified view.
 * @param view_id The QUuid of the view.
 * @return QVector<LogEntry> containing all entries.
 */
auto LogViewerController::get_log_entries(const QUuid& view_id) const -> QVector<LogEntry>
{
    QVector<LogEntry> result;

    if (m_view_models.contains(view_id))
    {
        const auto* model = m_view_models[view_id];
        result = model->get_entries();
    }

    return result;
}

/**
 * @brief Returns all log entries for a given file in the current view.
 * @param file_info The LogFileInfo for the file.
 * @return QVector<LogEntry> containing all entries for the file.
 */
auto LogViewerController::get_entries_for_file(const LogFileInfo& file_info) -> QVector<LogEntry>
{
    return get_entries_for_file(m_current_view_id, file_info);
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

    if (m_view_models.contains(view_id))
    {
        const auto* model = m_view_models[view_id];

        for (const auto& entry: model->get_entries())
        {
            if (entry.get_file_info().get_file_path() == file_info.get_file_path())
            {
                result.append(entry);
            }
        }
    }

    return result;
}

/**
 * @brief Checks if a log file is already loaded based on its file path.
 * @param file_path The file path to check.
 * @return True if the file is already loaded, false otherwise.
 */
auto LogViewerController::is_file_loaded(const QString& file_path) const -> bool
{
    bool found = false;

    for (const auto& log_file_list: m_loaded_log_files)
    {
        auto it = std::find_if(
            log_file_list.begin(), log_file_list.end(),
            [&file_path](const LogFileInfo& info) { return info.get_file_path() == file_path; });

        if (it != log_file_list.end())
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

    if (m_loaded_log_files.contains(view_id))
    {
        const auto& log_file_list = m_loaded_log_files[view_id];

        auto it = std::find_if(
            log_file_list.begin(), log_file_list.end(),
            [&file_path](const LogFileInfo& info) { return info.get_file_path() == file_path; });

        if (it != log_file_list.end())
        {
            found = true;
        }
    }

    return found;
}

/**
 * @brief Applies a "show only file" filter for the specified view.
 * @param view_id Target view id.
 * @param file_path File path to show exclusively, or empty to reset.
 *
 * Behavior:
 * - Non-empty file_path: activate show-only and ensure that file is visible (remove it from the
 * hidden set).
 * - Empty file_path: deactivate show-only and clear the hidden set so all files are visible again.
 */
auto LogViewerController::set_show_only_file(const QUuid& view_id, const QString& file_path) -> void
{
    auto* proxy = get_sort_filter_proxy(view_id);

    if (proxy != nullptr)
    {
        proxy->set_show_only_file_path(file_path);

        if (file_path.isEmpty())
        {
            // "Show All Files"
            proxy->clear_hidden_files();
        }
        else
        {
            // Ensure the selected file is visible even if it was explicitly hidden
            proxy->unhide_file(file_path);
        }
    }
}

/**
 * @brief Toggles a file's visibility (hide/show) in the specified view.
 *
 * Behavior:
 * - No show-only active:
 *   - Toggle hide/unhide for the requested file.
 * - Show-only is active for file A:
 *   - Toggle on A:
 *     - Clear show-only and hide all files (empty view).
 *   - Toggle on a different file B:
 *     - Clear show-only and make B visible. Convert current effective-hidden into explicit hidden
 *       for all other files. Preserve previously explicit hidden files, excluding A and B.
 *
 * Explicit "Show only" action is handled via set_show_only_file(view_id, file_path).
 *
 * @param view_id Target view id.
 * @param file_path Absolute file path to toggle.
 */
auto LogViewerController::toggle_file_visibility(const QUuid& view_id,
                                                 const QString& file_path) -> void
{
    auto* proxy = get_sort_filter_proxy(view_id);

    // Guard: no-op if proxy is invalid or path is empty.
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

                const QVector<QString> files = get_view_file_paths(view_id);
                QSet<QString> all_hidden;
                for (const auto& p: files)
                {
                    all_hidden.insert(p);
                }
                proxy->set_hidden_file_paths(all_hidden);
            }
            else
            {
                // Toggling a different file while show-only is active:
                // Clear show-only and make this file visible (unhide if necessary).
                // Convert current effective-hidden into explicit hidden for all files except
                // the previous show-only (show_only) and the requested file (file_path).
                proxy->set_show_only_file_path(QString());

                if (is_hidden)
                {
                    proxy->unhide_file(file_path);
                }

                const QVector<QString> files = get_view_file_paths(view_id);
                QSet<QString> new_hidden;

                // Hide all files except {show_only, file_path}
                for (const auto& p: files)
                {
                    const bool keep_visible = (p == show_only) || (p == file_path);
                    if (!keep_visible)
                    {
                        new_hidden.insert(p);
                    }
                }

                // Preserve previously explicit hidden files, excluding {show_only, file_path}
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
 * @brief Hides (excludes) a specific file in the specified view.
 * @param view_id Target view id.
 * @param file_path File path to hide.
 */
auto LogViewerController::hide_file(const QUuid& view_id, const QString& file_path) -> void
{
    auto* proxy = get_sort_filter_proxy(view_id);

    if (proxy != nullptr)
    {
        proxy->hide_file(file_path);
    }
}

/**
 * @brief Returns absolute file paths loaded in the specified view.
 * @param view_id The QUuid of the view.
 * @return Vector of file paths loaded in the view (empty if none).
 */
auto LogViewerController::get_view_file_paths(const QUuid& view_id) const -> QVector<QString>
{
    QVector<QString> result;

    if (m_loaded_log_files.contains(view_id))
    {
        const auto& infos = m_loaded_log_files[view_id];
        for (const auto& info: infos)
        {
            result.append(info.get_file_path());
        }
    }

    return result;
}

/**
 * @brief Removes a single log file from all views and from the LogFileTreeModel.
 *        If a view becomes empty, it is deleted and view_removed() is emitted.
 * @param file The LogFileInfo object to remove.
 */
auto LogViewerController::remove_log_file(const LogFileInfo& file) -> void
{
    QList<QUuid> views_to_remove;

    // Remove file from all views and purge associated model entries
    for (auto it = m_loaded_log_files.begin(); it != m_loaded_log_files.end(); ++it)
    {
        auto& log_file_list = it.value();

        log_file_list.erase(std::remove_if(log_file_list.begin(), log_file_list.end(),
                                           [&file](const LogFileInfo& info) {
                                               return info.get_file_path() == file.get_file_path();
                                           }),
                            log_file_list.end());

        const QUuid& view_id = it.key();

        if (m_view_models.contains(view_id))
        {
            m_view_models[view_id]->remove_entries_by_file_path(file.get_file_path());

            if (m_view_models[view_id]->get_entries().isEmpty())
            {
                views_to_remove.append(view_id);
            }
        }

        // Notify with updated file paths even if the view becomes empty
        emit view_file_paths_changed(view_id, get_view_file_paths(view_id));

        // Preserve hidden-effective state when the show-only target is removed:
        // If show-only pointed to the removed file, clear show-only and mark all remaining files
        // explicitly hidden.
        auto* sort_proxy = get_sort_filter_proxy(view_id);
        if (sort_proxy != nullptr)
        {
            const QString show_only_path = sort_proxy->get_show_only_file_path();
            const bool show_only_target_removed = (show_only_path == file.get_file_path());

            if (show_only_target_removed)
            {
                sort_proxy->set_show_only_file_path(QString());

                const QVector<QString> remaining_files = get_view_file_paths(view_id);
                QSet<QString> all_hidden;
                for (const auto& p: remaining_files)
                {
                    all_hidden.insert(p);
                }
                sort_proxy->set_hidden_file_paths(all_hidden);
            }
        }
    }

    // Remove file from the tree model
    if (m_file_tree_model != nullptr)
    {
        m_file_tree_model->remove_log_file(file);
    }

    // Remove views that became empty and notify listeners
    for (const QUuid& view_id: views_to_remove)
    {
        remove_view(view_id);
        emit view_removed(view_id);
    }
}

/**
 * @brief Removes a single log file from the specified view only.
 *
 * If the target view becomes empty, it is removed and `view_removed(view_id)` is emitted.
 *
 * @param view_id The target view.
 * @param file_path Absolute file path to remove from the view.
 */
auto LogViewerController::remove_log_file(const QUuid& view_id, const QString& file_path) -> void
{
    const bool has_valid_args = !view_id.isNull() && !file_path.isEmpty();
    bool view_became_empty = false;

    if (has_valid_args)
    {
        // Remove file from the loaded list for this view
        if (m_loaded_log_files.contains(view_id))
        {
            auto& log_file_list = m_loaded_log_files[view_id];
            log_file_list.erase(std::remove_if(log_file_list.begin(), log_file_list.end(),
                                               [&file_path](const LogFileInfo& info) {
                                                   return info.get_file_path() == file_path;
                                               }),
                                log_file_list.end());
        }

        // Remove entries from the model for this view
        if (m_view_models.contains(view_id))
        {
            m_view_models[view_id]->remove_entries_by_file_path(file_path);
            view_became_empty = m_view_models[view_id]->get_entries().isEmpty();
        }

        // Keep per-file filters consistent in this view
        auto* sort_proxy = get_sort_filter_proxy(view_id);
        if (sort_proxy != nullptr)
        {
            const QString show_only = sort_proxy->get_show_only_file_path();
            const bool removed_was_show_only = (show_only == file_path);

            if (removed_was_show_only)
            {
                sort_proxy->set_show_only_file_path(QString());

                const QVector<QString> remaining = get_view_file_paths(view_id);
                QSet<QString> all_hidden;
                for (const auto& p: remaining)
                {
                    all_hidden.insert(p);
                }
                sort_proxy->set_hidden_file_paths(all_hidden);
            }
            else
            {
                QSet<QString> hidden = sort_proxy->get_hidden_file_paths();
                if (hidden.contains(file_path))
                {
                    hidden.remove(file_path);
                    sort_proxy->set_hidden_file_paths(hidden);
                }
            }
        }

        emit view_file_paths_changed(view_id, get_view_file_paths(view_id));
    }

    // Handle empty view cleanup outside the main branch to keep a single exit point
    if (has_valid_args && view_became_empty)
    {
        remove_view(view_id);
        emit view_removed(view_id);
    }
}

/**
 * @brief Enqueues an asynchronous load request for a log file.
 * @param view_id The QUuid of the view to load into.
 * @param file_path The path to the log file.
 */
auto LogViewerController::enqueue_async(const QUuid& view_id, const QString& file_path) -> void
{
    m_async_queue.append(qMakePair(view_id, file_path));
}

/**
 * @brief Attempts to start the next asynchronous load if none is active.
 * @param batch_size Number of entries per batch.
 */
auto LogViewerController::try_start_next_async(qsizetype batch_size) -> void
{
    bool started = false;

    if (m_active_file_path.isEmpty() && !m_async_queue.isEmpty())
    {
        const auto pair = m_async_queue.front();
        m_async_queue.pop_front();
        m_active_view_id = pair.first;
        m_active_file_path = pair.second;
        m_active_batch_size = batch_size;

        if (m_current_view_id != m_active_view_id)
        {
            set_current_view(m_active_view_id);
        }

        m_loader.load_log_file_async(m_active_file_path, m_active_batch_size);
        started = true;
    }

    if (!started)
    {
        // Nothing to start; remain idle.
    }
}

/**
 * @brief Callback when an asynchronous load batch is received.
 * @param view_id The QUuid of the view receiving data.
 * @param file_path The path to the log file.
 * @param entries The batch of log entries.
 */
auto LogViewerController::clear_pending_for_view(const QUuid& view_id) -> void
{
    QList<QPair<QUuid, QString>> kept;

    for (const auto& it: std::as_const(m_async_queue))
    {
        if (it.first != view_id)
        {
            kept.append(it);
        }
    }

    m_async_queue = kept;

    if (m_active_view_id == view_id)
    {
        // Active will be stopped by cancel, try to start next after it finishes via loader signal.
    }
}

/**
 * @brief Ensures that models and proxies exist for the specified view ID.
 * @param view_id The QUuid of the view.
 */
auto LogViewerController::ensure_view_models(const QUuid& view_id) -> void
{
    if (!m_view_models.contains(view_id))
    {
        auto* model = new LogModel(this);
        auto* sort_proxy = new LogSortFilterProxyModel(this);
        sort_proxy->setSourceModel(model);
        auto* paging_proxy = new PagingProxyModel(this);
        paging_proxy->setSourceModel(sort_proxy);

        m_view_models[view_id] = model;
        m_sort_filter_models[view_id] = sort_proxy;
        m_paging_models[view_id] = paging_proxy;
    }
}
