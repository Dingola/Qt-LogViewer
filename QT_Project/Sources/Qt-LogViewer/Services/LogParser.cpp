/**
 * @file LogParser.cpp
 * @brief This file contains the implementation of the LogParser class.
 */

#include "Qt-LogViewer/Services/LogParser.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>

#include "Qt-LogViewer/Models/LogFileInfo.h"

/**
 * @brief Constructs a LogParser object from a format string.
 * @param format_string The format string (e.g. "{timestamp} {level} {message} {app_name}").
 */
LogParser::LogParser(const QString& format_string)
{
    auto pair = format_string_to_regex(format_string);

    m_pattern = pair.first;
    m_field_order = pair.second;
}

/**
 * @brief Parses a log file and returns a list of LogEntry objects.
 * @param file_path The path to the log file.
 * @return A QVector of LogEntry objects parsed from the file.
 */
auto LogParser::parse_file(const QString& file_path) const -> QVector<LogEntry>
{
    QVector<LogEntry> entries;

    QFile file(file_path);
    bool file_opened = file.open(QIODevice::ReadOnly | QIODevice::Text);

    if (file_opened)
    {
        QTextStream in(&file);

        while (!in.atEnd())
        {
            QString line = in.readLine();
            LogEntry entry = parse_line(line, file_path);

            if (!entry.get_level().isEmpty())
            {
                entries.append(entry);
            }
        }
    }

    return entries;
}

/**
 * @brief Parses a single log line and returns a LogEntry object.
 * @param line The log line to parse.
 * @return The parsed LogEntry, or a default LogEntry if parsing fails.
 */
auto LogParser::parse_line(const QString& line, const QString& file_path) const -> LogEntry
{
    LogEntry result;
    QRegularExpressionMatch match = m_pattern.match(line);

    if (match.hasMatch())
    {
        QMap<QString, QString> values;

        for (int i = 0; i < m_field_order.fields.size(); ++i)
        {
            values[m_field_order.fields[i]] = match.captured(i + 1);
        }

        QDateTime timestamp;

        if (values.contains("timestamp"))
        {
            timestamp = QDateTime::fromString(values["timestamp"], "yyyy-MM-dd HH:mm:ss");
        }

        QString level = values.value("level");
        QString message = values.value("message").trimmed();
        QString app_name = values.value("app_name");

        LogFileInfo file_info(file_path, app_name);
        result = LogEntry(timestamp, level, message, file_info);
    }

    return result;
}

/**
 * @brief Returns the regular expression pattern used for parsing.
 * @return The QRegularExpression pattern.
 */
auto LogParser::get_pattern() const -> QRegularExpression
{
    return m_pattern;
}

/**
 * @brief Returns the field order used for parsing.
 * @return The LogFieldOrder struct.
 */
auto LogParser::get_field_order() const -> LogFieldOrder
{
    return m_field_order;
}

/**
 * @brief Converts a format string to a regular expression and field order.
 * @param format The format string (e.g. "{timestamp} {level} {message} {app_name}").
 * @return A pair containing the generated QRegularExpression and the LogFieldOrder.
 *
 * This helper function replaces placeholders in the format string with appropriate
 * regular expression groups and records the order of the fields for later extraction.
 */
auto LogParser::format_string_to_regex(const QString& format)
    -> QPair<QRegularExpression, LogFieldOrder>
{
    QVector<QString> fields;
    QString regex_pattern;
    int last_pos = 0;
    QRegularExpression placeholder(R"(\{(\w+)\})");
    QRegularExpressionMatchIterator it = placeholder.globalMatch(format);

    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        int start = match.capturedStart();
        int end = match.capturedEnd();

        regex_pattern += QRegularExpression::escape(format.mid(last_pos, start - last_pos));

        QString field = match.captured(1);
        fields.append(field);

        if (field == "timestamp")
        {
            regex_pattern += R"((\d{4}-\d{2}-\d{2}\ \d{2}:\d{2}:\d{2}))";
        }
        else if (field == "level")
        {
            regex_pattern += R"((\w+))";
        }
        else if (field == "app_name")
        {
            regex_pattern += R"((\S+))";
        }
        else if (field == "line")
        {
            regex_pattern += R"((\d+))";
        }
        else
        {
            regex_pattern += R"((.*?))";
        }

        last_pos = end;
    }

    regex_pattern += QRegularExpression::escape(format.mid(last_pos));
    regex_pattern = "^" + regex_pattern + "$";

    QPair<QRegularExpression, LogFieldOrder> result(QRegularExpression(regex_pattern),
                                                    LogFieldOrder{fields});

    return result;
}
