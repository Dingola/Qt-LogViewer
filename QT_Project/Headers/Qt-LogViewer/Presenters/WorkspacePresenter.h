#pragma once

#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QUuid>

#include "Qt-LogViewer/Models/SearchFields.h"

class DockController;
class FilterCoordinator;
class LogHistoryService;
class LogImportCoordinator;
class LogPageCoordinator;
class LogQueryController;
class LogFilterBarWidget;
class LogLevelPieChartWidget;
class LogTabWidget;
class LogViewPresenter;
class ViewLifecycleCoordinator;
class ViewRegistry;
class PaginationWidget;
class QPlainTextEdit;
class QStackedWidget;
class SessionController;
class SnapshotTransitionAnimator;
class StackedWidgetTransition;

/**
 * @file WorkspacePresenter.h
 * @brief Declares presentation coordination shared by all log-view tabs.
 */

/**
 * @class WorkspacePresenter
 * @brief Coordinates the active tab and shared workspace controls.
 *
 * The presenter owns no widgets or application state. It maps the active
 * LogViewWidget to the shared filter bar, pagination, detail view, level chart,
 * and start-page presentation. Search operations accept an explicit view ID so
 * their UI source can later move into each log view.
 */
class WorkspacePresenter: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs and connects the shared workspace presentation.
         * @param views Registry providing active views and models.
         * @param filters Per-view filter state.
         * @param history Persistent history used for filter values and counts.
         * @param pages Per-view pagination state and update signals.
         * @param queries Query operations triggered by shared controls.
         * @param imports Import completion notifications.
         * @param lifecycle View-closing operations.
         * @param session_controller Session state used for start-page selection.
         * @param tab_widget Workspace tab container.
         * @param filter_bar Shared filter and search controls.
         * @param pagination Shared pagination controls.
         * @param details_text Shared selected-entry details output.
         * @param level_chart Shared log-level distribution widget.
         * @param central_stack Stack containing workspace and start-page pages.
         * @param dock_controller Controller used to suspend docks on the start page.
         * @param parent Optional QObject parent.
         */
        explicit WorkspacePresenter(ViewRegistry* views, FilterCoordinator* filters,
                                    LogHistoryService* history, LogPageCoordinator* pages,
                                    LogQueryController* queries, LogImportCoordinator* imports,
                                    ViewLifecycleCoordinator* lifecycle,
                                    SessionController* session_controller, LogTabWidget* tab_widget,
                                    LogFilterBarWidget* filter_bar, PaginationWidget* pagination,
                                    QPlainTextEdit* details_text,
                                    LogLevelPieChartWidget* level_chart,
                                    QStackedWidget* central_stack, DockController* dock_controller,
                                    QObject* parent = nullptr);

        /**
         * @brief Connects one per-view presenter to shared detail presentation.
         * @param presenter Presenter associated with a log tab.
         */
        auto bind_log_view_presenter(LogViewPresenter* presenter) -> void;

        /** @brief Refreshes all shared controls from the active log tab. */
        auto refresh_active_view() -> void;

        /** @brief Refreshes shared pagination from the active log tab. */
        auto refresh_pagination() -> void;

        /**
         * @brief Refreshes start-page and dock visibility from current session state.
         */
        auto refresh_start_page() -> void;

        /**
         * @brief Selects either the workspace or start-page presentation.
         * @param session_active True when a session workspace is available.
         */
        auto set_session_active(bool session_active) -> void;

        /** @brief Clears shared controls after the workspace is closed. */
        auto reset_presentation() -> void;

        /**
         * @brief Applies a search to one explicit view.
         * @param view_id Target view identifier.
         * @param text Search text or regular expression.
         * @param field Field selected for searching.
         * @param use_regex True when regular-expression matching is enabled.
         */
        auto apply_search(const QUuid& view_id, const QString& text, SearchField field,
                          bool use_regex) -> void;

    private:
        /**
         * @brief Changes the active log table page through a directional snapshot transition.
         * @param page Requested one-based page number.
         */
        auto navigate_to_page(int page) -> void;

        /** @brief Connects shared widgets to active-view operations. */
        auto connect_workspace_actions() -> void;

        /** @brief Connects runtime and session changes to presentation refreshes. */
        auto connect_workspace_updates() -> void;

        /**
         * @brief Returns the view ID represented by the selected tab.
         * @return Active log-view ID, or a null ID for non-log tabs.
         */
        [[nodiscard]] auto get_active_view_id() const -> QUuid;

        /**
         * @brief Updates shared page and level-count presentation for one view.
         * @param view_id View whose query state changed.
         * @param current_page Current one-based page.
         * @param total_pages Total available pages.
         */
        auto update_page_state(const QUuid& view_id, qsizetype current_page,
                               qsizetype total_pages) -> void;

        /**
         * @brief Displays the selected row for its originating view.
         * @param view_id Originating view identifier.
         * @param current Selected model index.
         */
        auto update_log_details(const QUuid& view_id, const QModelIndex& current) -> void;

        ViewRegistry* m_views{nullptr};
        FilterCoordinator* m_filters{nullptr};
        LogHistoryService* m_history{nullptr};
        LogPageCoordinator* m_pages{nullptr};
        LogQueryController* m_queries{nullptr};
        LogImportCoordinator* m_imports{nullptr};
        ViewLifecycleCoordinator* m_lifecycle{nullptr};
        SessionController* m_session_controller{nullptr};
        LogTabWidget* m_tab_widget{nullptr};
        LogFilterBarWidget* m_filter_bar{nullptr};
        PaginationWidget* m_pagination{nullptr};
        QPlainTextEdit* m_details_text{nullptr};
        LogLevelPieChartWidget* m_level_chart{nullptr};
        QStackedWidget* m_central_stack{nullptr};
        DockController* m_dock_controller{nullptr};
        StackedWidgetTransition* m_page_transition{nullptr};
        SnapshotTransitionAnimator* m_table_transition{nullptr};
};
