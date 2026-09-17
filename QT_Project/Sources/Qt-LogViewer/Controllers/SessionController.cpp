/**
 * @file SessionController.cpp
 * @brief Implements SessionController for coordinating session lifecycle operations.
 */

#include "Qt-LogViewer/Controllers/SessionController.h"

#include <QUuid>

// Concrete includes for forward-declared types and value usage
#include "Qt-LogViewer/Controllers/LogViewerController.h"
#include "Qt-LogViewer/Models/LogFileTreeModel.h"
#include "Qt-LogViewer/Models/SessionTypes.h"
#include "Qt-LogViewer/Services/SessionCodec.h"
#include "Qt-LogViewer/Services/SessionManager.h"

/**
 * @brief Constructs a SessionController.
 * @param session_manager The session manager for persistence.
 * @param tree_model The tree model for UI representation.
 * @param controller The main log viewer controller.
 * @param parent Optional QObject parent.
 */
SessionController::SessionController(SessionManager* session_manager, LogFileTreeModel* tree_model,
                                     LogViewerController* controller, QObject* parent)
    : QObject(parent),
      m_session_manager(session_manager),
      m_tree_model(tree_model),
      m_controller(controller)
{
    if (m_tree_model != nullptr)
    {
        connect(m_tree_model, &LogFileTreeModel::all_sessions_removed, this,
                &SessionController::all_sessions_removed);
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

    if (!session_id.isEmpty() && m_controller != nullptr)
    {
        m_controller->add_log_files_to_session(session_id, file_paths);
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
    if (!session_id.isEmpty() && m_controller != nullptr)
    {
        m_controller->add_log_file_to_session(session_id, file_path);
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
            if (m_controller != nullptr)
            {
                const QVector<QUuid> view_ids = m_controller->get_all_view_ids();
                nonempty_view_ids.reserve(view_ids.size());

                for (const QUuid& vid: view_ids)
                {
                    if (!m_controller->get_view_file_paths(vid).isEmpty())
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
 * @brief Exports the current session as typed state.
 * @return The session state.
 */
auto SessionController::export_session_state() const -> SessionState
{
    SessionState state;
    state.id = get_current_session_id();

    if (m_controller != nullptr)
    {
        const QVector<QUuid> view_ids = m_controller->get_all_view_ids();

        for (const QUuid& vid: view_ids)
        {
            if (!m_controller->get_view_file_paths(vid).isEmpty())
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
    if (m_controller != nullptr)
    {
        m_controller->clear_all_views();
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

    if (m_tree_model == nullptr || session_id.isEmpty())
    {
        return files;
    }

    const QModelIndex session_index = m_tree_model->get_session_index(session_id);
    if (!session_index.isValid())
    {
        return files;
    }

    // Iterate through all groups in the session
    const int group_count = m_tree_model->rowCount(session_index);
    for (int g = 0; g < group_count; ++g)
    {
        const QModelIndex group_index = m_tree_model->index(g, 0, session_index);
        const int file_count = m_tree_model->rowCount(group_index);

        for (int f = 0; f < file_count; ++f)
        {
            const QModelIndex file_index = m_tree_model->index(f, 0, group_index);
            const QString file_path = file_index.data(LogFileTreeModel::FilePathRole).toString();
            const QString app_name = file_index.data(LogFileTreeModel::AppNameRole).toString();

            if (!file_path.isEmpty())
            {
                files.append(LogFileInfo(file_path, app_name));
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
    if (m_session_manager == nullptr || session_id.isEmpty())
    {
        return;
    }

    QString session_name = QStringLiteral("Session");

    // Try to get existing session name
    const std::optional<SessionState> existing_state =
        SessionCodec::from_json(m_session_manager->load_session(session_id), session_id);
    if (existing_state.has_value() && !existing_state->name.isEmpty())
    {
        session_name = existing_state->name;
    }

    SessionState state;
    state.id = session_id;
    state.explorer_files = tree_files;

    for (const QUuid& vid: nonempty_view_ids)
    {
        const SessionViewState view_state = build_view_state(vid);
        state.views.append(view_state);

        // Use first tab title as session name if not set
        if (state.views.size() == 1)
        {
            if (!view_state.tab_title.isEmpty() && session_name == QStringLiteral("Session"))
            {
                session_name = view_state.tab_title;
            }
        }
    }

    state.name = session_name;

    m_session_manager->save_session(session_id, SessionCodec::to_json(state));
    m_session_manager->upsert_session_metadata(session_id, session_name, false);
}

/**
 * @brief Exports the runtime state of one view.
 * @param view_id The view ID.
 * @return The view state.
 */
auto SessionController::build_view_state(const QUuid& view_id) const -> SessionViewState
{
    SessionViewState state;

    if (m_controller != nullptr)
    {
        state = m_controller->export_view_state(view_id);
    }

    return state;
}
