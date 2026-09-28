#pragma once

#include <QModelIndex>
#include <QObject>
#include <QUuid>

class LogViewWidget;
class LogViewerController;
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
         * @param controller Controller providing view-specific operations and state.
         * @param widget Widget displaying the runtime view.
         * @param view_id Immutable identifier of the bound runtime view.
         * @param state Initial typed presentation state.
         * @param parent QObject owning the presenter, normally @p widget.
         */
        explicit LogViewPresenter(LogViewerController* controller, LogViewWidget* widget,
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
        LogViewerController* m_controller{nullptr};
        LogViewWidget* m_widget{nullptr};
        QUuid m_view_id;
};
