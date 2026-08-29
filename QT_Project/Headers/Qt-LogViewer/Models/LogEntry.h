#pragma once

#include <QDateTime>
#include <QMap>
#include <QString>
#include <QVariant>

#include "Qt-LogViewer/Models/LogFileInfo.h"

/**
 * @file LogEntry.h
 * @brief This file contains the definition of the LogEntry class.
 */

/**
 * @class LogEntry
 * @brief Represents a parsed log entry and its lossless source data.
 *
 * Besides the standard fields used by the table model, the entry retains the raw
 * record, its source line and all dynamically parsed values. These values may also
 * be restored from persistent storage without requiring a parser result.
 */
class LogEntry
{
    public:
        /** Dynamically named, converted values retained from the parser result. */
        using ParsedFields = QMap<QString, QVariant>;

        /**
         * @brief Constructs a LogEntry object.
         * @param timestamp The timestamp of the log entry.
         * @param level The log level (e.g., "INFO", "ERROR").
         * @param message The log message.
         * @param file_info Source file and application information.
         */
        LogEntry(QDateTime timestamp = QDateTime(), QString level = QString(),
                 QString message = QString(), LogFileInfo file_info = LogFileInfo());

        /**
         * @brief Destroys the LogEntry object.
         */
        ~LogEntry() = default;

        /**
         * @brief Returns the timestamp of the log entry.
         * @return The timestamp.
         */
        [[nodiscard]] auto get_timestamp() const -> QDateTime;

        /**
         * @brief Returns the log level.
         * @return The log level.
         */
        [[nodiscard]] auto get_level() const -> QString;

        /**
         * @brief Returns the log message.
         * @return The log message.
         */
        [[nodiscard]] auto get_message() const -> QString;

        /**
         * @brief Returns the application name.
         * @return The application name.
         */
        [[nodiscard]] auto get_app_name() const -> QString;

        /**
         * @brief Returns the LogFileInfo associated with this log entry.
         * @return The LogFileInfo object.
         */
        [[nodiscard]] auto get_file_info() const -> LogFileInfo;

        /**
         * @brief Returns the unmodified source record.
         * @return Raw record supplied to the parser.
         */
        [[nodiscard]] auto get_raw_record() const noexcept -> const QString&;

        /**
         * @brief Returns the one-based source line.
         * @return Source line, or -1 when unknown.
         */
        [[nodiscard]] auto get_source_line() const noexcept -> qsizetype;

        /**
         * @brief Returns all dynamically named, converted parser values.
         * @return Parsed values keyed by stable field identifier.
         */
        [[nodiscard]] auto get_parsed_fields() const noexcept -> const ParsedFields&;

        /**
         * @brief Returns one dynamically parsed value.
         * @param field_id Stable field identifier.
         * @return Converted value, or an invalid QVariant when the field is absent.
         */
        [[nodiscard]] auto get_parsed_field(const QString& field_id) const -> QVariant;

        /**
         * @brief Sets the timestamp.
         * @param timestamp The new timestamp.
         */
        auto set_timestamp(const QDateTime& timestamp) -> void;

        /**
         * @brief Sets the log level.
         * @param level The new log level.
         */
        auto set_level(const QString& level) -> void;

        /**
         * @brief Sets the log message.
         * @param message The new log message.
         */
        auto set_message(const QString& message) -> void;

        /**
         * @brief Sets the application name.
         * @param app_name The new application name.
         */
        auto set_app_name(const QString& app_name) -> void;

        /**
         * @brief Sets the LogFileInfo.
         * @param file_info The new LogFileInfo object.
         */
        auto set_file_info(const LogFileInfo& file_info) -> void;

        /**
         * @brief Sets the lossless metadata retained from a successful parser result.
         * @param raw_record Unmodified source record.
         * @param source_line One-based source line, or -1 when unknown.
         * @param parsed_fields Converted values keyed by stable field identifier.
         */
        auto set_parse_metadata(QString raw_record, qsizetype source_line,
                                ParsedFields parsed_fields) -> void;

    private:
        QDateTime m_timestamp;
        QString m_level;
        QString m_message;
        LogFileInfo m_file_info;
        QString m_raw_record;
        qsizetype m_source_line{-1};
        ParsedFields m_parsed_fields;
};
