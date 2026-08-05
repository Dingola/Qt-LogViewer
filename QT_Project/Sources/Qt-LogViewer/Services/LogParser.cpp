/**
 * @file LogParser.cpp
 * @brief Implements the logspecific QtRecordParser adapter.
 */

#include "Qt-LogViewer/Services/LogParser.h"

#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <utility>

#include "Qt-LogViewer/Models/LogFieldDefinition.h"
#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "QtRecordParser/BuiltInConverters.h"

/**
 * @brief Constructs the standard parser for a log format.
 * @param format_string User-selected format.
 */
LogParser::LogParser(const QString& format_string)
    : m_parser(create_log_configuration(format_string),
               QtRecordParser::ConverterRegistry::create_default())
{}

/**
 * @brief Constructs a parser from a complete configuration.
 * @param configuration User-, file- or AI-generated configuration.
 * @param registry Registry containing built-in and custom converters.
 */
LogParser::LogParser(QtRecordParser::ParserConfiguration configuration,
                     QtRecordParser::ConverterRegistry registry)
    : m_parser(apply_log_defaults(std::move(configuration)), std::move(registry))
{}

/**
 * @brief Parses a complete log file.
 * @param file_path File to parse.
 * @return Successfully adapted log entries.
 */
auto LogParser::parse_file(const QString& file_path) const -> QVector<LogEntry>
{
    QVector<LogEntry> entries;
    QFile file(file_path);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        QString line;

        while (stream.readLineInto(&line))
        {
            const LogEntry entry = parse_line(line, file_path);

            if (!entry.get_level().isEmpty())
            {
                entries.append(entry);
            }
        }
    }

    return entries;
}

/**
 * @brief Parses one line and retains all dynamic fields.
 * @param line Input line.
 * @param source File or stream identifier.
 * @return Generic parser result.
 */
auto LogParser::parse_record(const QString& line,
                             const QString& source) const -> QtRecordParser::ParseResult
{
    return m_parser.parse(line, source);
}

/**
 * @brief Parses one line and adapts its log fields.
 * @param line Input line.
 * @param file_path Originating file path.
 * @return Adapted entry or a default entry after failure.
 */
auto LogParser::parse_line(const QString& line, const QString& file_path) const -> LogEntry
{
    const QtRecordParser::ParseResult result = parse_record(line, file_path);

    return create_log_entry(result);
}

/**
 * @brief Returns the generated parsing pattern.
 * @return Anchored regular expression.
 */
auto LogParser::get_pattern() const -> QRegularExpression
{
    return m_parser.get_pattern();
}

/**
 * @brief Returns fields in placeholder order.
 * @return Ordered field identifiers.
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
 * @brief Returns the complete parser configuration.
 * @return Current serializable configuration.
 */
auto LogParser::get_configuration() const -> const QtRecordParser::ParserConfiguration&
{
    return m_parser.get_configuration();
}

/**
 * @brief Sets accepted timestamp formats.
 * @param formats Formats tried after ISO-8601.
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

    m_parser.set_configuration(configuration);
}

/**
 * @brief Returns accepted timestamp formats.
 * @return Formats tried after ISO-8601.
 */
auto LogParser::get_timestamp_formats() const -> QVector<QString>
{
    QVector<QString> formats;

    for (const QtRecordParser::FieldConfiguration& field: m_parser.get_configuration().fields)
    {
        if (formats.isEmpty() && field.id == LogField::Timestamp)
        {
            const QStringList configured_formats =
                field.converter_options.value(QStringLiteral("formats")).toStringList();

            formats = QVector<QString>(configured_formats.cbegin(), configured_formats.cend());
        }
    }

    return formats;
}

/**
 * @brief Creates the standard configuration for a log format.
 * @param format_string User-selected format.
 * @return Log-aware parser configuration.
 */
auto LogParser::create_log_configuration(const QString& format_string)
    -> QtRecordParser::ParserConfiguration
{
    QtRecordParser::ParserConfiguration configuration;

    configuration.format = format_string;
    configuration.fields = get_default_log_fields();
    configuration.allow_unknown_fields = true;

    return configuration;
}

/**
 * @brief Adds missing standard log field definitions.
 * @param configuration Configuration to complete.
 * @return Completed configuration.
 */
auto LogParser::apply_log_defaults(QtRecordParser::ParserConfiguration configuration)
    -> QtRecordParser::ParserConfiguration
{
    const QVector<QtRecordParser::FieldConfiguration> defaults = get_default_log_fields();

    for (const QtRecordParser::FieldConfiguration& default_field: defaults)
    {
        bool field_exists = false;

        for (const QtRecordParser::FieldConfiguration& configured_field: configuration.fields)
        {
            if (!field_exists && configured_field.id == default_field.id)
            {
                field_exists = true;
            }
        }

        if (!field_exists)
        {
            configuration.fields.append(default_field);
        }
    }

    return configuration;
}

/**
 * @brief Returns standard log field configurations.
 * @return Known log fields and converters.
 */
auto LogParser::get_default_log_fields() -> QVector<QtRecordParser::FieldConfiguration>
{
    const QString timestamp_pattern = QStringLiteral(
        R"(\d{4}-\d{2}-\d{2}[ T]\d{2}:\d{2}:\d{2}(?:\.\d{1,3})?(?:Z|[+\-]\d{2}:\d{2})?|\d{2}\.\d{2}\.\d{4}\s+\d{2}:\d{2}:\d{2}(?:\.\d{1,3})?|\d{2}/\d{2}/\d{4}\s+\d{2}:\d{2}:\d{2}(?:\.\d{1,3})?|\d{4}/\d{2}/\d{2}\s+\d{2}:\d{2}:\d{2}(?:\.\d{1,3})?)");

    const QStringList timestamp_formats{
        QStringLiteral("yyyy-MM-dd HH:mm:ss"), QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"),
        QStringLiteral("yyyy-MM-ddTHH:mm:ss"), QStringLiteral("yyyy-MM-ddTHH:mm:ss.zzz"),
        QStringLiteral("dd.MM.yyyy HH:mm:ss"), QStringLiteral("dd.MM.yyyy HH:mm:ss.zzz"),
        QStringLiteral("MM/dd/yyyy HH:mm:ss"), QStringLiteral("MM/dd/yyyy HH:mm:ss.zzz"),
        QStringLiteral("yyyy/MM/dd HH:mm:ss"), QStringLiteral("yyyy/MM/dd HH:mm:ss.zzz")};

    QVariantMap timestamp_options;

    timestamp_options.insert(QStringLiteral("accept_iso"), true);

    timestamp_options.insert(QStringLiteral("formats"), timestamp_formats);

    return {{LogField::Timestamp, QStringLiteral("Timestamp"), timestamp_pattern,
             QtRecordParser::ConverterId::DateTime, timestamp_options, true},

            {LogField::Level, QStringLiteral("Level"), QStringLiteral(R"(\w+)"),
             QtRecordParser::ConverterId::Text, QVariantMap(), true},

            {LogField::Message, QStringLiteral("Message"), QStringLiteral(".*?"),
             QtRecordParser::ConverterId::Text, QVariantMap(), true},

            {LogField::AppName, QStringLiteral("Application"), QStringLiteral(R"(\S+)"),
             QtRecordParser::ConverterId::Text, QVariantMap(), true},

            {QStringLiteral("file"), QStringLiteral("File"), QStringLiteral(".*?"),
             QtRecordParser::ConverterId::Text, QVariantMap(), true},

            {QStringLiteral("line"), QStringLiteral("Line"), QStringLiteral(R"([+-]?\d+)"),
             QtRecordParser::ConverterId::Integer, QVariantMap(), true},

            {QStringLiteral("function"), QStringLiteral("Function"), QStringLiteral(".*?"),
             QtRecordParser::ConverterId::Text, QVariantMap(), true}};
}

/**
 * @brief Adapts a successful generic result to LogEntry.
 * @param result Generic parser result.
 * @return Adapted entry or a default entry after failure.
 */
auto LogParser::create_log_entry(const QtRecordParser::ParseResult& result) -> LogEntry
{
    LogEntry entry;

    if (result.succeeded())
    {
        const QDateTime timestamp = result.record.value(LogField::Timestamp).toDateTime();

        const QString level = result.record.value(LogField::Level).toString();

        const QString message = result.record.value(LogField::Message).toString();

        const QString app_name = result.record.value(LogField::AppName).toString();

        entry = LogEntry(timestamp, level, message, LogFileInfo(result.record.source, app_name));
    }

    return entry;
}
