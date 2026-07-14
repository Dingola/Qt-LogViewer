#include "Qt-LogViewer/Services/LogHistoryServiceTest.h"

#include <QDateTime>

/**
 * @brief Creates an isolated history-service instance.
 */
void LogHistoryServiceTest::SetUp()
{
    m_history_service = new LogHistoryService();
    m_view_id = QUuid::createUuid();
}

/**
 * @brief Destroys the isolated history-service instance.
 */
void LogHistoryServiceTest::TearDown()
{
    if (m_history_service != nullptr)
    {
        m_history_service->remove_view_entries(m_view_id);
    }

    delete m_history_service;
    m_history_service = nullptr;
}
/**
 * @brief Creates one deterministic test entry.
 * @param message Entry message.
 * @param file_path Source file path.
 * @return Constructed parsed log entry.
 */
auto LogHistoryServiceTest::create_entry(const QString& message,
                                         const QString& file_path) const -> LogEntry
{
    const LogEntry entry(
        QDateTime::fromString(QStringLiteral("2026-01-01T12:00:00.000Z"), Qt::ISODateWithMs),
        QStringLiteral("INFO"), message, LogFileInfo(file_path, QStringLiteral("HistoryTestApp")));
    return entry;
}

/**
 * @brief Verifies that the SQLite schema and FTS5 index are available.
 */
TEST_F(LogHistoryServiceTest, CreatesAvailableSqliteAndFtsStorage)
{
    ASSERT_NE(m_history_service, nullptr);
    EXPECT_TRUE(m_history_service->is_available());
    EXPECT_FALSE(m_history_service->get_database_path().isEmpty());
}

/**
 * @brief Verifies archived entries can be retrieved by FTS5 message search.
 */
TEST_F(LogHistoryServiceTest, StoresAndSearchesEntriesWithFts)
{
    ASSERT_TRUE(m_history_service->is_available());

    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("network connection established"),
                                QStringLiteral("first.log")));
    entries.append(
        create_entry(QStringLiteral("database migration completed"), QStringLiteral("second.log")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    const QVector<LogEntry> results = m_history_service->search_entries(
        m_view_id, QStringLiteral("network"), SearchField::Message);

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results.first().get_message(), QStringLiteral("network connection established"));
    EXPECT_EQ(results.first().get_file_info().get_file_path(), QStringLiteral("first.log"));
}

/**
 * @brief Verifies file-specific cleanup preserves entries belonging to other files.
 */
TEST_F(LogHistoryServiceTest, RemovesOnlySpecifiedFileHistory)
{
    ASSERT_TRUE(m_history_service->is_available());

    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("alpha message"), QStringLiteral("first.log")));
    entries.append(create_entry(QStringLiteral("beta message"), QStringLiteral("second.log")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    m_history_service->remove_file_entries(m_view_id, QStringLiteral("first.log"));

    const QVector<LogEntry> first_results =
        m_history_service->search_entries(m_view_id, QStringLiteral("alpha"), SearchField::Message);
    const QVector<LogEntry> second_results =
        m_history_service->search_entries(m_view_id, QStringLiteral("beta"), SearchField::Message);

    EXPECT_TRUE(first_results.isEmpty());
    ASSERT_EQ(second_results.size(), 1);
    EXPECT_EQ(second_results.first().get_file_info().get_file_path(), QStringLiteral("second.log"));
}

/**
 * @brief Verifies view cleanup deletes every archived entry for the removed view.
 */
TEST_F(LogHistoryServiceTest, RemovesAllHistoryForView)
{
    ASSERT_TRUE(m_history_service->is_available());

    QVector<LogEntry> entries;
    entries.append(
        create_entry(QStringLiteral("retained until view deletion"), QStringLiteral("view.log")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    m_history_service->remove_view_entries(m_view_id);

    const QVector<LogEntry> results = m_history_service->search_entries(
        m_view_id, QStringLiteral("retained"), SearchField::Message);

    EXPECT_TRUE(results.isEmpty());
}
