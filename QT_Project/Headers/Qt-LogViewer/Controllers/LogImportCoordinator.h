#pragma once

#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>
#include <QUuid>
#include <QVector>

#include "Qt-LogViewer/Services/LogParsingProfile.h"

class HistoryWriteService;
class LiveTailingCoordinator;
class LogHistoryService;
class LogIngestController;
class LogPageCoordinator;
class LogQueryController;
class ViewRegistry;

/**
 * @file LogImportCoordinator.h
 * @brief Declares the complete synchronous and asynchronous log-import workflow.
 */

/**
 * @class LogImportCoordinator
 * @brief Coordinates validation, view registration, parsing, persistence, and completion.
 *
 * The coordinator owns import rules but not the injected services. View and file removal remain
 * lifecycle operations and are requested through signals until their own boundary is extracted.
 */
class LogImportCoordinator final: public QObject
{
        Q_OBJECT

    public:
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
        explicit LogImportCoordinator(const LogParsingProfile& default_profile,
                                      LogIngestController* ingest, ViewRegistry* views,
                                      LogHistoryService* history,
                                      HistoryWriteService* history_writer,
                                      LogPageCoordinator* pages, LogQueryController* queries,
                                      LiveTailingCoordinator* live_tailing,
                                      QObject* parent = nullptr);

        /**
         * @brief Imports one file synchronously into a new view.
         * @param file_path File to import.
         * @param profile Parsing profile selected for the file.
         * @return Created view id, or a null id when validation or storage fails.
         */
        auto import_file(const QString& file_path, const LogParsingProfile& profile) -> QUuid;

        /**
         * @brief Imports one file synchronously into an existing view.
         * @param view_id Target view.
         * @param file_path File to import.
         * @param profile Parsing profile selected for the file.
         * @return True when parsing, storage, registration, and page refresh succeed.
         */
        auto import_file(const QUuid& view_id, const QString& file_path,
                         const LogParsingProfile& profile) -> bool;

        /**
         * @brief Imports multiple files synchronously into one new view.
         * @param file_paths Files to import.
         * @param profile Parsing profile selected for every file.
         * @return Created view id, or a null id when validation or storage fails.
         */
        auto import_files(const QVector<QString>& file_paths,
                          const LogParsingProfile& profile) -> QUuid;

        /**
         * @brief Enqueues one asynchronous import in a new view.
         *
         * Live tailing is enabled for the view but starts only after the import was stored
         * successfully, ensuring the selected profile is registered before appended records are
         * parsed.
         *
         * @param file_path File to import.
         * @param profile Parsing profile selected for the file.
         * @param batch_size Number of parsed entries per storage batch.
         * @return Created view id.
         */
        auto import_file_async(const QString& file_path, const LogParsingProfile& profile,
                               qsizetype batch_size) -> QUuid;

        /**
         * @brief Enqueues one asynchronous import in an existing view.
         * @param view_id Target view.
         * @param file_path File to import.
         * @param profile Parsing profile selected for the file.
         * @param batch_size Number of parsed entries per storage batch.
         * @return True when the request was registered and enqueued.
         */
        auto import_file_async(const QUuid& view_id, const QString& file_path,
                               const LogParsingProfile& profile, qsizetype batch_size) -> bool;

        /**
         * @brief Enqueues multiple asynchronous imports in one new view.
         *
         * Live tailing is enabled without registering files prematurely. Each file starts tailing
         * with its retained profile after its asynchronous import was stored successfully.
         *
         * @param file_paths Files to import.
         * @param profile Parsing profile selected for every file.
         * @param batch_size Number of parsed entries per storage batch.
         * @return Created view id, or a null id for an empty request.
         */
        auto import_files_async(const QVector<QString>& file_paths,
                                const LogParsingProfile& profile, qsizetype batch_size) -> QUuid;

        /**
         * @brief Enqueues a file that has already been registered during state restoration.
         * @param view_id Restored target view.
         * @param file_path Registered file path.
         * @param profile Restored parsing profile.
         * @param batch_size Number of parsed entries per storage batch.
         */
        auto enqueue_registered_file(const QUuid& view_id, const QString& file_path,
                                     const LogParsingProfile& profile,
                                     qsizetype batch_size) -> void;

        /**
         * @brief Cancels active and pending imports belonging to one view.
         * @param view_id View whose imports are cancelled.
         */
        auto cancel(const QUuid& view_id) -> void;

        /**
         * @brief Queues removal of stored history belonging to one view.
         * @param view_id View whose stored history is discarded.
         * @return True when asynchronous history cleanup was queued.
         */
        auto discard_history(const QUuid& view_id) -> bool;

        /**
         * @brief Stops accepting import callbacks and cancels every registered view.
         */
        auto shutdown() -> void;

    signals:
        /** @brief Reports asynchronous byte progress for one view. */
        void progress(const QUuid& view_id, qint64 bytes_read, qint64 total_bytes);

        /** @brief Reports completion after storage and visible-page refresh succeed. */
        void finished(const QUuid& view_id, const QString& file_path);

        /** @brief Reports parsing or storage failure for one file. */
        void error(const QUuid& view_id, const QString& file_path, const QString& message);

        /** @brief Requests lifecycle cleanup of a failed registered file. */
        void file_removal_requested(const QUuid& view_id, const QString& file_path);

        /** @brief Requests lifecycle cleanup of a failed newly created view. */
        void view_removal_requested(const QUuid& view_id);

    private:
        /** @brief Connects low-level ingest and writer events to the import workflow. */
        auto connect_workflow() -> void;

        /** @brief Returns whether a path names a readable regular file. */
        [[nodiscard]] auto is_readable_file(const QString& file_path) const -> bool;

        /** @brief Stores a parsing profile in its view context. */
        auto remember_profile(const QUuid& view_id, const QString& file_path,
                              const LogParsingProfile& profile) -> void;

        /** @brief Returns a retained file profile or the coordinator default. */
        [[nodiscard]] auto get_profile(const QUuid& view_id,
                                       const QString& file_path) const -> LogParsingProfile;

        /**
         * @brief Registers and enqueues one asynchronous file.
         * @param view_id Target view.
         * @param file_path File to enqueue.
         * @param profile Selected parsing profile.
         */
        auto enqueue(const QUuid& view_id, const QString& file_path,
                     const LogParsingProfile& profile) -> void;

        /** @brief Starts the next queue item and selects its view. */
        auto start_next(qsizetype batch_size) -> void;

        /** @brief Loads or refreshes the bounded page representing imported history. */
        auto refresh_visible_page(const QUuid& view_id) -> bool;

        /** @brief Completes one asynchronous import after ordered history writes finish. */
        auto handle_write_finished(const QUuid& operation_id, const QUuid& view_id,
                                   const QString& file_path, bool succeeded,
                                   const QString& error_message) -> void;

    private:
        bool m_shutting_down{false};
        LogParsingProfile m_default_profile;
        LogIngestController* m_ingest{nullptr};
        ViewRegistry* m_views{nullptr};
        LogHistoryService* m_history{nullptr};
        HistoryWriteService* m_history_writer{nullptr};
        LogPageCoordinator* m_pages{nullptr};
        LogQueryController* m_queries{nullptr};
        LiveTailingCoordinator* m_live_tailing{nullptr};
        QHash<QUuid, QUuid> m_operation_views;
        QSet<QUuid> m_failed_operations;
};
