#include "Qt-LogViewer/Controllers/LogViewerController.h"

/**
 * @brief Constructs a LogViewerController.
 * @param log_format The log format string for parsing.
 * @param parent The parent QObject.
 */
LogViewerController::LogViewerController(const QString& log_format, QObject* parent)
    : QObject(parent), m_loader(log_format)
{
    m_proxy_model.setSourceModel(&m_log_model);
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

    m_log_model.set_entries(entries);
}

/**
 * @brief Sets the application name filter.
 * @param app_name The application name to filter by.
 */
auto LogViewerController::set_app_name_filter(const QString& app_name) -> void
{
    m_proxy_model.set_app_name_filter(app_name);
}

/**
 * @brief Sets the log level filter.
 * @param levels The set of log levels.
 */
auto LogViewerController::set_level_filter(const QSet<QString>& levels) -> void
{
    m_proxy_model.set_level_filter(levels);
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
    m_proxy_model.set_search_filter(search_text, field, use_regex);
}

/**
 * @brief Returns the LogModel.
 */
auto LogViewerController::get_log_model() -> LogModel*
{
    return &m_log_model;
}

/**
 * @brief Returns the LogFilterProxyModel.
 */
auto LogViewerController::get_proxy_model() -> LogFilterProxyModel*
{
    return &m_proxy_model;
}
