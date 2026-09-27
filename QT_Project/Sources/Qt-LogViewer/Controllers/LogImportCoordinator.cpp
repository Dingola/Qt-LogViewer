/**
 * @file LogImportCoordinator.cpp
 * @brief Implements synchronous and asynchronous log-import coordination.
 */

#include "Qt-LogViewer/Controllers/LogImportCoordinator.h"

#include <QDebug>
#include <QFileInfo>
#include <optional>

#include "Qt-LogViewer/Controllers/LiveTailingCoordinator.h"
#include "Qt-LogViewer/Controllers/LogIngestController.h"
#include "Qt-LogViewer/Controllers/LogPageCoordinator.h"
#include "Qt-LogViewer/Controllers/LogQueryController.h"
#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Services/HistoryWriteService.h"
#include "Qt-LogViewer/Services/LogHistoryService.h"
#include "Qt-LogViewer/Services/LogLoader.h"

/**
 * @brief Constructs an import coordinator from its workflow collaborators.
 * @param default_profile Profile used by compatibility overloads.
 * @param ingest Parser and asynchronous load queue.
 * @param views Registry receiving file registrations.
 * @param history Synchronous history storage.
 * @param history_writer Ordered asynchronous history writer.
 * @param pages Page state used to distinguish initial loads from refreshes.
 * @param queries Query component used to load bounded visible pages.
 * @param live_tailing Live-tail registrations started after successful imports.
 * @param parent Optional QObject parent.
 */
LogImportCoordinator::LogImportCoordinator(const LogParsingProfile& default_profile,
                                           LogIngestController* ingest, ViewRegistry* views,
                                           LogHistoryService* history,
                                           HistoryWriteService* history_writer,
                                           LogPageCoordinator* pages, LogQueryController* queries,
                                           LiveTailingCoordinator* live_tailing, QObject* parent)
    : QObject(parent),
      m_default_profile(default_profile),
      m_ingest(ingest),
      m_views(views),
      m_history(history),
      m_history_writer(history_writer),
      m_pages(pages),
      m_queries(queries),
      m_live_tailing(live_tailing)
{
    connect_workflow();
}

/**
 * @brief Imports one file synchronously into a new view.
 * @param file_path File to import.
 * @param profile Parsing profile selected for the file.
 * @return Created view id, or a null id when validation or storage fails.
 */
auto LogImportCoordinator::import_file(const QString& file_path,
                                       const LogParsingProfile& profile) -> QUuid
{
    QUuid view_id;

    if (is_readable_file(file_path) && m_ingest != nullptr && m_views != nullptr &&
        m_history != nullptr)
    {
        const QVector<LogEntry> entries = m_ingest->load_file_sync(file_path, profile);
        const QString app_name = !entries.isEmpty() ? entries.first().get_app_name()
                                                    : LogLoader::identify_app(file_path);
        const QUuid candidate_view_id = m_views->create_view();
        const bool stored = entries.isEmpty() || m_history->add_entries(candidate_view_id, entries);

        if (stored)
        {
            remember_profile(candidate_view_id, file_path, profile);
            m_views->set_loaded_files(candidate_view_id,
                                      QList<LogFileInfo>{LogFileInfo(file_path, app_name)});
            refresh_visible_page(candidate_view_id);

            if (m_live_tailing != nullptr)
            {
                m_live_tailing->set_enabled(candidate_view_id, true);
            }

            view_id = candidate_view_id;
        }
        else
        {
            emit view_removal_requested(candidate_view_id);
        }
    }
    else
    {
        qWarning().nospace() << "[Import] synchronous import rejected file=\""
                             << QFileInfo(file_path).absoluteFilePath() << '"';
    }

    return view_id;
}

/**
 * @brief Imports one file synchronously into an existing view.
 * @param view_id Target view.
 * @param file_path File to import.
 * @param profile Parsing profile selected for the file.
 * @return True when parsing, storage, registration, and page refresh succeed.
 */
auto LogImportCoordinator::import_file(const QUuid& view_id, const QString& file_path,
                                       const LogParsingProfile& profile) -> bool
{
    bool imported = false;

    if (m_views != nullptr)
    {
        m_views->ensure_view(view_id);
    }

    const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
    const bool can_import = m_views != nullptr && m_views->get_context(view_id) != nullptr &&
                            !m_views->get_file_paths(view_id).contains(absolute_file_path) &&
                            is_readable_file(absolute_file_path) && m_ingest != nullptr &&
                            m_history != nullptr;

    if (can_import)
    {
        const QVector<LogEntry> entries = m_ingest->load_file_sync(absolute_file_path, profile);
        const QString app_name = !entries.isEmpty() ? entries.first().get_app_name()
                                                    : LogLoader::identify_app(absolute_file_path);
        const bool stored = entries.isEmpty() || m_history->add_entries(view_id, entries);

        if (stored)
        {
            remember_profile(view_id, absolute_file_path, profile);
            m_views->add_loaded_file(view_id, LogFileInfo(absolute_file_path, app_name));
            imported = refresh_visible_page(view_id);

            if (m_live_tailing != nullptr && m_live_tailing->is_enabled(view_id))
            {
                m_live_tailing->start_file(view_id, absolute_file_path, profile);
            }
        }
    }

    return imported;
}

/**
 * @brief Imports multiple files synchronously into one new view.
 * @param file_paths Files to import.
 * @param profile Parsing profile selected for every file.
 * @return Created view id, or a null id when validation or storage fails.
 */
auto LogImportCoordinator::import_files(const QVector<QString>& file_paths,
                                        const LogParsingProfile& profile) -> QUuid
{
    QUuid view_id;
    bool valid_files = !file_paths.isEmpty();

    for (const QString& file_path: file_paths)
    {
        if (!is_readable_file(file_path))
        {
            valid_files = false;
            qWarning().nospace() << "[Import] synchronous import rejected file=\""
                                 << QFileInfo(file_path).absoluteFilePath() << '"';
        }
    }

    if (valid_files && m_ingest != nullptr && m_views != nullptr && m_history != nullptr)
    {
        const QUuid candidate_view_id = m_views->create_view();
        QList<LogFileInfo> loaded_files;
        bool stored = true;

        for (qsizetype index = 0; index < file_paths.size() && stored; ++index)
        {
            const QString absolute_file_path = QFileInfo(file_paths.at(index)).absoluteFilePath();
            const QVector<LogEntry> entries = m_ingest->load_file_sync(absolute_file_path, profile);
            const QString app_name = !entries.isEmpty()
                                         ? entries.first().get_app_name()
                                         : LogLoader::identify_app(absolute_file_path);
            stored = entries.isEmpty() || m_history->add_entries(candidate_view_id, entries);

            if (stored)
            {
                remember_profile(candidate_view_id, absolute_file_path, profile);
                loaded_files.append(LogFileInfo(absolute_file_path, app_name));
            }
        }

        if (stored)
        {
            m_views->set_loaded_files(candidate_view_id, loaded_files);
            refresh_visible_page(candidate_view_id);

            if (m_live_tailing != nullptr)
            {
                m_live_tailing->set_enabled(candidate_view_id, true);
            }

            view_id = candidate_view_id;
        }
        else
        {
            emit view_removal_requested(candidate_view_id);
        }
    }

    return view_id;
}

/**
 * @brief Enqueues one asynchronous import in a new view.
 * @param file_path File to import.
 * @param profile Parsing profile selected for the file.
 * @param batch_size Number of parsed entries per storage batch.
 * @return Created view id.
 */
auto LogImportCoordinator::import_file_async(const QString& file_path,
                                             const LogParsingProfile& profile,
                                             qsizetype batch_size) -> QUuid
{
    QUuid view_id;

    if (!m_shutting_down && m_views != nullptr)
    {
        view_id = m_views->create_view();
        const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
        const QString app_name = LogLoader::identify_app(absolute_file_path);
        m_views->set_loaded_files(view_id,
                                  QList<LogFileInfo>{LogFileInfo(absolute_file_path, app_name)});

        if (m_live_tailing != nullptr)
        {
            m_live_tailing->set_enabled(view_id, true);
        }

        enqueue(view_id, absolute_file_path, profile);
        start_next(batch_size);
    }

    return view_id;
}

/**
 * @brief Enqueues one asynchronous import in an existing view.
 * @param view_id Target view.
 * @param file_path File to import.
 * @param profile Parsing profile selected for the file.
 * @param batch_size Number of parsed entries per storage batch.
 * @return True when the request was registered and enqueued.
 */
auto LogImportCoordinator::import_file_async(const QUuid& view_id, const QString& file_path,
                                             const LogParsingProfile& profile,
                                             qsizetype batch_size) -> bool
{
    bool enqueued = false;

    if (!m_shutting_down && m_views != nullptr)
    {
        m_views->ensure_view(view_id);
        const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();

        if (!m_views->get_file_paths(view_id).contains(absolute_file_path))
        {
            const QString app_name = LogLoader::identify_app(absolute_file_path);
            m_views->add_loaded_file(view_id, LogFileInfo(absolute_file_path, app_name));
            enqueue(view_id, absolute_file_path, profile);
            start_next(batch_size);
            enqueued = true;
        }
    }

    return enqueued;
}

/**
 * @brief Enqueues multiple asynchronous imports in one new view.
 * @param file_paths Files to import.
 * @param profile Parsing profile selected for every file.
 * @param batch_size Number of parsed entries per storage batch.
 * @return Created view id, or a null id for an empty request.
 */
auto LogImportCoordinator::import_files_async(const QVector<QString>& file_paths,
                                              const LogParsingProfile& profile,
                                              qsizetype batch_size) -> QUuid
{
    QUuid view_id;

    if (!m_shutting_down && !file_paths.isEmpty() && m_views != nullptr)
    {
        view_id = m_views->create_view();
        QList<LogFileInfo> loaded_files;

        for (const QString& file_path: file_paths)
        {
            const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
            loaded_files.append(
                LogFileInfo(absolute_file_path, LogLoader::identify_app(absolute_file_path)));
            enqueue(view_id, absolute_file_path, profile);
        }

        m_views->set_loaded_files(view_id, loaded_files);

        if (m_live_tailing != nullptr)
        {
            m_live_tailing->set_enabled(view_id, true);
        }

        start_next(batch_size);
    }

    return view_id;
}

/**
 * @brief Enqueues a file that has already been registered during state restoration.
 * @param view_id Restored target view.
 * @param file_path Registered file path.
 * @param profile Restored parsing profile.
 * @param batch_size Number of parsed entries per storage batch.
 */
auto LogImportCoordinator::enqueue_registered_file(const QUuid& view_id, const QString& file_path,
                                                   const LogParsingProfile& profile,
                                                   qsizetype batch_size) -> void
{
    if (!m_shutting_down && m_views != nullptr && m_views->get_context(view_id) != nullptr)
    {
        enqueue(view_id, QFileInfo(file_path).absoluteFilePath(), profile);
        start_next(batch_size);
    }
}

/**
 * @brief Cancels active and pending imports belonging to one view.
 * @param view_id View whose imports are cancelled.
 */
auto LogImportCoordinator::cancel(const QUuid& view_id) -> void
{
    m_failed_files.remove(view_id);

    if (m_ingest != nullptr)
    {
        m_ingest->cancel_for_view(view_id);
    }
}

/**
 * @brief Stops accepting import callbacks and cancels every registered view.
 */
auto LogImportCoordinator::shutdown() -> void
{
    if (!m_shutting_down)
    {
        m_shutting_down = true;
        m_failed_files.clear();

        if (m_ingest != nullptr && m_views != nullptr)
        {
            const QVector<QUuid> view_ids = m_views->get_all_view_ids();
            for (const QUuid& view_id: view_ids)
            {
                m_ingest->cancel_for_view(view_id);
            }
        }
    }
}

/**
 * @brief Connects low-level ingest and writer events to the import workflow.
 */
auto LogImportCoordinator::connect_workflow() -> void
{
    if (m_ingest != nullptr)
    {
        connect(
            m_ingest, &LogIngestController::entry_batch_parsed, this,
            [this](const QUuid& view_id, const QString& file_path, const QVector<LogEntry>& batch) {
                const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
                const bool can_store =
                    !m_shutting_down && !view_id.isNull() && !batch.isEmpty() &&
                    !m_failed_files.value(view_id).contains(absolute_file_path) &&
                    m_views != nullptr && m_views->get_context(view_id) != nullptr &&
                    m_history_writer != nullptr;

                if (can_store)
                {
                    const bool accepted =
                        m_history_writer->store_batch(view_id, absolute_file_path, batch);

                    if (!accepted)
                    {
                        m_failed_files[view_id].insert(absolute_file_path);
                        emit error(view_id, absolute_file_path,
                                   tr("The imported entries could not be queued for storage."));
                        emit file_removal_requested(view_id, absolute_file_path);
                    }
                }
            });

        connect(
            m_ingest, &LogIngestController::progress, this,
            [this](const QUuid& view_id, const QString&, qint64 bytes_read, qint64 total_bytes) {
                if (!m_shutting_down && !view_id.isNull())
                {
                    emit progress(view_id, bytes_read, total_bytes);
                }
            });

        connect(m_ingest, &LogIngestController::error, this,
                [this](const QUuid& view_id, const QString& file_path, const QString& message) {
                    const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
                    const bool can_handle = !m_shutting_down && !view_id.isNull();

                    if (can_handle)
                    {
                        m_failed_files[view_id].insert(absolute_file_path);
                        emit error(view_id, absolute_file_path, message);

                        const bool registered =
                            m_views != nullptr && m_views->get_context(view_id) != nullptr &&
                            m_views->get_file_paths(view_id).contains(absolute_file_path);
                        if (registered)
                        {
                            emit file_removal_requested(view_id, absolute_file_path);
                        }
                    }
                });

        connect(m_ingest, &LogIngestController::finished, this,
                [this](const QUuid& view_id, const QString& file_path) {
                    if (!m_shutting_down && m_history_writer != nullptr)
                    {
                        m_history_writer->finish_import(view_id,
                                                        QFileInfo(file_path).absoluteFilePath());
                    }
                });

        connect(m_ingest, &LogIngestController::idle, this, [this]() {
            if (!m_shutting_down && m_ingest != nullptr)
            {
                start_next(m_ingest->get_active_batch_size());
            }
        });
    }

    if (m_history_writer != nullptr)
    {
        connect(m_history_writer, &HistoryWriteService::import_write_finished, this,
                &LogImportCoordinator::handle_write_finished, Qt::QueuedConnection);
    }
}

/**
 * @brief Returns whether a path names a readable regular file.
 * @param file_path Path to validate.
 * @return True when the file exists and is readable.
 */
auto LogImportCoordinator::is_readable_file(const QString& file_path) const -> bool
{
    const QFileInfo file_info(file_path);
    const bool readable = file_info.exists() && file_info.isFile() && file_info.isReadable();
    return readable;
}

/**
 * @brief Stores a parsing profile in its view context.
 * @param view_id View owning the file.
 * @param file_path Imported file path.
 * @param profile Selected parsing profile.
 */
auto LogImportCoordinator::remember_profile(const QUuid& view_id, const QString& file_path,
                                            const LogParsingProfile& profile) -> void
{
    LogViewContext* context = m_views != nullptr ? m_views->get_context(view_id) : nullptr;
    if (context != nullptr)
    {
        context->set_file_parsing_profile(file_path, profile);
    }
}

/**
 * @brief Returns a retained file profile or the coordinator default.
 * @param view_id View owning the file.
 * @param file_path Imported file path.
 * @return Retained parsing profile or the default profile.
 */
auto LogImportCoordinator::get_profile(const QUuid& view_id,
                                       const QString& file_path) const -> LogParsingProfile
{
    LogParsingProfile profile = m_default_profile;
    const LogViewContext* context = m_views != nullptr ? m_views->get_context(view_id) : nullptr;

    if (context != nullptr)
    {
        const std::optional<LogParsingProfile> stored_profile =
            context->get_file_parsing_profile(file_path);
        if (stored_profile.has_value())
        {
            profile = stored_profile.value();
        }
    }

    return profile;
}

/**
 * @brief Registers and enqueues one asynchronous file.
 * @param view_id Target view.
 * @param file_path File to enqueue.
 * @param profile Selected parsing profile.
 */
auto LogImportCoordinator::enqueue(const QUuid& view_id, const QString& file_path,
                                   const LogParsingProfile& profile) -> void
{
    remember_profile(view_id, file_path, profile);
    if (m_ingest != nullptr)
    {
        m_ingest->enqueue_stream(view_id, file_path, profile);
    }
}

/**
 * @brief Starts the next queue item and selects its view.
 * @param batch_size Number of parsed entries per storage batch.
 */
auto LogImportCoordinator::start_next(qsizetype batch_size) -> void
{
    if (m_ingest != nullptr)
    {
        m_ingest->start_next_if_idle(batch_size);
        const QUuid active_view_id = m_ingest->get_active_view_id();

        if (!active_view_id.isNull() && m_views != nullptr &&
            m_views->get_current_view() != active_view_id)
        {
            m_views->set_current_view(active_view_id);
        }
    }
}

/**
 * @brief Loads or refreshes the bounded page representing imported history.
 * @param view_id View whose stored entries changed.
 * @return True when a query or page refresh was applied.
 */
auto LogImportCoordinator::refresh_visible_page(const QUuid& view_id) -> bool
{
    const bool has_page = m_pages != nullptr && m_pages->get_page_state(view_id) != nullptr;
    bool refreshed = false;

    if (m_queries != nullptr)
    {
        if (has_page)
        {
            refreshed = m_queries->refresh_after_entries_appended(view_id);
        }
        else
        {
            refreshed = m_queries->reload_query(view_id);
        }
    }

    return refreshed;
}

/**
 * @brief Completes one asynchronous import after ordered history writes finish.
 * @param view_id View that owns the import.
 * @param file_path Imported source file.
 * @param succeeded Whether all queued history operations succeeded.
 * @param error_message Storage error for a failed import.
 */
auto LogImportCoordinator::handle_write_finished(const QUuid& view_id, const QString& file_path,
                                                 bool succeeded,
                                                 const QString& error_message) -> void
{
    const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
    const bool ingest_failed = m_failed_files.value(view_id).contains(absolute_file_path);
    const bool import_succeeded = succeeded && !ingest_failed;
    const bool view_exists =
        m_views != nullptr && !view_id.isNull() && m_views->get_context(view_id) != nullptr;

    m_failed_files[view_id].remove(absolute_file_path);
    if (m_failed_files.value(view_id).isEmpty())
    {
        m_failed_files.remove(view_id);
    }

    if (!m_shutting_down)
    {
        if (!succeeded && view_exists)
        {
            emit error(view_id, absolute_file_path, error_message);
        }

        if (!import_succeeded)
        {
            const bool registered =
                view_exists && m_views->get_file_paths(view_id).contains(absolute_file_path);
            if (registered)
            {
                emit file_removal_requested(view_id, absolute_file_path);
            }
        }
        else if (view_exists)
        {
            refresh_visible_page(view_id);
            emit finished(view_id, absolute_file_path);

            if (m_live_tailing != nullptr)
            {
                m_live_tailing->start_file(view_id, absolute_file_path,
                                           get_profile(view_id, absolute_file_path));
            }
        }
    }
}
