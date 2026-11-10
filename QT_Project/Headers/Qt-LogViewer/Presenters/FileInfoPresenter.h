#pragma once

#include <QObject>
#include <QUuid>

class FileInfoWidget;
class LiveTailingCoordinator;
class LogCacheCatalog;
class LogHistoryService;
class ViewRegistry;

/**
 * @file FileInfoPresenter.h
 * @brief Declares the presenter that assembles active-view file metadata.
 */

/**
 * @class FileInfoPresenter
 * @brief Translates source, history, cache, profile and tail state into inspector snapshots.
 */
class FileInfoPresenter final: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs and connects the file-information presenter.
         * @param views Runtime view registry.
         * @param history Archived entry query service.
         * @param cache_catalog Persistent cache metadata catalog.
         * @param live_tailing Live-tail state owner.
         * @param widget Inspector widget receiving snapshots.
         * @param parent Optional QObject parent.
         */
        explicit FileInfoPresenter(ViewRegistry* views, LogHistoryService* history,
                                   LogCacheCatalog* cache_catalog,
                                   LiveTailingCoordinator* live_tailing, FileInfoWidget* widget,
                                   QObject* parent = nullptr);

        /** @brief Rebuilds snapshots for the active view. */
        auto refresh() -> void;

    private:
        ViewRegistry* m_views{nullptr};                   ///< Runtime source/profile state.
        LogHistoryService* m_history{nullptr};            ///< Archived entry queries.
        LogCacheCatalog* m_cache_catalog{nullptr};        ///< Bound generation metadata.
        LiveTailingCoordinator* m_live_tailing{nullptr};  ///< Tail state and append times.
        FileInfoWidget* m_widget{nullptr};                ///< Snapshot presentation target.
        QUuid m_active_view_id;                           ///< Currently inspected view.
        qsizetype m_selected_index{0};                    ///< Retained selector index.
};
