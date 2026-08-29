#pragma once

#include <QRegularExpression>
#include <QString>
#include <QVector>

#include "Qt-LogViewer/Services/LogParseOutcome.h"
#include "Qt-LogViewer/Services/LogParsingProfile.h"
#include "QtRecordParser/ConverterRegistry.h"
#include "QtRecordParser/FormatRecordParser.h"

/**
 * @file LogParser.h
 * @brief Declares the log-specific QtRecordParser adapter.
 */

/**
 * @struct LogFieldOrder
 * @brief Stores field identifiers in the order of their configured placeholders.
 */
struct LogFieldOrder {
        /** Field identifiers resolved by QtRecordParser. */
        QVector<QString> fields;
};

/**
 * @class LogParser
 * @brief Parses records according to a profile and adapts successful results to LogEntry.
 *
 * Format matching, dynamic fields, converter lookup and structured conversion errors are
 * delegated to QtRecordParser. This adapter adds log-specific profile handling and converts
 * successful generic records to the current LogEntry model without using sentinel values to
 * represent parser failures.
 */
class LogParser
{
    public:
        /**
         * @brief Constructs a parser for a parsing profile.
         * @param profile Parsing profile used by this adapter.
         * @param registry Registry containing built-in and custom converters.
         */
        explicit LogParser(LogParsingProfile profile,
                           QtRecordParser::ConverterRegistry registry =
                               QtRecordParser::ConverterRegistry::create_default());

        /**
         * @brief Destroys the parser adapter.
         */
        ~LogParser() = default;

        /**
         * @brief Parses a complete log file and returns successfully adapted entries.
         *
         * Every input line is parsed independently. Failed records are omitted from the returned
         * vector; callers that require individual failure details should use parse_line().
         *
         * @param file_path Path of the file to read and use as record source.
         * @return Successfully parsed and adapted log entries in source order.
         */
        [[nodiscard]] auto parse_file(const QString& file_path) const -> QVector<LogEntry>;

        /**
         * @brief Parses one record while retaining all dynamically configured fields.
         * @param line Raw input record.
         * @param source File, stream or dataset identifier stored in the parsed record.
         * @return Generic QtRecordParser result containing dynamic values or a structured error.
         */
        [[nodiscard]] auto parse_record(const QString& line,
                                        const QString& source) const -> QtRecordParser::ParseResult;

        /**
         * @brief Parses one line and explicitly reports success or failure.
         * @param line Raw input line.
         * @param file_path Originating file path.
         * @param line_number One-based source line number, or -1 when unknown.
         * @return Outcome containing source context, generic parse result and an optional entry.
         */
        [[nodiscard]] auto parse_line(const QString& line, const QString& file_path,
                                      qsizetype line_number = -1) const -> LogParseOutcome;

        /**
         * @brief Returns the generated parsing pattern.
         * @return Anchored regular expression used by QtRecordParser.
         */
        [[nodiscard]] auto get_pattern() const -> QRegularExpression;

        /**
         * @brief Returns fields in configured placeholder order.
         * @return Ordered field identifiers resolved by the underlying parser.
         */
        [[nodiscard]] auto get_field_order() const -> LogFieldOrder;

        /**
         * @brief Returns the active serializable parser configuration.
         * @return Configuration currently used by the underlying parser.
         */
        [[nodiscard]] auto get_configuration() const -> const QtRecordParser::ParserConfiguration&;

        /**
         * @brief Returns the parsing profile owned by this adapter.
         * @return Active parsing profile including its stable identity and configuration.
         */
        [[nodiscard]] auto get_profile() const noexcept -> const LogParsingProfile&;

        /**
         * @brief Replaces accepted timestamp formats while preserving the profile identity.
         * @param formats QDateTime format strings tried by the converter after ISO-8601.
         */
        auto set_timestamp_formats(const QVector<QString>& formats) -> void;

        /**
         * @brief Returns accepted timestamp formats.
         * @return Configured QDateTime format strings tried after ISO-8601.
         */
        [[nodiscard]] auto get_timestamp_formats() const -> QVector<QString>;

    private:
        /**
         * @brief Adapts a successful generic record to the current LogEntry model.
         * @param result Successful generic parser result.
         * @return Log entry containing the standard log fields and source information.
         * @pre result.succeeded() is true.
         */
        [[nodiscard]] static auto create_log_entry(const QtRecordParser::ParseResult& result)
            -> LogEntry;

    private:
        LogParsingProfile m_profile;
        QtRecordParser::FormatRecordParser m_parser;
};
