/**
 * @file LogParser.cpp
 * @brief Implements the log-specific QtRecordParser adapter.
 */

#include "Qt-LogViewer/Services/LogParser.h"

#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <utility>

#include "Qt-LogViewer/Models/LogFieldDefinition.h"
#include "Qt-LogViewer/Models/LogFileInfo.h"

/**
 * @brief Constructs the log-specific parser adapter for a profile.
 * @param profile Parsing profile used by this adapter.
 * @param registry Registry containing built-in and custom converters.
 *
 * The profile is copied into the adapter; the supplied registry provides built-in and optional
 * custom converters to the delegated parser.
 */
LogParser::LogParser(LogParsingProfile profile, QtRecordParser::ConverterRegistry registry)
    : m_profile(std::move(profile)), m_parser(m_profile.get_configuration(), std::move(registry))
{}

/**
 * @brief Parses a complete log file line by line.
 *
 * The source line number is included in every intermediate outcome. Failed records are skipped;
 * diagnostic aggregation is handled by a later ingestion layer rather than represented by an
 * empty LogEntry.
 *
 * The returned entries retain their original source order.
 *
 * @param file_path Path of the file to read and use as record source.
 * @return Successfully parsed and adapted log entries in source order.
 */
auto LogParser::parse_file(const QString& file_path) const -> QVector<LogEntry>
{
    QVector<LogEntry> entries;
    QFile file(file_path);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        QString line;
        qsizetype line_number = 0;

        while (stream.readLineInto(&line))
        {
            ++line_number;
            const LogParseOutcome outcome = parse_line(line, file_path, line_number);

            if (outcome.succeeded())
            {
                entries.append(outcome.entry.value());
            }
        }
    }

    return entries;
}

/**
 * @brief Delegates generic record parsing to QtRecordParser.
 *
 * QtRecordParser receives the raw input and its file, stream or dataset identifier unchanged and
 * returns either dynamic parsed values or structured failure information.
 *
 * @param line Raw input record.
 * @param source File, stream or dataset identifier stored in the parsed record.
 * @return Generic QtRecordParser result containing dynamic values or a structured error.
 */
auto LogParser::parse_record(const QString& line,
                             const QString& source) const -> QtRecordParser::ParseResult
{
    return m_parser.parse(line, source);
}

/**
 * @brief Parses one source line and adapts a successful result to LogEntry.
 *
 * The raw line, originating file path and optional one-based line number remain available in the
 * explicit outcome. LogEntry adaptation runs only after generic parsing succeeds.
 *
 * @param line Raw input line.
 * @param file_path Originating file path.
 * @param line_number One-based source line number, or -1 when unknown.
 * @return Outcome containing source context, generic parse result and an optional entry.
 */
auto LogParser::parse_line(const QString& line, const QString& file_path,
                           qsizetype line_number) const -> LogParseOutcome
{
    LogParseOutcome outcome;
    outcome.raw_record = line;
    outcome.line_number = line_number;
    outcome.parse_result = parse_record(line, file_path);

    if (outcome.parse_result.succeeded())
    {
        outcome.entry = create_log_entry(outcome.parse_result, line, line_number);
    }

    return outcome;
}

/**
 * @brief Returns the generated parsing pattern.
 * @return Anchored regular expression used by QtRecordParser.
 */
auto LogParser::get_pattern() const -> QRegularExpression
{
    return m_parser.get_pattern();
}

/**
 * @brief Returns fields in configured placeholder order.
 * @return Ordered field identifiers resolved by the underlying parser.
 */
auto LogParser::get_field_order() const -> LogFieldOrder
{
    LogFieldOrder order;

    for (const QtRecordParser::FieldConfiguration& field: m_parser.get_resolved_fields())
    {
        order.fields.append(field.id);
    }

    return order;
}

/**
 * @brief Returns the active parser configuration.
 * @return Configuration currently used by the underlying parser.
 */
auto LogParser::get_configuration() const -> const QtRecordParser::ParserConfiguration&
{
    return m_parser.get_configuration();
}

/**
 * @brief Returns the active parsing profile.
 * @return Active parsing profile including its stable identity and configuration.
 */
auto LogParser::get_profile() const noexcept -> const LogParsingProfile&
{
    return m_profile;
}

/**
 * @brief Replaces the timestamp converter's accepted non-ISO formats.
 *
 * The updated configuration is written back to the owned profile so profile and parser cannot
 * diverge. Profile identity and display name remain unchanged.
 *
 * @param formats QDateTime format strings tried by the converter after ISO-8601.
 */
auto LogParser::set_timestamp_formats(const QVector<QString>& formats) -> void
{
    QtRecordParser::ParserConfiguration configuration = m_parser.get_configuration();
    bool timestamp_found = false;

    for (QtRecordParser::FieldConfiguration& field: configuration.fields)
    {
        if (!timestamp_found && field.id == LogField::Timestamp)
        {
            field.converter_options.insert(QStringLiteral("formats"),
                                           QStringList(formats.cbegin(), formats.cend()));
            timestamp_found = true;
        }
    }

    m_profile = m_profile.with_configuration(configuration);
    m_parser.set_configuration(m_profile.get_configuration());
}

/**
 * @brief Returns accepted non-ISO timestamp formats.
 *
 * An empty vector is returned when the active configuration has no timestamp field.
 * @return Configured QDateTime format strings tried after ISO-8601.
 */
auto LogParser::get_timestamp_formats() const -> QVector<QString>
{
    QVector<QString> formats;
    bool timestamp_found = false;

    for (const QtRecordParser::FieldConfiguration& field: m_parser.get_configuration().fields)
    {
        if (!timestamp_found && field.id == LogField::Timestamp)
        {
            const QStringList configured_formats =
                field.converter_options.value(QStringLiteral("formats")).toStringList();

            formats = QVector<QString>(configured_formats.cbegin(), configured_formats.cend());
            timestamp_found = true;
        }
    }

    return formats;
}

/**
 * @brief Adapts standard values from a successful generic record to LogEntry.
 *
 * Missing optional standard fields convert to their default Qt values. Custom fields remain in
 * LogParseOutcome::parse_result and are not discarded by the parser boundary.
 *
 * The successful generic result supplies the standard fields and source information used by the
 * adapted entry.
 *
 * @param result Successful generic parser result.
 * @param raw_record Unmodified source record.
 * @param line_number One-based source line number, or -1 when unknown.
 * @return Log entry containing the standard log fields and source information.
 * @pre result.succeeded() is true.
 */
auto LogParser::create_log_entry(const QtRecordParser::ParseResult& result,
                                 const QString& raw_record, qsizetype line_number) -> LogEntry
{
    const QDateTime timestamp = result.record.value(LogField::Timestamp).toDateTime();
    const QString level = result.record.value(LogField::Level).toString();
    const QString message = result.record.value(LogField::Message).toString();
    const QString app_name = result.record.value(LogField::AppName).toString();

    LogEntry entry(timestamp, level, message, LogFileInfo(result.record.source, app_name));
    entry.set_parse_metadata(raw_record, line_number, result.record.values);
    return entry;
}
