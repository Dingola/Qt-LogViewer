#include "Qt-LogViewer/Presenters/LogViewPresenter.h"

#include <QHeaderView>
#include <QMap>
#include <QSet>
#include <QSignalBlocker>
#include <QVector>

#include "Qt-LogViewer/Controllers/LogViewerController.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogQuery.h"
#include "Qt-LogViewer/Models/SessionTypes.h"
#include "Qt-LogViewer/Views/App/LogTableView.h"
#include "Qt-LogViewer/Views/App/LogViewWidget.h"

/**
 * @file LogViewPresenter.cpp
 * @brief Implements the one-to-one binding between a log view widget and its
 * runtime view.
 */

/**
 * @brief Binds a widget to an existing runtime view.
 * @param controller Controller providing view-specific operations and state.
 * @param widget Widget displaying the runtime view.
 * @param view_id Immutable identifier of the bound runtime view.
 * @param state Initial typed presentation state.
 * @param parent QObject owning the presenter, normally @p widget.
 */
LogViewPresenter::LogViewPresenter(LogViewerController* controller, LogViewWidget* widget,
                                   const QUuid& view_id, const SessionViewState& state,
                                   QObject* parent)
    : QObject(parent), m_controller(controller), m_widget(widget), m_view_id(view_id)
{
    if (m_controller != nullptr && m_widget != nullptr && !m_view_id.isNull())
    {
        m_widget->set_view_id(m_view_id);
        m_widget->set_model(m_controller->get_log_model(m_view_id));

        if (m_controller->get_page_state(m_view_id) == nullptr)
        {
            m_controller->apply_view_query_state(m_view_id, state);
        }

        refresh_query_presentation();
        refresh_filter_presentation();
        m_widget->set_view_file_paths(m_controller->get_view_file_paths(m_view_id));

        connect_widget_actions();
        connect_view_updates();
    }
}

/**
 * @brief Connects user actions emitted by the bound widget.
 */
auto LogViewPresenter::connect_widget_actions() -> void
{
    QHeaderView* header = m_widget->get_table_view()->horizontalHeader();

    if (header != nullptr)
    {
        connect(header, &QHeaderView::sortIndicatorChanged, this,
                [this](int column, Qt::SortOrder order) {
                    if (!m_controller->set_page_sort(m_view_id, column, order))
                    {
                        restore_sort_indicator();
                    }
                });
    }

    connect(m_widget, &LogViewWidget::current_row_changed, this,
            [this](const QModelIndex& current, const QModelIndex&) {
                emit current_row_changed(m_view_id, current);
            });
    connect(m_widget, &LogViewWidget::app_filter_changed, this, [this](const QString& app_name) {
        m_controller->set_app_name_filter(m_view_id, app_name);
    });
    connect(m_widget, &LogViewWidget::log_level_filter_changed, this,
            [this](const QSet<QString>& levels) {
                m_controller->set_log_level_filters(m_view_id, levels);
            });
    connect(m_widget, &LogViewWidget::toggle_visibility_requested, this,
            [this](const QString& file_path) {
                m_controller->toggle_file_visibility(m_view_id, file_path);
                refresh_query_presentation();
            });
    connect(m_widget, &LogViewWidget::show_only_file_requested, this,
            [this](const QString& file_path) {
                m_controller->set_show_only_file(m_view_id, file_path);
                refresh_query_presentation();
            });
    connect(
        m_widget, &LogViewWidget::remove_file_requested, this,
        [this](const QString& file_path) { m_controller->remove_log_file(m_view_id, file_path); });
    connect(m_widget, &LogViewWidget::live_tailing_toggled, this,
            [this](bool enabled) { m_controller->set_live_tailing_enabled(m_view_id, enabled); });
}

/**
 * @brief Connects controller updates relevant to the bound view.
 */
auto LogViewPresenter::connect_view_updates() -> void
{
    connect(m_controller, &LogViewerController::view_file_paths_changed, this,
            [this](const QUuid& view_id, const QVector<QString>& file_paths) {
                if (view_id == m_view_id)
                {
                    m_widget->set_view_file_paths(file_paths);
                }
            });
    connect(m_controller, &LogViewerController::page_loaded, this,
            [this](const QUuid& view_id, qsizetype, qsizetype, qsizetype) {
                if (view_id == m_view_id)
                {
                    refresh_filter_presentation();
                    refresh_query_presentation();
                }
            });
    connect(m_controller, &LogViewerController::page_state_updated, this,
            [this](const QUuid& view_id, qsizetype, qsizetype, qsizetype) {
                if (view_id == m_view_id)
                {
                    refresh_filter_presentation();
                }
            });
    connect(m_controller, &LogViewerController::loading_finished, this,
            [this](const QUuid& view_id, const QString&) {
                if (view_id == m_view_id)
                {
                    refresh_filter_presentation();
                    refresh_query_presentation();
                    m_widget->set_view_file_paths(m_controller->get_view_file_paths(m_view_id));
                    m_widget->auto_resize_columns();
                }
            });
    connect(m_controller, &LogViewerController::view_removed, this, [this](const QUuid& view_id) {
        if (view_id == m_view_id)
        {
            m_widget->deleteLater();
        }
    });
}

/**
 * @brief Refreshes filter controls and counts from current runtime state.
 */
auto LogViewPresenter::refresh_filter_presentation() -> void
{
    const QSignalBlocker blocker(m_widget);
    m_widget->set_app_names(m_controller->get_app_names(m_view_id));
    m_widget->set_current_app_name_filter(m_controller->get_app_name_filter(m_view_id));
    m_widget->set_available_log_levels(m_controller->get_available_log_levels(m_view_id));
    m_widget->set_log_levels(m_controller->get_log_level_filters(m_view_id));
    m_widget->set_log_level_counts(m_controller->get_log_level_counts(m_view_id));
    m_widget->set_live_tailing_enabled(m_controller->get_live_tailing_enabled(m_view_id));
}

/**
 * @brief Refreshes search, file visibility, and sorting presentation.
 */
auto LogViewPresenter::refresh_query_presentation() -> void
{
    const LogQuery query = m_controller->create_page_query(m_view_id);
    m_widget->set_search_highlight(query.search_text, m_controller->get_search_field(m_view_id),
                                   query.use_regex);
    m_widget->set_file_visibility_state(query.show_only_file, query.hidden_files);
    restore_sort_indicator();
}

/**
 * @brief Refreshes the sort indicator from the current runtime query.
 */
auto LogViewPresenter::restore_sort_indicator() -> void
{
    QHeaderView* header = m_widget->get_table_view()->horizontalHeader();
    LogModel* model = m_controller->get_log_model(m_view_id);

    if (header != nullptr)
    {
        const LogQuery query = m_controller->create_page_query(m_view_id);
        const int sort_column = model != nullptr ? model->find_column(query.sort_field) : -1;
        const bool blocked = header->blockSignals(true);
        header->setSortIndicatorShown(sort_column >= 0);

        if (sort_column >= 0)
        {
            header->setSortIndicator(sort_column, query.sort_order);
        }

        header->blockSignals(blocked);
    }
}
