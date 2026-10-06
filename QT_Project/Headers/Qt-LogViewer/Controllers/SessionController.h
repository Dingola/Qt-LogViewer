#pragma once

#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>
#include <QUuid>
#include <QVector>
#include <optional>

// Value types used by value in API
#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Models/SessionTypes.h"
#include "Qt-LogViewer/Services/LogParsingProfile.h"

// Forward declarations (pointers only)
class FileCatalogController;
class FilterCoordinator;
class LiveTailingCoordinator;
class LogHistoryService;
class LogImportCoordinator;
class LogPageCoordinator;
class LogQueryController;
class ViewLifecycleCoordinator;
class ViewRegistry;
class SessionManager;
class LogFileTreeModel;

/**
 * @file SessionController.h
 * @brief Controller responsible for coordinating session lifecycle operations.
 *
 * Responsibilities:
 * - Ensure a current session exists before file operations.
 * - Add files to sessions through the file catalog.
 * - Create, close, delete, and rename sessions.
 * - Convert persisted sessions into typed state through SessionCodec.
 * - Restore typed sessions and defer final view queries until their imports complete.
 * - Coordinate persisted session state with focused runtime components.
 */
class SessionController: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a SessionController.
         * @param session_manager The session manager for persistence.
         * @param tree_model The tree model for UI representation.
         * @param default_profile Fallback profile for legacy session files.
         * @param catalog File catalog used for session explorer entries.
         * @param views Runtime view registry.
         * @param filters Per-view filter state.
         * @param history Persistent log history.
         * @param pages Per-view pagination state.
         * @param queries Query state and reload operations.
         * @param imports Asynchronous import workflow.
         * @param lifecycle View cleanup workflow.
         * @param live_tailing Per-view live-tailing state.
         * @param parent Optional QObject parent.
         */
        explicit SessionController(SessionManager* session_manager, LogFileTreeModel* tree_model,
                                   const LogParsingProfile& default_profile,
                                   FileCatalogController* catalog, ViewRegistry* views,
                                   FilterCoordinator* filters, LogHistoryService* history,
                                   LogPageCoordinator* pages, LogQueryController* queries,
                                   LogImportCoordinator* imports,
                                   ViewLifecycleCoordinator* lifecycle,
                                   LiveTailingCoordinator* live_tailing, QObject* parent = nullptr);

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
         * @return Typed session state, or no value if not found.
         */
        [[nodiscard]] auto load_session(const QString& session_id) -> std::optional<SessionState>;

        /**
         * @brief Restores a complete typed session through one coordinated workflow.
         *
         * Existing views are closed and restored tabs receive empty deferred query state
         * immediately. A connected presenter may defer their imports until its workspace
         * transition finishes. Without a presenter, imports begin on the next event-loop turn.
         * Final query state is applied after all files of that view finish.
         *
         * @param state Typed session snapshot to restore.
         * @param available_profiles Parsing profiles available for persisted profile references.
         * @return True when the session snapshot was accepted.
         */
        auto restore_session(const SessionState& state,
                             const QVector<LogParsingProfile>& available_profiles = {}) -> bool;

        /**
         * @brief Prevents registered restore imports from starting automatically.
         *
         * A presentation coordinator calls this synchronously while handling
         * session_views_registered(), then calls start_deferred_restore_imports() after its visual
         * transition finishes.
         */
        auto defer_restored_imports_until_presented() -> void;

        /** @brief Starts every file import retained for the active session restoration. */
        auto start_deferred_restore_imports() -> void;

        /**
         * @brief Exports the current session as typed state.
         * @return The session state.
         */
        [[nodiscard]] auto export_session_state() const -> SessionState;

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

        /**
         * @brief Emitted after existing controller views were cleared for a session restore.
         * @param session_id Session being restored.
         */
        void session_restore_started(const QString& session_id);

        /**
         * @brief Requests creation of a tab for one registered restored view.
         * @param view_id Restored view identifier.
         * @param state Typed presentation state associated with the view.
         */
        void view_restored(const QUuid& view_id, const SessionViewState& state);

        /**
         * @brief Emitted after every restored view has been registered for presentation.
         * @param session_id Session whose tabs can now be captured by the workspace transition.
         *
         * A synchronous receiver may call defer_restored_imports_until_presented() to retain
         * persistent cache imports until its workspace transition has finished.
         */
        void session_views_registered(const QString& session_id);

        /**
         * @brief Emitted after every restored view has completed its imports and state application.
         * @param session_id Restored session identifier.
         */
        void session_restored(const QString& session_id);

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
         * @brief Exports the runtime state of one view.
         * @param view_id The view ID.
         * @return The view state.
         */
        [[nodiscard]] auto build_view_state(const QUuid& view_id) const -> SessionViewState;

        /**
         * @brief Marks one restored file import as resolved and applies the view state when ready.
         * @param view_id View receiving the import result.
         * @param file_path File whose import finished or failed.
         */
        auto complete_restored_file(const QUuid& view_id, const QString& file_path) -> void;

        /**
         * @brief Emits session_restored() once no restored view is awaiting file imports.
         */
        auto finish_session_restore_if_ready() -> void;

    private:
        /** @brief File import retained until the restored workspace may query persistent data. */
        struct PendingRestoreImport {
                /** @brief Restored view receiving the file. */
                QUuid view_id;
                /** @brief Source file registered on the restored view. */
                QString file_path;
                /** @brief Parsing profile persisted for the source file. */
                LogParsingProfile profile;
        };

        SessionManager* m_session_manager{nullptr};
        LogFileTreeModel* m_tree_model{nullptr};
        LogParsingProfile m_default_profile;
        FileCatalogController* m_catalog{nullptr};
        ViewRegistry* m_views{nullptr};
        FilterCoordinator* m_filters{nullptr};
        LogHistoryService* m_history{nullptr};
        LogPageCoordinator* m_pages{nullptr};
        LogQueryController* m_queries{nullptr};
        LogImportCoordinator* m_imports{nullptr};
        ViewLifecycleCoordinator* m_lifecycle{nullptr};
        LiveTailingCoordinator* m_live_tailing{nullptr};
        QHash<QUuid, SessionViewState> m_pending_restore_states;
        QHash<QUuid, QSet<QString>> m_pending_restore_files;
        QVector<PendingRestoreImport> m_pending_restore_imports;
        QString m_restoring_session_id;
        bool m_registering_restored_views{false};
        bool m_restore_imports_deferred{false};
};
