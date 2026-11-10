#pragma once

#include <QHash>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QUuid>
#include <atomic>
#include <memory>

#include "Qt-LogViewer/Models/LogPageState.h"

class LogHistoryService;
class ViewRegistry;
class QThread;

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

        /** @brief Cancels and joins active regular-expression queries. */
        ~LogPageCoordinator() override;

        /**
         * @brief Sets the query for a view and loads its first page.
         * @param view_id Target view.
         * @param query Query describing the result set.
         * @return True when the view and required services are available.
         */
        auto set_query(const QUuid& view_id, const LogQuery& query) -> bool;

        /**
         * @brief Changes result ordering and reloads page one without recounting unchanged rows.
         * @param view_id Target view.
         * @param sort_field Stable identifier of the field used for sorting.
         * @param sort_order Requested sort direction.
         * @return True when page state exists for the view.
         */
        auto set_sorting(const QUuid& view_id, const QString& sort_field,
                         Qt::SortOrder sort_order) -> bool;

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
         * @brief Applies query, page size, and requested page with one model reload.
         * @param view_id Target view.
         * @param query Query describing filtering and sorting.
         * @param page_size Positive number of entries per page.
         * @param current_page Requested one-based page number.
         * @return True when the complete state was applied and loaded.
         */
        auto apply_state(const QUuid& view_id, const LogQuery& query, qsizetype page_size,
                         qsizetype current_page) -> bool;

        /**
         * @brief Installs page state without querying storage or populating the model.
         * @param view_id Target view.
         * @param query Query describing filtering and sorting.
         * @param page_size Positive number of entries per page.
         * @param current_page Requested one-based page number.
         * @return True when empty deferred state was installed for the view.
         */
        auto prepare_state(const QUuid& view_id, const LogQuery& query, qsizetype page_size,
                           qsizetype current_page) -> bool;

        /**
         * @brief Reloads the current page for a view.
         * @param view_id Target view.
         * @return True when the view and its page state are available.
         */
        auto reload(const QUuid& view_id) -> bool;

        /**
         * @brief Refreshes the result count without replacing the visible page.
         * @param view_id Target view.
         * @return True when the view and its page state are available.
         */
        auto refresh_total_entries(const QUuid& view_id) -> bool;

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

        /**
         * @brief Emitted after pagination totals have changed without reloading the model.
         * @param view_id Updated view.
         * @param current_page Current one-based page.
         * @param total_pages Total number of available pages.
         * @param total_entries Total number of matching entries.
         */
        void page_state_updated(const QUuid& view_id, qsizetype current_page, qsizetype total_pages,
                                qsizetype total_entries);

        /**
         * @brief Emitted after a query or its underlying entries changed aggregate results.
         *
         * Pure page-number, page-size, and sorting changes do not emit this signal because they
         * preserve the result set, its facets, and its total entry count.
         *
         * @param view_id View whose result-set aggregates may need presentation updates.
         */
        void result_set_changed(const QUuid& view_id);

    private:
        /**
         * @brief Loads the selected page while preserving the known result count.
         * @param view_id Target view.
         * @return True when the view and its page state are available.
         */
        auto load_current_page(const QUuid& view_id) -> bool;

        /**
         * @brief Starts a replaceable background regular-expression page query.
         * @param view_id Target view.
         * @param notify_result_set_changed True when aggregates changed with the query.
         * @param preserve_visible_entries True when only pagination totals may be updated.
         * @return True when a background operation was started.
         */
        auto load_regex_page_async(const QUuid& view_id, bool notify_result_set_changed,
                                   bool preserve_visible_entries) -> bool;

        /**
         * @brief Cooperatively cancels the active regular-expression query for a view.
         * @param view_id Target view.
         */
        auto cancel_regex_query(const QUuid& view_id) -> void;

        LogHistoryService* m_history_service;
        ViewRegistry* m_views;
        QMap<QUuid, LogPageState> m_page_states;
        /** @brief Monotonic request revisions used to discard stale results. */
        QHash<QUuid, quint64> m_request_revisions;
        /** @brief Cooperative cancellation states grouped by view. */
        QHash<QUuid, std::shared_ptr<std::atomic_bool>> m_regex_cancellations;
        /** @brief Active worker objects joined during destruction. */
        QSet<QThread*> m_regex_workers;
};
