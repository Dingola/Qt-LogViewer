/**
 * @file LogQueryControllerTest.cpp
 * @brief Verifies complete query updates and single-reload state restoration.
 */

#include "Qt-LogViewer/Controllers/LogQueryControllerTest.h"

#include <QSignalSpy>

#include "Qt-LogViewer/Controllers/FilterCoordinator.h"
#include "Qt-LogViewer/Controllers/LogPageCoordinator.h"
#include "Qt-LogViewer/Controllers/LogQueryController.h"
#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogPageState.h"
#include "Qt-LogViewer/Services/LogHistoryService.h"

/**
 * @brief Creates the isolated controller graph used by each test.
 */
void LogQueryControllerTest::SetUp()
{
    m_history = new LogHistoryService();
    m_views = new ViewRegistry();
    m_filters = new FilterCoordinator(m_views);
    m_pages = new LogPageCoordinator(m_history, m_views);
    m_queries = new LogQueryController(m_filters, m_pages, m_views);
    m_view_id = m_views->create_view();
}

/**
 * @brief Removes test history and destroys the controller graph.
 */
void LogQueryControllerTest::TearDown()
{
    if (m_history != nullptr && !m_view_id.isNull())
    {
        m_history->remove_view_entries(m_view_id);
    }

    delete m_queries;
    delete m_pages;
    delete m_filters;
    delete m_views;
    delete m_history;

    m_queries = nullptr;
    m_pages = nullptr;
    m_filters = nullptr;
    m_views = nullptr;
    m_history = nullptr;
}

/**
 * @brief Creates a deterministic log entry.
 * @param index Sequence number included in the message and timestamp.
 * @param file_path Source file path.
 * @return Constructed log entry.
 */
auto LogQueryControllerTest::create_entry(int index, const QString& file_path) const -> LogEntry
{
    return LogEntry(
        QDateTime::fromString(
            QStringLiteral("2026-01-01T12:%1:00.000Z").arg(index, 2, 10, QLatin1Char('0')),
            Qt::ISODateWithMs),
        index % 2 == 0 ? QStringLiteral("INFO") : QStringLiteral("ERROR"),
        QStringLiteral("entry-%1").arg(index), LogFileInfo(file_path, QStringLiteral("TestApp")));
}

/**
 * @brief Verifies that a filter change resets paging and emits one completed page update.
 */
TEST_F(LogQueryControllerTest, FilterChangeResetsPageAndReloadsOnce)
{
    QVector<LogEntry> entries;
    for (int index = 1; index <= 30; ++index)
    {
        entries.append(create_entry(index, QStringLiteral("test.log")));
    }

    ASSERT_TRUE(m_history->add_entries(m_view_id, entries));

    SessionViewState initial_state;
    initial_state.page_size = 10;
    initial_state.current_page = 2;
    ASSERT_TRUE(m_queries->apply_view_state(m_view_id, initial_state));

    QSignalSpy page_loaded_spy(m_pages, &LogPageCoordinator::page_loaded);

    ASSERT_TRUE(
        m_queries->set_search(m_view_id, QStringLiteral("entry-2"), SearchField::Message, false));

    EXPECT_EQ(page_loaded_spy.count(), 1);

    const LogPageState* page_state = m_pages->get_page_state(m_view_id);
    ASSERT_NE(page_state, nullptr);
    EXPECT_EQ(page_state->get_current_page(), 1);
    EXPECT_EQ(page_state->get_query().search_text, QStringLiteral("entry-2"));
    EXPECT_EQ(page_state->get_query().search_fields, QSet<QString>{LogField::Message});

    const QVector<LogEntry> visible_entries = m_views->get_context(m_view_id)->get_entries();
    ASSERT_FALSE(visible_entries.isEmpty());
    for (const LogEntry& entry: visible_entries)
    {
        EXPECT_TRUE(entry.get_message().contains(QStringLiteral("entry-2")));
    }
}

/**
 * @brief Verifies that restoring filters, sorting, and paging performs one model reload.
 */
TEST_F(LogQueryControllerTest, AppliesRestoredStateWithSingleReload)
{
    QVector<LogEntry> entries;
    for (int index = 1; index <= 6; ++index)
    {
        entries.append(create_entry(index, QStringLiteral("test.log")));
    }

    ASSERT_TRUE(m_history->add_entries(m_view_id, entries));

    SessionViewState state;
    state.filters.log_levels = {QStringLiteral("INFO")};
    state.page_size = 1;
    state.current_page = 3;
    state.sort_column = LogModel::Timestamp;
    state.sort_order = Qt::AscendingOrder;

    QSignalSpy page_loaded_spy(m_pages, &LogPageCoordinator::page_loaded);

    ASSERT_TRUE(m_queries->apply_view_state(m_view_id, state));

    EXPECT_EQ(page_loaded_spy.count(), 1);

    const LogPageState* page_state = m_pages->get_page_state(m_view_id);
    ASSERT_NE(page_state, nullptr);
    EXPECT_EQ(page_state->get_current_page(), 3);
    EXPECT_EQ(page_state->get_page_size(), 1);
    EXPECT_EQ(page_state->get_total_entries(), 3);
    EXPECT_EQ(page_state->get_query().log_levels, QSet<QString>{QStringLiteral("info")});
    EXPECT_EQ(page_state->get_query().sort_order, Qt::AscendingOrder);

    const QVector<LogEntry> visible_entries = m_views->get_context(m_view_id)->get_entries();
    ASSERT_EQ(visible_entries.size(), 1);
    EXPECT_EQ(visible_entries.first().get_message(), QStringLiteral("entry-6"));
}

/**
 * @brief Verifies that visibility and sorting operations each trigger one complete update.
 */
TEST_F(LogQueryControllerTest, VisibilityAndSortingReloadOncePerOperation)
{
    const QString first_file = QStringLiteral("first.log");
    const QString second_file = QStringLiteral("second.log");
    m_views->set_loaded_files(m_view_id, {LogFileInfo(first_file), LogFileInfo(second_file)});
    ASSERT_TRUE(m_history->add_entries(
        m_view_id, {create_entry(1, first_file), create_entry(2, second_file)}));
    ASSERT_TRUE(m_queries->reload_query(m_view_id));

    QSignalSpy page_loaded_spy(m_pages, &LogPageCoordinator::page_loaded);

    ASSERT_TRUE(m_queries->toggle_file_visibility(m_view_id, first_file));
    EXPECT_EQ(page_loaded_spy.count(), 1);
    EXPECT_TRUE(m_queries->create_query(m_view_id).hidden_files.contains(first_file));

    page_loaded_spy.clear();

    ASSERT_TRUE(m_queries->set_sort(m_view_id, LogModel::Message, Qt::AscendingOrder));
    EXPECT_EQ(page_loaded_spy.count(), 1);
    EXPECT_EQ(m_queries->create_query(m_view_id).sort_field, LogField::Message);
    EXPECT_EQ(m_queries->create_query(m_view_id).sort_order, Qt::AscendingOrder);
}
