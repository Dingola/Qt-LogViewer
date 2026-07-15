#include "Qt-LogViewer/Controllers/LogPageCoordinatorTest.h"

#include <QDateTime>

#include "Qt-LogViewer/Controllers/LogPageCoordinator.h"
#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Models/LogPageState.h"
#include "Qt-LogViewer/Services/LogHistoryService.h"

/**
 * @brief Creates the services and view used by each test.
 */
void LogPageCoordinatorTest::SetUp()
{
    m_history_service = new LogHistoryService();
    m_views = new ViewRegistry();
    m_view_id = m_views->create_view();
    m_coordinator = new LogPageCoordinator(m_history_service, m_views);
}

/**
 * @brief Removes archived test data and destroys the test objects.
 */
void LogPageCoordinatorTest::TearDown()
{
    if (m_history_service != nullptr && !m_view_id.isNull())
    {
        m_history_service->remove_view_entries(m_view_id);
    }

    delete m_coordinator;
    delete m_views;
    delete m_history_service;

    m_coordinator = nullptr;
    m_views = nullptr;
    m_history_service = nullptr;
}

/**
 * @brief Creates one deterministic test entry.
 * @param message Entry message.
 * @param timestamp Timestamp assigned to the entry.
 * @return Constructed log entry.
 */
auto LogPageCoordinatorTest::create_entry(const QString& message,
                                          const QDateTime& timestamp) const -> LogEntry
{
    return LogEntry(timestamp, QStringLiteral("INFO"), message,
                    LogFileInfo(QStringLiteral("test.log"), QStringLiteral("TestApp")));
}

/**
 * @brief Verifies that assigning a query loads entries into the existing model.
 */
TEST_F(LogPageCoordinatorTest, LoadsQueryIntoExistingLogModel)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(
        QStringLiteral("oldest"),
        QDateTime::fromString(QStringLiteral("2026-01-01T08:00:00.000Z"), Qt::ISODateWithMs)));
    entries.append(create_entry(
        QStringLiteral("newest"),
        QDateTime::fromString(QStringLiteral("2026-01-01T12:00:00.000Z"), Qt::ISODateWithMs)));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;

    ASSERT_TRUE(m_coordinator->set_query(m_view_id, query));

    LogViewContext* context = m_views->get_context(m_view_id);
    ASSERT_NE(context, nullptr);

    const QVector<LogEntry> loaded_entries = context->get_entries();

    ASSERT_EQ(loaded_entries.size(), 2);
    EXPECT_EQ(loaded_entries.at(0).get_message(), QStringLiteral("newest"));
    EXPECT_EQ(loaded_entries.at(1).get_message(), QStringLiteral("oldest"));
}

/**
 * @brief Verifies that the query is scoped to the target view.
 */
TEST_F(LogPageCoordinatorTest, AssignsTargetViewToQuery)
{
    LogQuery query;

    ASSERT_TRUE(m_coordinator->set_query(m_view_id, query));

    const LogPageState* state = m_coordinator->get_page_state(m_view_id);

    ASSERT_NE(state, nullptr);
    EXPECT_EQ(state->get_query().view_id, m_view_id);
}

/**
 * @brief Verifies page-size and page-number changes.
 */
TEST_F(LogPageCoordinatorTest, LoadsSelectedPage)
{
    QVector<LogEntry> entries;

    for (int index = 1; index <= 30; ++index)
    {
        entries.append(create_entry(
            QStringLiteral("entry-%1").arg(index),
            QDateTime::fromString(QStringLiteral("2026-01-01T12:00:00.000Z"), Qt::ISODateWithMs)));
    }

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;

    ASSERT_TRUE(m_coordinator->set_query(m_view_id, query));
    ASSERT_TRUE(m_coordinator->set_page_size(m_view_id, 10));
    ASSERT_TRUE(m_coordinator->set_current_page(m_view_id, 2));

    const LogPageState* state = m_coordinator->get_page_state(m_view_id);

    ASSERT_NE(state, nullptr);
    EXPECT_EQ(state->get_current_page(), 2);
    EXPECT_EQ(state->get_total_pages(), 3);
    EXPECT_EQ(state->get_total_entries(), 30);
    EXPECT_EQ(state->get_offset(), 10);

    const QVector<LogEntry> loaded_entries = m_views->get_context(m_view_id)->get_entries();

    ASSERT_EQ(loaded_entries.size(), 10);
    EXPECT_EQ(loaded_entries.first().get_message(), QStringLiteral("entry-20"));
    EXPECT_EQ(loaded_entries.last().get_message(), QStringLiteral("entry-11"));
}

/**
 * @brief Verifies that query filters are applied before loading the model.
 */
TEST_F(LogPageCoordinatorTest, LoadsFilteredEntries)
{
    QVector<LogEntry> entries;
    entries.append(LogEntry(
        QDateTime::fromString(QStringLiteral("2026-01-01T12:00:00.000Z"), Qt::ISODateWithMs),
        QStringLiteral("ERROR"), QStringLiteral("network failed"),
        LogFileInfo(QStringLiteral("backend.log"), QStringLiteral("Backend"))));
    entries.append(LogEntry(
        QDateTime::fromString(QStringLiteral("2026-01-01T11:00:00.000Z"), Qt::ISODateWithMs),
        QStringLiteral("INFO"), QStringLiteral("network connected"),
        LogFileInfo(QStringLiteral("backend.log"), QStringLiteral("Backend"))));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.log_levels = {QStringLiteral("ERROR")};
    query.search_text = QStringLiteral("network");
    query.search_fields = {LogField::Message};

    ASSERT_TRUE(m_coordinator->set_query(m_view_id, query));

    const QVector<LogEntry> loaded_entries = m_views->get_context(m_view_id)->get_entries();

    ASSERT_EQ(loaded_entries.size(), 1);
    EXPECT_EQ(loaded_entries.first().get_message(), QStringLiteral("network failed"));
}

/**
 * @brief Verifies that removing a view removes its page state.
 */
TEST_F(LogPageCoordinatorTest, RemovesPageStateWithView)
{
    LogQuery query;

    ASSERT_TRUE(m_coordinator->set_query(m_view_id, query));
    ASSERT_NE(m_coordinator->get_page_state(m_view_id), nullptr);

    ASSERT_TRUE(m_views->remove_view(m_view_id));

    EXPECT_EQ(m_coordinator->get_page_state(m_view_id), nullptr);

    m_history_service->remove_view_entries(m_view_id);
    m_view_id = {};
}

/**
 * @brief Verifies that invalid page sizes are rejected.
 */
TEST_F(LogPageCoordinatorTest, RejectsInvalidPageSize)
{
    LogQuery query;

    ASSERT_TRUE(m_coordinator->set_query(m_view_id, query));

    EXPECT_FALSE(m_coordinator->set_page_size(m_view_id, 0));
    EXPECT_FALSE(m_coordinator->set_page_size(m_view_id, -1));

    const LogPageState* state = m_coordinator->get_page_state(m_view_id);

    ASSERT_NE(state, nullptr);
    EXPECT_EQ(state->get_page_size(), 25);
}
