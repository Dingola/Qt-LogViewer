#include "Qt-LogViewer/Controllers/LogViewerController.h"

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
 * @brief Fügt ein einzelnes LogFileInfo dem LogFileTreeModel hinzu.
 * @param file Das LogFileInfo-Objekt, das hinzugefügt werden soll.
 */
auto LogViewerController::add_log_file(const LogFileInfo& file) -> void
{
    if (m_file_tree_model != nullptr)
    {
        m_file_tree_model->add_log_file(file);
    }
}

/**
 * @brief Fügt mehrere LogFileInfo-Objekte dem LogFileTreeModel hinzu.
 * @param files Die Liste der LogFileInfo-Objekte, die hinzugefügt werden sollen.
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
 * @brief Loads log files and updates the model.
 * @param file_paths The list of log file paths.
 */
auto LogViewerController::load_logs(const QVector<QString>& file_paths) -> void
{
    QVector<LogEntry> entries;
    QMap<QString, QVector<LogEntry>> app_logs = m_loader.load_logs_by_app(file_paths);

    // Flatten all entries into a single list for the model
    for (const auto& entry_list: app_logs)
    {
        entries += entry_list;
    }

    m_log_model->add_entries(entries);
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
