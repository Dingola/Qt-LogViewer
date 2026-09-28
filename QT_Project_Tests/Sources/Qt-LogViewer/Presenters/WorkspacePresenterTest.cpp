#include "Qt-LogViewer/Presenters/WorkspacePresenterTest.h"

#include <QFile>
#include <QPlainTextEdit>
#include <QStackedWidget>
#include <QTemporaryFile>
#include <QTextStream>
#include <QWidget>

#include "Qt-LogViewer/Controllers/LogViewContext.h"
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
    m_presenter = new WorkspacePresenter(
        &m_runtime->views(), &m_runtime->filters(), &m_runtime->history(), &m_runtime->pages(),
        &m_runtime->queries(), &m_runtime->imports(), &m_runtime->lifecycle(), nullptr,
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
