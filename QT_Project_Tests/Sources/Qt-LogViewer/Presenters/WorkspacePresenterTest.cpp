#include "Qt-LogViewer/Presenters/WorkspacePresenterTest.h"

#include <QApplication>
#include <QDockWidget>
#include <QFile>
#include <QHeaderView>
#include <QLabel>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QStackedWidget>
#include <QTemporaryFile>
#include <QTest>
#include <QTextStream>
#include <QWidget>

#include "Qt-LogViewer/Animations/SnapshotTransitionAnimator.h"
#include "Qt-LogViewer/Controllers/DockController.h"
#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/SessionController.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogPageState.h"
#include "Qt-LogViewer/Models/LogQuery.h"
#include "Qt-LogViewer/Models/SessionTypes.h"
#include "Qt-LogViewer/Presenters/LogViewPresenter.h"
#include "Qt-LogViewer/Presenters/WorkspacePresenter.h"
#include "Qt-LogViewer/Support/TestLogRuntime.h"
#include "Qt-LogViewer/Views/App/LogFilterBarWidget.h"
#include "Qt-LogViewer/Views/App/LogLevelPieChartWidget.h"
#include "Qt-LogViewer/Views/App/LogTabWidget.h"
#include "Qt-LogViewer/Views/App/LogTableView.h"
#include "Qt-LogViewer/Views/App/LogViewWidget.h"
#include "Qt-LogViewer/Views/Shared/PaginationWidget.h"

/**
 * @file WorkspacePresenterTest.cpp
 * @brief Implements integration tests for shared workspace presentation.
 */

/**
 * @brief Creates controller state and all shared workspace widgets.
 */
void WorkspacePresenterTest::SetUp()
{
    m_runtime = new TestLogRuntime(m_profile);
    m_tab_widget = new LogTabWidget();
    m_tab_widget->setup_default_behavior();
    m_filter_bar = new LogFilterBarWidget();
    m_pagination = new PaginationWidget();
    m_details_text = new QPlainTextEdit();
    m_level_chart = new LogLevelPieChartWidget();
    m_central_stack = new QStackedWidget();
    m_central_stack->addWidget(new QWidget());
    m_central_stack->addWidget(new QWidget());
    m_session_controller = new SessionController(
        nullptr, m_runtime->catalog().get_model(), m_profile, &m_runtime->catalog(),
        &m_runtime->views(), &m_runtime->filters(), &m_runtime->history(), &m_runtime->pages(),
        &m_runtime->queries(), &m_runtime->imports(), &m_runtime->lifecycle(),
        &m_runtime->live_tailing());
    m_presenter = new WorkspacePresenter(
        &m_runtime->views(), &m_runtime->filters(), &m_runtime->history(), &m_runtime->pages(),
        &m_runtime->queries(), &m_runtime->imports(), &m_runtime->lifecycle(), m_session_controller,
        m_tab_widget, m_filter_bar, m_pagination, m_details_text, m_level_chart, m_central_stack,
        nullptr);
}

/**
 * @brief Destroys the workspace object graph and temporary files.
 */
void WorkspacePresenterTest::TearDown()
{
    delete m_presenter;
    m_presenter = nullptr;
    delete m_session_controller;
    m_session_controller = nullptr;
    delete m_tab_widget;
    m_tab_widget = nullptr;
    delete m_filter_bar;
    m_filter_bar = nullptr;
    delete m_pagination;
    m_pagination = nullptr;
    delete m_details_text;
    m_details_text = nullptr;
    delete m_level_chart;
    m_level_chart = nullptr;
    delete m_central_stack;
    m_central_stack = nullptr;
    delete m_runtime;
    m_runtime = nullptr;

    for (QTemporaryFile* file: m_temp_files)
    {
        if (file != nullptr)
        {
            QFile::remove(file->fileName());
            delete file;
        }
    }

    m_temp_files.clear();
}

/**
 * @brief Creates a runtime view, widget, and per-view presenter.
 * @param records Complete log records written to the temporary source file.
 * @return Identifier of the added runtime view.
 */
auto WorkspacePresenterTest::add_log_view(const QVector<QString>& records) -> QUuid
{
    QUuid view_id;
    auto* file = new QTemporaryFile();

    if (file->open())
    {
        file->setAutoRemove(false);
        QTextStream stream(file);
        for (const QString& record: records)
        {
            stream << record << '\n';
        }
        stream.flush();
        file->close();
        m_temp_files.append(file);

        view_id = m_runtime->imports().import_file(file->fileName(), m_profile);

        if (!view_id.isNull())
        {
            auto* widget = new LogViewWidget(m_tab_widget);
            SessionViewState state;
            state.id = view_id;
            auto* view_presenter = new LogViewPresenter(
                &m_runtime->views(), &m_runtime->filters(), &m_runtime->history(),
                &m_runtime->pages(), &m_runtime->queries(), &m_runtime->imports(),
                &m_runtime->lifecycle(), &m_runtime->live_tailing(), widget, view_id, state,
                widget);
            m_presenter->bind_log_view_presenter(view_presenter);
            m_tab_widget->add_log_view_tab(widget, QStringLiteral("Test View"), false);
        }
    }
    else
    {
        delete file;
    }

    return view_id;
}

/**
 * @test Verifies that switching tabs selects and presents each independent
 * runtime view.
 */
TEST_F(WorkspacePresenterTest, SwitchesBetweenLogViews)
{
    const QUuid first_view =
        add_log_view({QStringLiteral("2024-01-01 12:00:00 INFO FirstMessage FirstApp")});
    const QUuid second_view =
        add_log_view({QStringLiteral("2024-01-01 13:00:00 ERROR SecondMessage SecondApp")});
    ASSERT_FALSE(first_view.isNull());
    ASSERT_FALSE(second_view.isNull());

    m_runtime->queries().set_app_name(first_view, QStringLiteral("FirstApp"));
    m_runtime->queries().set_app_name(second_view, QStringLiteral("SecondApp"));

    m_tab_widget->setCurrentIndex(0);
    EXPECT_EQ(m_runtime->views().get_current_view(), first_view);
    EXPECT_EQ(m_filter_bar->get_current_app_name(), QStringLiteral("FirstApp"));

    m_tab_widget->setCurrentIndex(1);
    EXPECT_EQ(m_runtime->views().get_current_view(), second_view);
    EXPECT_EQ(m_filter_bar->get_current_app_name(), QStringLiteral("SecondApp"));
}

/** @test Verifies hidden loaded views resize their columns only after becoming visible. */
TEST_F(WorkspacePresenterTest, ResizesHiddenLoadedViewWhenActivated)
{
    m_tab_widget->resize(900, 500);
    m_tab_widget->show();
    QApplication::processEvents();

    const QUuid first_view =
        add_log_view({QStringLiteral("2024-01-01 12:00:00 INFO FirstMessage FirstApp")});
    const QUuid second_view =
        add_log_view({QStringLiteral("2024-01-01 12:00:01 INFO SecondMessage SecondApp")});
    ASSERT_FALSE(first_view.isNull());
    ASSERT_FALSE(second_view.isNull());

    const int first_index = m_tab_widget->find_view_index(first_view);
    const int second_index = m_tab_widget->find_view_index(second_view);
    ASSERT_GE(first_index, 0);
    ASSERT_GE(second_index, 0);
    LogViewWidget* first_widget = m_tab_widget->log_view_at(first_index);
    ASSERT_NE(first_widget, nullptr);
    LogTableView* first_table = first_widget->get_table_view();
    ASSERT_NE(first_table, nullptr);
    first_table->setColumnWidth(0, first_table->horizontalHeader()->minimumSectionSize());

    m_tab_widget->setCurrentIndex(second_index);
    emit m_runtime->imports().finished(first_view, QStringLiteral("hidden.log"));
    QApplication::processEvents();
    const int hidden_width = first_table->columnWidth(0);

    m_tab_widget->setCurrentIndex(first_index);
    QTRY_VERIFY_WITH_TIMEOUT(first_table->columnWidth(0) > hidden_width, 1000);
}

/**
 * @test Verifies that a non-log import tab disables active-view actions.
 */
TEST_F(WorkspacePresenterTest, KeepsSharedActionsInactiveForImportTab)
{
    const QUuid view_id =
        add_log_view({QStringLiteral("2024-01-01 12:00:00 INFO OriginalMessage ImportApp")});
    ASSERT_FALSE(view_id.isNull());
    auto* import_tab = new QWidget(m_tab_widget);
    m_tab_widget->addTab(import_tab, QStringLiteral("Import"));
    m_tab_widget->setCurrentWidget(import_tab);

    emit m_filter_bar->search_requested(QStringLiteral("Ignored"), SearchField::Message, false);

    EXPECT_TRUE(m_runtime->filters().get_search_text(view_id).isEmpty());
    EXPECT_TRUE(m_pagination->isHidden());
}

/**
 * @test Verifies that global search and pagination target only the active view.
 */
TEST_F(WorkspacePresenterTest, RoutesSearchAndPaginationToActiveView)
{
    const QUuid view_id =
        add_log_view({QStringLiteral("2024-01-01 12:00:00 INFO First SearchApp"),
                      QStringLiteral("2024-01-01 12:01:00 INFO Second SearchApp"),
                      QStringLiteral("2024-01-01 12:02:00 INFO Third SearchApp")});
    ASSERT_FALSE(view_id.isNull());

    emit m_filter_bar->search_requested(QStringLiteral("SearchApp"), SearchField::AppName, false);
    emit m_pagination->items_per_page_changed(1);
    emit m_pagination->page_changed(2);

    const LogQuery query = m_runtime->queries().create_query(view_id);
    EXPECT_EQ(query.search_text, QStringLiteral("SearchApp"));
    EXPECT_TRUE(query.search_fields.contains(LogField::AppName));

    const LogPageState* page_state = m_runtime->pages().get_page_state(view_id);
    ASSERT_NE(page_state, nullptr);
    EXPECT_EQ(page_state->get_page_size(), 1);
    EXPECT_EQ(page_state->get_current_page(), 2);
}

/** @test Verifies page navigation reuses the generic animator for the complete log table. */
TEST_F(WorkspacePresenterTest, AnimatesVisibleTablePageChanges)
{
    const QUuid view_id = add_log_view({QStringLiteral("2024-01-01 12:00:00 INFO First PageApp"),
                                        QStringLiteral("2024-01-01 12:01:00 INFO Second PageApp"),
                                        QStringLiteral("2024-01-01 12:02:00 INFO Third PageApp")});
    ASSERT_FALSE(view_id.isNull());
    emit m_pagination->items_per_page_changed(1);
    m_tab_widget->show();
    QApplication::processEvents();
    SnapshotTransitionAnimator* table_transition =
        m_presenter->findChild<SnapshotTransitionAnimator*>(QString(), Qt::FindDirectChildrenOnly);
    ASSERT_NE(table_transition, nullptr);

    emit m_pagination->page_changed(2);

    EXPECT_TRUE(table_transition->is_running());
    WidgetTransitionConfiguration configuration = table_transition->get_configuration();
    EXPECT_EQ(configuration.effect, WidgetTransitionEffect::FadeAndSlide);
    EXPECT_EQ(configuration.direction, WidgetTransitionDirection::Left);
    EXPECT_EQ(configuration.easing_curve.type(), QEasingCurve::InOutQuad);
    EXPECT_EQ(configuration.duration_ms, 560);
    EXPECT_EQ(configuration.motion_distance, 128);
    const LogPageState* page_state = m_runtime->pages().get_page_state(view_id);
    ASSERT_NE(page_state, nullptr);
    EXPECT_EQ(page_state->get_current_page(), 2);
    QTRY_VERIFY_WITH_TIMEOUT(!table_transition->is_running(), 1000);

    emit m_pagination->page_changed(1);

    EXPECT_TRUE(table_transition->is_running());
    configuration = table_transition->get_configuration();
    EXPECT_EQ(configuration.direction, WidgetTransitionDirection::Right);
    EXPECT_EQ(page_state->get_current_page(), 1);
    QTRY_VERIFY_WITH_TIMEOUT(!table_transition->is_running(), 1000);
}

/**
 * @test Verifies that row selection is presented through the shared details
 * view.
 */
TEST_F(WorkspacePresenterTest, PresentsSelectedRowDetails)
{
    const QUuid view_id =
        add_log_view({QStringLiteral("2024-01-01 12:00:00 INFO DetailMessage DetailApp")});
    ASSERT_FALSE(view_id.isNull());
    LogViewWidget* widget = m_tab_widget->current_log_view();
    ASSERT_NE(widget, nullptr);
    LogViewPresenter* view_presenter = widget->findChild<LogViewPresenter*>();
    ASSERT_NE(view_presenter, nullptr);
    LogModel* model = m_runtime->views().get_context(view_id)->get_model();
    ASSERT_NE(model, nullptr);

    emit view_presenter->current_row_changed(view_id, model->index(0, 0));

    EXPECT_TRUE(m_details_text->toPlainText().contains(QStringLiteral("DetailMessage")));
}

/**
 * @test Verifies explicit workspace and start-page selection.
 */
TEST_F(WorkspacePresenterTest, SwitchesBetweenWorkspaceAndStartPage)
{
    m_presenter->set_session_active(true);
    EXPECT_EQ(m_central_stack->currentIndex(), 0);

    m_presenter->set_session_active(false);
    EXPECT_EQ(m_central_stack->currentIndex(), 1);
}

/** @test Verifies registered session views animate into the workspace before imports begin. */
TEST_F(WorkspacePresenterTest, AnimatesSessionRestoreIntoWorkspace)
{
    m_central_stack->resize(480, 320);
    m_central_stack->setCurrentIndex(1);
    m_central_stack->show();
    QApplication::processEvents();

    emit m_session_controller->session_restore_started(QStringLiteral("restored-session"));
    emit m_session_controller->session_views_registered(QStringLiteral("restored-session"));

    EXPECT_EQ(m_central_stack->currentIndex(), 0);
    EXPECT_GE(m_central_stack->findChildren<QLabel*>(QString(), Qt::FindDirectChildrenOnly).size(),
              3);
}

/** @test Verifies docks are suspended when the start page is already selected. */
TEST_F(WorkspacePresenterTest, SuspendsDocksWithoutStartingAnotherPageTransition)
{
    delete m_presenter;
    m_presenter = nullptr;

    QMainWindow main_window;
    auto* central_stack = new QStackedWidget(&main_window);
    central_stack->addWidget(new QWidget());
    central_stack->addWidget(new QWidget());
    central_stack->setCurrentIndex(1);
    main_window.setCentralWidget(central_stack);
    auto* dock_widget = new QDockWidget(&main_window);
    main_window.addDockWidget(Qt::LeftDockWidgetArea, dock_widget);
    DockController dock_controller(&main_window);
    dock_controller.register_dock(dock_widget);

    WorkspacePresenter presenter(&m_runtime->views(), &m_runtime->filters(), &m_runtime->history(),
                                 &m_runtime->pages(), &m_runtime->queries(), &m_runtime->imports(),
                                 &m_runtime->lifecycle(), nullptr, m_tab_widget, m_filter_bar,
                                 m_pagination, m_details_text, m_level_chart, central_stack,
                                 &dock_controller);

    EXPECT_EQ(central_stack->currentIndex(), 1);
    EXPECT_TRUE(dock_widget->isHidden());
}
/**
 * @test Verifies that removing the final view clears shared workspace controls.
 */
TEST_F(WorkspacePresenterTest, ClearsPresentationAfterLastViewCloses)
{
    const QUuid view_id =
        add_log_view({QStringLiteral("2024-01-01 12:00:00 INFO ClosingMessage ClosingApp")});
    ASSERT_FALSE(view_id.isNull());

    ASSERT_TRUE(m_runtime->lifecycle().close_view(view_id));

    EXPECT_EQ(m_tab_widget->count(), 0);
    EXPECT_TRUE(m_pagination->isHidden());
    EXPECT_TRUE(m_details_text->toPlainText().isEmpty());
}
