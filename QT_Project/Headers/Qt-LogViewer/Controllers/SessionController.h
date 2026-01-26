#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QUuid>

#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Models/LogFileTreeModel.h"
#include "Qt-LogViewer/Services/SessionManager.h"

class LogViewerController;

/**
 * @file SessionController.h
 * @brief Controller responsible for coordinating session lifecycle operations.
 *
 * Responsibilities:
 * - Ensure a current session exists before file operations.
 * - Add files to sessions via the main controller.
 * - Create, close, delete, and rename sessions.
 * - Serialize and deserialize session state to/from JSON.
 * - Coordinate between SessionManager, LogFileTreeModel, and LogViewerController.
 */
class SessionController: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a SessionController.
         * @param session_manager The session manager for persistence.
         * @param tree_model The tree model for UI representation.
         * @param controller The main log viewer controller.
         * @param parent Optional QObject parent.
         */
        explicit SessionController(SessionManager* session_manager, LogFileTreeModel* tree_model,
                                   LogViewerController* controller, QObject* parent = nullptr);

        /**
         * @brief Ensures a current session exists, creating one if necessary.
         * @param default_name Default name for a newly created session.
         * @return The current session ID (existing or newly created).
         */
        [[nodiscard]] auto ensure_current_session(const QString& default_name) -> QString;

        /**
         * @brief Checks if a current session exists.
         * @return True if a session is active.
         */
        [[nodiscard]] auto has_current_session() const -> bool;

        /**
         * @brief Returns the current session ID.
         * @return The current session ID, or empty if none.
         */
        [[nodiscard]] auto get_current_session_id() const -> QString;

        /**
         * @brief Returns the last session ID from storage.
         * @return The last session ID, or empty if none.
         */
        [[nodiscard]] auto get_last_session_id() const -> QString;

        /**
         * @brief Adds files to the current session.
         * @param file_paths Absolute paths to log files.
         */
        auto add_files_to_current_session(const QVector<QString>& file_paths) -> void;

        /**
         * @brief Adds a single file to a specific session.
         * @param session_id The session ID.
         * @param file_path Absolute path to the log file.
         */
        auto add_file_to_session(const QString& session_id, const QString& file_path) -> void;

        /**
         * @brief Adds a recent log file record.
         * @param file_info The log file info.
         */
        auto add_recent_log_file(const LogFileInfo& file_info) -> void;

        /**
         * @brief Clears all recent log files.
         */
        auto clear_recent_log_files() -> void;

        /**
         * @brief Saves the current session state.
         */
        auto save_current_session() -> void;

        /**
         * @brief Closes a session (removes from view, preserves persistence).
         * @param session_id The session ID to close.
         * @return True if the session was closed successfully.
         */
        auto close_session(const QString& session_id) -> bool;

        /**
         * @brief Deletes a session permanently.
         * @param session_id The session ID to delete.
         * @return True if the session was deleted successfully.
         */
        auto delete_session(const QString& session_id) -> bool;

        /**
         * @brief Renames a session.
         * @param session_id The session ID.
         * @param new_name The new session name.
         * @return True if the session was renamed successfully.
         */
        auto rename_session(const QString& session_id, const QString& new_name) -> bool;

        /**
         * @brief Loads and restores a session from storage.
         * @param session_id The session ID to load.
         * @return The session JSON object, or empty if not found.
         */
        [[nodiscard]] auto load_session(const QString& session_id) -> QJsonObject;

        /**
         * @brief Exports the current session state to JSON.
         * @return The session JSON object.
         */
        [[nodiscard]] auto export_session_to_json() const -> QJsonObject;

        /**
         * @brief Gets the session count in the tree model.
         * @return Number of sessions.
         */
        [[nodiscard]] auto get_session_count() const -> int;

        /**
         * @brief Expands a session in the tree model (for UI notification).
         * @param session_id The session ID to expand.
         */
        auto request_expand_session(const QString& session_id) -> void;

        /**
         * @brief Clears all views from the controller.
         *
         * This method is called when closing a session to ensure no stale view data remains.
         */
        auto clear_all_views() -> void;

    signals:
        /**
         * @brief Emitted when a new session is created.
         * @param session_id The new session ID.
         * @param session_name The session name.
         */
        void session_created(const QString& session_id, const QString& session_name);

        /**
         * @brief Emitted when a session is closed.
         * @param session_id The closed session ID.
         */
        void session_closed(const QString& session_id);

        /**
         * @brief Emitted when a session is deleted.
         * @param session_id The deleted session ID.
         */
        void session_deleted(const QString& session_id);

        /**
         * @brief Emitted when a session is renamed.
         * @param session_id The session ID.
         * @param new_name The new session name.
         */
        void session_renamed(const QString& session_id, const QString& new_name);

        /**
         * @brief Emitted when all sessions are removed.
         */
        void all_sessions_removed();

        /**
         * @brief Emitted to request UI expansion of a session.
         * @param session_id The session ID to expand.
         */
        void expand_session_requested(const QString& session_id);

        /**
         * @brief Emitted when the current session changes.
         * @param session_id The new current session ID.
         */
        void current_session_changed(const QString& session_id);

    private:
        /**
         * @brief Creates a new session with the given name.
         * @param session_name The session name.
         * @return The new session ID.
         */
        auto create_session(const QString& session_name) -> QString;

        /**
         * @brief Collects all files from the tree model for a given session.
         * @param session_id The session identifier.
         * @return List of LogFileInfo objects in the session's tree.
         */
        [[nodiscard]] auto collect_session_files_from_tree(const QString& session_id) const
            -> QList<LogFileInfo>;

        /**
         * @brief Internal implementation of session saving.
         * @param session_id The session identifier.
         * @param nonempty_view_ids Vector of view IDs that have files loaded.
         * @param tree_files Files from the tree model (explorer) to persist.
         */
        auto save_session_impl(const QString& session_id, const QVector<QUuid>& nonempty_view_ids,
                               const QList<LogFileInfo>& tree_files) -> void;

        /**
         * @brief Builds a JSON object from a view state.
         * @param view_id The view ID.
         * @return The view JSON object.
         */
        [[nodiscard]] auto build_view_json(const QUuid& view_id) const -> QJsonObject;

    private:
        SessionManager* m_session_manager{nullptr};
        LogFileTreeModel* m_tree_model{nullptr};
        LogViewerController* m_controller{nullptr};
};
