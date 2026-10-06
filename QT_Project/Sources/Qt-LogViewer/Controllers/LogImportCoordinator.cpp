/**
 * @file LogImportCoordinator.cpp
 * @brief Implements synchronous and asynchronous log-import coordination.
 */

#include "Qt-LogViewer/Controllers/LogImportCoordinator.h"

#include <QDebug>
#include <QFileInfo>
#include <QList>
#include <optional>

#include "Qt-LogViewer/Controllers/LiveTailingCoordinator.h"
#include "Qt-LogViewer/Controllers/LogIngestController.h"
#include "Qt-LogViewer/Controllers/LogPageCoordinator.h"
#include "Qt-LogViewer/Controllers/LogQueryController.h"
#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Services/HistoryWriteService.h"
#include "Qt-LogViewer/Services/LogCacheIdentity.h"
#include "Qt-LogViewer/Services/LogFileCacheDatabase.h"
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
 * @param cache_catalog Persistent cache-generation catalog.
 * @param parent Optional QObject parent.
 */
LogImportCoordinator::LogImportCoordinator(const LogParsingProfile& default_profile,
                                           LogIngestController* ingest, ViewRegistry* views,
                                           LogHistoryService* history,
                                           HistoryWriteService* history_writer,
                                           LogPageCoordinator* pages, LogQueryController* queries,
                                           LiveTailingCoordinator* live_tailing,
                                           LogCacheCatalog* cache_catalog, QObject* parent)
    : QObject(parent),
      m_default_profile(default_profile),
      m_ingest(ingest),
      m_views(views),
      m_history(history),
      m_history_writer(history_writer),
      m_pages(pages),
      m_queries(queries),
      m_live_tailing(live_tailing),
      m_cache_catalog(cache_catalog)
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
        const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
        const std::optional<LogCacheGeneration> cache_generation =
            prepare_cache_generation(absolute_file_path, profile);
        const bool cache_prepared = m_cache_catalog == nullptr || cache_generation.has_value();
        const bool cache_hit = cache_generation.has_value() &&
                               cache_generation->state == LogCacheGenerationState::Complete;
        const QVector<LogEntry> entries =
            cache_prepared && !cache_hit ? m_ingest->load_file_sync(absolute_file_path, profile)
                                         : QVector<LogEntry>();
        const QString app_name = !entries.isEmpty() ? entries.first().get_app_name()
                                                    : LogLoader::identify_app(absolute_file_path);
        const QUuid candidate_view_id = m_views->create_view();
        const bool cache_completed =
            cache_prepared && (!cache_generation.has_value() ||
                               complete_synchronous_cache(cache_generation.value(), entries));
        const bool history_stored =
            cache_completed && (cache_generation.has_value() || entries.isEmpty() ||
                                m_history->add_entries(candidate_view_id, entries));
        const bool stored = cache_completed && history_stored;

        if (!stored)
        {
            qWarning().nospace() << "[Import] synchronous storage failed cache_prepared="
                                 << cache_prepared << " cache_completed=" << cache_completed
                                 << " entries=" << entries.size()
                                 << " history_stored=" << history_stored;
        }

        if (stored)
        {
            remember_profile(candidate_view_id, absolute_file_path, profile);
            m_views->set_loaded_files(
                candidate_view_id, QList<LogFileInfo>{LogFileInfo(absolute_file_path, app_name)});
            if (cache_generation.has_value())
            {
                remember_view_generation(candidate_view_id, absolute_file_path,
                                         cache_generation->id);
                bind_view_generations(candidate_view_id);
            }
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
        const std::optional<LogCacheGeneration> cache_generation =
            prepare_cache_generation(absolute_file_path, profile);
        const bool cache_prepared = m_cache_catalog == nullptr || cache_generation.has_value();
        const bool cache_hit = cache_generation.has_value() &&
                               cache_generation->state == LogCacheGenerationState::Complete;
        const QVector<LogEntry> entries =
            cache_prepared && !cache_hit ? m_ingest->load_file_sync(absolute_file_path, profile)
                                         : QVector<LogEntry>();
        const QString app_name = !entries.isEmpty() ? entries.first().get_app_name()
                                                    : LogLoader::identify_app(absolute_file_path);
        const bool cache_completed =
            cache_prepared && (!cache_generation.has_value() ||
                               complete_synchronous_cache(cache_generation.value(), entries));
        const bool stored = cache_completed && (cache_generation.has_value() || entries.isEmpty() ||
                                                m_history->add_entries(view_id, entries));

        if (stored)
        {
            remember_profile(view_id, absolute_file_path, profile);
            m_views->add_loaded_file(view_id, LogFileInfo(absolute_file_path, app_name));
            if (cache_generation.has_value())
            {
                remember_view_generation(view_id, absolute_file_path, cache_generation->id);
                bind_view_generations(view_id);
            }
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
            const std::optional<LogCacheGeneration> cache_generation =
                prepare_cache_generation(absolute_file_path, profile);
            const bool cache_prepared = m_cache_catalog == nullptr || cache_generation.has_value();
            const bool cache_hit = cache_generation.has_value() &&
                                   cache_generation->state == LogCacheGenerationState::Complete;
            const QVector<LogEntry> entries =
                cache_prepared && !cache_hit ? m_ingest->load_file_sync(absolute_file_path, profile)
                                             : QVector<LogEntry>();
            const QString app_name = !entries.isEmpty()
                                         ? entries.first().get_app_name()
                                         : LogLoader::identify_app(absolute_file_path);
            const bool cache_completed =
                cache_prepared && (!cache_generation.has_value() ||
                                   complete_synchronous_cache(cache_generation.value(), entries));
            stored = cache_completed && (cache_generation.has_value() || entries.isEmpty() ||
                                         m_history->add_entries(candidate_view_id, entries));

            if (stored)
            {
                remember_profile(candidate_view_id, absolute_file_path, profile);
                loaded_files.append(LogFileInfo(absolute_file_path, app_name));
                if (cache_generation.has_value())
                {
                    remember_view_generation(candidate_view_id, absolute_file_path,
                                             cache_generation->id);
                }
            }
        }

        if (stored)
        {
            m_views->set_loaded_files(candidate_view_id, loaded_files);
            bind_view_generations(candidate_view_id);
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
 *
 * The view retains enabled live-tail state without starting a file registration
 * until the import has completed. This prevents the default profile from being
 * captured before the selected file profile is retained.
 *
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
            m_live_tailing->reset_view(view_id, true);
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
 *
 * Live-tail registrations are deferred until the corresponding import
 * completes, so every file starts with the profile retained by enqueue().
 *
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
        }

        m_views->set_loaded_files(view_id, loaded_files);

        if (m_live_tailing != nullptr)
        {
            m_live_tailing->reset_view(view_id, true);
        }

        for (const QString& file_path: file_paths)
        {
            enqueue(view_id, QFileInfo(file_path).absoluteFilePath(), profile);
        }

        start_next(batch_size);
    }

    return view_id;
}

/**
 * @brief Enqueues a file that has already been registered during state
 * restoration.
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
    QList<QUuid> cancelled_operations;
    for (auto iterator = m_operation_views.cbegin(); iterator != m_operation_views.cend();
         ++iterator)
    {
        if (iterator.value() == view_id)
        {
            cancelled_operations.append(iterator.key());
        }
    }

    for (const QUuid& operation_id: cancelled_operations)
    {
        const LogCacheGeneration generation = m_operation_generations.take(operation_id);
        if (m_cache_catalog != nullptr && generation.id >= 0 &&
            generation.state == LogCacheGenerationState::Building)
        {
            m_cache_catalog->mark_failed(generation.id, QStringLiteral("Import cancelled."));
        }
        m_operation_views.remove(operation_id);
        m_failed_operations.remove(operation_id);
    }

    if (m_ingest != nullptr)
    {
        m_ingest->cancel_for_view(view_id);
    }

    if (m_history_writer != nullptr)
    {
        m_history_writer->cancel_view(view_id);
    }
}

/**
 * @brief Removes one file generation from a view's persistent cache mapping.
 * @param view_id View that no longer contains the file.
 * @param file_path Removed source file.
 * @return True when the mapping was removed, rebound, or safely deferred.
 */
auto LogImportCoordinator::discard_file_cache_binding(const QUuid& view_id,
                                                      const QString& file_path) -> bool
{
    const bool valid_request = !view_id.isNull() && !file_path.isEmpty();
    bool discarded = valid_request;

    if (valid_request)
    {
        const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
        auto view_iterator = m_view_generations.find(view_id);
        if (view_iterator != m_view_generations.end())
        {
            view_iterator->remove(absolute_file_path);
            if (view_iterator->isEmpty())
            {
                m_view_generations.erase(view_iterator);
            }
        }

        const bool has_active_import = m_operation_views.values().contains(view_id);
        if (m_cache_catalog != nullptr && !has_active_import)
        {
            const bool view_is_empty = m_views == nullptr ||
                                       m_views->get_context(view_id) == nullptr ||
                                       m_views->get_file_paths(view_id).isEmpty();
            discarded = view_is_empty ? m_cache_catalog->remove_view(view_id)
                                      : bind_view_generations(view_id);
        }
    }

    return discarded;
}

/**
 * @brief Queues removal of stored history belonging to one view.
 * @param view_id View whose stored history is discarded.
 * @return True when asynchronous history cleanup was queued.
 */
auto LogImportCoordinator::discard_history(const QUuid& view_id) -> bool
{
    m_view_generations.remove(view_id);
    const bool cache_mapping_removed =
        m_cache_catalog == nullptr || m_cache_catalog->remove_view(view_id);
    const bool history_discarded =
        m_history_writer != nullptr && m_history_writer->discard_view(view_id);
    return cache_mapping_removed && history_discarded;
}

/**
 * @brief Stops accepting import callbacks and cancels every registered view.
 */
auto LogImportCoordinator::shutdown() -> void
{
    if (!m_shutting_down)
    {
        m_shutting_down = true;

        if (m_views != nullptr)
        {
            const QVector<QUuid> view_ids = m_views->get_all_view_ids();
            for (const QUuid& view_id: view_ids)
            {
                cancel(view_id);
            }
        }

        m_operation_views.clear();
        m_operation_generations.clear();
        m_view_generations.clear();
        m_failed_operations.clear();
    }
}

/**
 * @brief Connects low-level ingest and writer events to the import workflow.
 */
auto LogImportCoordinator::connect_workflow() -> void
{
    if (m_ingest != nullptr)
    {
        connect(m_ingest, &LogIngestController::entry_batch_parsed, this,
                [this](const QUuid& operation_id, const QUuid& view_id, const QString& file_path,
                       const QVector<LogEntry>& batch) {
                    const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
                    const bool can_store =
                        !m_shutting_down && !view_id.isNull() && !operation_id.isNull() &&
                        m_operation_views.value(operation_id) == view_id && !batch.isEmpty() &&
                        !m_failed_operations.contains(operation_id) && m_views != nullptr &&
                        m_views->get_context(view_id) != nullptr && m_history_writer != nullptr;

                    if (can_store)
                    {
                        const bool accepted = m_history_writer->store_batch(
                            operation_id, view_id, absolute_file_path, batch);

                        if (!accepted)
                        {
                            const LogCacheGeneration generation =
                                m_operation_generations.take(operation_id);
                            if (m_cache_catalog != nullptr && generation.id >= 0 &&
                                generation.state == LogCacheGenerationState::Building)
                            {
                                m_cache_catalog->mark_failed(
                                    generation.id, QStringLiteral("Writer rejected a batch."));
                            }
                            m_operation_views.remove(operation_id);
                            if (m_ingest != nullptr)
                            {
                                m_ingest->cancel_for_view(view_id);
                            }
                            emit error(view_id, absolute_file_path,
                                       tr("The imported entries could not be queued for storage."));
                            emit file_removal_requested(view_id, absolute_file_path);
                        }
                    }
                });

        connect(m_ingest, &LogIngestController::progress, this,
                [this](const QUuid& operation_id, const QUuid& view_id, const QString&,
                       qint64 bytes_read, qint64 total_bytes) {
                    if (!m_shutting_down && !view_id.isNull() &&
                        m_operation_views.value(operation_id) == view_id)
                    {
                        emit progress(view_id, bytes_read, total_bytes);
                    }
                });

        connect(m_ingest, &LogIngestController::error, this,
                [this](const QUuid& operation_id, const QUuid& view_id, const QString& file_path,
                       const QString& message) {
                    const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
                    const bool can_handle = !m_shutting_down && !view_id.isNull() &&
                                            m_operation_views.value(operation_id) == view_id;

                    if (can_handle)
                    {
                        const LogCacheGeneration generation =
                            m_operation_generations.take(operation_id);
                        if (m_cache_catalog != nullptr && generation.id >= 0 &&
                            generation.state == LogCacheGenerationState::Building)
                        {
                            m_cache_catalog->mark_failed(
                                generation.id, QStringLiteral("Reader reported an error."));
                        }
                        m_operation_views.remove(operation_id);
                        m_failed_operations.remove(operation_id);
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
                [this](const QUuid& operation_id, const QUuid& view_id, const QString& file_path) {
                    if (!m_shutting_down && !operation_id.isNull() &&
                        m_operation_views.value(operation_id) == view_id &&
                        m_history_writer != nullptr)
                    {
                        m_history_writer->finish_import(operation_id, view_id,
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
        connect(m_history_writer, &HistoryWriteService::batches_processed, this,
                [this](const QUuid& operation_id, qsizetype batch_count) {
                    if (!m_shutting_down && m_ingest != nullptr &&
                        m_operation_views.contains(operation_id))
                    {
                        m_ingest->acknowledge_batches(operation_id, batch_count);
                    }
                });

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

    if (m_ingest != nullptr && m_history_writer != nullptr)
    {
        const std::optional<LogCacheGeneration> cache_generation =
            prepare_cache_generation(file_path, profile);
        const bool cache_ready = m_cache_catalog == nullptr || cache_generation.has_value();
        if (!cache_ready)
        {
            emit error(view_id, file_path,
                       tr("The persistent cache generation could not be prepared."));
            emit file_removal_requested(view_id, QFileInfo(file_path).absoluteFilePath());
        }
        else
        {
            const bool cache_hit = cache_generation.has_value() &&
                                   cache_generation->state == LogCacheGenerationState::Complete;
            if (cache_hit)
            {
                remember_view_generation(view_id, file_path, cache_generation->id);
                const bool view_bound = bind_view_generations(view_id);
                if (view_bound)
                {
                    refresh_visible_page(view_id);
                }
                emit finished(view_id, QFileInfo(file_path).absoluteFilePath());
                if (m_live_tailing != nullptr)
                {
                    m_live_tailing->start_file(view_id, file_path, profile);
                }
            }
            else
            {
                const QUuid operation_id = m_ingest->enqueue_stream(view_id, file_path, profile);
                std::optional<LogCacheGeneration> writer_generation;
                if (cache_generation.has_value() &&
                    cache_generation->state == LogCacheGenerationState::Building)
                {
                    writer_generation = cache_generation;
                }
                const bool registered = !operation_id.isNull() &&
                                        m_history_writer->begin_import(
                                            operation_id, view_id, file_path, writer_generation);

                if (!operation_id.isNull() && registered)
                {
                    m_operation_views.insert(operation_id, view_id);
                    if (cache_generation.has_value())
                    {
                        m_operation_generations.insert(operation_id, cache_generation.value());
                    }
                }

                if (!registered)
                {
                    if (cache_generation.has_value() && m_cache_catalog != nullptr &&
                        cache_generation->state == LogCacheGenerationState::Building)
                    {
                        m_cache_catalog->discard_generation(
                            cache_generation->id, QStringLiteral("Writer registration failed."));
                    }
                    m_ingest->cancel_for_view(view_id);
                    emit error(view_id, file_path,
                               tr("The import could not be registered for background storage."));
                    emit file_removal_requested(view_id, QFileInfo(file_path).absoluteFilePath());
                }
            }
        }
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
 * @param operation_id Unique identifier of the completed import attempt.
 * @param view_id View that owns the import.
 * @param file_path Imported source file.
 * @param succeeded Whether all queued history operations succeeded.
 * @param error_message Storage error for a failed import.
 */
auto LogImportCoordinator::handle_write_finished(const QUuid& operation_id, const QUuid& view_id,
                                                 const QString& file_path, bool succeeded,
                                                 const QString& error_message) -> void
{
    const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
    const bool operation_is_active = m_operation_views.value(operation_id) == view_id;
    const bool ingest_failed = m_failed_operations.contains(operation_id);
    bool import_succeeded = succeeded && !ingest_failed;
    const bool view_exists =
        m_views != nullptr && !view_id.isNull() && m_views->get_context(view_id) != nullptr;
    const std::optional<LogCacheGeneration> cache_generation =
        m_operation_generations.contains(operation_id)
            ? std::optional<LogCacheGeneration>(m_operation_generations.value(operation_id))
            : std::nullopt;

    m_operation_views.remove(operation_id);
    m_operation_generations.remove(operation_id);
    m_failed_operations.remove(operation_id);

    if (cache_generation.has_value() &&
        cache_generation->state == LogCacheGenerationState::Building && m_cache_catalog != nullptr)
    {
        if (import_succeeded)
        {
            import_succeeded = complete_asynchronous_cache(cache_generation.value());
            if (!import_succeeded && !m_shutting_down && operation_is_active)
            {
                emit error(view_id, absolute_file_path,
                           tr("The completed import cache could not be activated."));
            }
        }
        else
        {
            m_cache_catalog->discard_generation(
                cache_generation->id, QStringLiteral("Import did not complete successfully."));
        }
    }

    if (!m_shutting_down && operation_is_active)
    {
        if (!succeeded && view_exists)
        {
            emit error(view_id, absolute_file_path, error_message);
        }

        if (!import_succeeded)
        {
            m_view_generations[view_id].remove(absolute_file_path);
            const bool registered =
                view_exists && m_views->get_file_paths(view_id).contains(absolute_file_path);
            if (registered)
            {
                emit file_removal_requested(view_id, absolute_file_path);
            }
        }
        else if (view_exists)
        {
            if (cache_generation.has_value())
            {
                remember_view_generation(view_id, absolute_file_path, cache_generation->id);
                bind_view_generations(view_id);
            }
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

/**
 * @brief Finds or starts the cache generation matching a source and parser
 * profile.
 * @param file_path Readable source file.
 * @param profile Effective parsing profile.
 * @return Existing complete or newly building generation, or no value on
 * failure.
 */
auto LogImportCoordinator::prepare_cache_generation(
    const QString& file_path, const LogParsingProfile& profile) -> std::optional<LogCacheGeneration>
{
    std::optional<LogCacheGeneration> generation;
    const std::optional<LogCacheIdentity> identity =
        m_cache_catalog != nullptr ? LogCacheIdentity::create(file_path, profile) : std::nullopt;
    if (identity.has_value())
    {
        generation = m_cache_catalog->find_complete_generation(identity.value());
        if (generation.has_value())
        {
            m_cache_catalog->touch_generation(generation->id);
        }
        else
        {
            generation = m_cache_catalog->begin_generation(identity.value());
        }
    }
    return generation;
}

/**
 * @brief Builds and completes one cache generation during a synchronous import.
 * @param generation Building or already complete generation.
 * @param entries Parsed source entries carrying byte ranges.
 * @return True when the generation is complete and reusable.
 */
auto LogImportCoordinator::complete_synchronous_cache(const LogCacheGeneration& generation,
                                                      const QVector<LogEntry>& entries) -> bool
{
    bool completed = generation.state == LogCacheGenerationState::Complete;
    if (!completed && m_cache_catalog != nullptr &&
        generation.state == LogCacheGenerationState::Building)
    {
        qint64 entry_count = -1;
        qint64 storage_bytes = -1;
        bool built = false;
        {
            LogFileCacheDatabase database(generation.database_path, generation.identity);
            built = database.is_available() && database.reset_entries() &&
                    (entries.isEmpty() || database.append_entries(entries)) &&
                    database.finalize_writes();
            if (built)
            {
                entry_count = database.get_entry_count();
                storage_bytes = database.get_storage_bytes();
                built = entry_count >= 0 && storage_bytes >= 0;
            }
        }

        completed =
            built && m_cache_catalog->mark_complete(generation.id, generation.identity.file_size,
                                                    entry_count, storage_bytes);
        if (!completed)
        {
            qWarning().nospace() << "[Cache] synchronous generation activation failed id="
                                 << generation.id << " built=" << built
                                 << " entries=" << entry_count << " storage=" << storage_bytes;
            m_cache_catalog->discard_generation(generation.id,
                                                QStringLiteral("Synchronous cache build failed."));
        }
    }
    return completed;
}

/**
 * @brief Validates writer output and atomically completes an asynchronous
 * generation.
 * @param generation Building generation whose writer connection has closed.
 * @return True when counts and storage metadata were committed to the catalog.
 */
auto LogImportCoordinator::complete_asynchronous_cache(const LogCacheGeneration& generation) -> bool
{
    bool completed = false;
    qint64 entry_count = -1;
    qint64 storage_bytes = -1;
    const bool can_complete =
        m_cache_catalog != nullptr && generation.state == LogCacheGenerationState::Building;
    if (can_complete)
    {
        bool valid = false;
        {
            LogFileCacheDatabase database(generation.database_path, generation.identity);
            valid = database.is_available();
            if (valid)
            {
                entry_count = database.get_entry_count();
                storage_bytes = database.get_storage_bytes();
                valid = entry_count >= 0 && storage_bytes >= 0;
            }
        }

        completed =
            valid && m_cache_catalog->mark_complete(generation.id, generation.identity.file_size,
                                                    entry_count, storage_bytes);
        if (!completed)
        {
            m_cache_catalog->discard_generation(
                generation.id, QStringLiteral("Asynchronous cache activation failed."));
        }
    }
    return completed;
}

/**
 * @brief Retains one complete generation for later atomic view binding.
 * @param view_id View owning the source registration.
 * @param file_path Registered source path.
 * @param generation_id Complete catalog generation primary key.
 */
auto LogImportCoordinator::remember_view_generation(const QUuid& view_id, const QString& file_path,
                                                    qint64 generation_id) -> void
{
    if (!view_id.isNull() && generation_id >= 0)
    {
        m_view_generations[view_id].insert(QFileInfo(file_path).absoluteFilePath(), generation_id);
    }
}

/**
 * @brief Binds a view after all its active imports have complete generations.
 * @param view_id View whose ordered generation mapping may be published.
 * @return True when a complete ordered mapping was committed or caching is
 * disabled.
 */
auto LogImportCoordinator::bind_view_generations(const QUuid& view_id) -> bool
{
    bool bound = m_cache_catalog == nullptr;
    QVector<qint64> generation_ids;
    const bool can_bind = m_cache_catalog != nullptr && m_views != nullptr &&
                          !m_operation_views.values().contains(view_id);
    if (can_bind)
    {
        const QVector<QString> file_paths = m_views->get_file_paths(view_id);
        const QHash<QString, qint64> generations = m_view_generations.value(view_id);
        generation_ids.reserve(file_paths.size());
        bool mapping_complete = true;
        for (qsizetype index = 0; index < file_paths.size() && mapping_complete; ++index)
        {
            const auto iterator =
                generations.constFind(QFileInfo(file_paths.at(index)).absoluteFilePath());
            mapping_complete = iterator != generations.cend();
            if (mapping_complete)
            {
                generation_ids.append(iterator.value());
            }
        }
        bound = mapping_complete && m_cache_catalog->bind_view(view_id, generation_ids);
    }
    return bound;
}
