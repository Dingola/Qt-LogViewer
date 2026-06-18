#pragma once

#include <QString>

/**
 * @class DesktopServices
 * @brief Utility class for opening local files, folders, and common application locations.
 *
 * Provides convenience methods around QDesktopServices for opening folders/files
 * in the operating system. Includes helpers that are directly useful for settings UI,
 * such as opening the application config folder.
 */
class DesktopServices
{
    public:
        /**
         * @brief Opens a folder in the system file manager.
         *
         * @param path The folder path to open.
         * @return True if the OS accepted the request, false otherwise.
         */
        [[nodiscard]] static auto open_folder(const QString& path) -> bool;

        /**
         * @brief Opens a local file using the system default application.
         *
         * @param file_path The file path to open.
         * @return True if the OS accepted the request, false otherwise.
         */
        [[nodiscard]] static auto open_file(const QString& file_path) -> bool;

        /**
         * @brief Opens an existing path (folder or file).
         *
         * If the path is a directory, it opens the folder. If it is a file,
         * it opens the file with the associated default app.
         *
         * @param path The path to open.
         * @return True if the OS accepted the request, false otherwise.
         */
        [[nodiscard]] static auto open_path(const QString& path) -> bool;

        /**
         * @brief Returns the writable app config folder path.
         *
         * @return Absolute app config folder path.
         */
        [[nodiscard]] static auto app_config_folder_path() -> QString;

        /**
         * @brief Opens the writable app config folder in the system file manager.
         *
         * Ensures the folder exists before opening.
         *
         * @return True if the OS accepted the request, false otherwise.
         */
        [[nodiscard]] static auto open_app_config_folder() -> bool;
};