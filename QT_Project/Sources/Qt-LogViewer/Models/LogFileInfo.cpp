#include "Qt-LogViewer/Models/LogFileInfo.h"

#include <QFileInfo>

/**
 * @brief Constructs a LogFileInfo object.
 * @param file_path The full path to the log file.
 * @param app_name The name of the application (optional).
 */
LogFileInfo::LogFileInfo(const QString& file_path, const QString& app_name)
    : m_file_path(file_path), m_app_name(app_name)
{}

/**
 * @brief Returns the file path.
 * @return The full file path.
 */
auto LogFileInfo::get_file_path() const -> QString
{
    return m_file_path;
}

/**
 * @brief Returns the file name (without path).
 * @return The file name.
 */
auto LogFileInfo::get_file_name() const -> QString
{
    return QFileInfo(m_file_path).fileName();
}

/**
 * @brief Returns the application name.
 * @return The application name, or empty if not set.
 */
auto LogFileInfo::get_app_name() const -> QString
{
    return m_app_name;
}

/**
 * @brief Sets the application name.
 * @param app_name The new application name.
 */
auto LogFileInfo::set_app_name(const QString& app_name) -> void
{
    m_app_name = app_name;
}
