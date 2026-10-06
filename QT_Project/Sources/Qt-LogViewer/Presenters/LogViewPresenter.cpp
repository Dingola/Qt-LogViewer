#include "Qt-LogViewer/Presenters/LogViewPresenter.h"

#include <QHeaderView>
#include <QMap>
#include <QSet>
#include <QSignalBlocker>
#include <QVector>

#include "Qt-LogViewer/Controllers/FilterCoordinator.h"
#include "Qt-LogViewer/Controllers/LiveTailingCoordinator.h"
#include "Qt-LogViewer/Controllers/LogImportCoordinator.h"
#include "Qt-LogViewer/Controllers/LogPageCoordinator.h"
#include "Qt-LogViewer/Controllers/LogQueryController.h"
#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/ViewLifecycleCoordinator.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogPageState.h"
#include "Qt-LogViewer/Models/LogQuery.h"
#include "Qt-LogViewer/Models/SessionTypes.h"
#include "Qt-LogViewer/Services/LogHistoryService.h"
#include "Qt-LogViewer/Views/App/LogTableView.h"
#include "Qt-LogViewer/Views/App/LogViewWidget.h"

/**
 * @file LogViewPresenter.cpp
 * @brief Implements the one-to-one binding between a log view widget and its
 * runtime view.
 */

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
LogViewPresenter::LogViewPresenter(ViewRegistry* views, FilterCoordinator* filters,
                                   LogHistoryService* history, LogPageCoordinator* pages,
                                   LogQueryController* queries, LogImportCoordinator* imports,
                                   ViewLifecycleCoordinator* lifecycle,
                                   LiveTailingCoordinator* live_tailing, LogViewWidget* widget,
                                   const QUuid& view_id, const SessionViewState& state,
                                   QObject* parent)
    : QObject(parent),
      m_views(views),
      m_filters(filters),
      m_history(history),
      m_pages(pages),
      m_queries(queries),
      m_imports(imports),
      m_lifecycle(lifecycle),
      m_live_tailing(live_tailing),
      m_widget(widget),
      m_view_id(view_id)
{
    if (m_views != nullptr && m_filters != nullptr && m_history != nullptr && m_pages != nullptr &&
        m_queries != nullptr && m_imports != nullptr && m_lifecycle != nullptr &&
        m_live_tailing != nullptr && m_widget != nullptr && !m_view_id.isNull())
    {
        m_widget->set_view_id(m_view_id);
        LogViewContext* context = m_views->get_context(m_view_id);
        m_widget->set_model(context != nullptr ? context->get_model() : nullptr);

        const LogPageState* page_state = m_pages->get_page_state(m_view_id);
        if (page_state == nullptr)
        {
            m_queries->apply_view_state(m_view_id, state);
            page_state = m_pages->get_page_state(m_view_id);
        }

        const bool deferred_restore = !state.loaded_files.isEmpty() && page_state != nullptr &&
                                      page_state->get_total_entries() == 0;
        refresh_query_presentation();
        refresh_filter_presentation(!deferred_restore);
        m_widget->set_view_file_paths(m_views->get_file_paths(m_view_id));

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
                    if (!m_queries->set_sort(m_view_id, column, order))
                    {
                        restore_sort_indicator();
                    }
                });
    }

    connect(m_widget, &LogViewWidget::current_row_changed, this,
            [this](const QModelIndex& current, const QModelIndex&) {
                emit current_row_changed(m_view_id, current);
            });
    connect(m_widget, &LogViewWidget::app_filter_changed, this,
            [this](const QString& app_name) { m_queries->set_app_name(m_view_id, app_name); });
    connect(m_widget, &LogViewWidget::log_level_filter_changed, this,
            [this](const QSet<QString>& levels) { m_queries->set_log_levels(m_view_id, levels); });
    connect(m_widget, &LogViewWidget::toggle_visibility_requested, this,
            [this](const QString& file_path) {
                m_queries->toggle_file_visibility(m_view_id, file_path);
                refresh_query_presentation();
            });
    connect(m_widget, &LogViewWidget::show_only_file_requested, this,
            [this](const QString& file_path) {
                m_queries->set_show_only_file(m_view_id, file_path);
                refresh_query_presentation();
            });
    connect(m_widget, &LogViewWidget::remove_file_requested, this,
            [this](const QString& file_path) { m_lifecycle->remove_file(m_view_id, file_path); });
    connect(m_widget, &LogViewWidget::live_tailing_toggled, this,
            [this](bool enabled) { m_live_tailing->set_enabled(m_view_id, enabled); });
}

/**
 * @brief Connects controller updates relevant to the bound view.
 */
auto LogViewPresenter::connect_view_updates() -> void
{
    connect(m_views, &ViewRegistry::view_file_paths_changed, this,
            [this](const QUuid& view_id, const QVector<QString>& file_paths) {
                if (view_id == m_view_id)
                {
                    m_widget->set_view_file_paths(file_paths);
                }
            });
    connect(m_pages, &LogPageCoordinator::page_loaded, this,
            [this](const QUuid& view_id, qsizetype, qsizetype, qsizetype) {
                if (view_id == m_view_id)
                {
                    refresh_filter_presentation(true);
                    refresh_query_presentation();
                }
            });
    connect(m_pages, &LogPageCoordinator::page_state_updated, this,
            [this](const QUuid& view_id, qsizetype, qsizetype, qsizetype) {
                if (view_id == m_view_id)
                {
                    refresh_filter_presentation(true);
                }
            });
    connect(m_imports, &LogImportCoordinator::finished, this,
            [this](const QUuid& view_id, const QString&) {
                if (view_id == m_view_id)
                {
                    refresh_filter_presentation(true);
                    refresh_query_presentation();
                    m_widget->set_view_file_paths(m_views->get_file_paths(m_view_id));
                }
            });
    connect(m_views, &ViewRegistry::view_removed, this, [this](const QUuid& view_id) {
        if (view_id == m_view_id)
        {
            m_widget->deleteLater();
        }
    });
}

/**
 * @brief Refreshes filter controls and optionally queries persistent values and counts.
 * @param include_history True to query stored application names and level counts.
 */
auto LogViewPresenter::refresh_filter_presentation(bool include_history) -> void
{
    const QSignalBlocker blocker(m_widget);
    QSet<QString> app_names;
    QMap<QString, qsizetype> history_counts;
    if (include_history)
    {
        app_names = m_history->get_distinct_values(m_view_id, LogField::AppName);
        history_counts = m_history->get_log_level_counts(m_queries->create_query(m_view_id));
    }

    m_widget->set_app_names(app_names);
    m_widget->set_current_app_name_filter(m_filters->get_app_name(m_view_id));
    m_widget->set_available_log_levels(FilterCoordinator::get_available_log_levels());
    m_widget->set_log_levels(m_filters->get_log_levels(m_view_id));

    QMap<QString, int> level_counts;
    for (auto iterator = history_counts.cbegin(); iterator != history_counts.cend(); ++iterator)
    {
        level_counts.insert(iterator.key(), static_cast<int>(iterator.value()));
    }
    m_widget->set_log_level_counts(level_counts);
    m_widget->set_live_tailing_enabled(m_live_tailing->is_enabled(m_view_id));
}

/**
 * @brief Refreshes search, file visibility, and sorting presentation.
 */
auto LogViewPresenter::refresh_query_presentation() -> void
{
    const LogQuery query = m_queries->create_query(m_view_id);
    m_widget->set_search_highlight(query.search_text, m_filters->get_search_field(m_view_id),
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
    LogViewContext* context = m_views->get_context(m_view_id);
    LogModel* model = context != nullptr ? context->get_model() : nullptr;

    if (header != nullptr)
    {
        const LogQuery query = m_queries->create_query(m_view_id);
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
