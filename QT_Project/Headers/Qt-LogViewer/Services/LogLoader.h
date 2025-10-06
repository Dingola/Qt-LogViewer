#pragma once

#include <QMap>
#include <QString>
#include <QVector>

#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Services/LogParser.h"

/**
 * @file LogLoader.h
 * @brief This file contains the definition of the LogLoader class.
 */

/**
 * @class LogLoader
 * @brief Loads log files, identifies their application, and parses them into LogEntry objects.
 *
 * This class is responsible for loading log files from disk, determining the application
 * source for each log, and using LogParser to extract LogEntry objects.
 */
class LogLoader
{
    public:
        /**
         * @brief Constructs a LogLoader object.
         * @param format_string The log format string for parsing.
         */
        explicit LogLoader(const QString& format_string);

        /**
         * @brief Destroys the LogLoader object.
         */
        ~LogLoader() = default;

        /**
         * @brief Loads and parses a single log file.
         * @param file_path The path to the log file.
         * @return A QVector of LogEntry objects parsed from the file.
         */
        [[nodiscard]] auto load_log_file(const QString& file_path) const -> QVector<LogEntry>;

        /**
         * @brief Loads and parses multiple log files.
         * @param file_paths A list of log file paths.
         * @return A map from application name to a vector of LogEntry objects.
         */
        [[nodiscard]] auto load_logs_by_app(const QVector<QString>& file_paths) const
            -> QMap<QString, QVector<LogEntry>>;

        /**
         * @brief Reads only the first log entry from the given file.
         * @param file_path The path to the log file.
         * @return The first LogEntry if available, otherwise a default LogEntry.
         */
        [[nodiscard]] auto read_first_log_entry(const QString& file_path) const -> LogEntry;

        /**
         * @brief Identifies the application name for a given log file path.
         * @param file_path The path to the log file.
         * @return The application name, or an empty string if not identifiable.
         */
        [[nodiscard]] static auto identify_app(const QString& file_path) -> QString;

    private:
        LogParser m_parser;
};
