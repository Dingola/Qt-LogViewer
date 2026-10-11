#pragma once

#include <QString>

/**
 * @file ApplicationStoragePaths.h
 * @brief Declares the canonical storage layout used by Qt-LogViewer.
 */

/**
 * @class ApplicationStoragePaths
 * @brief Resolves settings, session, cache, and diagnostic paths below one application root.
 *
 * The default root is QStandardPaths::AppConfigLocation. An explicit root can be supplied for
 * tests and other isolated application compositions. The class only resolves paths and does not
 * create directories.
 */
class ApplicationStoragePaths final
{
    public:
        /** @brief Constructs paths below the standard application configuration location. */
        ApplicationStoragePaths();

        /**
         * @brief Constructs paths below an explicit application root.
         * @param application_root Root directory used for every managed application path.
         */
        explicit ApplicationStoragePaths(QString application_root);

        /**
         * @brief Returns the root containing all managed application data.
         * @return Absolute application storage root.
         */
        [[nodiscard]] auto get_application_root() const -> QString;

        /**
         * @brief Returns the application settings file path.
         * @return Absolute path to `settings.ini`.
         */
        [[nodiscard]] auto get_settings_file() const -> QString;

        /**
         * @brief Returns the directory containing session metadata and documents.
         * @return Absolute `sessions` directory path.
         */
        [[nodiscard]] auto get_sessions_directory() const -> QString;

        /**
         * @brief Returns the session index file path.
         * @return Absolute path to `sessions/index.json`.
         */
        [[nodiscard]] auto get_session_index_file() const -> QString;

        /**
         * @brief Returns the root for reconstructible application data.
         * @return Absolute `cache` directory path.
         */
        [[nodiscard]] auto get_cache_directory() const -> QString;

        /**
         * @brief Returns the transient history database path.
         * @return Absolute path to `cache/history.sqlite`.
         */
        [[nodiscard]] auto get_history_database_file() const -> QString;

        /**
         * @brief Returns the persistent log-index directory.
         * @return Absolute `cache/index-v2` directory path.
         */
        [[nodiscard]] auto get_log_index_directory() const -> QString;

        /**
         * @brief Returns the directory containing diagnostic output.
         * @return Absolute `diagnostics` directory path.
         */
        [[nodiscard]] auto get_diagnostics_directory() const -> QString;

        /**
         * @brief Returns the application diagnostic log path.
         * @return Absolute path to `diagnostics/Qt-LogViewer.log`.
         */
        [[nodiscard]] auto get_diagnostics_log_file() const -> QString;

    private:
        QString m_application_root;
};
