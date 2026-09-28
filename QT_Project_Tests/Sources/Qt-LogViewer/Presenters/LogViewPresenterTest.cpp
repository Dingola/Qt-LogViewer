#include "Qt-LogViewer/Presenters/LogViewPresenterTest.h"

#include <QCoreApplication>
#include <QEvent>
#include <QHeaderView>
#include <QPointer>
#include <QSignalSpy>
#include <QTemporaryFile>
#include <QTextStream>

#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Models/LogFieldDefinition.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogQuery.h"
#include "Qt-LogViewer/Models/SessionTypes.h"
#include "Qt-LogViewer/Presenters/LogViewPresenter.h"
#include "Qt-LogViewer/Services/LogParsingProfile.h"
#include "Qt-LogViewer/Support/TestLogRuntime.h"
#include "Qt-LogViewer/Views/App/LogTableView.h"
#include "Qt-LogViewer/Views/App/LogViewWidget.h"

/**
 * @file LogViewPresenterTest.cpp
 * @brief Implements tests for the per-view presentation binding.
 */

/**
 * @brief Creates one controller view, widget, and bound presenter.
 */
void LogViewPresenterTest::SetUp()
{
    const LogParsingProfile profile = LogParsingProfile::create_default(
        QStringLiteral("{timestamp} {level} {message} {app_name}"),
        QStringLiteral("Presenter test default"));
    m_log_file = new QTemporaryFile();
    ASSERT_TRUE(m_log_file->open());
    QTextStream stream(m_log_file);
    stream << "2024-01-01 12:00:00 INFO PresenterMessage PresenterApp\n";
    stream.flush();
    m_file_path = m_log_file->fileName();
    m_log_file->close();

    m_runtime = new TestLogRuntime(profile);
    m_view_id = m_runtime->imports().import_file(m_file_path, profile);
    m_runtime->live_tailing().set_enabled(m_view_id, false);
    m_widget = new LogViewWidget();

    SessionViewState state;
    state.id = m_view_id;
    state.filters.live_tailing_enabled = false;
    m_presenter = new LogViewPresenter(
        &m_runtime->views(), &m_runtime->filters(), &m_runtime->history(), &m_runtime->pages(),
        &m_runtime->queries(), &m_runtime->imports(), &m_runtime->lifecycle(),
        &m_runtime->live_tailing(), m_widget, m_view_id, state, m_widget);
}

/**
 * @brief Destroys the widget-owned presenter and controller.
 */
void LogViewPresenterTest::TearDown()
{
    delete m_widget;
    m_widget = nullptr;
    m_presenter = nullptr;
    delete m_runtime;
    m_runtime = nullptr;
    delete m_log_file;
    m_log_file = nullptr;
    m_file_path.clear();
}

/**
 * @test Verifies that construction binds the widget and initializes its query state.
 */
TEST_F(LogViewPresenterTest, BindsWidgetToRuntimeView)
{
    ASSERT_NE(m_runtime, nullptr);
    ASSERT_NE(m_widget, nullptr);
    ASSERT_NE(m_presenter, nullptr);
    ASSERT_FALSE(m_view_id.isNull());

    EXPECT_EQ(m_widget->get_view_id(), m_view_id);
    ASSERT_NE(m_runtime->views().get_context(m_view_id), nullptr);
    EXPECT_EQ(m_widget->get_table_view()->model(),
              m_runtime->views().get_context(m_view_id)->get_model());
    EXPECT_NE(m_runtime->pages().get_page_state(m_view_id), nullptr);
    EXPECT_FALSE(m_widget->get_live_tailing_enabled());
    EXPECT_EQ(m_presenter->parent(), m_widget);
}

/**
 * @test Verifies that per-tab filter and tailing actions target the bound view.
 */
TEST_F(LogViewPresenterTest, RoutesWidgetActionsToBoundView)
{
    const QSet<QString> levels{QStringLiteral("ERROR"), QStringLiteral("WARNING")};

    emit m_widget->app_filter_changed(QStringLiteral("PresenterApp"));
    emit m_widget->log_level_filter_changed(levels);
    emit m_widget->live_tailing_toggled(true);

    EXPECT_EQ(m_runtime->filters().get_app_name(m_view_id), QStringLiteral("PresenterApp"));
    EXPECT_EQ(m_runtime->filters().get_log_levels(m_view_id),
              QSet<QString>({QStringLiteral("error"), QStringLiteral("warning")}));
    EXPECT_TRUE(m_runtime->live_tailing().is_enabled(m_view_id));
}

/**
 * @test Verifies that file visibility actions update query and widget presentation together.
 */
TEST_F(LogViewPresenterTest, RoutesFileVisibilityActionsToBoundView)
{
    emit m_widget->show_only_file_requested(m_file_path);
    EXPECT_EQ(m_runtime->queries().create_query(m_view_id).show_only_file, m_file_path);

    emit m_widget->show_only_file_requested({});
    EXPECT_TRUE(m_runtime->queries().create_query(m_view_id).show_only_file.isEmpty());

    emit m_widget->toggle_visibility_requested(m_file_path);
    EXPECT_TRUE(m_runtime->queries().create_query(m_view_id).hidden_files.contains(m_file_path));
}

/**
 * @test Verifies that table sorting is translated into the view query.
 */
TEST_F(LogViewPresenterTest, RoutesTableSortingToBoundView)
{
    LogModel* model = m_runtime->views().get_context(m_view_id)->get_model();
    ASSERT_NE(model, nullptr);
    const int message_column = model->find_column(LogField::Message);
    ASSERT_GE(message_column, 0);

    QHeaderView* header = m_widget->get_table_view()->horizontalHeader();
    ASSERT_NE(header, nullptr);
    header->setSortIndicator(message_column, Qt::AscendingOrder);

    const LogQuery query = m_runtime->queries().create_query(m_view_id);
    EXPECT_EQ(query.sort_field, LogField::Message);
    EXPECT_EQ(query.sort_order, Qt::AscendingOrder);
}

/**
 * @test Verifies that row selections are forwarded without exposing widget wiring to MainWindow.
 */
TEST_F(LogViewPresenterTest, ForwardsCurrentRowChanges)
{
    QSignalSpy row_spy(m_presenter, &LogViewPresenter::current_row_changed);

    emit m_widget->current_row_changed(QModelIndex(), QModelIndex());

    EXPECT_EQ(row_spy.count(), 1);
}

/**
 * @test Verifies that closing the widget also destroys its presenter safely.
 */
TEST_F(LogViewPresenterTest, SharesLifetimeWithBoundWidget)
{
    QPointer<LogViewPresenter> guarded_presenter(m_presenter);

    delete m_widget;
    m_widget = nullptr;
    m_presenter = nullptr;

    EXPECT_TRUE(guarded_presenter.isNull());

    emit m_runtime->pages().page_loaded(m_view_id, 1, 1, 0);
}

/**
 * @test Verifies that removing the runtime view releases its bound widget and presenter.
 */
TEST_F(LogViewPresenterTest, ReleasesBindingAfterRuntimeViewRemoval)
{
    QPointer<LogViewWidget> guarded_widget(m_widget);
    QPointer<LogViewPresenter> guarded_presenter(m_presenter);

    ASSERT_TRUE(m_runtime->lifecycle().close_view(m_view_id));
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    m_widget = nullptr;
    m_presenter = nullptr;

    EXPECT_TRUE(guarded_widget.isNull());
    EXPECT_TRUE(guarded_presenter.isNull());
}
