/**
 * @file LogLoader.cpp
 * @brief This file contains the implementation of the LogLoader class.
 */

#include "Qt-LogViewer/Services/LogLoader.h"

#include <QFileInfo>

/**
 * @brief Constructs a LogLoader object.
 * @param format_string The log format string for parsing.
 */
LogLoader::LogLoader(const QString& format_string): m_parser(format_string) {}

/**
 * @brief Loads and parses a single log file.
 * @param file_path The path to the log file.
 * @return A QVector of LogEntry objects parsed from the file.
 */
auto LogLoader::load_log_file(const QString& file_path) const -> QVector<LogEntry>
{
    return m_parser.parse_file(file_path);
}

/**
 * @brief Loads and parses multiple log files, grouping entries by application name.
 * @param file_paths A list of log file paths.
 * @return A map from application name to a vector of LogEntry objects.
 */
auto LogLoader::load_logs_by_app(const QVector<QString>& file_paths) const
    -> QMap<QString, QVector<LogEntry>>
{
    QMap<QString, QVector<LogEntry>> app_logs;

    for (const QString& file_path: file_paths)
    {
        QVector<LogEntry> entries = load_log_file(file_path);
        QString app_name;

        if (!entries.isEmpty())
        {
            app_name = entries.first().get_app_name();
        }
        else
        {
            app_name = identify_app(file_path);
        }

        if (!app_name.isEmpty())
        {
            app_logs[app_name] += entries;
        }
    }

    return app_logs;
}

/**
 * @brief Identifies the application name for a given log file path.
 *        This implementation uses the base file name (without extension) as the app name.
 * @param file_path The path to the log file.
 * @return The application name, or an empty string if not identifiable.
 */
auto LogLoader::identify_app(const QString& file_path) -> QString
{
    QFileInfo info(file_path);
    return info.baseName();
}
