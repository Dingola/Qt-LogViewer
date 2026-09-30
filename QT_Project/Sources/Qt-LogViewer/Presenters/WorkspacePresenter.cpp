#include "Qt-LogViewer/Presenters/WorkspacePresenter.h"

#include <QMap>
#include <QPlainTextEdit>
#include <QSet>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QVector>

#include "Qt-LogViewer/Animations/SnapshotTransitionAnimator.h"
#include "Qt-LogViewer/Animations/StackedWidgetTransition.h"
#include "Qt-LogViewer/Controllers/DockController.h"
#include "Qt-LogViewer/Controllers/FilterCoordinator.h"
#include "Qt-LogViewer/Controllers/LogImportCoordinator.h"
#include "Qt-LogViewer/Controllers/LogPageCoordinator.h"
#include "Qt-LogViewer/Controllers/LogQueryController.h"
#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/SessionController.h"
#include "Qt-LogViewer/Controllers/ViewLifecycleCoordinator.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogPageState.h"
#include "Qt-LogViewer/Presenters/LogViewPresenter.h"
#include "Qt-LogViewer/Services/LogHistoryService.h"
#include "Qt-LogViewer/Views/App/LogFilterBarWidget.h"
#include "Qt-LogViewer/Views/App/LogLevelPieChartWidget.h"
#include "Qt-LogViewer/Views/App/LogTabWidget.h"
#include "Qt-LogViewer/Views/App/LogTableView.h"
#include "Qt-LogViewer/Views/App/LogViewWidget.h"
#include "Qt-LogViewer/Views/Shared/PaginationWidget.h"
#include "Qt-LogViewer/Views/Shared/TabWidget.h"

/**
 * @file WorkspacePresenter.cpp
 * @brief Implements presentation coordination shared by all log-view tabs.
 */

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
WorkspacePresenter::WorkspacePresenter(
    ViewRegistry* views, FilterCoordinator* filters, LogHistoryService* history,
    LogPageCoordinator* pages, LogQueryController* queries, LogImportCoordinator* imports,
    ViewLifecycleCoordinator* lifecycle, SessionController* session_controller,
    LogTabWidget* tab_widget, LogFilterBarWidget* filter_bar, PaginationWidget* pagination,
    QPlainTextEdit* details_text, LogLevelPieChartWidget* level_chart,
    QStackedWidget* central_stack, DockController* dock_controller, QObject* parent)
    : QObject(parent),
      m_views(views),
      m_filters(filters),
      m_history(history),
      m_pages(pages),
      m_queries(queries),
      m_imports(imports),
      m_lifecycle(lifecycle),
      m_session_controller(session_controller),
      m_tab_widget(tab_widget),
      m_filter_bar(filter_bar),
      m_pagination(pagination),
      m_details_text(details_text),
      m_level_chart(level_chart),
      m_central_stack(central_stack),
      m_dock_controller(dock_controller)
{
    m_table_transition = new SnapshotTransitionAnimator(this);

    if (m_central_stack != nullptr)
    {
        m_page_transition = new StackedWidgetTransition(m_central_stack, this);
        m_page_transition->set_transition_surface(m_central_stack->window());
        connect(m_page_transition, &StackedWidgetTransition::target_page_activated, this,
                [this](int target_index) {
                    if (m_dock_controller != nullptr)
                    {
                        m_dock_controller->set_docks_suspended(target_index != 0);
                    }
                });
    }

    connect_workspace_actions();
    connect_workspace_updates();
    refresh_active_view();
    refresh_start_page();
}

/**
 * @brief Connects one per-view presenter to shared detail presentation.
 * @param presenter Presenter associated with a log tab.
 */
auto WorkspacePresenter::bind_log_view_presenter(LogViewPresenter* presenter) -> void
{
    if (presenter != nullptr)
    {
        connect(presenter, &LogViewPresenter::current_row_changed, this,
                &WorkspacePresenter::update_log_details);
    }
}

/**
 * @brief Refreshes all shared controls from the active log tab.
 */
auto WorkspacePresenter::refresh_active_view() -> void
{
    const QUuid view_id = get_active_view_id();

    if (!view_id.isNull() && m_filters != nullptr && m_history != nullptr && m_pages != nullptr &&
        m_queries != nullptr && m_filter_bar != nullptr)
    {
        const QSignalBlocker blocker(m_filter_bar);
        m_filter_bar->set_search_bar_enabled(true);
        m_filter_bar->set_app_names(m_history->get_distinct_values(view_id, LogField::AppName));
        m_filter_bar->set_current_app_name_filter(m_filters->get_app_name(view_id));
        m_filter_bar->set_available_log_levels(FilterCoordinator::get_available_log_levels());
        m_filter_bar->set_log_levels(m_filters->get_log_levels(view_id));

        QMap<QString, int> level_counts;
        const QMap<QString, qsizetype> history_counts =
            m_history->get_log_level_counts(m_queries->create_query(view_id));
        for (auto iterator = history_counts.cbegin(); iterator != history_counts.cend(); ++iterator)
        {
            level_counts.insert(iterator.key(), static_cast<int>(iterator.value()));
        }
        m_filter_bar->set_log_level_counts(level_counts);

        if (m_level_chart != nullptr)
        {
            m_level_chart->set_log_level_counts(level_counts);
        }

        if (m_pages->get_page_state(view_id) == nullptr)
        {
            m_queries->reload_query(view_id);
        }
    }
    else
    {
        reset_presentation();
    }

    refresh_pagination();
}

/**
 * @brief Refreshes shared pagination from the active log tab.
 */
auto WorkspacePresenter::refresh_pagination() -> void
{
    int current_page = 1;
    int total_pages = 1;
    const QUuid view_id = get_active_view_id();

    if (!view_id.isNull() && m_pages != nullptr)
    {
        const LogPageState* state = m_pages->get_page_state(view_id);

        if (state != nullptr)
        {
            current_page = static_cast<int>(state->get_current_page());
            total_pages = static_cast<int>(state->get_total_pages());
        }
    }

    if (m_pagination != nullptr)
    {
        m_pagination->setVisible(!view_id.isNull());
        m_pagination->set_pagination(current_page, total_pages);
    }
}

/**
 * @brief Refreshes start-page and dock visibility from current session state.
 */
auto WorkspacePresenter::refresh_start_page() -> void
{
    const bool session_active =
        m_session_controller != nullptr && m_session_controller->has_current_session();
    set_session_active(session_active);
}

/**
 * @brief Selects either the workspace or start-page presentation.
 * @param session_active True when a session workspace is available.
 */
auto WorkspacePresenter::set_session_active(bool session_active) -> void
{
    const int target_index = session_active ? 0 : 1;

    if (m_page_transition != nullptr)
    {
        const bool page_changes =
            m_central_stack != nullptr && m_central_stack->currentIndex() != target_index;

        if (!page_changes && m_dock_controller != nullptr)
        {
            m_dock_controller->set_docks_suspended(!session_active);
        }

        m_page_transition->transition_to(target_index);
    }
    else if (m_central_stack != nullptr)
    {
        m_central_stack->setCurrentIndex(target_index);
        if (m_dock_controller != nullptr)
        {
            m_dock_controller->set_docks_suspended(!session_active);
        }
    }
}

/**
 * @brief Clears shared controls after the workspace is closed.
 */
auto WorkspacePresenter::reset_presentation() -> void
{
    if (m_filter_bar != nullptr)
    {
        const QSignalBlocker blocker(m_filter_bar);
        m_filter_bar->set_search_bar_enabled(false);
        m_filter_bar->set_app_names({});
        m_filter_bar->set_log_levels({});
        m_filter_bar->set_log_level_counts({});
    }

    if (m_level_chart != nullptr)
    {
        m_level_chart->set_log_level_counts({});
    }

    if (m_details_text != nullptr)
    {
        m_details_text->clear();
    }
}

/**
 * @brief Applies a search to one explicit view.
 * @param view_id Target view identifier.
 * @param text Search text or regular expression.
 * @param field Field selected for searching.
 * @param use_regex True when regular-expression matching is enabled.
 */
auto WorkspacePresenter::apply_search(const QUuid& view_id, const QString& text, SearchField field,
                                      bool use_regex) -> void
{
    if (!view_id.isNull() && m_queries != nullptr)
    {
        m_queries->set_search(view_id, text, field, use_regex);
    }
}

/**
 * @brief Changes the active log table page through a directional snapshot transition.
 * @param page Requested one-based page number.
 */
auto WorkspacePresenter::navigate_to_page(int page) -> void
{
    const QUuid view_id = get_active_view_id();

    if (!view_id.isNull() && m_pages != nullptr)
    {
        const LogPageState* page_state = m_pages->get_page_state(view_id);
        LogViewWidget* log_view_widget =
            m_tab_widget != nullptr ? m_tab_widget->current_log_view() : nullptr;
        LogTableView* table_view =
            log_view_widget != nullptr ? log_view_widget->get_table_view() : nullptr;

        if (page_state != nullptr && table_view != nullptr &&
            page_state->get_current_page() != page)
        {
            if (m_table_transition->is_running())
            {
                m_table_transition->finish();
            }

            WidgetTransitionConfiguration configuration;
            configuration.effect = WidgetTransitionEffect::FadeAndSlide;
            configuration.direction = page > page_state->get_current_page()
                                          ? WidgetTransitionDirection::Left
                                          : WidgetTransitionDirection::Right;
            configuration.easing_curve = QEasingCurve::InOutQuad;
            configuration.duration_ms = 560;
            configuration.motion_distance = 128;
            m_table_transition->set_configuration(configuration);
            m_table_transition->transition(
                table_view, [this, view_id, page] { m_pages->set_current_page(view_id, page); });
        }
        else if (page_state == nullptr || table_view == nullptr)
        {
            m_pages->set_current_page(view_id, page);
        }
    }
}

/**
 * @brief Connects shared widgets to active-view operations.
 */
auto WorkspacePresenter::connect_workspace_actions() -> void
{
    if (m_pagination != nullptr)
    {
        connect(m_pagination, &PaginationWidget::page_changed, this,
                &WorkspacePresenter::navigate_to_page);
        connect(m_pagination, &PaginationWidget::items_per_page_changed, this,
                [this](int items_per_page) {
                    const QUuid view_id = get_active_view_id();
                    if (!view_id.isNull() && m_pages != nullptr)
                    {
                        m_pages->set_page_size(view_id, items_per_page);
                    }
                });
    }

    if (m_filter_bar != nullptr)
    {
        connect(m_filter_bar, &LogFilterBarWidget::app_filter_changed, this,
                [this](const QString& app_name) {
                    const QUuid view_id = get_active_view_id();
                    if (!view_id.isNull() && m_queries != nullptr)
                    {
                        m_queries->set_app_name(view_id, app_name);
                    }
                });
        connect(m_filter_bar, &LogFilterBarWidget::log_level_filter_changed, this,
                [this](const QSet<QString>& levels) {
                    const QUuid view_id = get_active_view_id();
                    if (!view_id.isNull() && m_queries != nullptr)
                    {
                        m_queries->set_log_levels(view_id, levels);
                    }
                });
        connect(m_filter_bar, &LogFilterBarWidget::search_requested, this,
                [this](const QString& text, SearchField field, bool use_regex) {
                    apply_search(get_active_view_id(), text, field, use_regex);
                });
        connect(m_filter_bar, &LogFilterBarWidget::search_field_changed, this, [this](SearchField) {
            apply_search(get_active_view_id(), m_filter_bar->get_search_text(),
                         m_filter_bar->get_search_field(), m_filter_bar->get_use_regex());
        });
        connect(m_filter_bar, &LogFilterBarWidget::regex_toggled, this, [this](bool) {
            apply_search(get_active_view_id(), m_filter_bar->get_search_text(),
                         m_filter_bar->get_search_field(), m_filter_bar->get_use_regex());
        });
    }

    if (m_tab_widget != nullptr)
    {
        connect(m_tab_widget, &QTabWidget::currentChanged, this, [this](int) {
            const QUuid view_id = get_active_view_id();
            if (!view_id.isNull() && m_views != nullptr)
            {
                m_views->set_current_view(view_id);
            }
            refresh_active_view();
        });
        connect(m_tab_widget, &TabWidget::about_to_close_tab, this, [this](int index, QWidget*) {
            LogViewWidget* log_view_widget = m_tab_widget->log_view_at(index);
            if (log_view_widget != nullptr && m_lifecycle != nullptr)
            {
                m_lifecycle->close_view(log_view_widget->get_view_id());
            }
        });
        connect(m_tab_widget, &TabWidget::close_tab_requested, this,
                [this](int) { refresh_active_view(); });
    }
}

/**
 * @brief Connects runtime and session changes to presentation refreshes.
 */
auto WorkspacePresenter::connect_workspace_updates() -> void
{
    if (m_views != nullptr && m_pages != nullptr && m_imports != nullptr)
    {
        connect(m_views, &ViewRegistry::current_view_id_changed, this,
                [this](const QUuid&) { refresh_active_view(); });
        connect(m_views, &ViewRegistry::view_removed, this, [this](const QUuid& view_id) {
            if (m_tab_widget != nullptr)
            {
                m_tab_widget->remove_view_tab_by_id(view_id);
            }
            refresh_active_view();
        });
        connect(m_pages, &LogPageCoordinator::page_loaded, this,
                [this](const QUuid& view_id, qsizetype current_page, qsizetype total_pages,
                       qsizetype) { update_page_state(view_id, current_page, total_pages); });
        connect(m_pages, &LogPageCoordinator::page_state_updated, this,
                [this](const QUuid& view_id, qsizetype current_page, qsizetype total_pages,
                       qsizetype) { update_page_state(view_id, current_page, total_pages); });
        connect(m_imports, &LogImportCoordinator::finished, this,
                [this](const QUuid& view_id, const QString&) {
                    if (view_id == get_active_view_id())
                    {
                        refresh_active_view();
                    }
                });
    }

    if (m_session_controller != nullptr)
    {
        connect(
            m_session_controller, &SessionController::current_session_changed, this,
            [this](const QString&) { refresh_start_page(); }, Qt::QueuedConnection);
        connect(m_session_controller, &SessionController::session_restore_started, this,
                [this](const QString&) { set_session_active(true); });
        connect(m_session_controller, &SessionController::session_restored, this,
                [this](const QString&) { refresh_active_view(); });
        connect(m_session_controller, &SessionController::all_sessions_removed, this,
                [this] { refresh_start_page(); });
    }
}

/**
 * @brief Returns the view ID represented by the selected tab.
 * @return Active log-view ID, or a null ID for non-log tabs.
 */
auto WorkspacePresenter::get_active_view_id() const -> QUuid
{
    QUuid view_id;

    if (m_tab_widget != nullptr)
    {
        const LogViewWidget* log_view_widget = m_tab_widget->current_log_view();
        if (log_view_widget != nullptr)
        {
            view_id = log_view_widget->get_view_id();
        }
    }

    return view_id;
}

/**
 * @brief Updates shared page and level-count presentation for one view.
 * @param view_id View whose query state changed.
 * @param current_page Current one-based page.
 * @param total_pages Total available pages.
 */
auto WorkspacePresenter::update_page_state(const QUuid& view_id, qsizetype current_page,
                                           qsizetype total_pages) -> void
{
    if (view_id == get_active_view_id())
    {
        if (m_pagination != nullptr)
        {
            m_pagination->set_pagination(static_cast<int>(current_page),
                                         static_cast<int>(total_pages));
        }

        if (m_history != nullptr && m_queries != nullptr)
        {
            QMap<QString, int> level_counts;
            const QMap<QString, qsizetype> history_counts =
                m_history->get_log_level_counts(m_queries->create_query(view_id));
            for (auto iterator = history_counts.cbegin(); iterator != history_counts.cend();
                 ++iterator)
            {
                level_counts.insert(iterator.key(), static_cast<int>(iterator.value()));
            }

            if (m_filter_bar != nullptr)
            {
                const QSignalBlocker blocker(m_filter_bar);
                m_filter_bar->set_log_level_counts(level_counts);
            }

            if (m_level_chart != nullptr)
            {
                m_level_chart->set_log_level_counts(level_counts);
            }
        }
    }
}

/**
 * @brief Displays the selected row for its originating view.
 * @param view_id Originating view identifier.
 * @param current Selected model index.
 */
auto WorkspacePresenter::update_log_details(const QUuid& view_id,
                                            const QModelIndex& current) -> void
{
    QString details;

    if (view_id == get_active_view_id() && current.isValid() && m_views != nullptr)
    {
        LogViewContext* context = m_views->get_context(view_id);
        LogModel* model = context != nullptr ? context->get_model() : nullptr;

        if (model != nullptr)
        {
            const LogEntry entry = model->get_entry(current.row());
            details =
                QStringLiteral(
                    "Timestamp: %1\n"
                    "Level: %2\n"
                    "App: %3\n"
                    "Message: %4")
                    .arg(entry.get_timestamp().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")))
                    .arg(entry.get_level())
                    .arg(entry.get_app_name())
                    .arg(entry.get_message());
        }
    }

    if (m_details_text != nullptr)
    {
        m_details_text->setPlainText(details);
    }
}
