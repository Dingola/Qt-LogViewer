/**
 * @file LogEntry.cpp
 * @brief This file contains the implementation of the LogEntry class.
 */

#include "Qt-LogViewer/Models/LogEntry.h"

#include <utility>

/**
 * @brief Constructs a LogEntry object.
 * @param timestamp The timestamp of the log entry.
 * @param level The log level.
 * @param message The log message.
 * @param file_info Source file and application information.
 */
LogEntry::LogEntry(QDateTime timestamp, QString level, QString message, LogFileInfo file_info)
    : m_timestamp{std::move(timestamp)},
      m_level{std::move(level)},
      m_message{std::move(message)},
      m_file_info{std::move(file_info)}
{}

/**
 * @brief Returns the timestamp of the log entry.
 * @return The timestamp.
 */
auto LogEntry::get_timestamp() const -> QDateTime
{
    return m_timestamp;
}

/**
 * @brief Returns the log level.
 * @return The log level.
 */
auto LogEntry::get_level() const -> QString
{
    return m_level;
}

/**
 * @brief Returns the log message.
 * @return The log message.
 */
auto LogEntry::get_message() const -> QString
{
    return m_message;
}

/**
 * @brief Returns the application name.
 * @return The application name.
 */
auto LogEntry::get_app_name() const -> QString
{
    return m_file_info.get_app_name();
}

/**
 * @brief Returns the LogFileInfo associated with this log entry.
 * @return The LogFileInfo object.
 */
auto LogEntry::get_file_info() const -> LogFileInfo
{
    return m_file_info;
}

/**
 * @brief Returns the unmodified source record.
 * @return Raw record supplied to the parser.
 */
auto LogEntry::get_raw_record() const noexcept -> const QString&
{
    return m_raw_record;
}

/**
 * @brief Returns the one-based source line.
 * @return Source line, or -1 when unknown.
 */
auto LogEntry::get_source_line() const noexcept -> qsizetype
{
    return m_source_line;
}

/**
 * @brief Returns all dynamically named, converted parser values.
 * @return Parsed values keyed by stable field identifier.
 */
auto LogEntry::get_parsed_fields() const noexcept -> const ParsedFields&
{
    return m_parsed_fields;
}

/**
 * @brief Returns one dynamically parsed value.
 * @param field_id Stable field identifier.
 * @return Converted value, or an invalid QVariant when the field is absent.
 */
auto LogEntry::get_parsed_field(const QString& field_id) const -> QVariant
{
    return m_parsed_fields.value(field_id);
}

/**
 * @brief Sets the timestamp.
 * @param timestamp The new timestamp.
 */
auto LogEntry::set_timestamp(const QDateTime& timestamp) -> void
{
    m_timestamp = timestamp;
}

/**
 * @brief Sets the log level.
 * @param level The new log level.
 */
auto LogEntry::set_level(const QString& level) -> void
{
    m_level = level;
}

/**
 * @brief Sets the log message.
 * @param message The new log message.
 */
auto LogEntry::set_message(const QString& message) -> void
{
    m_message = message;
}

/**
 * @brief Sets the application name.
 * @param app_name The new application name.
 */
auto LogEntry::set_app_name(const QString& app_name) -> void
{
    m_file_info.set_app_name(app_name);
}

/**
 * @brief Sets the LogFileInfo.
 * @param file_info The new LogFileInfo object.
 */
auto LogEntry::set_file_info(const LogFileInfo& file_info) -> void
{
    m_file_info = file_info;
}

/**
 * @brief Sets the lossless metadata retained from a successful parser result.
 * @param raw_record Unmodified source record.
 * @param source_line One-based source line, or -1 when unknown.
 * @param parsed_fields Converted values keyed by stable field identifier.
 */
auto LogEntry::set_parse_metadata(QString raw_record, qsizetype source_line,
                                  ParsedFields parsed_fields) -> void
{
    m_raw_record = std::move(raw_record);
    m_source_line = source_line;
    m_parsed_fields = std::move(parsed_fields);
}
