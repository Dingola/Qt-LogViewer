/**
 * @file SessionController.cpp
 * @brief Implements SessionController for coordinating session lifecycle operations.
 */

#include "Qt-LogViewer/Controllers/SessionController.h"

#include <QDebug>
#include <QFileInfo>
#include <QTimer>
#include <QUuid>

// Concrete includes for forward-declared types and value usage
#include "Qt-LogViewer/Controllers/FileCatalogController.h"
#include "Qt-LogViewer/Controllers/FilterCoordinator.h"
#include "Qt-LogViewer/Controllers/LiveTailingCoordinator.h"
#include "Qt-LogViewer/Controllers/LogImportCoordinator.h"
#include "Qt-LogViewer/Controllers/LogPageCoordinator.h"
#include "Qt-LogViewer/Controllers/LogQueryController.h"
#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/ViewLifecycleCoordinator.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Models/LogFileTreeModel.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/SessionTypes.h"
#include "Qt-LogViewer/Services/LogHistoryService.h"
#include "Qt-LogViewer/Services/SessionCodec.h"
#include "Qt-LogViewer/Services/SessionManager.h"

namespace
{
/**
 * @brief Resolves the persisted parsing profile for one restored file.
 * @param state Restored view state containing profile identifiers.
 * @param file_path File whose profile is requested.
 * @param available_profiles Profiles currently configured by the application.
 * @param default_profile Fallback for legacy or missing profiles.
 * @return Matching configured profile or the fallback profile.
 */
[[nodiscard]] auto get_session_file_profile(const SessionViewState& state, const QString& file_path,
                                            const QVector<LogParsingProfile>& available_profiles,
                                            const LogParsingProfile& default_profile)
    -> LogParsingProfile
{
    const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
    const auto profile_id = state.file_parsing_profile_ids.constFind(absolute_file_path);
    LogParsingProfile profile = default_profile;

    if (profile_id != state.file_parsing_profile_ids.cend())
    {
        bool profile_found = false;
        for (const LogParsingProfile& available_profile: available_profiles)
        {
            if (available_profile.get_id() == profile_id.value())
            {
                profile = available_profile;
                profile_found = true;
            }
        }

        if (!profile_found)
        {
            qWarning().nospace() << "Could not resolve parsing profile "
                                 << profile_id->toString(QUuid::WithoutBraces) << " for file=\""
                                 << absolute_file_path << "\"; using the default profile.";
        }
    }

    return profile;
}

/**
 * @brief Maps a stable sort field to its model column.
 * @param field Stable field identifier.
 * @return Matching LogModel column.
 */
[[nodiscard]] auto get_model_sort_column(const QString& field) -> int
{
    int column = LogModel::Timestamp;
    if (field == LogField::Level)
    {
        column = LogModel::Level;
    }
    else if (field == LogField::Message)
    {
        column = LogModel::Message;
    }
    else if (field == LogField::AppName)
    {
        column = LogModel::AppName;
    }
    return column;
}
}  // namespace

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
SessionController::SessionController(SessionManager* session_manager, LogFileTreeModel* tree_model,
                                     const LogParsingProfile& default_profile,
                                     FileCatalogController* catalog, ViewRegistry* views,
                                     FilterCoordinator* filters, LogHistoryService* history,
                                     LogPageCoordinator* pages, LogQueryController* queries,
                                     LogImportCoordinator* imports,
                                     ViewLifecycleCoordinator* lifecycle,
                                     LiveTailingCoordinator* live_tailing, QObject* parent)
    : QObject(parent),
      m_session_manager(session_manager),
      m_tree_model(tree_model),
      m_default_profile(default_profile),
      m_catalog(catalog),
      m_views(views),
      m_filters(filters),
      m_history(history),
      m_pages(pages),
      m_queries(queries),
      m_imports(imports),
      m_lifecycle(lifecycle),
      m_live_tailing(live_tailing)
{
    if (m_tree_model != nullptr)
    {
        connect(m_tree_model, &LogFileTreeModel::all_sessions_removed, this,
                &SessionController::all_sessions_removed);
    }

    if (m_imports != nullptr)
    {
        connect(m_imports, &LogImportCoordinator::finished, this,
                [this](const QUuid& view_id, const QString& file_path) {
                    complete_restored_file(view_id, file_path);
                });
        connect(m_imports, &LogImportCoordinator::error, this,
                [this](const QUuid& view_id, const QString& file_path, const QString&) {
                    complete_restored_file(view_id, file_path);
                });
    }
}

/**
 * @brief Ensures a current session exists, creating one if necessary.
 * @param default_name Default name for a newly created session.
 * @return The current session ID (existing or newly created).
 */
auto SessionController::ensure_current_session(const QString& default_name) -> QString
{
    QString session_id;

    if (m_session_manager != nullptr && !m_session_manager->has_current_session())
    {
        session_id = create_session(default_name);
    }
    else if (m_session_manager != nullptr)
    {
        session_id = m_session_manager->get_current_session_id();
    }

    return session_id;
}

/**
 * @brief Checks if a current session exists.
 * @return True if a session is active.
 */
auto SessionController::has_current_session() const -> bool
{
    bool has_session = false;

    if (m_session_manager != nullptr)
    {
        has_session = m_session_manager->has_current_session();
    }

    return has_session;
}

/**
 * @brief Returns the current session ID.
 * @return The current session ID, or empty if none.
 */
auto SessionController::get_current_session_id() const -> QString
{
    QString session_id;

    if (m_session_manager != nullptr)
    {
        session_id = m_session_manager->get_current_session_id();
    }

    return session_id;
}

/**
 * @brief Returns the last session ID from storage.
 * @return The last session ID, or empty if none.
 */
auto SessionController::get_last_session_id() const -> QString
{
    QString session_id;

    if (m_session_manager != nullptr)
    {
        session_id = m_session_manager->get_last_session_id();
    }

    return session_id;
}

/**
 * @brief Adds files to the current session.
 * @param file_paths Absolute paths to log files.
 */
auto SessionController::add_files_to_current_session(const QVector<QString>& file_paths) -> void
{
    const QString session_id = get_current_session_id();

    if (!session_id.isEmpty() && m_catalog != nullptr)
    {
        m_catalog->add_files_to_session(session_id, file_paths);
    }
}

/**
 * @brief Adds a single file to a specific session.
 * @param session_id The session ID.
 * @param file_path Absolute path to the log file.
 */
auto SessionController::add_file_to_session(const QString& session_id,
                                            const QString& file_path) -> void
{
    if (!session_id.isEmpty() && m_catalog != nullptr)
    {
        m_catalog->add_file_to_session(session_id, file_path);
    }
}

/**
 * @brief Adds a recent log file record.
 * @param file_info The log file info.
 */
auto SessionController::add_recent_log_file(const LogFileInfo& file_info) -> void
{
    if (m_session_manager != nullptr)
    {
        m_session_manager->add_recent_log_file(file_info);
    }
}

/**
 * @brief Clears all recent log files.
 */
auto SessionController::clear_recent_log_files() -> void
{
    if (m_session_manager != nullptr)
    {
        m_session_manager->clear_recent_log_files();
    }
}

/**
 * @brief Saves the current session state.
 */
auto SessionController::save_current_session() -> void
{
    const bool can_save = (m_session_manager != nullptr);

    if (can_save)
    {
        const QString session_id = m_session_manager->get_current_session_id();

        if (!session_id.isEmpty())
        {
            // Collect views with files
            QVector<QUuid> nonempty_view_ids;
            if (m_views != nullptr)
            {
                const QVector<QUuid> view_ids = m_views->get_all_view_ids();
                nonempty_view_ids.reserve(view_ids.size());

                for (const QUuid& vid: view_ids)
                {
                    if (!m_views->get_file_paths(vid).isEmpty())
                    {
                        nonempty_view_ids.append(vid);
                    }
                }
            }

            // Collect files from tree model (explorer) for this session
            QList<LogFileInfo> tree_files = collect_session_files_from_tree(session_id);

            // Save session with both views and tree files
            save_session_impl(session_id, nonempty_view_ids, tree_files);
        }
    }
}

/**
 * @brief Closes a session (removes from view, preserves persistence).
 * @param session_id The session ID to close.
 * @return True if the session was closed successfully.
 */
auto SessionController::close_session(const QString& session_id) -> bool
{
    bool closed = false;
    const bool valid_session = !session_id.isEmpty();

    if (valid_session)
    {
        // Save before closing if it's the current session
        const bool is_current_session = (m_session_manager != nullptr &&
                                         m_session_manager->get_current_session_id() == session_id);

        if (is_current_session)
        {
            save_current_session();
        }

        // Clear all views from the controller to prevent stale data
        if (is_current_session)
        {
            clear_all_views();
        }

        // Remove from tree model only
        if (m_tree_model != nullptr)
        {
            m_tree_model->remove_session(session_id);
            closed = true;
        }

        // Clear current session if it was closed
        if (is_current_session && m_session_manager != nullptr)
        {
            m_session_manager->set_current_session_id(QString());
            emit current_session_changed(QString());
        }

        if (closed)
        {
            emit session_closed(session_id);
        }
    }

    return closed;
}

/**
 * @brief Deletes a session permanently.
 * @param session_id The session ID to delete.
 * @return True if the session was deleted successfully.
 */
auto SessionController::delete_session(const QString& session_id) -> bool
{
    bool deleted = false;
    const bool valid_session = !session_id.isEmpty();

    if (valid_session)
    {
        // Remove from tree model
        if (m_tree_model != nullptr)
        {
            m_tree_model->remove_session(session_id);
        }

        // Remove from persistence
        if (m_session_manager != nullptr)
        {
            deleted = m_session_manager->delete_session(session_id);

            if (m_session_manager->get_current_session_id() == session_id)
            {
                m_session_manager->set_current_session_id(QString());
                emit current_session_changed(QString());
            }
        }

        if (deleted)
        {
            emit session_deleted(session_id);
        }
    }

    return deleted;
}

/**
 * @brief Renames a session.
 * @param session_id The session ID.
 * @param new_name The new session name.
 * @return True if the session was renamed successfully.
 */
auto SessionController::rename_session(const QString& session_id, const QString& new_name) -> bool
{
    bool renamed = false;
    const bool valid_args = (!session_id.isEmpty() && !new_name.isEmpty());

    if (valid_args)
    {
        // Check if model needs update (inline edit may have updated already)
        bool model_updated = false;

        if (m_tree_model != nullptr)
        {
            const QModelIndex session_index = m_tree_model->get_session_index(session_id);
            const QString current_name = session_index.isValid()
                                             ? session_index.data(Qt::DisplayRole).toString()
                                             : QString();

            if (current_name != new_name)
            {
                model_updated = m_tree_model->rename_session(session_id, new_name);
            }
            else
            {
                model_updated = true;  // Already updated via inline edit
            }
        }

        if (model_updated && m_session_manager != nullptr)
        {
            m_session_manager->upsert_session_metadata(session_id, new_name, false);

            std::optional<SessionState> stored_state =
                SessionCodec::from_json(m_session_manager->load_session(session_id), session_id);

            if (stored_state.has_value())
            {
                stored_state->name = new_name;
                m_session_manager->save_session(session_id, SessionCodec::to_json(*stored_state));
            }
            else
            {
                SessionState new_state = export_session_state();
                new_state.name = new_name;
                new_state.id = session_id;
                m_session_manager->save_session(session_id, SessionCodec::to_json(new_state));
            }

            renamed = true;
            emit session_renamed(session_id, new_name);
        }
    }

    return renamed;
}

/**
 * @brief Loads and restores a session from storage.
 * @param session_id The session ID to load.
 * @return Typed session state, or no value if not found.
 */
auto SessionController::load_session(const QString& session_id) -> std::optional<SessionState>
{
    std::optional<SessionState> state;
    const bool valid_args = (m_session_manager != nullptr && !session_id.isEmpty());

    if (valid_args)
    {
        state = SessionCodec::from_json(m_session_manager->load_session(session_id), session_id);

        if (state.has_value())
        {
            state->id = session_id;
            m_session_manager->set_current_session_id(session_id);
            m_session_manager->set_last_session_id(session_id);

            const QString& session_name = state->name;
            m_session_manager->upsert_session_metadata(session_id, session_name, true);

            // Ensure session exists in tree model
            if (m_tree_model != nullptr && !m_tree_model->has_session(session_id))
            {
                m_tree_model->add_session(session_id, session_name);
            }

            emit current_session_changed(session_id);
        }
    }

    return state;
}

/**
 * @brief Restores a complete typed session through one coordinated workflow.
 *
 * Runtime views are cleared before the UI is asked to discard remaining temporary tabs. Each
 * restored view and its deferred query state are registered immediately. Once all tabs exist, the
 * UI is notified so its transition can capture the complete workspace before persistent entries
 * are read. File imports begin on a later event-loop turn, while final query application and
 * session completion still wait for every file result. Persisted profile identifiers are resolved
 * by the import boundary.
 *
 * @param state Typed session snapshot to restore.
 * @param available_profiles Parsing profiles available for persisted profile references.
 * @return True when the session snapshot was accepted.
 */
auto SessionController::restore_session(
    const SessionState& state, const QVector<LogParsingProfile>& available_profiles) -> bool
{
    const bool can_restore = !state.id.isEmpty() && m_views != nullptr && m_filters != nullptr &&
                             m_history != nullptr && m_queries != nullptr && m_imports != nullptr &&
                             m_lifecycle != nullptr && m_live_tailing != nullptr;
    bool restored = false;

    if (can_restore)
    {
        m_pending_restore_states.clear();
        m_pending_restore_files.clear();
        m_pending_restore_imports.clear();
        m_restore_imports_deferred = false;
        m_restoring_session_id = state.id;
        m_registering_restored_views = true;

        m_lifecycle->close_all_views();
        emit session_restore_started(state.id);

        if (m_tree_model != nullptr)
        {
            for (const LogFileInfo& file_info: state.explorer_files)
            {
                m_tree_model->add_log_file(state.id, file_info);
            }
        }

        for (const SessionViewState& stored_view_state: state.views)
        {
            SessionViewState view_state = stored_view_state;
            if (view_state.id.isNull())
            {
                view_state.id = QUuid::createUuid();
            }

            QSet<QString> pending_files;
            for (const LogFileInfo& file_info: view_state.loaded_files)
            {
                const QString stored_path = file_info.get_file_path();
                if (!stored_path.isEmpty())
                {
                    pending_files.insert(QFileInfo(stored_path).absoluteFilePath());
                }
            }

            if (!pending_files.isEmpty())
            {
                m_pending_restore_states.insert(view_state.id, view_state);
                m_pending_restore_files.insert(view_state.id, pending_files);
            }

            const QUuid view_id = m_views->import_view_state(view_state);

            if (!view_id.isNull())
            {
                m_imports->cancel(view_id);
                const bool cleanup_queued = m_imports->discard_history(view_id);
                if (!cleanup_queued)
                {
                    m_history->remove_view_entries(view_id);
                }
                m_live_tailing->reset_view(view_id, view_state.filters.live_tailing_enabled);

                QVector<QString> paths;
                for (const LogFileInfo& file_info: view_state.loaded_files)
                {
                    const QString path = file_info.get_file_path();
                    if (!path.isEmpty())
                    {
                        paths.append(path);
                    }
                }
                if (m_catalog != nullptr && !paths.isEmpty())
                {
                    m_catalog->add_files_to_session(state.id, paths);
                }

                m_queries->prepare_view_state(view_id, view_state);
                emit view_restored(view_id, view_state);

                for (const QString& path: paths)
                {
                    const LogParsingProfile profile = get_session_file_profile(
                        view_state, path, available_profiles, m_default_profile);
                    m_pending_restore_imports.append(PendingRestoreImport{view_id, path, profile});
                }
            }
            else
            {
                m_pending_restore_states.remove(view_state.id);
                m_pending_restore_files.remove(view_state.id);
            }
        }

        m_registering_restored_views = false;
        emit session_views_registered(state.id);
        if (!m_restore_imports_deferred)
        {
            const QString restore_session_id = state.id;
            QTimer::singleShot(0, this, [this, restore_session_id] {
                if (m_restoring_session_id == restore_session_id && !m_restore_imports_deferred)
                {
                    start_deferred_restore_imports();
                }
            });
        }
        finish_session_restore_if_ready();
        restored = true;
    }

    return restored;
}

/**
 * @brief Prevents registered restore imports from starting automatically.
 */
auto SessionController::defer_restored_imports_until_presented() -> void
{
    if (!m_restoring_session_id.isEmpty() && !m_pending_restore_imports.isEmpty())
    {
        m_restore_imports_deferred = true;
    }
}

/** @brief Starts every file import retained for the active session restoration. */
auto SessionController::start_deferred_restore_imports() -> void
{
    const QVector<PendingRestoreImport> imports = m_pending_restore_imports;
    m_pending_restore_imports.clear();
    m_restore_imports_deferred = false;

    if (m_imports != nullptr)
    {
        for (const PendingRestoreImport& import: imports)
        {
            m_imports->enqueue_registered_file(import.view_id, import.file_path, import.profile,
                                               1000);
        }
    }
}

/**
 * @brief Exports the current session as typed state.
 * @return The session state.
 */
auto SessionController::export_session_state() const -> SessionState
{
    SessionState state;
    state.id = get_current_session_id();

    if (m_views != nullptr)
    {
        const QVector<QUuid> view_ids = m_views->get_all_view_ids();

        for (const QUuid& vid: view_ids)
        {
            if (!m_views->get_file_paths(vid).isEmpty())
            {
                state.views.append(build_view_state(vid));
            }
        }
    }

    if (!state.id.isEmpty())
    {
        state.explorer_files = collect_session_files_from_tree(state.id);
    }

    return state;
}

/**
 * @brief Gets the session count in the tree model.
 * @return Number of sessions.
 */
auto SessionController::get_session_count() const -> int
{
    int count = 0;

    if (m_tree_model != nullptr)
    {
        count = m_tree_model->get_session_count();
    }

    return count;
}

/**
 * @brief Expands a session in the tree model (for UI notification).
 * @param session_id The session ID to expand.
 */
auto SessionController::request_expand_session(const QString& session_id) -> void
{
    if (!session_id.isEmpty())
    {
        emit expand_session_requested(session_id);
    }
}

/**
 * @brief Clears all views from the controller.
 *
 * This method is called when closing a session to ensure no stale view data remains.
 */
auto SessionController::clear_all_views() -> void
{
    if (m_lifecycle != nullptr)
    {
        m_lifecycle->close_all_views();
    }
}

/**
 * @brief Creates a new session with the given name.
 * @param session_name The session name.
 * @return The new session ID.
 */
auto SessionController::create_session(const QString& session_name) -> QString
{
    const QString session_id = QUuid::createUuid().toString(QUuid::WithoutBraces);

    if (m_session_manager != nullptr)
    {
        m_session_manager->set_current_session_id(session_id);
        m_session_manager->upsert_session_metadata(session_id, session_name, true);
    }

    if (m_tree_model != nullptr)
    {
        m_tree_model->add_session(session_id, session_name);
    }

    SessionState state;
    state.id = session_id;
    state.name = session_name;

    if (m_session_manager != nullptr)
    {
        m_session_manager->save_session(session_id, SessionCodec::to_json(state));
    }

    emit session_created(session_id, session_name);
    emit current_session_changed(session_id);

    return session_id;
}

/**
 * @brief Collects all files from the tree model for a given session.
 * @param session_id The session identifier.
 * @return List of LogFileInfo objects in the session's tree.
 */
auto SessionController::collect_session_files_from_tree(const QString& session_id) const
    -> QList<LogFileInfo>
{
    QList<LogFileInfo> files;

    const bool can_collect = m_tree_model != nullptr && !session_id.isEmpty();
    QModelIndex session_index;
    if (can_collect)
    {
        session_index = m_tree_model->get_session_index(session_id);
    }

    if (session_index.isValid())
    {
        const int group_count = m_tree_model->rowCount(session_index);
        for (int group = 0; group < group_count; ++group)
        {
            const QModelIndex group_index = m_tree_model->index(group, 0, session_index);
            const int file_count = m_tree_model->rowCount(group_index);

            for (int file = 0; file < file_count; ++file)
            {
                const QModelIndex file_index = m_tree_model->index(file, 0, group_index);
                const QString file_path =
                    file_index.data(LogFileTreeModel::FilePathRole).toString();
                const QString app_name = file_index.data(LogFileTreeModel::AppNameRole).toString();

                if (!file_path.isEmpty())
                {
                    files.append(LogFileInfo(file_path, app_name));
                }
            }
        }
    }

    return files;
}

/**
 * @brief Internal implementation of session saving.
 * @param session_id The session identifier.
 * @param nonempty_view_ids Vector of view IDs that have files loaded.
 * @param tree_files Files from the tree model (explorer) to persist.
 */
auto SessionController::save_session_impl(const QString& session_id,
                                          const QVector<QUuid>& nonempty_view_ids,
                                          const QList<LogFileInfo>& tree_files) -> void
{
    if (m_session_manager != nullptr && !session_id.isEmpty())
    {
        QString session_name = QStringLiteral("Session");

        const std::optional<SessionState> existing_state =
            SessionCodec::from_json(m_session_manager->load_session(session_id), session_id);
        if (existing_state.has_value() && !existing_state->name.isEmpty())
        {
            session_name = existing_state->name;
        }

        SessionState state;
        state.id = session_id;
        state.explorer_files = tree_files;

        for (const QUuid& view_id: nonempty_view_ids)
        {
            const SessionViewState view_state = build_view_state(view_id);
            state.views.append(view_state);

            if (state.views.size() == 1 && !view_state.tab_title.isEmpty() &&
                session_name == QStringLiteral("Session"))
            {
                session_name = view_state.tab_title;
            }
        }

        state.name = session_name;
        m_session_manager->save_session(session_id, SessionCodec::to_json(state));
        m_session_manager->upsert_session_metadata(session_id, session_name, false);
    }
}

/**
 * @brief Exports the runtime state of one view.
 * @param view_id The view ID.
 * @return The view state.
 */
auto SessionController::build_view_state(const QUuid& view_id) const -> SessionViewState
{
    SessionViewState state;

    if (m_views != nullptr && m_filters != nullptr && m_pages != nullptr &&
        m_live_tailing != nullptr)
    {
        state = m_views->export_view_state(view_id, *m_filters);
        state.filters.live_tailing_enabled = m_live_tailing->is_enabled(view_id);

        const LogViewContext* context = m_views->get_context(view_id);
        const QHash<QString, LogParsingProfile> profiles =
            context != nullptr ? context->get_file_parsing_profiles()
                               : QHash<QString, LogParsingProfile>();
        for (const LogFileInfo& file_info: state.loaded_files)
        {
            const QString path = QFileInfo(file_info.get_file_path()).absoluteFilePath();
            const auto profile = profiles.constFind(path);
            if (profile != profiles.cend())
            {
                state.file_parsing_profile_ids.insert(path, profile->get_id());
            }
        }

        const LogPageState* page_state = m_pages->get_page_state(view_id);
        if (page_state != nullptr)
        {
            state.page_size = static_cast<int>(page_state->get_page_size());
            state.current_page = static_cast<int>(page_state->get_current_page());
            state.sort_column = get_model_sort_column(page_state->get_query().sort_field);
            state.sort_order = page_state->get_query().sort_order;
        }
    }

    return state;
}

/**
 * @brief Marks one restored file import as resolved and applies the view state when ready.
 * @param view_id View receiving the import result.
 * @param file_path File whose import finished or failed.
 */
auto SessionController::complete_restored_file(const QUuid& view_id,
                                               const QString& file_path) -> void
{
    auto pending_files = m_pending_restore_files.find(view_id);

    if (pending_files != m_pending_restore_files.end())
    {
        pending_files->remove(QFileInfo(file_path).absoluteFilePath());

        if (pending_files->isEmpty())
        {
            const SessionViewState state = m_pending_restore_states.take(view_id);
            m_pending_restore_files.erase(pending_files);

            if (m_queries != nullptr)
            {
                m_queries->apply_view_state(view_id, state);
            }

            finish_session_restore_if_ready();
        }
    }
}

/**
 * @brief Emits session_restored() once no restored view is awaiting file imports.
 */
auto SessionController::finish_session_restore_if_ready() -> void
{
    const bool restore_finished = !m_registering_restored_views &&
                                  !m_restoring_session_id.isEmpty() &&
                                  m_pending_restore_files.isEmpty();

    if (restore_finished)
    {
        const QString session_id = m_restoring_session_id;
        m_restoring_session_id.clear();
        emit session_restored(session_id);
    }
}
