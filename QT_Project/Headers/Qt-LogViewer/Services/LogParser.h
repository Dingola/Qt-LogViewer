#pragma once

#include <QMap>
#include <QRegularExpression>
#include <QString>
#include <QVector>

#include "Qt-LogViewer/Models/LogEntry.h"

/**
 * @file LogParser.h
 * @brief This file contains the definition of the LogParser class.
 */

/**
 * @struct LogFieldOrder
 * @brief Stores the order of fields as they appear in the format string.
 */
struct LogFieldOrder {
        QVector<QString> fields;
};

/**
 * @class LogParser
 * @brief Parses log files and extracts LogEntry objects from each line using a format string.
 *
 * This class provides methods to parse log files or log lines and convert them
 * into LogEntry objects for use in the LogModel. The log format is defined by a format string.
 */
class LogParser
{
    public:
        /**
         * @brief Constructs a LogParser object from a format string.
         * @param format_string The format string (e.g. "{timestamp} {level} {message} {app_name}").
         */
        explicit LogParser(const QString& format_string);

        /**
         * @brief Destroys the LogParser object.
         */
        ~LogParser() = default;

        /**
         * @brief Parses a log file and returns a list of LogEntry objects.
         * @param file_path The path to the log file.
         * @return A QVector of LogEntry objects parsed from the file.
         */
        [[nodiscard]] auto parse_file(const QString& file_path) const -> QVector<LogEntry>;

        /**
         * @brief Parses a single log line and returns a LogEntry object.
         * @param line The log line to parse.
         * @return The parsed LogEntry, or a default LogEntry if parsing fails.
         */
        [[nodiscard]] auto parse_line(const QString& line) const -> LogEntry;

        /**
         * @brief Returns the regular expression pattern used for parsing.
         * @return The QRegularExpression pattern.
         */
        [[nodiscard]] auto get_pattern() const -> QRegularExpression;

        /**
         * @brief Returns the field order used for parsing.
         * @return The LogFieldOrder struct.
         */
        [[nodiscard]] auto get_field_order() const -> LogFieldOrder;

    private:
        QRegularExpression m_pattern;
        LogFieldOrder m_field_order;

    private:
        static QPair<QRegularExpression, LogFieldOrder> format_string_to_regex(
            const QString& format);
};
