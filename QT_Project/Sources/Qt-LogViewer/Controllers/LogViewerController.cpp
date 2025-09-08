#include "Qt-LogViewer/Controllers/LogViewerController.h"

#include <algorithm>

/**
 * @brief Constructs a LogViewerController.
 * @param log_format The log format string for parsing.
 * @param parent The parent QObject.
 */
LogViewerController::LogViewerController(const QString& log_format, QObject* parent)
    : QObject(parent),
      m_loader(log_format),
      m_log_model(new LogModel(this)),
      m_file_tree_model(new LogFileTreeModel(this)),
      m_proxy_model(new LogFilterProxyModel(this))
{
    m_proxy_model->setSourceModel(m_log_model);
}

/**
 * @brief Loads log files from the specified file paths.
 * @param file_paths A vector of file paths to load logs from.
 *
 * This method uses the LogLoader service to parse log files and populate the LogModel.
 * It also updates the LogFileTreeModel with information about the loaded log files.
 */
auto LogViewerController::load_logs(const QVector<QString>& file_paths) -> void
{
    QVector<LogEntry> all_entries;
    QVector<LogEntry> entries;
    QList<LogFileInfo> log_file_infos;
    QString app_name;
    LogFileInfo loaded_log_file;

    for (const QString& file_path: file_paths)
    {
        if (!is_file_loaded(file_path))
        {
            entries = m_loader.load_log_file(file_path);

            if (!entries.isEmpty())
            {
                app_name = entries.first().get_app_name();
            }
            else
            {
                app_name = LogLoader::identify_app(file_path);
            }

            loaded_log_file = LogFileInfo(file_path, app_name);
            log_file_infos.append(loaded_log_file);
            m_loaded_log_files.append(loaded_log_file);
            all_entries += entries;
        }
    }

    add_log_files(log_file_infos);
    m_log_model->add_entries(all_entries);
}

/**
 * @brief Sets the application name filter.
 * @param app_name The application name to filter by.
 */
auto LogViewerController::set_app_name_filter(const QString& app_name) -> void
{
    m_proxy_model->set_app_name_filter(app_name);
}

/**
 * @brief Sets the log level filter.
 * @param levels The set of log levels.
 */
auto LogViewerController::set_level_filter(const QSet<QString>& levels) -> void
{
    m_proxy_model->set_level_filter(levels);
}

/**
 * @brief Sets the search filter.
 * @param search_text The search string or regex.
 * @param field The field to search in.
 * @param use_regex Whether to use regex.
 */
auto LogViewerController::set_search_filter(const QString& search_text, const QString& field,
                                            bool use_regex) -> void
{
    m_proxy_model->set_search_filter(search_text, field, use_regex);
}

/**
 * @brief Returns the LogModel.
 */
auto LogViewerController::get_log_model() -> LogModel*
{
    return m_log_model;
}

/**
 * @brief Returns the LogFilterProxyModel.
 */
auto LogViewerController::get_proxy_model() -> LogFilterProxyModel*
{
    return m_proxy_model;
}

/**
 * @brief Returns the set of unique application names from the loaded logs.
 * @return A set of application names.
 */
auto LogViewerController::get_app_names() const -> QSet<QString>
{
    QSet<QString> app_names;

    for (const LogEntry& entry: m_log_model->get_entries())
    {
        app_names.insert(entry.get_app_name());
    }

    return app_names;
}

/**
 * @brief Returns the LogFileTreeModel.
 *
 * This model provides a hierarchical view of log files and their applications.
 */
auto LogViewerController::get_log_file_tree_model() -> LogFileTreeModel*
{
    return m_file_tree_model;
}

/**
 * @brief Returns all log entries for a given file.
 * @param file_info The LogFileInfo for the file.
 * @return QVector<LogEntry> containing all entries for the file.
 */
auto LogViewerController::get_entries_for_file(const LogFileInfo& file_info) -> QVector<LogEntry>
{
    QVector<LogEntry> result;

    for (const auto& entry: m_log_model->get_entries())
    {
        if (entry.get_file_info().get_file_path() == file_info.get_file_path())
        {
            result.append(entry);
        }
    }

    return result;
}

/**
 * @brief Adds a single log file to the model.
 * @param file The LogFileInfo object to add.
 *
 * This method adds a single log file to the LogFileTreeModel. If the model is not initialized,
 * it does nothing.
 */
auto LogViewerController::add_log_file(const LogFileInfo& file) -> void
{
    if (m_file_tree_model != nullptr)
    {
        m_file_tree_model->add_log_file(file);
    }
}

/**
 * @brief Adds multiple log files to the model.
 * @param files The list of LogFileInfo objects to add.
 *
 * This method iterates through the provided list of log files and adds each one to the
 * LogFileTreeModel.
 */
auto LogViewerController::add_log_files(const QList<LogFileInfo>& files) -> void
{
    if (m_file_tree_model != nullptr)
    {
        for (const auto& file: files)
        {
            m_file_tree_model->add_log_file(file);
        }
    }
}

/**
 * @brief Checks if a log file is already loaded based on its file path.
 * @param file_path The file path to check.
 * @return True if the file is already loaded, false otherwise.
 */
auto LogViewerController::is_file_loaded(const QString& file_path) const -> bool
{
    auto it = std::find_if(
        m_loaded_log_files.begin(), m_loaded_log_files.end(),
        [&file_path](const LogFileInfo& info) { return info.get_file_path() == file_path; });

    return (it != m_loaded_log_files.end());
}

/**
 * @brief Removes a single log file from the model.
 * @param log_file_infos The LogFileInfo object to remove.
 *
 * This method removes the specified log file from the LogFileTreeModel and also
 * from the internal list of loaded log files.
 */
auto LogViewerController::remove_log_file(const LogFileInfo& log_file_infos) -> void
{
    m_file_tree_model->remove_log_file(log_file_infos);
    m_log_model->remove_entries_by_file_path(log_file_infos.get_file_path());

    auto it = std::remove_if(m_loaded_log_files.begin(), m_loaded_log_files.end(),
                             [&log_file_infos](const LogFileInfo& info) {
                                 return info.get_file_path() == log_file_infos.get_file_path();
                             });

    if (it != m_loaded_log_files.end())
    {
        m_loaded_log_files.erase(it, m_loaded_log_files.end());
    }
}
