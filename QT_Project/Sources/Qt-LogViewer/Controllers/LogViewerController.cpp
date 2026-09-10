/**
 * @file LogViewerController.cpp
 * @brief Implements the LogViewerController which orchestrates log loading, view contexts,
 * filtering, and streaming.
 */

#include "Qt-LogViewer/Controllers/LogViewerController.h"

#include <QDebug>
#include <QFileInfo>
#include <QMetaObject>
#include <QThread>
#include <QTimer>
#include <algorithm>
#include <optional>

// Concrete includes for forward-declared types used in implementation
#include "Qt-LogViewer/Controllers/FileCatalogController.h"
#include "Qt-LogViewer/Controllers/FilterCoordinator.h"
#include "Qt-LogViewer/Controllers/LogIngestController.h"
#include "Qt-LogViewer/Controllers/LogPageCoordinator.h"
#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Models/LogFileTreeModel.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Services/LogHistoryService.h"
#include "Qt-LogViewer/Services/LogHistoryWriter.h"
#include "Qt-LogViewer/Services/LogLoader.h"
#include "Qt-LogViewer/Services/LogLoadingService.h"
#include "Qt-LogViewer/Services/LogParser.h"
#include "Qt-LogViewer/Services/LogTailerService.h"

namespace
{
/**
 * @brief Creates display columns from the placeholders actually used by a profile.
 * @param
 * profile Parsing profile whose resolved field order is used.
 * @return Ordered display
 * definitions with database capabilities for built-in fields.
 */
[[nodiscard]] auto get_profile_columns(const LogParsingProfile& profile)
    -> QVector<LogFieldDefinition>
{
    const LogParser parser(profile);
    const QVector<QString> field_order = parser.get_field_order().fields;
    const auto& configuration = parser.get_configuration();
    const QVector<LogFieldDefinition>& builtins = get_builtin_log_field_definitions();
    QVector<LogFieldDefinition> columns;

    for (const QString& field_id: field_order)
    {
        LogFieldDefinition definition;
        definition.id = field_id;
        definition.display_name = field_id;

        for (const LogFieldDefinition& builtin: builtins)
        {
            if (builtin.id == field_id)
            {
                definition = builtin;
            }
        }

        for (const QtRecordParser::FieldConfiguration& configured_field: configuration.fields)
        {
            if (configured_field.id == field_id && !configured_field.display_name.isEmpty())
            {
                definition.display_name = configured_field.display_name;
            }
        }

        if (definition.display_name == field_id)
        {
            definition.display_name.replace(QLatin1Char('_'), QLatin1Char(' '));

            if (!definition.display_name.isEmpty())
            {
                definition.display_name[0] = definition.display_name.at(0).toUpper();
            }
        }

        bool already_added = false;
        for (const LogFieldDefinition& column: columns)
        {
            if (column.id == definition.id)
            {
                already_added = true;
            }
        }

        if (!already_added)
        {
            columns.append(definition);
        }
    }

    return columns;
}

/**
 * @brief Resolves the persisted profile identifier for a session file.
 * @param state Restored
 * view state containing optional per-file profile identifiers.
 * @param file_path File whose
 * parsing profile is requested.
 * @param available_profiles Profiles loaded from the application
 * settings.
 * @param default_profile Profile used for sessions saved before profile persistence.

 * * @return Matching application profile when available; otherwise the supplied default.
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
 * @brief Maps the current search-field selection to log field identifiers.
 * @param
 * search_field Current search-field selection.
 * @return Fields included in text searching.
 */
[[nodiscard]] auto get_query_search_fields(SearchField search_field) -> QSet<QString>
{
    QSet<QString> fields;

    switch (search_field)
    {
    case SearchField::Message:
        fields.insert(LogField::Message);
        break;

    case SearchField::Level:
        fields.insert(LogField::Level);
        break;

    case SearchField::AppName:
        fields.insert(LogField::AppName);
        break;

    case SearchField::AllFields:
    case SearchField::Count:
        break;
    }

    return fields;
}

/**
 * @brief Maps a LogModel column to a stable log field identifier.
 * @param column LogModel column index.
 * @return Stable field identifier.
 */
[[nodiscard]] auto get_query_sort_field(int column) -> QString
{
    QString field = LogField::Timestamp;

    switch (column)
    {
    case LogModel::Timestamp:
        field = LogField::Timestamp;
        break;

    case LogModel::Level:
        field = LogField::Level;
        break;

    case LogModel::Message:
        field = LogField::Message;
        break;

    case LogModel::AppName:
        field = LogField::AppName;
        break;

    case LogModel::Spacer:
    case LogModel::ColumnCount:
    default:
        break;
    }

    return field;
}

/**
 * @brief Maps a stable sort-field identifier to a LogModel column.
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
 * @brief Constructs a LogViewerController.
 * @param profile Parsing profile used for ingest
 * and live tailing.
 * @param parent Optional parent QObject.
 *
 * Ingest and live-tailing
 * services share the supplied parsing profile. QObject ownership follows
 * the optional parent
 * supplied by the caller.
 */
LogViewerController::LogViewerController(const LogParsingProfile& profile, QObject* parent)
    : QObject(parent),
      m_is_shutting_down(false),
      m_default_profile(profile),
      m_ingest(new LogIngestController(profile, this)),
      m_catalog(new FileCatalogController(m_ingest, this)),
      m_views(new ViewRegistry(this)),
      m_filters(new FilterCoordinator(m_views, this))
{
    // Initialize services
    m_history_service = new LogHistoryService(this);
    m_history_writer_thread = new QThread(this);
    m_history_writer = new LogHistoryWriter(m_history_service->get_database_path());
    m_history_writer->moveToThread(m_history_writer_thread);
    connect(m_history_writer_thread, &QThread::finished, m_history_writer, &QObject::deleteLater);
    connect(m_history_writer, &LogHistoryWriter::import_write_finished, this,
            &LogViewerController::handle_history_write_finished, Qt::QueuedConnection);
    m_history_writer_thread->start();

    m_page_coordinator = new LogPageCoordinator(m_history_service, m_views, this);
    m_tailer_service = new LogTailerService(profile, this);

    m_tail_refresh_timer = new QTimer(this);
    m_tail_refresh_timer->setSingleShot(true);
    m_tail_refresh_timer->setInterval(150);

    connect(m_page_coordinator, &LogPageCoordinator::page_loaded, this,
            [this](const QUuid& view_id, qsizetype current_page, qsizetype total_pages,
                   qsizetype total_entries) {
                emit page_loaded(view_id, current_page, total_pages, total_entries);
            });
    connect(m_page_coordinator, &LogPageCoordinator::page_state_updated, this,
            [this](const QUuid& view_id, qsizetype current_page, qsizetype total_pages,
                   qsizetype total_entries) {
                emit page_state_updated(view_id, current_page, total_pages, total_entries);
            });

    connect(m_tail_refresh_timer, &QTimer::timeout, this, [this]() {
        const QSet<QUuid> views_to_refresh = m_pending_tail_refresh_views;

        m_pending_tail_refresh_views.clear();

        for (const QUuid& view_id: views_to_refresh)
        {
            const LogPageState* page_state = m_page_coordinator->get_page_state(view_id);

            const bool can_update = !m_is_shutting_down &&
                                    m_views->get_context(view_id) != nullptr &&
                                    page_state != nullptr;

            if (can_update)
            {
                if (page_state->get_current_page() == 1)
                {
                    m_page_coordinator->reload(view_id);
                }
                else
                {
                    m_page_coordinator->refresh_total_entries(view_id);
                }
            }
        }
    });

    connect(m_tailer_service, &LogTailerService::entries_available, this,
            [this](const QUuid& view_id, const QString&, const QVector<LogEntry>& entries) {
                const bool can_process = !m_is_shutting_down && !entries.isEmpty() &&
                                         m_views->get_context(view_id) != nullptr;

                if (can_process)
                {
                    const bool entries_added = m_history_service->add_entries(view_id, entries);

                    if (entries_added)
                    {
                        m_pending_tail_refresh_views.insert(view_id);
                        m_tail_refresh_timer->start();
                    }
                }
            });

    connect(m_views, &ViewRegistry::current_view_id_changed, this,
            [this](const QUuid& view_id) { emit current_view_id_changed(view_id); });
    connect(m_views, &ViewRegistry::view_removed, this,
            [this](const QUuid& view_id) { emit view_removed(view_id); });
    connect(m_views, &ViewRegistry::view_file_paths_changed, this,
            [this](const QUuid& view_id, const QVector<QString>& paths) {
                emit view_file_paths_changed(view_id, paths);
            });

    connect(m_ingest, &LogIngestController::entry_batch_parsed, this,
            [this](const QUuid& view_id, const QString& file_path, const QVector<LogEntry>& batch) {
                const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();

                const bool can_store =
                    !m_is_shutting_down && !view_id.isNull() && !batch.isEmpty() &&
                    !m_failed_ingest_files.value(view_id).contains(absolute_file_path) &&
                    m_views->get_context(view_id) != nullptr;

                if (can_store)
                {
                    qDebug().nospace()
                        << "[Controller] batch for view=" << view_id.toString() << " file=\""
                        << absolute_file_path << "\" count=" << batch.size();

                    queue_history_batch(view_id, absolute_file_path, batch);
                }
            });

    // Progress pass-through.
    connect(m_ingest, &LogIngestController::progress, this,
            [this](const QUuid& view_id, const QString& file_path, qint64 bytes_read,
                   qint64 total_bytes) {
                if (!m_is_shutting_down)
                {
                    qDebug().nospace()
                        << "[Controller] progress view=" << view_id.toString() << " file=\""
                        << file_path << "\" " << bytes_read << '/' << total_bytes;

                    if (!view_id.isNull())
                    {
                        emit loading_progress(view_id, bytes_read, total_bytes);
                    }
                }
            });

    // Error pass-through.
    connect(m_ingest, &LogIngestController::error, this,
            [this](const QUuid& view_id, const QString& file_path, const QString& message) {
                const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();

                const bool can_handle = !m_is_shutting_down && !view_id.isNull();

                if (can_handle)
                {
                    qWarning().nospace()
                        << "[Controller] error view=" << view_id.toString() << " file=\""
                        << absolute_file_path << "\" msg=\"" << message << '"';

                    emit loading_error(view_id, absolute_file_path, message);

                    m_failed_ingest_files[view_id].insert(absolute_file_path);

                    const bool file_is_registered = m_views->get_context(view_id) != nullptr &&
                                                    is_file_loaded(view_id, absolute_file_path);

                    if (file_is_registered)
                    {
                        remove_log_file(view_id, absolute_file_path);
                    }
                }
            });

    connect(m_ingest, &LogIngestController::finished, this,
            [this](const QUuid& view_id, const QString& file_path) {
                if (!m_is_shutting_down)
                {
                    queue_history_finish(view_id, QFileInfo(file_path).absoluteFilePath());
                }
            });

    // Advance queue only after thread cleanup (safe to start next).
    connect(m_ingest, &LogIngestController::idle, this, [this]() {
        if (!m_is_shutting_down)
        {
            qDebug().nospace() << "[Controller] streaming_idle: force idle then try start next. "
                               << "pending=" << m_ingest->get_pending_count();

            // Start next first, then read new active view id (fix ordering).
            const bool started = (m_ingest->start_next_if_idle(m_ingest->get_active_batch_size()),
                                  !m_ingest->get_active_view_id().isNull());

            if (started)
            {
                const QUuid new_active = m_ingest->get_active_view_id();
                qDebug().nospace() << "[Controller] started next view=" << new_active.toString()
                                   << " file=\"" << m_ingest->get_active_file_path() << '"';

                if (!new_active.isNull() && (m_views->get_current_view() != new_active))
                {
                    m_views->set_current_view(new_active);
                }
            }
            else
            {
                qDebug().nospace() << "[Controller] no next item started (idle or empty queue).";
            }
        }
    });
}

/**
 * @brief Destroys the LogViewerController.
 *
 * Stops ingestion and live-tail callbacks before QObject-owned services are destroyed so no
 * background or watcher callback can target a removed view.
 */
LogViewerController::~LogViewerController()
{
    m_is_shutting_down = true;

    if (m_tail_refresh_timer != nullptr)
    {
        m_tail_refresh_timer->stop();
    }

    m_pending_tail_refresh_views.clear();
    m_failed_ingest_files.clear();

    if (m_tailer_service != nullptr)
    {
        m_tailer_service->stop_all_tailing();
    }

    if (m_ingest != nullptr)
    {
        const QVector<QUuid> view_ids = m_views->get_all_view_ids();

        for (const QUuid& view_id: view_ids)
        {
            m_ingest->cancel_for_view(view_id);
        }
    }

    if (m_history_writer != nullptr)
    {
        QObject::disconnect(m_history_writer, nullptr, this, nullptr);
    }

    if (m_history_writer_thread != nullptr)
    {
        m_history_writer_thread->quit();
        m_history_writer_thread->wait();
        m_history_writer = nullptr;
    }
}

/**
 * @brief Sets the current view to the given QUuid if it exists.
 * @param view_id The QUuid of the view to set as current.
 * @return True if the view was set successfully, false if the view_id does not exist.
 */
auto LogViewerController::set_current_view(const QUuid& view_id) -> bool
{
    bool success = m_views->set_current_view(view_id);
    return success;
}

/**
 * @brief Returns the QUuid of the current view.
 * @return The QUuid of the current view.
 */
auto LogViewerController::get_current_view() const -> QUuid
{
    QUuid id = m_views->get_current_view();
    return id;
}

/**
 * @brief Returns all registered view ids.
 * @return Vector of QUuid representing all views currently tracked.
 */
auto LogViewerController::get_all_view_ids() const -> QVector<QUuid>
{
    QVector<QUuid> ids;

    if (m_views != nullptr)
    {
        ids = m_views->get_all_view_ids();
    }

    return ids;
}

/**
 * @brief Removes a view and all resources associated with it.
 * @param view_id View identifier to remove.
 * @return True if the view existed and was removed.
 *
 * Tailing is stopped and archived history is deleted before the view context is destroyed.
 */
auto LogViewerController::remove_view(const QUuid& view_id) -> bool
{
    bool removed = false;

    if (!view_id.isNull() && m_views->get_context(view_id) != nullptr)
    {
        cancel_loading(view_id);

        if (m_tailer_service != nullptr)
        {
            m_tailer_service->stop_tailing_view(view_id);
        }

        if (m_history_service != nullptr)
        {
            discard_queued_history(view_id);
            m_history_service->remove_view_entries(view_id);
        }

        m_live_tailing_views.remove(view_id);
        m_pending_tail_refresh_views.remove(view_id);
        m_failed_ingest_files.remove(view_id);
        m_file_parsing_profiles.remove(view_id);
        removed = m_views->remove_view(view_id);
    }

    return removed;
}

/**
 * @brief Removes every view and releases associated loading, tailing, and history resources.
 *
 * Each view is removed through remove_view() so lifecycle cleanup is identical for normal removal,
 * session closing, and application shutdown.
 */
auto LogViewerController::clear_all_views() -> void
{
    const QVector<QUuid> view_ids = m_views->get_all_view_ids();

    for (const QUuid& view_id: view_ids)
    {
        remove_view(view_id);
    }
}

/**
 * @brief Adds a single log file to the LogFileTreeModel.
 * @param file_path The path to the log file.
 */
auto LogViewerController::add_log_file_to_tree(const QString& file_path) -> void
{
    if (m_catalog != nullptr)
    {
        m_catalog->add_file(file_path);
    }
}

/**
 * @brief Adds multiple log files to the LogFileTreeModel.
 * @param file_paths The vector of log file paths.
 */
auto LogViewerController::add_log_files_to_tree(const QVector<QString>& file_paths) -> void
{
    if (m_catalog != nullptr)
    {
        m_catalog->add_files(file_paths);
    }
}

/**
 * @brief Adds a single log file to a specific session in the LogFileTreeModel.
 * @param session_id The session identifier.
 * @param file_path The path to the log file.
 */
auto LogViewerController::add_log_file_to_session(const QString& session_id,
                                                  const QString& file_path) -> void
{
    if (m_catalog != nullptr)
    {
        m_catalog->add_file_to_session(session_id, file_path);
    }
}

/**
 * @brief Adds multiple log files to a specific session in the LogFileTreeModel.
 * @param session_id The session identifier.
 * @param file_paths The vector of log file paths.
 */
auto LogViewerController::add_log_files_to_session(const QString& session_id,
                                                   const QVector<QString>& file_paths) -> void
{
    if (m_catalog != nullptr)
    {
        m_catalog->add_files_to_session(session_id, file_paths);
    }
}

/**
 * @brief Loads a single log file into a new view.
 * @param file_path Path to the log file.
 * @return Identifier of the created view.
 */
auto LogViewerController::load_log_file(const QString& file_path) -> QUuid
{
    return load_log_file(file_path, m_default_profile);
}

/**
 * @brief Loads a single log file into a new view with a selected parsing profile.
 * @param
 * file_path Path of the log file.
 * @param profile Parsing profile selected for this import.
 *
 * @return Identifier of the created view, or a null identifier when loading fails.
 */
auto LogViewerController::load_log_file(const QString& file_path,
                                        const LogParsingProfile& profile) -> QUuid
{
    QUuid view_id;

    const QFileInfo file_info(file_path);

    const bool can_load = file_info.exists() && file_info.isFile() && file_info.isReadable();

    if (can_load)
    {
        const QVector<LogEntry> entries = m_ingest->load_file_sync(file_path, profile);

        const QString app_name = !entries.isEmpty() ? entries.first().get_app_name()
                                                    : LogLoader::identify_app(file_path);

        const QUuid candidate_view_id = m_views->create_view();

        const bool entries_stored =
            entries.isEmpty() || m_history_service->add_entries(candidate_view_id, entries);

        if (entries_stored)
        {
            remember_file_profile(candidate_view_id, file_path, profile);
            m_views->set_loaded_files(candidate_view_id,
                                      QList<LogFileInfo>{LogFileInfo(file_path, app_name)});

            const LogQuery query = create_page_query(candidate_view_id);

            m_page_coordinator->set_query(candidate_view_id, query);

            m_live_tailing_views.insert(candidate_view_id);
            m_tailer_service->start_tailing(candidate_view_id, file_path, profile);

            view_id = candidate_view_id;
        }
        else
        {
            qWarning().nospace() << "[Controller] synchronous import failed for view="
                                 << candidate_view_id.toString() << " file=\""
                                 << file_info.absoluteFilePath() << '"';

            remove_view(candidate_view_id);
        }
    }
    else
    {
        qWarning().nospace() << "[Controller] synchronous import rejected file=\""
                             << file_info.absoluteFilePath() << '"';
    }

    return view_id;
}

/**
 * @brief Loads a single log file into an existing view.
 * @param view_id Target view identifier.
 * @param file_path Path to the log file.
 * @return True when the file was loaded.
 */
auto LogViewerController::load_log_file(const QUuid& view_id, const QString& file_path) -> bool
{
    return load_log_file(view_id, file_path, m_default_profile);
}

/**
 * @brief Loads a log file into an existing view with a selected parsing profile.
 * @param
 * view_id Target view identifier.
 * @param file_path Path of the log file.
 * @param profile
 * Parsing profile selected for this import.
 * @return True when the file was loaded and registered
 * successfully.
 */
auto LogViewerController::load_log_file(const QUuid& view_id, const QString& file_path,
                                        const LogParsingProfile& profile) -> bool
{
    bool loaded = false;

    ensure_view_models(view_id);

    LogViewContext* context = m_views->get_context(view_id);

    const QFileInfo file_info(file_path);

    const bool can_load = context != nullptr && !is_file_loaded(view_id, file_path) &&
                          file_info.exists() && file_info.isFile() && file_info.isReadable();

    if (can_load)
    {
        const QVector<LogEntry> entries = m_ingest->load_file_sync(file_path, profile);

        const QString app_name = !entries.isEmpty() ? entries.first().get_app_name()
                                                    : LogLoader::identify_app(file_path);

        const bool entries_stored =
            entries.isEmpty() || m_history_service->add_entries(view_id, entries);

        if (entries_stored)
        {
            remember_file_profile(view_id, file_path, profile);
            m_views->add_loaded_file(view_id, LogFileInfo(file_path, app_name));

            const LogPageState* page_state = m_page_coordinator->get_page_state(view_id);

            if (page_state != nullptr)
            {
                m_page_coordinator->reload(view_id);
            }
            else
            {
                const LogQuery query = create_page_query(view_id);

                m_page_coordinator->set_query(view_id, query);
            }

            if (get_live_tailing_enabled(view_id))
            {
                m_tailer_service->start_tailing(view_id, file_path, profile);
            }

            loaded = true;
        }
        else
        {
            qWarning().nospace() << "[Controller] synchronous import failed for view="
                                 << view_id.toString() << " file=\"" << file_info.absoluteFilePath()
                                 << '"';
        }
    }

    return loaded;
}

/**
 * @brief Loads multiple log files into a new view.
 * @param file_paths Paths to load.
 * @return Identifier of the created view, or a null identifier for an empty request.
 */
auto LogViewerController::load_log_files(const QVector<QString>& file_paths) -> QUuid
{
    return load_log_files(file_paths, m_default_profile);
}

/**
 * @brief Loads multiple log files into a new view with one selected profile.
 * @param
 * file_paths Paths of the log files.
 * @param profile Parsing profile selected for these imports.

 * * @return Identifier of the created view, or a null identifier when loading fails.
 */
auto LogViewerController::load_log_files(const QVector<QString>& file_paths,
                                         const LogParsingProfile& profile) -> QUuid
{
    QUuid view_id;
    bool valid_files = !file_paths.isEmpty();

    for (const QString& file_path: file_paths)
    {
        const QFileInfo file_info(file_path);

        const bool valid_file = file_info.exists() && file_info.isFile() && file_info.isReadable();

        if (!valid_file)
        {
            valid_files = false;

            qWarning().nospace() << "[Controller] synchronous import rejected file=\""
                                 << file_info.absoluteFilePath() << '"';
        }
    }

    if (valid_files)
    {
        const QUuid candidate_view_id = m_views->create_view();

        QList<LogFileInfo> loaded_files;
        bool entries_stored = true;

        for (qsizetype index = 0; index < file_paths.size() && entries_stored; ++index)
        {
            const QString& file_path = file_paths.at(index);

            const QVector<LogEntry> entries = m_ingest->load_file_sync(file_path, profile);

            const QString app_name = !entries.isEmpty() ? entries.first().get_app_name()
                                                        : LogLoader::identify_app(file_path);

            entries_stored =
                entries.isEmpty() || m_history_service->add_entries(candidate_view_id, entries);

            if (entries_stored)
            {
                remember_file_profile(candidate_view_id, file_path, profile);
                loaded_files.append(LogFileInfo(file_path, app_name));
            }
            else
            {
                qWarning().nospace() << "[Controller] synchronous import failed for view="
                                     << candidate_view_id.toString() << " file=\""
                                     << QFileInfo(file_path).absoluteFilePath() << '"';
            }
        }

        if (entries_stored)
        {
            m_views->set_loaded_files(candidate_view_id, loaded_files);

            const LogQuery query = create_page_query(candidate_view_id);

            m_page_coordinator->set_query(candidate_view_id, query);

            m_live_tailing_views.insert(candidate_view_id);

            for (const QString& file_path: file_paths)
            {
                m_tailer_service->start_tailing(candidate_view_id, file_path, profile);
            }

            view_id = candidate_view_id;
        }
        else
        {
            remove_view(candidate_view_id);
        }
    }

    return view_id;
}

/**
 * @brief Parses a bounded file sample without importing or registering the file.
 * @param
 * file_path Path of the file to preview.
 * @param profile Parsing profile to evaluate.
 * @param
 * maximum_record_count Maximum number of non-empty records returned.
 * @return Parse outcomes in
 * source order, including structured failures.
 */
auto LogViewerController::preview_log_file(
    const QString& file_path, const LogParsingProfile& profile,
    qsizetype maximum_record_count) const -> QVector<LogParseOutcome>
{
    return m_ingest->preview_file(file_path, profile, maximum_record_count);
}

/**
 * @brief Starts streaming load of a single log file and creates a new view (model/proxy).
 * @param file_path The path to the log file to stream.
 * @param batch_size Number of entries per batch appended to the model.
 * @return QUuid of the created view.
 */
auto LogViewerController::load_log_file_async(const QString& file_path,
                                              qsizetype batch_size) -> QUuid
{
    return load_log_file_async(file_path, m_default_profile, batch_size);
}

/**
 * @brief Streams a log file into a new view with a selected parsing profile.
 * @param
 * file_path Path of the log file.
 * @param profile Parsing profile selected for this import and
 * subsequent live tailing.
 * @param batch_size Number of entries per emitted batch.
 * @return
 * Identifier of the created view.
 */
auto LogViewerController::load_log_file_async(const QString& file_path,
                                              const LogParsingProfile& profile,
                                              qsizetype batch_size) -> QUuid
{
    QUuid view_id = m_views->create_view();
    set_live_tailing_enabled(view_id, true);

    const QString app_name = LogLoader::identify_app(file_path);
    LogFileInfo loaded_log_file(file_path, app_name);

    auto* ctx = m_views->get_context(view_id);
    if (ctx != nullptr)
    {
        m_views->set_loaded_files(view_id, QList<LogFileInfo>{loaded_log_file});
    }

    enqueue_async(view_id, file_path, profile);
    try_start_next_async(batch_size);

    return view_id;
}

/**
 * @brief Starts streaming load of a single log file into an existing view (model/proxy).
 * @param view_id The target view to load the file into.
 * @param file_path The path to the log file to stream.
 * @param batch_size Number of entries per batch appended to the model.
 * @return True if the file was enqueued; false if already present or view missing.
 */
auto LogViewerController::load_log_file_async(const QUuid& view_id, const QString& file_path,
                                              qsizetype batch_size) -> bool
{
    return load_log_file_async(view_id, file_path, m_default_profile, batch_size);
}

/**
 * @brief Streams a log file into an existing view with a selected parsing profile.
 * @param
 * view_id Target view identifier.
 * @param file_path Path of the log file.
 * @param profile
 * Parsing profile selected for this import and subsequent live tailing.
 * @param batch_size Number
 * of entries per emitted batch.
 * @return True when the file was enqueued successfully.
 */
auto LogViewerController::load_log_file_async(const QUuid& view_id, const QString& file_path,
                                              const LogParsingProfile& profile,
                                              qsizetype batch_size) -> bool
{
    bool success = false;

    ensure_view_models(view_id);

    if (!is_file_loaded(view_id, file_path))
    {
        const QString app_name = LogLoader::identify_app(file_path);
        LogFileInfo info(file_path, app_name);

        m_views->add_loaded_file(view_id, info);

        enqueue_async(view_id, file_path, profile);
        try_start_next_async(batch_size);

        success = true;
    }

    return success;
}

/**
 * @brief Starts streaming load of multiple log files into a single new view (model/proxy).
 * @param file_paths Paths to stream.
 * @param batch_size Number of entries per batch appended to the model.
 * @return QUuid of the created view.
 */
auto LogViewerController::load_log_files_async(const QVector<QString>& file_paths,
                                               qsizetype batch_size) -> QUuid
{
    return load_log_files_async(file_paths, m_default_profile, batch_size);
}

/**
 * @brief Streams multiple files into a new view with one selected parsing profile.
 * @param
 * file_paths Paths of the log files.
 * @param profile Parsing profile selected for these imports
 * and live tailing.
 * @param batch_size Number of entries per emitted batch.
 * @return Identifier
 * of the created view, or a null identifier for an empty request.
 */
auto LogViewerController::load_log_files_async(const QVector<QString>& file_paths,
                                               const LogParsingProfile& profile,
                                               qsizetype batch_size) -> QUuid
{
    QUuid view_id;

    if (!file_paths.isEmpty())
    {
        view_id = m_views->create_view();
        set_live_tailing_enabled(view_id, true);
        QList<LogFileInfo> files_info;

        for (const QString& file_path: file_paths)
        {
            const QString app_name = LogLoader::identify_app(file_path);
            files_info.append(LogFileInfo(file_path, app_name));
            enqueue_async(view_id, file_path, profile);
        }

        m_views->set_loaded_files(view_id, files_info);
        try_start_next_async(batch_size);
    }

    return view_id;
}

/**
 * @brief Cancels any ongoing streaming for the specified view and clears its pending queue.
 * @param view_id The view to cancel streaming for.
 */
auto LogViewerController::cancel_loading(const QUuid& view_id) -> void
{
    m_ingest->cancel_for_view(view_id);
}

/**
 * @brief Sets the application name filter for the current view.
 * @param app_name The application name to filter by.
 */
auto LogViewerController::set_app_name_filter(const QString& app_name) -> void
{
    set_app_name_filter(m_views->get_current_view(), app_name);
}

/**
 * @brief Sets the application name filter for the specified view.
 * @param view_id The QUuid of the view.
 * @param app_name The application name to filter by.
 */
auto LogViewerController::set_app_name_filter(const QUuid& view_id, const QString& app_name) -> void
{
    m_filters->set_app_name(view_id, app_name);
}

/**
 * @brief Sets the log level filter for the current view.
 * @param levels The set of log levels.
 */
auto LogViewerController::set_log_level_filters(const QSet<QString>& levels) -> void
{
    set_log_level_filters(m_views->get_current_view(), levels);
}

/**
 * @brief Sets the log level filter for the specified view.
 * @param view_id The QUuid of the view.
 * @param levels The set of log levels.
 */
auto LogViewerController::set_log_level_filters(const QUuid& view_id,
                                                const QSet<QString>& levels) -> void
{
    m_filters->set_log_levels(view_id, levels);
}

/**
 * @brief Sets the search filter for the current view.
 * @param search_text The search string or regex.
 * @param field The field to search in.
 * @param use_regex Whether to use regex.
 */
auto LogViewerController::set_search_filter(const QString& search_text, SearchField field,
                                            bool use_regex) -> void
{
    set_search_filter(m_views->get_current_view(), search_text, field, use_regex);
}

/**
 * @brief Sets the search filter for the specified view.
 * @param view_id The QUuid of the view.
 * @param search_text The search string or regex.
 * @param field The field to search in.
 * @param use_regex Whether to use regex.
 */
auto LogViewerController::set_search_filter(const QUuid& view_id, const QString& search_text,
                                            SearchField field, bool use_regex) -> void
{
    m_filters->set_search(view_id, search_text, field, use_regex);
}

/**
 * @brief Returns the LogModel for the current view.
 * @return Pointer to the LogModel.
 */
auto LogViewerController::get_log_model() -> LogModel*
{
    return get_log_model(m_views->get_current_view());
}

/**
 * @brief Returns the LogModel for the specified view.
 * @param view_id The QUuid of the view.
 * @return Pointer to the LogModel, or nullptr if not found.
 */
auto LogViewerController::get_log_model(const QUuid& view_id) -> LogModel*
{
    auto* ctx = m_views->get_context(view_id);
    LogModel* model = (ctx != nullptr) ? ctx->get_model() : nullptr;
    return model;
}

/**
 * @brief Assigns a paged log query to a view and loads its first page.
 * @param view_id Target view.
 * @param query Query describing filtering and sorting.
 * @return True when the query was assigned and loaded.
 */
auto LogViewerController::set_page_query(const QUuid& view_id, const LogQuery& query) -> bool
{
    bool loaded = false;

    if (m_page_coordinator != nullptr)
    {
        loaded = m_page_coordinator->set_query(view_id, query);
    }

    return loaded;
}

/**
 * @brief Selects and loads a page for a view.
 * @param view_id Target view.
 * @param page One-based page number.
 * @return True when the page was loaded.
 */
auto LogViewerController::set_current_page(const QUuid& view_id, qsizetype page) -> bool
{
    bool loaded = false;

    if (m_page_coordinator != nullptr)
    {
        loaded = m_page_coordinator->set_current_page(view_id, page);
    }

    return loaded;
}

/**
 * @brief Changes the page size and loads the first page.
 * @param view_id Target view.
 * @param page_size Positive number of entries per page.
 * @return True when the page size was applied.
 */
auto LogViewerController::set_page_size(const QUuid& view_id, qsizetype page_size) -> bool
{
    bool loaded = false;

    if (m_page_coordinator != nullptr)
    {
        loaded = m_page_coordinator->set_page_size(view_id, page_size);
    }

    return loaded;
}

/**
 * @brief Changes the sorting of a paged view and loads its first page.
 * @param view_id Target view.
 * @param column LogModel column.
 * @param order Sort direction.
 * @return True when the sorted page was loaded.
 */
auto LogViewerController::set_page_sort(const QUuid& view_id, int column,
                                        Qt::SortOrder order) -> bool
{
    bool sorted = false;
    LogQuery query = create_page_query(view_id);
    LogModel* model = get_log_model(view_id);

    if (model != nullptr && model->is_column_sortable(column))
    {
        query.sort_field = model->get_column_field_id(column);
        query.sort_order = order;
        sorted = set_page_query(view_id, query);
    }

    return sorted;
}

/**
 * @brief Reloads the current page for a view.
 * @param view_id Target view.
 * @return True when the page was loaded.
 */
auto LogViewerController::reload_page(const QUuid& view_id) -> bool
{
    bool loaded = false;

    if (m_page_coordinator != nullptr)
    {
        loaded = m_page_coordinator->reload(view_id);
    }

    return loaded;
}

/**
 * @brief Returns the paged query state for a view.
 * @param view_id Target view.
 * @return Page state, or nullptr when no query is assigned.
 */
auto LogViewerController::get_page_state(const QUuid& view_id) const -> const LogPageState*
{
    const LogPageState* state = nullptr;

    if (m_page_coordinator != nullptr)
    {
        state = m_page_coordinator->get_page_state(view_id);
    }

    return state;
}

/**
 * @brief Creates a log query from the filter and sorting state of a view.
 * @param view_id Source view.
 * @return Query representing the view state.
 */
auto LogViewerController::create_page_query(const QUuid& view_id) const -> LogQuery
{
    LogQuery query;
    LogViewContext* context = get_view_context(view_id);

    if (context != nullptr && m_filters != nullptr)
    {
        const FilterState filters = m_filters->export_filters(view_id);

        query.view_id = view_id;
        query.app_name = filters.app_name;
        query.log_levels = filters.log_levels;
        query.search_text = filters.search_text;
        query.search_fields = get_query_search_fields(filters.search_field);
        query.use_regex = filters.use_regex;
        query.show_only_file = filters.show_only_file;
        query.hidden_files = filters.hidden_files;

        const LogPageState* page_state = get_page_state(view_id);

        if (page_state != nullptr)
        {
            query.sort_field = page_state->get_query().sort_field;
            query.sort_order = page_state->get_query().sort_order;
        }
    }

    return query;
}

/**
 * @brief Returns the set of unique application names from the current view.
 * @return A set of application names.
 */
auto LogViewerController::get_app_names() const -> QSet<QString>
{
    return get_app_names(m_views->get_current_view());
}

/**
 * @brief Returns the set of unique application names from the specified view.
 * @param view_id The QUuid of the view.
 * @return A set of application names.
 */
auto LogViewerController::get_app_names(const QUuid& view_id) const -> QSet<QString>
{
    QSet<QString> app_names;

    if (m_history_service != nullptr && !view_id.isNull())
    {
        app_names = m_history_service->get_distinct_values(view_id, LogField::AppName);
    }

    return app_names;
}

/**
 * @brief Returns the current application name filter.
 * @return The application name filter string.
 */
auto LogViewerController::get_app_name_filter() const -> QString
{
    return get_app_name_filter(m_views->get_current_view());
}

/**
 * @brief Returns the application name filter for the specified view.
 * @param view_id The QUuid of the view.
 * @return The application name filter string.
 */
auto LogViewerController::get_app_name_filter(const QUuid& view_id) const -> QString
{
    QString filter = m_filters->get_app_name(view_id);
    return filter;
}

/**
 * @brief Returns the available log levels for the specified view.
 * @param view_id The QUuid of the view.
 * @return Vector of log level names (same list for all views).
 */
auto LogViewerController::get_available_log_levels(const QUuid& view_id) const -> QVector<QString>
{
    QVector<QString> log_levels = FilterCoordinator::get_available_log_levels();
    Q_UNUSED(view_id);
    return log_levels;
}

/**
 * @brief Returns the current set of log levels being filtered.
 * @return The set of log levels.
 */
auto LogViewerController::get_log_level_filters() const -> QSet<QString>
{
    return get_log_level_filters(m_views->get_current_view());
}

/**
 * @brief Returns a map of log level names to their counts in the specified view.
 * @param view_id The QUuid of the view.
 * @return QMap of log level names to counts.
 */
auto LogViewerController::get_log_level_counts(const QUuid& view_id) const -> QMap<QString, int>
{
    QMap<QString, int> counts;

    if (!view_id.isNull() && m_history_service != nullptr)
    {
        const LogQuery query = create_page_query(view_id);

        const QMap<QString, qsizetype> history_counts =
            m_history_service->get_log_level_counts(query);

        for (auto it = history_counts.cbegin(); it != history_counts.cend(); ++it)
        {
            counts.insert(it.key(), static_cast<int>(it.value()));
        }
    }

    return counts;
}

/**
 * @brief Returns a map of log level names to their counts in the current view.
 * @return QMap of log level names to counts.
 */
auto LogViewerController::get_log_level_counts() const -> QMap<QString, int>
{
    QMap<QString, int> counts = get_log_level_counts(m_views->get_current_view());
    return counts;
}

/**
 * @brief Returns the set of log levels being filtered for the specified view.
 * @param view_id The QUuid of the view.
 * @return The set of log levels.
 */
auto LogViewerController::get_log_level_filters(const QUuid& view_id) const -> QSet<QString>
{
    QSet<QString> levels = m_filters->get_log_levels(view_id);
    return levels;
}

/**
 * @brief Returns the current search text.
 * @return The search text string.
 */
auto LogViewerController::get_search_text() const -> QString
{
    return get_search_text(m_views->get_current_view());
}

/**
 * @brief Returns the search text for the specified view.
 * @param view_id The QUuid of the view.
 * @return The search text string.
 */
auto LogViewerController::get_search_text(const QUuid& view_id) const -> QString
{
    QString text = m_filters->get_search_text(view_id);
    return text;
}

/**
 * @brief Returns the current search field.
 * @return The search field.
 */
auto LogViewerController::get_search_field() const -> SearchField
{
    SearchField field = get_search_field(m_views->get_current_view());
    return field;
}

/**
 * @brief Returns the search field for the specified view.
 * @param view_id The QUuid of the view.
 * @return The search field.
 */
auto LogViewerController::get_search_field(const QUuid& view_id) const -> SearchField
{
    SearchField field = m_filters->get_search_field(view_id);
    return field;
}

/**
 * @brief Returns whether the search text is treated as a regex.
 * @return True if using regex, false if plain text.
 */
auto LogViewerController::is_search_regex() const -> bool
{
    return is_search_regex(m_views->get_current_view());
}

/**
 * @brief Returns whether the search text is treated as a regex for the specified view.
 * @param view_id The QUuid of the view.
 * @return True if using regex, false if plain text.
 */
auto LogViewerController::is_search_regex(const QUuid& view_id) const -> bool
{
    bool regex = m_filters->is_search_regex(view_id);
    return regex;
}

/**
 * @brief Returns the LogFileTreeModel.
 *
 * This model provides a hierarchical view of log files and their applications.
 * @return Pointer to the LogFileTreeModel.
 */
auto LogViewerController::get_log_file_tree_model() -> LogFileTreeModel*
{
    LogFileTreeModel* model = (m_catalog != nullptr) ? m_catalog->get_model() : nullptr;
    return model;
}

/**
 * @brief Returns the entries held by the current view's page model.
 * @return Entries of the currently loaded database page.
 */
auto LogViewerController::get_page_entries() const -> QVector<LogEntry>
{
    return get_page_entries(m_views->get_current_view());
}

/**
 * @brief Returns the entries held by a view's page model.
 * @param view_id Target view.
 * @return Entries of the currently loaded database page.
 */
auto LogViewerController::get_page_entries(const QUuid& view_id) const -> QVector<LogEntry>
{
    QVector<LogEntry> entries;

    if (m_views != nullptr)
    {
        entries = m_views->get_entries(view_id);
    }

    return entries;
}

/**
 * @brief Returns entries for one file from the current page.
 * @param file_info File whose visible page entries are requested.
 * @return Matching entries from the currently loaded page.
 */
auto LogViewerController::get_page_entries_for_file(const LogFileInfo& file_info) const
    -> QVector<LogEntry>
{
    return get_page_entries_for_file(m_views->get_current_view(), file_info);
}

/**
 * @brief Returns entries for one file from a view's current page.
 * @param view_id Target view.
 * @param file_info File whose visible page entries are requested.
 * @return Matching entries from the currently loaded page.
 */
auto LogViewerController::get_page_entries_for_file(
    const QUuid& view_id, const LogFileInfo& file_info) const -> QVector<LogEntry>
{
    QVector<LogEntry> matching_entries;

    const QVector<LogEntry> page_entries = get_page_entries(view_id);

    for (const LogEntry& entry: page_entries)
    {
        const bool belongs_to_file =
            entry.get_file_info().get_file_path() == file_info.get_file_path();

        if (belongs_to_file)
        {
            matching_entries.append(entry);
        }
    }

    return matching_entries;
}

/**
 * @brief Checks if a log file with the given file path is already loaded.
 * @param file_path The file path to check.
 * @return True if the file is already loaded, false otherwise.
 */
auto LogViewerController::is_file_loaded(const QString& file_path) const -> bool
{
    bool found = false;

    QVector<QUuid> ids = m_views->get_all_view_ids();
    for (const QUuid& id: ids)
    {
        const auto paths = m_views->get_file_paths(id);
        const auto match = std::find_if(paths.begin(), paths.end(),
                                        [&file_path](const QString& p) { return p == file_path; });
        if (match != paths.end())
        {
            found = true;
        }
    }

    return found;
}

/**
 * @brief Checks if a log file with the given file path is loaded in the specified view.
 * @param view_id The QUuid of the view.
 * @param file_path The file path to check.
 * @return True if the file is loaded in the view, false otherwise.
 */
auto LogViewerController::is_file_loaded(const QUuid& view_id,
                                         const QString& file_path) const -> bool
{
    bool found = false;

    const auto paths = m_views->get_file_paths(view_id);
    const auto it = std::find_if(paths.begin(), paths.end(),
                                 [&file_path](const QString& p) { return p == file_path; });
    if (it != paths.end())
    {
        found = true;
    }

    return found;
}

/**
 * @brief Applies a "show only file" filter for the specified view.
 * @param view_id Target view id.
 * @param file_path File path to show exclusively, or empty to reset.
 */
auto LogViewerController::set_show_only_file(const QUuid& view_id, const QString& file_path) -> void
{
    m_filters->set_show_only(view_id, file_path);
}

/**
 * @brief Toggles a file's visibility (hide/show) in the specified view.
 * @param view_id Target view id.
 * @param file_path Absolute file path to toggle.
 */
auto LogViewerController::toggle_file_visibility(const QUuid& view_id,
                                                 const QString& file_path) -> void
{
    m_filters->toggle_visibility(view_id, file_path);
}

/**
 * @brief Hides (excludes) a specific file in the specified view.
 * @param view_id Target view id.
 * @param file_path File path to hide.
 */
auto LogViewerController::hide_file(const QUuid& view_id, const QString& file_path) -> void
{
    m_filters->hide_file(view_id, file_path);
}

/**
 * @brief Returns absolute file paths loaded in the specified view.
 * @param view_id The QUuid of the view.
 * @return Vector of file paths loaded in the view (empty if none).
 */
auto LogViewerController::get_view_file_paths(const QUuid& view_id) const -> QVector<QString>
{
    QVector<QString> result = m_views->get_file_paths(view_id);
    return result;
}

/**
 * @brief Exports serializable state for a view.
 * @param view_id Target view identifier.
 * @return Session snapshot including the per-view live-tail setting.
 */
auto LogViewerController::export_view_state(const QUuid& view_id) const -> SessionViewState
{
    SessionViewState state;

    if (!view_id.isNull())
    {
        state = m_views->export_view_state(view_id, *m_filters);
        state.filters.live_tailing_enabled = get_live_tailing_enabled(view_id);

        const auto view_profiles = m_file_parsing_profiles.constFind(view_id);

        if (view_profiles != m_file_parsing_profiles.cend())
        {
            for (const LogFileInfo& file_info: state.loaded_files)
            {
                const QString file_path = QFileInfo(file_info.get_file_path()).absoluteFilePath();
                const auto profile = view_profiles->constFind(file_path);

                if (profile != view_profiles->cend())
                {
                    state.file_parsing_profile_ids.insert(file_path, profile->get_id());
                }
            }
        }

        const LogPageState* page_state = get_page_state(view_id);

        if (page_state != nullptr)
        {
            state.page_size = static_cast<int>(page_state->get_page_size());

            state.current_page = static_cast<int>(page_state->get_current_page() - 1);

            state.sort_column = get_model_sort_column(page_state->get_query().sort_field);

            state.sort_order = page_state->get_query().sort_order;
        }
    }

    return state;
}

/**
 * @brief Imports a single view state (files, filters, paging, sort) and returns the ensured
 * view id.
 * @param state The view state to apply.
 * @return QUuid of the imported/ensured view.
 */
auto LogViewerController::import_view_state(const SessionViewState& state) -> QUuid
{
    QUuid result;

    if (m_views != nullptr && m_filters != nullptr)
    {
        result = m_views->import_view_state(state, *m_filters);

        if (!result.isNull())
        {
            cancel_loading(result);

            m_tailer_service->stop_tailing_view(result);

            m_history_service->remove_view_entries(result);

            if (state.filters.live_tailing_enabled)
            {
                m_live_tailing_views.insert(result);
            }
            else
            {
                m_live_tailing_views.remove(result);
            }

            LogQuery query = create_page_query(result);
            query.sort_field = get_query_sort_field(state.sort_column);
            query.sort_order = state.sort_order;

            set_page_query(result, query);

            if (state.page_size > 0)
            {
                set_page_size(result, state.page_size);
            }

            set_current_page(result, state.current_page + 1);
        }

        // Update explorer tree
        if (m_catalog != nullptr && !state.loaded_files.isEmpty())
        {
            QVector<QString> paths;
            paths.reserve(state.loaded_files.size());
            for (const auto& lf: state.loaded_files)
            {
                const QString p = lf.get_file_path();
                if (!p.isEmpty())
                {
                    paths.append(p);
                }
            }

            if (!paths.isEmpty())
            {
                add_log_files_to_tree(paths);
            }
        }

        if (!result.isNull())
        {
            for (const auto& lf: state.loaded_files)
            {
                const QString path = lf.get_file_path();
                if (!path.isEmpty())
                {
                    ensure_view_models(result);
                    enqueue_async(result, path);
                    try_start_next_async(1000);
                }
            }
        }
    }

    return result;
}

/**
 * @brief Imports a single view state for a specific session.
 * @param session_id The session identifier for the tree model.
 * @param state The view state to apply.
 * @return QUuid of the imported/ensured view.
 */
auto LogViewerController::import_view_state_for_session(
    const QString& session_id, const SessionViewState& state,
    const QVector<LogParsingProfile>& available_profiles) -> QUuid
{
    QUuid result;

    if (m_views != nullptr && m_filters != nullptr)
    {
        result = m_views->import_view_state(state, *m_filters);

        if (!result.isNull())
        {
            cancel_loading(result);

            m_tailer_service->stop_tailing_view(result);

            m_history_service->remove_view_entries(result);

            if (state.filters.live_tailing_enabled)
            {
                m_live_tailing_views.insert(result);
            }
            else
            {
                m_live_tailing_views.remove(result);
            }
        }

        // Update explorer tree with session context
        if (m_catalog != nullptr && !state.loaded_files.isEmpty() && !session_id.isEmpty())
        {
            QVector<QString> paths;
            paths.reserve(state.loaded_files.size());
            for (const auto& lf: state.loaded_files)
            {
                const QString p = lf.get_file_path();
                if (!p.isEmpty())
                {
                    paths.append(p);
                }
            }

            if (!paths.isEmpty())
            {
                add_log_files_to_session(session_id, paths);
            }
        }

        if (!result.isNull())
        {
            for (const auto& lf: state.loaded_files)
            {
                const QString path = lf.get_file_path();
                if (!path.isEmpty())
                {
                    const LogParsingProfile profile = get_session_file_profile(
                        state, path, available_profiles, m_default_profile);
                    ensure_view_models(result);
                    enqueue_async(result, path, profile);
                    try_start_next_async(1000);
                }
            }
        }
    }

    return result;
}

/**
 * @brief Enables or disables live tailing for a view.
 * @param view_id Target view.
 * @param enabled True to start tailing loaded files.
 */
auto LogViewerController::set_live_tailing_enabled(const QUuid& view_id, bool enabled) -> void
{
    if (enabled)
    {
        m_live_tailing_views.insert(view_id);

        const QVector<QString> file_paths = get_view_file_paths(view_id);

        for (const QString& file_path: file_paths)
        {
            m_tailer_service->start_tailing(view_id, file_path,
                                            get_file_profile(view_id, file_path));
        }
    }
    else
    {
        m_live_tailing_views.remove(view_id);
        m_tailer_service->stop_tailing_view(view_id);
    }
}

/**
 * @brief Returns whether live tailing is enabled for a view.
 * @param view_id Target view.
 * @return True when enabled.
 */
auto LogViewerController::get_live_tailing_enabled(const QUuid& view_id) const -> bool
{
    const bool enabled = m_live_tailing_views.contains(view_id);
    return enabled;
}

/**
 * @brief Searches every SQLite-archived entry for a view.
 * @param view_id Target view.
 * @param search_text Plain-text FTS query.
 * @param search_field Field to search.
 * @param limit Maximum result count.
 * @return Matching archive entries.
 */
auto LogViewerController::search_history(const QUuid& view_id, const QString& search_text,
                                         SearchField search_field,
                                         int limit) const -> QVector<LogEntry>
{
    QVector<LogEntry> entries;

    if (m_history_service != nullptr)
    {
        entries = m_history_service->search_entries(view_id, search_text, search_field, limit);
    }

    return entries;
}

/**
 * @brief Removes a file from every view and from the file tree.
 * @param file File metadata identifying the path to remove.
 *
 * Tailing is stopped and the corresponding archived entries are deleted before the file is
 * removed from each view model.
 */
auto LogViewerController::remove_log_file(const LogFileInfo& file) -> void
{
    QList<QUuid> views_to_remove;
    const QString file_path = file.get_file_path();
    const QVector<QUuid> view_ids = m_views->get_all_view_ids();

    for (const QUuid& view_id: view_ids)
    {
        LogViewContext* context = m_views->get_context(view_id);

        const bool file_is_loaded = context != nullptr && is_file_loaded(view_id, file_path);

        if (file_is_loaded)
        {
            m_tailer_service->stop_tailing(view_id, file_path);

            discard_queued_history(view_id, file_path);
            m_history_service->remove_file_entries(view_id, file_path);

            QList<LogFileInfo> files = context->get_loaded_files();

            files.erase(std::remove_if(files.begin(), files.end(),
                                       [&file_path](const LogFileInfo& info) {
                                           return info.get_file_path() == file_path;
                                       }),
                        files.end());

            context->set_loaded_files(files);
            context->remove_entries_by_file_path(file_path);

            m_filters->adjust_visibility_on_file_removed(view_id, file_path);

            forget_file_profile(view_id, file_path);

            const bool view_became_empty = files.isEmpty();

            if (view_became_empty)
            {
                views_to_remove.append(view_id);
            }
            else if (m_page_coordinator->get_page_state(view_id) != nullptr)
            {
                const LogQuery query = create_page_query(view_id);

                m_page_coordinator->set_query(view_id, query);
            }

            emit view_file_paths_changed(view_id, context->get_file_paths());
        }
    }

    if (m_catalog != nullptr)
    {
        m_catalog->remove_file(file);
    }

    for (const QUuid& view_id: views_to_remove)
    {
        remove_view(view_id);
    }
}

/**
 * @brief Removes a file from one view.
 * @param view_id Target view identifier.
 * @param file_path Absolute file path to remove.
 *
 * Tailing is stopped and file-specific archive history is deleted before the live model is
 * changed. An empty view is then removed through remove_view().
 */
auto LogViewerController::remove_log_file(const QUuid& view_id, const QString& file_path) -> void
{
    const bool can_remove =
        !view_id.isNull() && !file_path.isEmpty() && is_file_loaded(view_id, file_path);

    bool view_became_empty = false;

    if (can_remove)
    {
        m_tailer_service->stop_tailing(view_id, file_path);

        discard_queued_history(view_id, file_path);
        m_history_service->remove_file_entries(view_id, file_path);

        m_views->remove_entries_by_file(view_id, file_path);

        m_filters->adjust_visibility_on_file_removed(view_id, file_path);

        forget_file_profile(view_id, file_path);

        view_became_empty = get_view_file_paths(view_id).isEmpty();

        if (!view_became_empty && m_page_coordinator->get_page_state(view_id) != nullptr)
        {
            const LogQuery query = create_page_query(view_id);

            m_page_coordinator->set_query(view_id, query);
        }
    }

    if (can_remove && view_became_empty)
    {
        remove_view(view_id);
    }
}

/**
 * @brief Queues one parsed batch for storage on the history writer thread.
 * @param view_id
 * View that owns the imported entries.
 * @param file_path Imported source file.
 * @param entries
 * Parsed entries to store.
 */
auto LogViewerController::queue_history_batch(const QUuid& view_id, const QString& file_path,
                                              const QVector<LogEntry>& entries) -> void
{
    if (m_history_writer != nullptr && m_history_writer_thread != nullptr &&
        m_history_writer_thread->isRunning())
    {
        LogHistoryWriter* writer = m_history_writer;
        QMetaObject::invokeMethod(
            writer,
            [writer, view_id, file_path, entries]() {
                writer->store_batch(view_id, file_path, entries);
            },
            Qt::QueuedConnection);
    }
}

/**
 * @brief Queues an import completion marker behind all preceding history batches.
 * @param
 * view_id View that owns the import.
 * @param file_path Imported source file.
 */
auto LogViewerController::queue_history_finish(const QUuid& view_id,
                                               const QString& file_path) -> void
{
    if (m_history_writer != nullptr && m_history_writer_thread != nullptr &&
        m_history_writer_thread->isRunning())
    {
        LogHistoryWriter* writer = m_history_writer;
        QMetaObject::invokeMethod(
            writer, [writer, view_id, file_path]() { writer->finish_import(view_id, file_path); },
            Qt::QueuedConnection);
    }
}

/**
 * @brief Handles completion of all queued history writes for one import.
 * @param view_id View
 * that owns the import.
 * @param file_path Imported source file.
 * @param succeeded True when
 * every history batch was committed.
 * @param error_message Storage error for a failed import.
 */
auto LogViewerController::handle_history_write_finished(const QUuid& view_id,
                                                        const QString& file_path, bool succeeded,
                                                        const QString& error_message) -> void
{
    const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
    const bool ingest_failed = m_failed_ingest_files.value(view_id).contains(absolute_file_path);
    const bool import_succeeded = succeeded && !ingest_failed;
    const bool view_exists = !view_id.isNull() && m_views->get_context(view_id) != nullptr;

    m_failed_ingest_files[view_id].remove(absolute_file_path);

    if (m_failed_ingest_files.value(view_id).isEmpty())
    {
        m_failed_ingest_files.remove(view_id);
    }

    if (!m_is_shutting_down)
    {
        qDebug().nospace() << "[Controller] history finished view=" << view_id.toString()
                           << " file=\"" << absolute_file_path << "\" success=" << import_succeeded;

        if (!succeeded && view_exists)
        {
            emit loading_error(view_id, absolute_file_path, error_message);
        }

        if (!import_succeeded)
        {
            const bool file_is_registered =
                view_exists && is_file_loaded(view_id, absolute_file_path);

            if (file_is_registered)
            {
                remove_log_file(view_id, absolute_file_path);
            }
        }
        else if (view_exists)
        {
            emit loading_finished(view_id, absolute_file_path);

            if (m_live_tailing_views.contains(view_id))
            {
                m_tailer_service->start_tailing(view_id, absolute_file_path,
                                                get_file_profile(view_id, absolute_file_path));
            }
        }
    }
}

/**
 * @brief Queues cleanup for entries that reached the writer before a view was removed.
 *
 * @param view_id Removed view.
 */
auto LogViewerController::discard_queued_history(const QUuid& view_id) -> void
{
    if (m_history_writer != nullptr && m_history_writer_thread != nullptr &&
        m_history_writer_thread->isRunning())
    {
        LogHistoryWriter* writer = m_history_writer;
        QMetaObject::invokeMethod(
            writer, [writer, view_id]() { writer->discard_view(view_id); }, Qt::QueuedConnection);
    }
}

/**
 * @brief Queues cleanup for entries that reached the writer before a file was removed.
 *
 * @param view_id View that owned the file.
 * @param file_path Removed source file.
 */
auto LogViewerController::discard_queued_history(const QUuid& view_id,
                                                 const QString& file_path) -> void
{
    if (m_history_writer != nullptr && m_history_writer_thread != nullptr &&
        m_history_writer_thread->isRunning())
    {
        LogHistoryWriter* writer = m_history_writer;
        QMetaObject::invokeMethod(
            writer, [writer, view_id, file_path]() { writer->discard_file(view_id, file_path); },
            Qt::QueuedConnection);
    }
}

/**
 * @brief Enqueues an asynchronous load request for a log file.
 * @param view_id The QUuid of
 * the view to load into.
 * @param file_path The path to the log file.
 */
auto LogViewerController::enqueue_async(const QUuid& view_id, const QString& file_path) -> void
{
    enqueue_async(view_id, file_path, m_default_profile);
}

/**
 * @brief Enqueues an asynchronous load with an explicitly selected profile.
 * @param view_id
 * Target view identifier.
 * @param file_path Path of the log file.
 * @param profile Parsing
 * profile used by this request.
 */
auto LogViewerController::enqueue_async(const QUuid& view_id, const QString& file_path,
                                        const LogParsingProfile& profile) -> void
{
    remember_file_profile(view_id, file_path, profile);
    m_ingest->enqueue_stream(view_id, file_path, profile);
}

/**
 * @brief Retains the selected parsing profile for later live-tailing restarts.
 * @param
 * view_id View containing the imported file.
 * @param file_path Imported file path.
 * @param
 * profile Parsing profile selected for the file.
 */
auto LogViewerController::remember_file_profile(const QUuid& view_id, const QString& file_path,
                                                const LogParsingProfile& profile) -> void
{
    if (!view_id.isNull() && !file_path.isEmpty())
    {
        const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
        const bool first_profile = !m_file_parsing_profiles.contains(view_id) ||
                                   m_file_parsing_profiles.value(view_id).isEmpty();
        m_file_parsing_profiles[view_id].insert(absolute_file_path, profile);

        LogModel* model = get_log_model(view_id);
        if (model != nullptr)
        {
            const QVector<LogFieldDefinition> columns = get_profile_columns(profile);

            if (first_profile)
            {
                model->set_columns(columns);
            }
            else
            {
                model->append_columns(columns);
            }
        }
    }
}

/**
 * @brief Removes a retained profile when its file registration is removed.
 * @param view_id
 * View that contained the file.
 * @param file_path Removed file path.
 */
auto LogViewerController::forget_file_profile(const QUuid& view_id,
                                              const QString& file_path) -> void
{
    auto view_iterator = m_file_parsing_profiles.find(view_id);

    if (view_iterator != m_file_parsing_profiles.end())
    {
        view_iterator->remove(QFileInfo(file_path).absoluteFilePath());

        if (view_iterator->isEmpty())
        {
            m_file_parsing_profiles.erase(view_iterator);
        }
    }

    LogViewContext* context = get_view_context(view_id);
    LogModel* model = get_log_model(view_id);

    if (context != nullptr && model != nullptr && !context->get_loaded_files().isEmpty())
    {
        QVector<LogFieldDefinition> columns;

        for (const LogFileInfo& file_info: context->get_loaded_files())
        {
            const QVector<LogFieldDefinition> file_columns =
                get_profile_columns(get_file_profile(view_id, file_info.get_file_path()));

            for (const LogFieldDefinition& file_column: file_columns)
            {
                bool already_added = false;

                for (const LogFieldDefinition& column: columns)
                {
                    if (column.id == file_column.id)
                    {
                        already_added = true;
                    }
                }

                if (!already_added)
                {
                    columns.append(file_column);
                }
            }
        }

        model->set_columns(columns);
    }
}

/**
 * @brief Returns the profile retained for a view/file registration.
 * @param view_id View
 * containing the file.
 * @param file_path Registered file path.
 * @return Retained profile, or
 * the controller default when none was recorded.
 */
auto LogViewerController::get_file_profile(const QUuid& view_id,
                                           const QString& file_path) const -> LogParsingProfile
{
    LogParsingProfile profile = m_default_profile;
    const auto view_iterator = m_file_parsing_profiles.constFind(view_id);

    if (view_iterator != m_file_parsing_profiles.cend())
    {
        const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
        const auto profile_iterator = view_iterator->constFind(absolute_file_path);

        if (profile_iterator != view_iterator->cend())
        {
            profile = profile_iterator.value();
        }
    }

    return profile;
}

/**
 * @brief Attempts to start the next asynchronous load if none is active.
 * @param batch_size Number of entries per batch.
 */
auto LogViewerController::try_start_next_async(qsizetype batch_size) -> void
{
    m_ingest->start_next_if_idle(batch_size);

    const QUuid active_view_id = m_ingest->get_active_view_id();
    if (!active_view_id.isNull() && (m_views->get_current_view() != active_view_id))
    {
        m_views->set_current_view(active_view_id);
    }
}

/**
 * @brief Clears pending async requests for a specific view.
 * @param view_id The QUuid of the view.
 */
auto LogViewerController::clear_pending_for_view(const QUuid& view_id) -> void
{
    m_ingest->cancel_for_view(view_id);
}

/**
 * @brief Ensures that models and proxies exist for the specified view ID.
 * @param view_id The QUuid of the view.
 */
auto LogViewerController::ensure_view_models(const QUuid& view_id) -> void
{
    m_views->ensure_view(view_id);
}

/**
 * @brief Returns the context for a view or nullptr if not present.
 * @param view_id The QUuid of the view.
 * @return Pointer to LogViewContext.
 */
auto LogViewerController::get_view_context(const QUuid& view_id) const -> LogViewContext*
{
    LogViewContext* ctx = m_views->get_context(view_id);
    return ctx;
}
