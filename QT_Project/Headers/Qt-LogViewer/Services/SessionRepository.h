#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>

/**
 * @file SessionRepository.h
 * @brief Declares the SessionRepository service responsible for persisting and loading
 *        recent files and sessions as JSON using atomic writes.
 *
 * Storage location:
 * - Uses QStandardPaths::AppConfigLocation/<app_config_subdir> as base directory.
 * - Root JSON: "<base>/sessions.json"
 * - Per-session JSONs: "<base>/sessions/<session_id>.json"
 *
 * Schema (v1):
 * {
 *   "schema_version": 1,
 *   "recent_files": [ ... ],
 *   "recent_sessions": [ ... ],
 *   "last_session_id": "..."
 * }
 */
class SessionRepository: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a repository rooted at the application configuration location.
         * @param app_config_subdir Optional subdirectory name under AppConfigLocation,
         *        defaults to "Qt-LogViewer".
         * @param parent Optional QObject parent.
         *
         * The repository will create the directory structure on demand during save operations.
         */
        explicit SessionRepository(QString app_config_subdir = QStringLiteral("Qt-LogViewer"),
                                   QObject* parent = nullptr);

        /**
         * @brief Returns the absolute path of the root JSON file.
         * @return Absolute file path to the root document.
         */
        [[nodiscard]] auto get_root_file_path() const -> QString;

        /**
         * @brief Loads the entire root JSON document.
         * @return QJsonObject containing the parsed JSON. If missing or invalid, returns
         *         an object with "schema_version" set and empty arrays for known fields.
         *
         * Defaults ensured:
         * - "schema_version" (int)
         * - "recent_files" (array)
         * - "recent_sessions" (array)
         */
        [[nodiscard]] auto load_all() const -> QJsonObject;

        /**
         * @brief Atomically saves the entire root JSON document.
         * @param root The JSON object to serialize and write.
         *
         * Ensures the target directory exists and uses QSaveFile to avoid partial writes.
         * Automatically injects "schema_version" if not present.
         */
        auto save_all(const QJsonObject& root) const -> void;

        /**
         * @brief Returns the absolute directory path used for per-session files.
         * @return Absolute directory path: "<base>/sessions".
         */
        [[nodiscard]] auto get_sessions_dir_path() const -> QString;

        /**
         * @brief Computes the absolute file path for a specific session id.
         * @param session_id The session identifier.
         * @return Absolute file path within the sessions directory: "<base>/sessions/<id>.json".
         */
        [[nodiscard]] auto get_session_file_path(const QString& session_id) const -> QString;

        /**
         * @brief Loads a single session JSON document by id.
         * @param session_id The session identifier.
         * @return Parsed session JSON object, or an empty object if the file does not exist or is
         * invalid.
         */
        [[nodiscard]] auto load_session(const QString& session_id) const -> QJsonObject;

        /**
         * @brief Atomically saves a single session JSON document under its id.
         * @param session_id The session identifier.
         * @param obj The session JSON object to write.
         *
         * Ensures "schema_version" is written into the file if not provided.
         */
        auto save_session(const QString& session_id, const QJsonObject& obj) const -> void;

        /**
         * @brief Deletes a stored session JSON file if present.
         * @param session_id The session identifier.
         * @return True if a file existed and was deleted; false otherwise.
         */
        [[nodiscard]] auto delete_session(const QString& session_id) const -> bool;

        /**
         * @brief Returns the schema version handled by this repository.
         * @return Integer schema version.
         */
        [[nodiscard]] static auto get_schema_version() -> int;

    private:
        /**
         * @brief Returns the base writable app configuration directory.
         * @return Absolute base directory path, potentially including the configured subdir.
         */
        [[nodiscard]] auto get_app_config_root() const -> QString;

    private:
        QString m_app_config_subdir;
};
