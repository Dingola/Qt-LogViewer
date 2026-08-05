#pragma once

#include <QRegularExpression>
#include <QString>
#include <QVector>

#include "Qt-LogViewer/Models/LogEntry.h"
#include "QtRecordParser/ConverterRegistry.h"
#include "QtRecordParser/FormatRecordParser.h"

/**
 * @file LogParser.h
 * @brief Declares the logspecific QtRecordParser adapter.
 */

/**
 * @struct LogFieldOrder
 * @brief Stores fields in configured placeholder order.
 */
struct LogFieldOrder {
        QVector<QString> fields;
};

/**
 * @class LogParser
 * @brief Adapts generic parsed records to LogEntry.
 *
 * The class defines the known log fields while format parsing, custom fields,
 * converter lookup and conversion errors are handled by QtRecordParser.
 */
class LogParser
{
    public:
        /**
         * @brief Constructs the standard parser for a log format.
         * @param format_string User-selected format.
         */
        explicit LogParser(const QString& format_string);

        /**
         * @brief Constructs a parser from a complete configuration.
         * @param configuration User-, file- or AI-generated configuration.
         * @param registry Registry containing built-in and custom converters.
         */
        explicit LogParser(QtRecordParser::ParserConfiguration configuration,
                           QtRecordParser::ConverterRegistry registry =
                               QtRecordParser::ConverterRegistry::create_default());

        /**
         * @brief Destroys the parser.
         */
        ~LogParser() = default;

        /**
         * @brief Parses a complete log file.
         * @param file_path File to parse.
         * @return Successfully adapted log entries.
         */
        [[nodiscard]] auto parse_file(const QString& file_path) const -> QVector<LogEntry>;

        /**
         * @brief Parses one line and retains all dynamic fields.
         * @param line Input line.
         * @param source File or stream identifier.
         * @return Generic parser result.
         */
        [[nodiscard]] auto parse_record(const QString& line,
                                        const QString& source) const -> QtRecordParser::ParseResult;

        /**
         * @brief Parses one line and adapts its log fields.
         * @param line Input line.
         * @param file_path Originating file path.
         * @return Adapted entry or a default entry after failure.
         */
        [[nodiscard]] auto parse_line(const QString& line,
                                      const QString& file_path) const -> LogEntry;

        /**
         * @brief Returns the generated parsing pattern.
         * @return Anchored regular expression.
         */
        [[nodiscard]] auto get_pattern() const -> QRegularExpression;

        /**
         * @brief Returns fields in placeholder order.
         * @return Ordered field identifiers.
         */
        [[nodiscard]] auto get_field_order() const -> LogFieldOrder;

        /**
         * @brief Returns the complete parser configuration.
         * @return Current serializable configuration.
         */
        [[nodiscard]] auto get_configuration() const -> const QtRecordParser::ParserConfiguration&;

        /**
         * @brief Sets accepted timestamp formats.
         * @param formats Formats tried after ISO-8601.
         */
        auto set_timestamp_formats(const QVector<QString>& formats) -> void;

        /**
         * @brief Returns accepted timestamp formats.
         * @return Formats tried after ISO-8601.
         */
        [[nodiscard]] auto get_timestamp_formats() const -> QVector<QString>;

    private:
        /**
         * @brief Creates the standard configuration for a log format.
         * @param format_string User-selected format.
         * @return Log-aware parser configuration.
         */
        [[nodiscard]] static auto create_log_configuration(const QString& format_string)
            -> QtRecordParser::ParserConfiguration;

        /**
         * @brief Adds missing standard log field definitions.
         * @param configuration Configuration to complete.
         * @return Completed configuration.
         */
        [[nodiscard]] static auto apply_log_defaults(
            QtRecordParser::ParserConfiguration configuration)
            -> QtRecordParser::ParserConfiguration;

        /**
         * @brief Returns standard log field configurations.
         * @return Known log fields and converters.
         */
        [[nodiscard]] static auto get_default_log_fields()
            -> QVector<QtRecordParser::FieldConfiguration>;

        /**
         * @brief Adapts a successful generic result to LogEntry.
         * @param result Generic parser result.
         * @return Adapted entry or a default entry after failure.
         */
        [[nodiscard]] static auto create_log_entry(const QtRecordParser::ParseResult& result)
            -> LogEntry;

    private:
        QtRecordParser::FormatRecordParser m_parser;
};
