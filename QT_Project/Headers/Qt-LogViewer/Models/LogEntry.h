#pragma once

#include <QDateTime>
#include <QString>

/**
 * @file LogEntry.h
 * @brief This file contains the definition of the LogEntry class.
 */

/**
 * @class LogEntry
 * @brief Represents a single log entry with timestamp, level, message and app name.
 *
 * This class encapsulates the data for a single log line, including timestamp,
 * log level, message and the application name.
 */
class LogEntry
{
    public:
        /**
         * @brief Constructs a LogEntry object.
         * @param timestamp The timestamp of the log entry.
         * @param level The log level (e.g., "INFO", "ERROR").
         * @param message The log message.
         * @param app_name The name of the application that generated the log.
         */
        LogEntry(const QDateTime& timestamp = QDateTime(), const QString& level = QString(),
                 const QString& message = QString(), const QString& app_name = QString());

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

    private:
        QDateTime m_timestamp;
        QString m_level;
        QString m_message;
        QString m_app_name;
};
