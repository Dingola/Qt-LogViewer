#pragma once

#include <QString>

/**
 * @class LogFileInfo
 * @brief Encapsulates metadata for a log file, including file path and application name.
 *
 * This class provides accessors for the log file's path, name, and associated application name.
 */
class LogFileInfo
{
    public:
        /**
         * @brief Constructs a LogFileInfo object.
         * @param file_path The full path to the log file.
         * @param app_name The name of the application (optional).
         */
        explicit LogFileInfo(const QString& file_path = QString(),
                             const QString& app_name = QString());

        /**
         * @brief Returns the file path.
         * @return The full file path.
         */
        [[nodiscard]] auto get_file_path() const -> QString;

        /**
         * @brief Returns the file name (without path).
         * @return The file name.
         */
        [[nodiscard]] auto get_file_name() const -> QString;

        /**
         * @brief Returns the application name.
         * @return The application name, or empty if not set.
         */
        [[nodiscard]] auto get_app_name() const -> QString;

        /**
         * @brief Sets the application name.
         * @param app_name The new application name.
         */
        auto set_app_name(const QString& app_name) -> void;

    private:
        QString m_file_path;
        QString m_app_name;
};
