#pragma once

#include <QObject>
#include <QString>
#include <QUuid>

#include "Qt-LogViewer/Models/LogFileInfo.h"

class FileCatalogController;
class FilterCoordinator;
class HistoryWriteService;
class LiveTailingCoordinator;
class LogHistoryService;
class LogImportCoordinator;
class LogPageCoordinator;
class LogQueryController;
class ViewRegistry;

/**
 * @file ViewLifecycleCoordinator.h
 * @brief Declares ordered cleanup operations for log views and their files.
 */

/**
 * @class ViewLifecycleCoordinator
 * @brief Owns the application workflow for closing views and removing registered files.
 *
 * Every operation coordinates import cancellation, live-tailing shutdown, pending writer cleanup,
 * persisted-history removal, filter repair, and registry mutation. The coordinator does not own
 * its collaborators.
 */
class ViewLifecycleCoordinator final: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs the lifecycle coordinator from the affected application components.
         * @param views Registry containing the views and file registrations.
         * @param filters Filter state repaired after file removal.
         * @param catalog File catalog updated by global removal.
         * @param imports Import workflow cancelled before view destruction.
         * @param live_tailing Live-tail registrations stopped before removal.
         * @param history Persisted history removed during cleanup.
         * @param history_writer Ordered writer receiving discard operations.
         * @param pages Page state used to detect views with active queries.
         * @param queries Query component used to refresh remaining files.
         * @param parent Optional QObject parent.
         */
        explicit ViewLifecycleCoordinator(
            ViewRegistry* views, FilterCoordinator* filters, FileCatalogController* catalog,
            LogImportCoordinator* imports, LiveTailingCoordinator* live_tailing,
            LogHistoryService* history, HistoryWriteService* history_writer,
            LogPageCoordinator* pages, LogQueryController* queries, QObject* parent = nullptr);

        /**
         * @brief Closes one view after cancelling and cleaning all related work.
         * @param view_id View to close.
         * @return True when an existing view was removed.
         */
        auto close_view(const QUuid& view_id) -> bool;

        /**
         * @brief Closes every registered view through the same ordered cleanup path.
         */
        auto close_all_views() -> void;

        /**
         * @brief Removes a file from every view and from the global file catalog.
         * @param file File metadata identifying the path to remove.
         */
        auto remove_file(const LogFileInfo& file) -> void;

        /**
         * @brief Removes one file registration from a specific view.
         * @param view_id View containing the file.
         * @param file_path Registered file path.
         * @return True when the file existed and was removed.
         */
        auto remove_file(const QUuid& view_id, const QString& file_path) -> bool;

    private:
        /**
         * @brief Removes one registered file while optionally retaining an empty view temporarily.
         * @param view_id View containing the file.
         * @param file_path Registered file path.
         * @param close_empty_view Whether an emptied view is closed immediately.
         * @return True when the file existed and was removed.
         */
        auto remove_registered_file(const QUuid& view_id, const QString& file_path,
                                    bool close_empty_view) -> bool;

    private:
        ViewRegistry* m_views{nullptr};
        FilterCoordinator* m_filters{nullptr};
        FileCatalogController* m_catalog{nullptr};
        LogImportCoordinator* m_imports{nullptr};
        LiveTailingCoordinator* m_live_tailing{nullptr};
        LogHistoryService* m_history{nullptr};
        HistoryWriteService* m_history_writer{nullptr};
        LogPageCoordinator* m_pages{nullptr};
        LogQueryController* m_queries{nullptr};
};
