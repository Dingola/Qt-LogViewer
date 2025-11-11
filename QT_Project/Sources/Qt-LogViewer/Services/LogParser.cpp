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

    // Default timestamp formats to try (ISO formats are tried separately first).
    m_timestamp_formats = QVector<QString>{"yyyy-MM-dd HH:mm:ss", "yyyy-MM-dd HH:mm:ss.zzz",
                                           "yyyy-MM-ddTHH:mm:ss", "yyyy-MM-ddTHH:mm:ss.zzz",
                                           "dd.MM.yyyy HH:mm:ss", "dd.MM.yyyy HH:mm:ss.zzz",
                                           "MM/dd/yyyy HH:mm:ss", "MM/dd/yyyy HH:mm:ss.zzz",
                                           "yyyy/MM/dd HH:mm:ss", "yyyy/MM/dd HH:mm:ss.zzz"};
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
 * @param file_path The originating file path for contextual metadata.
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
            timestamp = parse_timestamp(values["timestamp"]);
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
 * @brief Sets the list of accepted timestamp formats attempted during parsing.
 *        ISO-8601 formats are always tried first.
 * @param formats A list of timestamp format strings understood by QDateTime::fromString.
 */
auto LogParser::set_timestamp_formats(const QVector<QString>& formats) -> void
{
    m_timestamp_formats = formats;
}

/**
 * @brief Returns the list of accepted timestamp formats attempted during parsing.
 * @return The list of timestamp formats.
 */
auto LogParser::get_timestamp_formats() const -> QVector<QString>
{
    return m_timestamp_formats;
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
            // Relaxed capture for timestamp; actual validation is done in parse_timestamp().
            regex_pattern += R"((.*?))";
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

/**
 * @brief Attempts to parse a timestamp value using ISO-8601 and configured formats.
 * @param value The timestamp string as captured from the log line.
 * @return The parsed QDateTime, or an invalid QDateTime if parsing fails.
 */
auto LogParser::parse_timestamp(const QString& value) const -> QDateTime
{
    QDateTime parsed;

    // Try ISO-8601 with and without milliseconds first.
    if (!parsed.isValid())
    {
        parsed = QDateTime::fromString(value, Qt::ISODateWithMs);
    }
    if (!parsed.isValid())
    {
        parsed = QDateTime::fromString(value, Qt::ISODate);
    }

    // Try configured explicit formats.
    if (!parsed.isValid())
    {
        for (qsizetype i = 0; i < m_timestamp_formats.size(); ++i)
        {
            const QString fmt = m_timestamp_formats.at(i);
            QDateTime candidate = QDateTime::fromString(value, fmt);
            if (candidate.isValid())
            {
                parsed = candidate;
                i = m_timestamp_formats.size();
            }
        }
    }

    return parsed;
}
