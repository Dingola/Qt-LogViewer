#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QObject>
#include <QVector>

#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Models/SessionTypes.h"
#include "Qt-LogViewer/Services/SessionRepository.h"

/**
 * @file SessionManager.h
 * @brief Declares the SessionManager service that manages recent files and session metadata
 *        in-memory and delegates JSON persistence to SessionRepository.
 *
 * Responsibilities:
 * - Maintain an in-memory list of recent files and recent sessions.
 * - Provide high-level APIs to add/clear recent files and to list/load/save/delete sessions.
 * - Emit signals so UI models (menus/start page) can update.
 *
 * Data records:
 * - RecentLogFileRecord: { file_path, app_name, last_opened }
 * - RecentSessionRecord: { id, name, created_at, last_opened }
 *
 * Persistence is handled via SessionRepository under AppConfigLocation.
 */
class SessionManager: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a SessionManager with a given repository and optional parent.
         * @param repository Persistence repository (owned by this manager).
         * @param parent Optional QObject parent.
         */
        explicit SessionManager(SessionRepository* repository, QObject* parent = nullptr);

        /**
         * @brief Loads state from repository into memory (recent files and sessions).
         *
         * Safe to call multiple times; replaces the in-memory lists.
         */
        auto initialize_from_storage() -> void;

        /**
         * @brief Returns the list of recent log files.
         * @return Vector of RecentLogFileRecord.
         */
        [[nodiscard]] auto get_recent_log_files() const -> QVector<RecentLogFileRecord>;

        /**
         * @brief Adds/updates a recent log file record and persists the root document.
         * @param file_info LogFileInfo from which file_path and app_name are derived.
         *
         * Behavior:
         * - Deduplicate by file_path (case-sensitive as stored).
         * - Update last_opened to current time.
         * - Move updated item to front (most recent first).
         */
        auto add_recent_log_file(const LogFileInfo& file_info) -> void;

        /**
         * @brief Clears the recent log files list and persists the change.
         */
        auto clear_recent_log_files() -> void;

        /**
         * @brief Returns the list of recent sessions metadata.
         * @return Vector of RecentSessionRecord.
         */
        [[nodiscard]] auto get_recent_sessions() const -> QVector<RecentSessionRecord>;

        /**
         * @brief Saves or updates a session metadata entry and persists the root document.
         * @param session_id Unique identifier of the session.
         * @param name Human-readable session name.
         * @param is_open_update If true, updates last_opened; if false, ensures created_at exists.
         *
         * Behavior:
         * - If the session id exists, update its name and timestamps accordingly.
         * - If not, create a new record with timestamps.
         * - Most recent sessions appear first.
         */
        auto upsert_session_metadata(const QString& session_id, const QString& name,
                                     bool is_open_update) -> void;

        /**
         * @brief Deletes a session JSON file and removes its metadata entry, then persists root.
         * @param session_id The session identifier.
         * @return True if file existed and metadata entry was removed; false otherwise.
         */
        [[nodiscard]] auto delete_session(const QString& session_id) -> bool;

        /**
         * @brief Loads a full session JSON object from repository.
         * @param session_id The session identifier.
         * @return QJsonObject representing the session, or empty object if not found.
         */
        [[nodiscard]] auto load_session(const QString& session_id) const -> QJsonObject;

        /**
         * @brief Saves a full session JSON object under its id (atomic write).
         * @param session_id The session identifier.
         * @param session_obj The JSON object to persist.
         */
        auto save_session(const QString& session_id, const QJsonObject& session_obj) -> void;

        /**
         * @brief Returns the last session id stored in the root document (optional).
         * @return Last session id or empty string if not set.
         */
        [[nodiscard]] auto get_last_session_id() const -> QString;

        /**
         * @brief Sets the last session id in the root document and persists the change.
         * @param session_id The session identifier to store.
         */
        auto set_last_session_id(const QString& session_id) -> void;

    signals:
        /**
         * @brief Emitted when the recent log files list changes.
         * @param items Current recent log files.
         */
        void recent_log_files_changed(const QVector<RecentLogFileRecord>& items);

        /**
         * @brief Emitted when the recent sessions list changes.
         * @param items Current recent sessions.
         */
        void recent_sessions_changed(const QVector<RecentSessionRecord>& items);

    private:
        /**
         * @brief Converts a RecentLogFileRecord to JSON.
         * @param rec The record to convert.
         * @return QJsonObject representation.
         */
        [[nodiscard]] static auto to_json(const RecentLogFileRecord& rec) -> QJsonObject;

        /**
         * @brief Converts a RecentSessionRecord to JSON.
         * @param rec The record to convert.
         * @return QJsonObject representation.
         */
        [[nodiscard]] static auto to_json(const RecentSessionRecord& rec) -> QJsonObject;

        /**
         * @brief Parses a recent file record from JSON.
         * @param obj The JSON object.
         * @return RecentLogFileRecord value.
         */
        [[nodiscard]] static auto from_json_recent_file(const QJsonObject& obj)
            -> RecentLogFileRecord;

        /**
         * @brief Parses a recent session record from JSON.
         * @param obj The JSON object.
         * @return RecentSessionRecord value.
         */
        [[nodiscard]] static auto from_json_recent_session(const QJsonObject& obj)
            -> RecentSessionRecord;

        /**
         * @brief Builds the root JSON document from current in-memory state.
         * @return QJsonObject root.
         */
        [[nodiscard]] auto make_root_from_current() const -> QJsonObject;

        /**
         * @brief Persists the root JSON document using repository (atomic write).
         * @param root Root object to save.
         */
        auto persist_root(const QJsonObject& root) -> void;

        /**
         * @brief Loads the root JSON document using repository; ensures defaults.
         * @return Root JSON object.
         */
        auto load_root() const -> QJsonObject;

        /**
         * @brief Sort recent files by last_opened descending, then by file_name ascending as
         * tiebreaker.
         */
        auto sort_recent_files_mru() -> void;

        /**
         * @brief Sort recent sessions by last_opened or created_at (most recent first).
         */
        auto sort_recent_sessions_mru() -> void;

        /**
         * @brief Removes a recent session by id.
         * @param session_id The id to remove.
         * @return True if removed.
         */
        auto remove_recent_session_by_id(const QString& session_id) -> bool;

        /**
         * @brief Upserts a recent session record into the list.
         * @param rec The record to upsert.
         */
        auto upsert_recent_session(const RecentSessionRecord& rec) -> void;

        /**
         * @brief Upserts a recent file record into the list (dedupe by file_path).
         * @param rec The record to upsert.
         */
        auto upsert_recent_file(const RecentLogFileRecord& rec) -> void;

    private:
        SessionRepository* m_repository{nullptr};
        QVector<RecentLogFileRecord> m_recent_files;
        QVector<RecentSessionRecord> m_recent_sessions;
        QString m_last_session_id;
};
