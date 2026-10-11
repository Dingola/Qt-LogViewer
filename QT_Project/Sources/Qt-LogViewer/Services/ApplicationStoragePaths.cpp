/**
 * @file ApplicationStoragePaths.cpp
 * @brief Implements canonical Qt-LogViewer storage path resolution.
 */

#include "Qt-LogViewer/Services/ApplicationStoragePaths.h"

#include <QDir>
#include <QStandardPaths>
#include <utility>

/** @brief Constructs paths below QStandardPaths::AppConfigLocation. */
ApplicationStoragePaths::ApplicationStoragePaths()
    : ApplicationStoragePaths(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation))
{}

/**
 * @brief Constructs paths below an explicit application root.
 * @param application_root Root directory used for every managed application path.
 */
ApplicationStoragePaths::ApplicationStoragePaths(QString application_root)
    : m_application_root(QDir(std::move(application_root)).absolutePath())
{}

/**
 * @brief Returns the root containing all managed application data.
 * @return Absolute application storage root.
 */
auto ApplicationStoragePaths::get_application_root() const -> QString
{
    const QString path = m_application_root;
    return path;
}

/**
 * @brief Returns the application settings file path.
 * @return Absolute path to `settings.ini`.
 */
auto ApplicationStoragePaths::get_settings_file() const -> QString
{
    const QString path = QDir(m_application_root).filePath(QStringLiteral("settings.ini"));
    return path;
}

/**
 * @brief Returns the directory containing session metadata and documents.
 * @return Absolute `sessions` directory path.
 */
auto ApplicationStoragePaths::get_sessions_directory() const -> QString
{
    const QString path = QDir(m_application_root).filePath(QStringLiteral("sessions"));
    return path;
}

/**
 * @brief Returns the session index file path.
 * @return Absolute path to `sessions/index.json`.
 */
auto ApplicationStoragePaths::get_session_index_file() const -> QString
{
    const QString path = QDir(get_sessions_directory()).filePath(QStringLiteral("index.json"));
    return path;
}

/**
 * @brief Returns the root for reconstructible application data.
 * @return Absolute `cache` directory path.
 */
auto ApplicationStoragePaths::get_cache_directory() const -> QString
{
    const QString path = QDir(m_application_root).filePath(QStringLiteral("cache"));
    return path;
}

/**
 * @brief Returns the transient history database path.
 * @return Absolute path to `cache/history.sqlite`.
 */
auto ApplicationStoragePaths::get_history_database_file() const -> QString
{
    const QString path = QDir(get_cache_directory()).filePath(QStringLiteral("history.sqlite"));
    return path;
}

/**
 * @brief Returns the persistent log-index directory.
 * @return Absolute `cache/index-v2` directory path.
 */
auto ApplicationStoragePaths::get_log_index_directory() const -> QString
{
    const QString path = QDir(get_cache_directory()).filePath(QStringLiteral("index-v2"));
    return path;
}

/**
 * @brief Returns the directory containing diagnostic output.
 * @return Absolute `diagnostics` directory path.
 */
auto ApplicationStoragePaths::get_diagnostics_directory() const -> QString
{
    const QString path = QDir(m_application_root).filePath(QStringLiteral("diagnostics"));
    return path;
}

/**
 * @brief Returns the application diagnostic log path.
 * @return Absolute path to `diagnostics/Qt-LogViewer.log`.
 */
auto ApplicationStoragePaths::get_diagnostics_log_file() const -> QString
{
    const QString path =
        QDir(get_diagnostics_directory()).filePath(QStringLiteral("Qt-LogViewer.log"));
    return path;
}
