/**
 * @file LiveTailingCoordinatorTest.cpp
 * @brief Tests live-tail persistence, batching, paging decisions, and cleanup.
 */

#include "Qt-LogViewer/Controllers/LiveTailingCoordinatorTest.h"

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTest>
#include <QTextStream>

#include "Qt-LogViewer/Controllers/FilterCoordinator.h"
#include "Qt-LogViewer/Controllers/LiveTailingCoordinator.h"
#include "Qt-LogViewer/Controllers/LogPageCoordinator.h"
#include "Qt-LogViewer/Controllers/LogQueryController.h"
#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogQuery.h"
#include "Qt-LogViewer/Services/LogHistoryService.h"

/**
 * @brief Creates the isolated live-tailing graph used by each test.
 */
void LiveTailingCoordinatorTest::SetUp()
{
    ASSERT_TRUE(m_temporary_directory.isValid());

    m_file_path = QDir(m_temporary_directory.path()).filePath(QStringLiteral("application.log"));
    QFile file(m_file_path);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("2026-01-01 12:00:00 INFO initial TestApp\n");
    file.close();

    const QString database_path =
        QDir(m_temporary_directory.path()).filePath(QStringLiteral("history.sqlite"));
    m_history = new LogHistoryService(database_path);
    m_views = new ViewRegistry();
    m_filters = new FilterCoordinator(m_views);
    m_pages = new LogPageCoordinator(m_history, m_views);
    m_queries = new LogQueryController(m_filters, m_pages, m_views);
    m_tailing = new LiveTailingCoordinator(m_profile, m_history, m_views, m_queries);
    m_tailing->set_refresh_interval_ms(0);

    m_view_id = m_views->create_view();
    LogViewContext* context = m_views->get_context(m_view_id);
    ASSERT_NE(context, nullptr);
    context->set_loaded_files({LogFileInfo(m_file_path, QStringLiteral("TestApp"))});
    context->set_file_parsing_profile(m_file_path, m_profile);

    ASSERT_TRUE(m_history->add_entries(m_view_id, {create_entry(0)}));
    ASSERT_TRUE(m_queries->reload_query(m_view_id));
}

/**
 * @brief Stops tailing and destroys the isolated graph.
 */
void LiveTailingCoordinatorTest::TearDown()
{
    if (m_tailing != nullptr)
    {
        m_tailing->shutdown();
    }

    if (m_history != nullptr && !m_view_id.isNull())
    {
        m_history->remove_view_entries(m_view_id);
    }

    delete m_tailing;
    delete m_queries;
    delete m_pages;
    delete m_filters;
    delete m_views;
    delete m_history;

    m_tailing = nullptr;
    m_queries = nullptr;
    m_pages = nullptr;
    m_filters = nullptr;
    m_views = nullptr;
    m_history = nullptr;
}

/**
 * @brief Creates a deterministic archived entry.
 * @param index Sequence number included in timestamp and message.
 * @return Entry associated with the fixture log file.
 */
auto LiveTailingCoordinatorTest::create_entry(int index) const -> LogEntry
{
    return LogEntry(
        QDateTime::fromString(
            QStringLiteral("2026-01-01T12:%1:00.000Z").arg(index, 2, 10, QLatin1Char('0')),
            Qt::ISODateWithMs),
        QStringLiteral("INFO"), QStringLiteral("entry-%1").arg(index),
        LogFileInfo(m_file_path, QStringLiteral("TestApp")));
}

/**
 * @brief Appends one complete record to the watched log file.
 * @param message Message field written to the record.
 * @param level Level field written to the record.
 */
auto LiveTailingCoordinatorTest::append_record(const QString& message, const QString& level) -> void
{
    QFile file(m_file_path);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text));
    QTextStream stream(&file);
    stream << "2026-01-01 12:10:00 " << level << ' ' << message << " TestApp\n";
    stream.flush();
    file.close();
}

/**
 * @brief Verifies that appended entries are stored and displayed on page one.
 */
TEST_F(LiveTailingCoordinatorTest, StoresAndDisplaysEntriesOnFirstPage)
{
    QSignalSpy page_loaded_spy(m_pages, &LogPageCoordinator::page_loaded);
    m_tailing->set_enabled(m_view_id, true);

    append_record(QStringLiteral("tailed-entry"));

    QTRY_COMPARE_WITH_TIMEOUT(m_pages->get_page_state(m_view_id)->get_total_entries(),
                              static_cast<qsizetype>(2), 5000);
    LogModel* model = m_views->get_context(m_view_id)->get_model();
    ASSERT_NE(model, nullptr);
    ASSERT_EQ(model->rowCount(), 2);
    bool tailed_entry_visible = false;
    for (int row = 0; row < model->rowCount(); ++row)
    {
        if (model->get_entry(row).get_message() == QStringLiteral("tailed-entry"))
        {
            tailed_entry_visible = true;
        }
    }
    EXPECT_TRUE(tailed_entry_visible);
    EXPECT_EQ(page_loaded_spy.count(), 1);
}

/**
 * @brief Verifies that historical pages keep their rows while totals are refreshed.
 */
TEST_F(LiveTailingCoordinatorTest, KeepsHistoricalPageStable)
{
    ASSERT_TRUE(m_history->add_entries(m_view_id, {create_entry(1), create_entry(2)}));
    ASSERT_TRUE(m_queries->reload_query(m_view_id));
    ASSERT_TRUE(m_pages->set_page_size(m_view_id, 1));
    ASSERT_TRUE(m_pages->set_current_page(m_view_id, 2));

    LogModel* model = m_views->get_context(m_view_id)->get_model();
    ASSERT_NE(model, nullptr);
    ASSERT_EQ(model->rowCount(), 1);
    const QString visible_message = model->get_entry(0).get_message();
    QSignalSpy page_loaded_spy(m_pages, &LogPageCoordinator::page_loaded);
    QSignalSpy page_state_spy(m_pages, &LogPageCoordinator::page_state_updated);
    m_tailing->set_enabled(m_view_id, true);

    append_record(QStringLiteral("newest-entry"));

    QTRY_COMPARE_WITH_TIMEOUT(m_pages->get_page_state(m_view_id)->get_total_entries(),
                              static_cast<qsizetype>(4), 5000);
    EXPECT_EQ(model->rowCount(), 1);
    EXPECT_EQ(model->get_entry(0).get_message(), visible_message);
    EXPECT_EQ(page_loaded_spy.count(), 0);
    EXPECT_EQ(page_state_spy.count(), 1);
}

/**
 * @brief Verifies that disabling a view stops further persistence and refreshes.
 */
TEST_F(LiveTailingCoordinatorTest, DisablingStopsTailing)
{
    m_tailing->set_enabled(m_view_id, true);
    ASSERT_TRUE(m_tailing->is_enabled(m_view_id));

    m_tailing->set_enabled(m_view_id, false);
    append_record(QStringLiteral("ignored-entry"));
    QTest::qWait(400);

    EXPECT_FALSE(m_tailing->is_enabled(m_view_id));
    EXPECT_EQ(m_pages->get_page_state(m_view_id)->get_total_entries(), 1);
}

/**
 * @brief Verifies that a closed view cannot receive delayed tail results.
 */
TEST_F(LiveTailingCoordinatorTest, ClosedViewReceivesNoDelayedEntries)
{
    m_tailing->set_enabled(m_view_id, true);
    m_tailing->remove_view(m_view_id);
    ASSERT_TRUE(m_views->remove_view(m_view_id));

    append_record(QStringLiteral("closed-entry"));
    QTest::qWait(400);

    EXPECT_FALSE(m_tailing->is_enabled(m_view_id));
    LogQuery query;
    query.view_id = m_view_id;
    EXPECT_EQ(m_history->count_entries(query), 1);
}
