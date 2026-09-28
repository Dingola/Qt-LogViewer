#pragma once

#include <QModelIndex>
#include <QObject>
#include <QUuid>

class LogViewWidget;
class FilterCoordinator;
class LiveTailingCoordinator;
class LogHistoryService;
class LogImportCoordinator;
class LogPageCoordinator;
class LogQueryController;
class ViewLifecycleCoordinator;
class ViewRegistry;
struct SessionViewState;

/**
 * @file LogViewPresenter.h
 * @brief Declares the presenter that binds one log view widget to one runtime view.
 */

/**
 * @class LogViewPresenter
 * @brief Coordinates presentation and user actions for exactly one log view.
 *
 * The presenter owns no application state. It translates widget signals into view-specific
 * controller operations and refreshes only the widget identified by its immutable view ID.
 * Its QObject parent is expected to be the bound widget so both share the same lifetime.
 */
class LogViewPresenter: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Binds a widget to an existing runtime view.
         * @param views Registry providing the bound model and file paths.
         * @param filters Per-view filter state.
         * @param history Persistent history used for filter values and counts.
         * @param pages Per-view pagination state and update signals.
         * @param queries Query operations triggered by the widget.
         * @param imports Import completion notifications.
         * @param lifecycle View and file removal operations.
         * @param live_tailing Live-tailing state and operations.
         * @param widget Widget displaying the runtime view.
         * @param view_id Immutable identifier of the bound runtime view.
         * @param state Initial typed presentation state.
         * @param parent QObject owning the presenter, normally @p widget.
         */
        explicit LogViewPresenter(ViewRegistry* views, FilterCoordinator* filters,
                                  LogHistoryService* history, LogPageCoordinator* pages,
                                  LogQueryController* queries, LogImportCoordinator* imports,
                                  ViewLifecycleCoordinator* lifecycle,
                                  LiveTailingCoordinator* live_tailing, LogViewWidget* widget,
                                  const QUuid& view_id, const SessionViewState& state,
                                  QObject* parent = nullptr);

    signals:
        /**
         * @brief Forwards a row selection for presentation outside the individual log tab.
         * @param view_id Originating view identifier.
         * @param current Newly selected model index.
         */
        void current_row_changed(const QUuid& view_id, const QModelIndex& current);

    private:
        /** @brief Connects user actions emitted by the bound widget. */
        auto connect_widget_actions() -> void;

        /** @brief Connects controller updates relevant to the bound view. */
        auto connect_view_updates() -> void;

        /** @brief Refreshes filter controls and counts from current runtime state. */
        auto refresh_filter_presentation() -> void;

        /** @brief Refreshes search, file visibility, and sorting presentation. */
        auto refresh_query_presentation() -> void;

        /** @brief Refreshes the sort indicator from the current runtime query. */
        auto restore_sort_indicator() -> void;

    private:
        ViewRegistry* m_views{nullptr};
        FilterCoordinator* m_filters{nullptr};
        LogHistoryService* m_history{nullptr};
        LogPageCoordinator* m_pages{nullptr};
        LogQueryController* m_queries{nullptr};
        LogImportCoordinator* m_imports{nullptr};
        ViewLifecycleCoordinator* m_lifecycle{nullptr};
        LiveTailingCoordinator* m_live_tailing{nullptr};
        LogViewWidget* m_widget{nullptr};
        QUuid m_view_id;
};
