#include "Qt-LogViewer/Controllers/LogViewerController.h"

#include <algorithm>

/**
 * @brief Constructs a LogViewerController.
 * @param log_format The log format string for parsing.
 * @param parent The parent QObject.
 */
LogViewerController::LogViewerController(const QString& log_format, QObject* parent)
    : QObject(parent), m_loader(log_format), m_file_tree_model(new LogFileTreeModel(this))
{}

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

    return removed;
}

/**
 * @brief Adds a single log file to the LogFileTreeModel.
 * @param file_path The path to the log file.
 */
auto LogViewerController::add_log_file_to_tree(const QString& file_path) -> void
{
    LogEntry entry = m_loader.read_first_log_entry(file_path);
    m_file_tree_model->add_log_file(entry.get_file_info());
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

    auto* model = new LogModel(this);
    model->add_entries(entries);

    auto* sort_proxy = new LogSortFilterProxyModel(this);
    sort_proxy->setSourceModel(model);

    auto* paging_proxy = new PagingProxyModel(this);
    paging_proxy->setSourceModel(sort_proxy);

    m_loaded_log_files[view_id] = QList<LogFileInfo>{loaded_log_file};
    m_view_models[view_id] = model;
    m_sort_filter_models[view_id] = sort_proxy;
    m_paging_models[view_id] = paging_proxy;

    return view_id;
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
    auto* model = new LogModel(this);

    for (const QString& file_path: file_paths)
    {
        auto entries = m_loader.load_log_file(file_path);
        QString app_name = (!entries.isEmpty()) ? entries.first().get_app_name()
                                                : LogLoader::identify_app(file_path);

        LogFileInfo log_file_info(file_path, app_name);
        loaded_log_files.append(log_file_info);
        model->add_entries(entries);
    }

    auto view_id = QUuid::createUuid();

    auto* sort_proxy = new LogSortFilterProxyModel(this);
    sort_proxy->setSourceModel(model);

    auto* paging_proxy = new PagingProxyModel(this);
    paging_proxy->setSourceModel(sort_proxy);

    m_loaded_log_files[view_id] = loaded_log_files;
    m_view_models[view_id] = model;
    m_sort_filter_models[view_id] = sort_proxy;
    m_paging_models[view_id] = paging_proxy;

    return view_id;
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
 * @brief Returns the current set of log levels being filtered.
 * @return The set of log levels.
 */
auto LogViewerController::get_log_level_filters() const -> QSet<QString>
{
    return get_log_level_filters(m_current_view_id);
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
    return false;
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
auto LogViewerController::get_log_entries() -> QVector<LogEntry>
{
    return get_log_entries(m_current_view_id);
}

/**
 * @brief Returns all log entries in the specified view.
 * @param view_id The QUuid of the view.
 * @return QVector<LogEntry> containing all entries.
 */
auto LogViewerController::get_log_entries(const QUuid& view_id) -> QVector<LogEntry>
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
 * @brief Removes a single log file from all views and from the LogFileTreeModel.
 *        If a view becomes empty, it is deleted and view_removed() is emitted.
 * @param file The LogFileInfo object to remove.
 */
auto LogViewerController::remove_log_file(const LogFileInfo& file) -> void
{
    QList<QUuid> views_to_remove;

    // Remove from all views
    for (auto it = m_loaded_log_files.begin(); it != m_loaded_log_files.end(); ++it)
    {
        auto& log_file_list = it.value();
        log_file_list.erase(std::remove_if(log_file_list.begin(), log_file_list.end(),
                                           [&file](const LogFileInfo& info) {
                                               return info.get_file_path() == file.get_file_path();
                                           }),
                            log_file_list.end());

        // Remove entries from the LogModel
        const QUuid& view_id = it.key();
        if (m_view_models.contains(view_id))
        {
            m_view_models[view_id]->remove_entries_by_file_path(file.get_file_path());

            if (m_view_models[view_id]->get_entries().isEmpty())
            {
                views_to_remove.append(view_id);
            }
        }
    }

    // Remove from the tree model
    if (m_file_tree_model != nullptr)
    {
        m_file_tree_model->remove_log_file(file);
    }

    for (const QUuid& view_id: views_to_remove)
    {
        remove_view(view_id);
        emit view_removed(view_id);
    }
}
