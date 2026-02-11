/**
 * @file SessionController.cpp
 * @brief Implements SessionController for coordinating session lifecycle operations.
 */

#include "Qt-LogViewer/Controllers/SessionController.h"

#include <QJsonArray>
#include <QUuid>

// Concrete includes for forward-declared types and value usage
#include "Qt-LogViewer/Controllers/LogViewerController.h"
#include "Qt-LogViewer/Models/LogFileTreeModel.h"
#include "Qt-LogViewer/Models/SessionTypes.h"
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

            QJsonObject session_obj = m_session_manager->load_session(session_id);
            if (!session_obj.isEmpty())
            {
                session_obj.insert(QStringLiteral("name"), new_name);
                m_session_manager->save_session(session_id, session_obj);
            }
            else
            {
                // Create new session JSON from current state
                QJsonObject new_session_obj = export_session_to_json();
                new_session_obj.insert(QStringLiteral("name"), new_name);
                new_session_obj.insert(QStringLiteral("id"), session_id);
                m_session_manager->save_session(session_id, new_session_obj);
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
 * @return The session JSON object, or empty if not found.
 */
auto SessionController::load_session(const QString& session_id) -> QJsonObject
{
    QJsonObject obj;
    const bool valid_args = (m_session_manager != nullptr && !session_id.isEmpty());

    if (valid_args)
    {
        obj = m_session_manager->load_session(session_id);

        if (!obj.isEmpty())
        {
            m_session_manager->set_current_session_id(session_id);
            m_session_manager->set_last_session_id(session_id);

            const QString session_name = obj.value(QStringLiteral("name")).toString();
            m_session_manager->upsert_session_metadata(session_id, session_name, true);

            // Ensure session exists in tree model
            if (m_tree_model != nullptr && !m_tree_model->has_session(session_id))
            {
                m_tree_model->add_session(session_id, session_name);
            }

            emit current_session_changed(session_id);
        }
    }

    return obj;
}

/**
 * @brief Exports the current session state to JSON.
 * @return The session JSON object.
 */
auto SessionController::export_session_to_json() const -> QJsonObject
{
    QJsonObject session_obj;
    session_obj.insert(QStringLiteral("schema_version"), 1);

    if (m_controller != nullptr)
    {
        QJsonArray views_array;
        const QVector<QUuid> view_ids = m_controller->get_all_view_ids();

        for (const QUuid& vid: view_ids)
        {
            if (!m_controller->get_view_file_paths(vid).isEmpty())
            {
                views_array.append(build_view_json(vid));
            }
        }

        session_obj.insert(QStringLiteral("views"), views_array);
    }

    // Also include files from tree model
    const QString session_id = get_current_session_id();
    if (!session_id.isEmpty())
    {
        QList<LogFileInfo> tree_files = collect_session_files_from_tree(session_id);
        QJsonArray explorer_files_array;
        for (const auto& file_info: tree_files)
        {
            QJsonObject file_obj;
            file_obj.insert(QStringLiteral("file_path"), file_info.get_file_path());
            file_obj.insert(QStringLiteral("app_name"), file_info.get_app_name());
            explorer_files_array.append(file_obj);
        }
        session_obj.insert(QStringLiteral("explorer_files"), explorer_files_array);
    }

    return session_obj;
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

    // Persist initial empty session
    QJsonObject session_obj;
    session_obj.insert(QStringLiteral("schema_version"), 1);
    session_obj.insert(QStringLiteral("name"), session_name);
    session_obj.insert(QStringLiteral("id"), session_id);
    session_obj.insert(QStringLiteral("views"), QJsonArray());
    session_obj.insert(QStringLiteral("explorer_files"), QJsonArray());

    if (m_session_manager != nullptr)
    {
        m_session_manager->save_session(session_id, session_obj);
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
    const QJsonObject existing_obj = m_session_manager->load_session(session_id);
    const QString existing_name = existing_obj.value(QStringLiteral("name")).toString();
    if (!existing_name.isEmpty())
    {
        session_name = existing_name;
    }

    QJsonArray views_array;
    for (const QUuid& vid: nonempty_view_ids)
    {
        QJsonObject view_obj = build_view_json(vid);
        views_array.append(view_obj);

        // Use first tab title as session name if not set
        if (views_array.size() == 1)
        {
            const QString tab_title = view_obj.value(QStringLiteral("tab_title")).toString();
            if (!tab_title.isEmpty() && session_name == QStringLiteral("Session"))
            {
                session_name = tab_title;
            }
        }
    }

    // Build explorer files array
    QJsonArray explorer_files_array;
    for (const auto& file_info: tree_files)
    {
        QJsonObject file_obj;
        file_obj.insert(QStringLiteral("file_path"), file_info.get_file_path());
        file_obj.insert(QStringLiteral("app_name"), file_info.get_app_name());
        explorer_files_array.append(file_obj);
    }

    QJsonObject session_obj;
    session_obj.insert(QStringLiteral("schema_version"), 1);
    session_obj.insert(QStringLiteral("id"), session_id);
    session_obj.insert(QStringLiteral("name"), session_name);
    session_obj.insert(QStringLiteral("views"), views_array);
    session_obj.insert(QStringLiteral("explorer_files"), explorer_files_array);

    m_session_manager->save_session(session_id, session_obj);
    m_session_manager->upsert_session_metadata(session_id, session_name, false);
}

/**
 * @brief Builds a JSON object from a view state.
 * @param view_id The view ID.
 * @return The view JSON object.
 */
auto SessionController::build_view_json(const QUuid& view_id) const -> QJsonObject
{
    QJsonObject view_obj;

    if (m_controller != nullptr)
    {
        const SessionViewState state = m_controller->export_view_state(view_id);

        view_obj.insert(QStringLiteral("id"), view_id.toString(QUuid::WithoutBraces));

        QJsonArray files_arr;
        for (const auto& lf: state.loaded_files)
        {
            QJsonObject fobj;
            fobj.insert(QStringLiteral("file_path"), lf.get_file_path());
            fobj.insert(QStringLiteral("app_name"), lf.get_app_name());
            files_arr.append(fobj);
        }
        view_obj.insert(QStringLiteral("loaded_files"), files_arr);

        QJsonObject filters_obj;
        filters_obj.insert(QStringLiteral("app_name"), state.filters.app_name);

        QJsonArray levels_arr;
        for (const auto& lvl: state.filters.log_levels)
        {
            levels_arr.append(lvl);
        }
        filters_obj.insert(QStringLiteral("log_levels"), levels_arr);
        filters_obj.insert(QStringLiteral("search_text"), state.filters.search_text);
        filters_obj.insert(QStringLiteral("search_field"), state.filters.search_field);
        filters_obj.insert(QStringLiteral("use_regex"), state.filters.use_regex);
        filters_obj.insert(QStringLiteral("show_only_file"), state.filters.show_only_file);

        QJsonArray hidden_arr;
        for (const auto& hf: state.filters.hidden_files)
        {
            hidden_arr.append(hf);
        }
        filters_obj.insert(QStringLiteral("hidden_files"), hidden_arr);
        view_obj.insert(QStringLiteral("filters"), filters_obj);

        view_obj.insert(QStringLiteral("page_size"), static_cast<int>(state.page_size));
        view_obj.insert(QStringLiteral("current_page"), static_cast<int>(state.current_page));
        view_obj.insert(QStringLiteral("sort_column"), static_cast<int>(state.sort_column));
        view_obj.insert(QStringLiteral("sort_order"), state.sort_order == Qt::AscendingOrder
                                                          ? QStringLiteral("asc")
                                                          : QStringLiteral("desc"));
        view_obj.insert(QStringLiteral("tab_title"), state.tab_title);
    }

    return view_obj;
}
