#include "Qt-LogViewer/Services/DesktopServices.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUrl>

/**
 * @brief Opens a folder in the system file manager.
 *
 * @param path The folder path to open.
 * @return True if the OS accepted the request, false otherwise.
 */
auto DesktopServices::open_folder(const QString& path) -> bool
{
    bool result = false;
    const QString trimmed_path = path.trimmed();

    if (!trimmed_path.isEmpty())
    {
        const QDir directory(trimmed_path);
        const QString absolute_path = directory.absolutePath();

        if (QDir(absolute_path).exists())
        {
            result = QDesktopServices::openUrl(QUrl::fromLocalFile(absolute_path));
        }
    }

    return result;
}

/**
 * @brief Opens a local file using the system default application.
 *
 * @param file_path The file path to open.
 * @return True if the OS accepted the request, false otherwise.
 */
auto DesktopServices::open_file(const QString& file_path) -> bool
{
    bool result = false;
    const QString trimmed_path = file_path.trimmed();

    if (!trimmed_path.isEmpty())
    {
        const QFileInfo file_info(trimmed_path);

        if (file_info.exists() && file_info.isFile())
        {
            result = QDesktopServices::openUrl(QUrl::fromLocalFile(file_info.absoluteFilePath()));
        }
    }

    return result;
}

/**
 * @brief Opens an existing path (folder or file).
 *
 * @param path The path to open.
 * @return True if the OS accepted the request, false otherwise.
 */
auto DesktopServices::open_path(const QString& path) -> bool
{
    bool result = false;
    const QString trimmed_path = path.trimmed();

    if (!trimmed_path.isEmpty())
    {
        const QFileInfo path_info(trimmed_path);

        if (path_info.exists())
        {
            if (path_info.isDir())
            {
                result = open_folder(path_info.absoluteFilePath());
            }
            else
            {
                result = open_file(path_info.absoluteFilePath());
            }
        }
    }

    return result;
}

/**
 * @brief Returns the writable app config folder path.
 *
 * @return Absolute app config folder path.
 */
auto DesktopServices::app_config_folder_path() -> QString
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return path;
}

/**
 * @brief Opens the writable app config folder in the system file manager.
 *
 * Ensures the folder exists before opening.
 *
 * @return True if the OS accepted the request, false otherwise.
 */
auto DesktopServices::open_app_config_folder() -> bool
{
    bool result = false;
    const QString folder_path = app_config_folder_path();

    if (!folder_path.isEmpty())
    {
        QDir directory;
        const bool folder_ready = directory.mkpath(folder_path);

        if (folder_ready)
        {
            result = open_folder(folder_path);
        }
    }

    return result;
}
