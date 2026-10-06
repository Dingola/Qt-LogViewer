#pragma once

#include <QObject>
#include <QSet>
#include <QString>
#include <QUuid>
#include <Qt>

#include "Qt-LogViewer/Models/LogQuery.h"
#include "Qt-LogViewer/Models/SearchFields.h"
#include "Qt-LogViewer/Models/SessionTypes.h"

class FilterCoordinator;
class LogPageCoordinator;
class ViewRegistry;

/**
 * @file LogQueryController.h
 * @brief Declares complete query-update operations for individual log views.
 */

/**
 * @class LogQueryController
 * @brief Coordinates filter state with database-backed paging.
 *
 * Each mutating operation updates the filter or sorting state, rebuilds the complete query,
 * resets paging where required, and performs exactly one visible page reload.
 */
class LogQueryController final: public QObject
{
    public:
        /**
         * @brief Constructs a query controller from the existing state coordinators.
         * @param filters Coordinator that owns per-view filter state.
         * @param pages Coordinator that owns per-view paging state.
         * @param views Registry used to access view contexts and column definitions.
         * @param parent Optional QObject parent.
         */
        explicit LogQueryController(FilterCoordinator* filters, LogPageCoordinator* pages,
                                    ViewRegistry* views, QObject* parent = nullptr);

        /**
         * @brief Sets the application-name filter and reloads page one.
         * @param view_id Target view.
         * @param app_name Application name, or an empty string for all applications.
         * @return True when the updated query was loaded.
         */
        auto set_app_name(const QUuid& view_id, const QString& app_name) -> bool;

        /**
         * @brief Sets the log-level filter and reloads page one.
         * @param view_id Target view.
         * @param levels Included log levels, or an empty set for all levels.
         * @return True when the updated query was loaded.
         */
        auto set_log_levels(const QUuid& view_id, const QSet<QString>& levels) -> bool;

        /**
         * @brief Sets the text-search filter and reloads page one.
         * @param view_id Target view.
         * @param text Search text or regular expression.
         * @param field Field selected for searching.
         * @param use_regex Whether text is interpreted as a regular expression.
         * @return True when the updated query was loaded.
         */
        auto set_search(const QUuid& view_id, const QString& text, SearchField field,
                        bool use_regex) -> bool;

        /**
         * @brief Applies a show-only-file filter and reloads page one.
         * @param view_id Target view.
         * @param file_path File to show exclusively, or an empty path to reset the filter.
         * @return True when the updated query was loaded.
         */
        auto set_show_only_file(const QUuid& view_id, const QString& file_path) -> bool;

        /**
         * @brief Toggles one file's visibility and reloads page one.
         * @param view_id Target view.
         * @param file_path File whose visibility changes.
         * @return True when the updated query was loaded.
         */
        auto toggle_file_visibility(const QUuid& view_id, const QString& file_path) -> bool;

        /**
         * @brief Hides one file and reloads page one.
         * @param view_id Target view.
         * @param file_path File to hide.
         * @return True when the updated query was loaded.
         */
        auto hide_file(const QUuid& view_id, const QString& file_path) -> bool;

        /**
         * @brief Changes sorting and reloads page one.
         * @param view_id Target view.
         * @param column Sortable model column.
         * @param order Requested sort direction.
         * @return True when the column is sortable and the query was loaded.
         */
        auto set_sort(const QUuid& view_id, int column, Qt::SortOrder order) -> bool;

        /**
         * @brief Rebuilds the query from current filter state and reloads page one.
         * @param view_id Target view.
         * @return True when the query was loaded.
         */
        auto reload_query(const QUuid& view_id) -> bool;

        /**
         * @brief Refreshes a view after new entries were appended to its stored history.
         *
         * Page one is reloaded so newest-first results become visible. A historical page keeps
         * its current rows and only receives updated result totals.
         *
         * @param view_id Target view.
         * @return True when the view has an active page state that could be refreshed.
         */
        auto refresh_after_entries_appended(const QUuid& view_id) -> bool;

        /**
         * @brief Applies filters, sorting, page size, and page number with one reload.
         * @param view_id Target view.
         * @param state Complete saved state of the view.
         * @return True when the complete state was applied and loaded.
         */
        auto apply_view_state(const QUuid& view_id, const SessionViewState& state) -> bool;

        /**
         * @brief Restores query state without reading persistent entries yet.
         * @param view_id Target view.
         * @param state Complete saved state of the view.
         * @return True when filters and empty deferred paging state were installed.
         */
        auto prepare_view_state(const QUuid& view_id, const SessionViewState& state) -> bool;

        /**
         * @brief Builds a query from the current filter and sorting state.
         * @param view_id Source view.
         * @return Query representing the view state.
         */
        [[nodiscard]] auto create_query(const QUuid& view_id) const -> LogQuery;

    private:
        FilterCoordinator* m_filters{nullptr};
        LogPageCoordinator* m_pages{nullptr};
        ViewRegistry* m_views{nullptr};
};
