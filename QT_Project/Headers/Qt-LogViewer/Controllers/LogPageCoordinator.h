#pragma once

#include <QMap>
#include <QObject>
#include <QUuid>

#include "Qt-LogViewer/Models/LogPageState.h"

class LogHistoryService;
class ViewRegistry;

/**
 * @file LogPageCoordinator.h
 * @brief Declares the coordinator for database-backed log pages.
 */

/**
 * @class LogPageCoordinator
 * @brief Loads filtered log pages into the existing per-view LogModel.
 */
class LogPageCoordinator final: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a page coordinator.
         * @param history_service Service used to query archived entries.
         * @param views Registry containing the target view contexts.
         * @param parent Optional QObject parent.
         */
        explicit LogPageCoordinator(LogHistoryService* history_service, ViewRegistry* views,
                                    QObject* parent = nullptr);

        /**
         * @brief Sets the query for a view and loads its first page.
         * @param view_id Target view.
         * @param query Query describing the result set.
         * @return True when the view and required services are available.
         */
        auto set_query(const QUuid& view_id, const LogQuery& query) -> bool;

        /**
         * @brief Sets the current page and reloads the view model.
         * @param view_id Target view.
         * @param page One-based page number.
         * @return True when page state exists for the view.
         */
        auto set_current_page(const QUuid& view_id, qsizetype page) -> bool;

        /**
         * @brief Sets the page size, selects page one, and reloads the view model.
         * @param view_id Target view.
         * @param page_size Positive number of entries per page.
         * @return True when page state exists and page_size is valid.
         */
        auto set_page_size(const QUuid& view_id, qsizetype page_size) -> bool;

        /**
         * @brief Reloads the current page for a view.
         * @param view_id Target view.
         * @return True when the view and its page state are available.
         */
        auto reload(const QUuid& view_id) -> bool;

        /**
         * @brief Returns the page state belonging to a view.
         * @param view_id Target view.
         * @return Page state, or nullptr when no query has been assigned.
         */
        [[nodiscard]] auto get_page_state(const QUuid& view_id) const -> const LogPageState*;

    signals:
        /**
         * @brief Emitted after a page has been loaded into a view model.
         * @param view_id Updated view.
         * @param current_page Current one-based page.
         * @param total_pages Total number of available pages.
         * @param total_entries Total number of matching entries.
         */
        void page_loaded(const QUuid& view_id, qsizetype current_page, qsizetype total_pages,
                         qsizetype total_entries);

    private:
        LogHistoryService* m_history_service;
        ViewRegistry* m_views;
        QMap<QUuid, LogPageState> m_page_states;
};
