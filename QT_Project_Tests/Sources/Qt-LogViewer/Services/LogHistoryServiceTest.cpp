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
 * @param level Entry log level.
 * @param app_name Entry application name.
 * @return Constructed parsed log entry.
 */
auto LogHistoryServiceTest::create_entry(const QString& message, const QString& file_path,
                                         const QString& level,
                                         const QString& app_name) const -> LogEntry
{
    const LogEntry entry(
        QDateTime::fromString(QStringLiteral("2026-01-01T12:00:00.000Z"), Qt::ISODateWithMs), level,
        message, LogFileInfo(file_path, app_name));
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

/**
 * @brief Verifies that count_entries returns every archived entry for one view.
 */
TEST_F(LogHistoryServiceTest, CountsEntriesForView)
{
    ASSERT_TRUE(m_history_service->is_available());

    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("first entry"), QStringLiteral("first.log")));
    entries.append(create_entry(QStringLiteral("second entry"), QStringLiteral("second.log")));
    entries.append(create_entry(QStringLiteral("third entry"), QStringLiteral("third.log")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;

    EXPECT_EQ(m_history_service->count_entries(query), 3);
}

/**
 * @brief Verifies that entries belonging to another view are not counted.
 */
TEST_F(LogHistoryServiceTest, CountsOnlyEntriesForRequestedView)
{
    ASSERT_TRUE(m_history_service->is_available());

    const QUuid other_view_id = QUuid::createUuid();

    QVector<LogEntry> current_view_entries;
    current_view_entries.append(
        create_entry(QStringLiteral("current view one"), QStringLiteral("current-one.log")));
    current_view_entries.append(
        create_entry(QStringLiteral("current view two"), QStringLiteral("current-two.log")));

    QVector<LogEntry> other_view_entries;
    other_view_entries.append(
        create_entry(QStringLiteral("other view entry"), QStringLiteral("other.log")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, current_view_entries));
    ASSERT_TRUE(m_history_service->add_entries(other_view_id, other_view_entries));

    LogQuery query;
    query.view_id = m_view_id;

    EXPECT_EQ(m_history_service->count_entries(query), 2);

    m_history_service->remove_view_entries(other_view_id);
}

/**
 * @brief Verifies that a view without archived entries has a count of zero.
 */
TEST_F(LogHistoryServiceTest, ReturnsZeroForEmptyView)
{
    ASSERT_TRUE(m_history_service->is_available());

    LogQuery query;
    query.view_id = m_view_id;

    EXPECT_EQ(m_history_service->count_entries(query), 0);
}

/**
 * @brief Verifies that a query without a view identifier is rejected.
 */
TEST_F(LogHistoryServiceTest, ReturnsZeroForNullViewId)
{
    ASSERT_TRUE(m_history_service->is_available());

    const LogQuery query;

    EXPECT_EQ(m_history_service->count_entries(query), 0);
}

/**
 * @brief Verifies that entries can be counted by application name.
 */
TEST_F(LogHistoryServiceTest, CountsEntriesMatchingAppName)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("first"), QStringLiteral("first.log"),
                                QStringLiteral("INFO"), QStringLiteral("Frontend")));
    entries.append(create_entry(QStringLiteral("second"), QStringLiteral("second.log"),
                                QStringLiteral("INFO"), QStringLiteral("Backend")));
    entries.append(create_entry(QStringLiteral("third"), QStringLiteral("third.log"),
                                QStringLiteral("ERROR"), QStringLiteral("Frontend")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;
    query.app_name = QStringLiteral("Frontend");

    EXPECT_EQ(m_history_service->count_entries(query), 2);
}

/**
 * @brief Verifies that entries can be counted by multiple log levels.
 */
TEST_F(LogHistoryServiceTest, CountsEntriesMatchingLogLevels)
{
    QVector<LogEntry> entries;
    entries.append(
        create_entry(QStringLiteral("info"), QStringLiteral("info.log"), QStringLiteral("INFO")));
    entries.append(create_entry(QStringLiteral("warning"), QStringLiteral("warning.log"),
                                QStringLiteral("Warning")));
    entries.append(create_entry(QStringLiteral("error"), QStringLiteral("error.log"),
                                QStringLiteral(" error ")));
    entries.append(create_entry(QStringLiteral("debug"), QStringLiteral("debug.log"),
                                QStringLiteral("DEBUG")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;
    query.log_levels = {QStringLiteral("WARNING"), QStringLiteral("ERROR")};

    EXPECT_EQ(m_history_service->count_entries(query), 2);
}

/**
 * @brief Verifies that only entries from the selected file are counted.
 */
TEST_F(LogHistoryServiceTest, CountsEntriesFromShowOnlyFile)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("first"), QStringLiteral("first.log")));
    entries.append(create_entry(QStringLiteral("second"), QStringLiteral("second.log")));
    entries.append(create_entry(QStringLiteral("third"), QStringLiteral("first.log")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;
    query.show_only_file = QStringLiteral("first.log");

    EXPECT_EQ(m_history_service->count_entries(query), 2);
}

/**
 * @brief Verifies that entries from hidden files are excluded.
 */
TEST_F(LogHistoryServiceTest, ExcludesEntriesFromHiddenFiles)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("first"), QStringLiteral("first.log")));
    entries.append(create_entry(QStringLiteral("second"), QStringLiteral("second.log")));
    entries.append(create_entry(QStringLiteral("third"), QStringLiteral("third.log")));
    entries.append(create_entry(QStringLiteral("fourth"), QStringLiteral("first.log")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;
    query.hidden_files = {QStringLiteral("first.log"), QStringLiteral("third.log")};

    EXPECT_EQ(m_history_service->count_entries(query), 1);
}

/**
 * @brief Verifies that file, application, and level filters are combined.
 */
TEST_F(LogHistoryServiceTest, CombinesStructuredQueryFilters)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("matching"), QStringLiteral("active.log"),
                                QStringLiteral("ERROR"), QStringLiteral("Backend")));
    entries.append(create_entry(QStringLiteral("wrong level"), QStringLiteral("active.log"),
                                QStringLiteral("INFO"), QStringLiteral("Backend")));
    entries.append(create_entry(QStringLiteral("wrong app"), QStringLiteral("active.log"),
                                QStringLiteral("ERROR"), QStringLiteral("Frontend")));
    entries.append(create_entry(QStringLiteral("hidden"), QStringLiteral("hidden.log"),
                                QStringLiteral("ERROR"), QStringLiteral("Backend")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;
    query.app_name = QStringLiteral("Backend");
    query.log_levels = {QStringLiteral("ERROR")};
    query.hidden_files = {QStringLiteral("hidden.log")};

    EXPECT_EQ(m_history_service->count_entries(query), 1);
}

/**
 * @brief Verifies that text can be searched across the FTS5 fields.
 */
TEST_F(LogHistoryServiceTest, CountsEntriesMatchingSearchText)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("network connection established"),
                                QStringLiteral("first.log")));
    entries.append(
        create_entry(QStringLiteral("database migration completed"), QStringLiteral("second.log")));
    entries.append(
        create_entry(QStringLiteral("network connection closed"), QStringLiteral("third.log")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;
    query.search_text = QStringLiteral("network");

    EXPECT_EQ(m_history_service->count_entries(query), 2);
}

/**
 * @brief Verifies that text searching can be restricted to the message field.
 */
TEST_F(LogHistoryServiceTest, CountsEntriesMatchingSelectedSearchField)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("needle in message"), QStringLiteral("first.log"),
                                QStringLiteral("INFO"), QStringLiteral("Frontend")));
    entries.append(create_entry(QStringLiteral("ordinary message"), QStringLiteral("second.log"),
                                QStringLiteral("INFO"), QStringLiteral("needle")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;
    query.search_text = QStringLiteral("needle");
    query.search_fields = {LogField::Message};

    EXPECT_EQ(m_history_service->count_entries(query), 1);
}

/**
 * @brief Verifies that text can be searched across multiple selected fields.
 */
TEST_F(LogHistoryServiceTest, CountsEntriesMatchingMultipleSearchFields)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("needle in message"), QStringLiteral("first.log"),
                                QStringLiteral("INFO"), QStringLiteral("Frontend")));
    entries.append(create_entry(QStringLiteral("ordinary message"), QStringLiteral("second.log"),
                                QStringLiteral("INFO"), QStringLiteral("needle")));
    entries.append(create_entry(QStringLiteral("ordinary message"), QStringLiteral("needle.log"),
                                QStringLiteral("INFO"), QStringLiteral("Backend")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;
    query.search_text = QStringLiteral("needle");
    query.search_fields = {LogField::Message, LogField::AppName};

    EXPECT_EQ(m_history_service->count_entries(query), 2);
}

/**
 * @brief Verifies that text search and structured filters are combined.
 */
TEST_F(LogHistoryServiceTest, CombinesTextSearchWithStructuredFilters)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("network failed"), QStringLiteral("backend.log"),
                                QStringLiteral("ERROR"), QStringLiteral("Backend")));
    entries.append(create_entry(QStringLiteral("network connected"), QStringLiteral("backend.log"),
                                QStringLiteral("INFO"), QStringLiteral("Backend")));
    entries.append(create_entry(QStringLiteral("network failed"), QStringLiteral("frontend.log"),
                                QStringLiteral("ERROR"), QStringLiteral("Frontend")));
    entries.append(create_entry(QStringLiteral("database failed"), QStringLiteral("database.log"),
                                QStringLiteral("ERROR"), QStringLiteral("Backend")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;
    query.app_name = QStringLiteral("Backend");
    query.log_levels = {QStringLiteral("ERROR")};
    query.search_text = QStringLiteral("network");
    query.search_fields = {LogField::Message};

    EXPECT_EQ(m_history_service->count_entries(query), 1);
}

/**
 * @brief Verifies that unsupported search fields do not broaden the query.
 */
TEST_F(LogHistoryServiceTest, RejectsUnsupportedSearchField)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("needle"), QStringLiteral("first.log")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;
    query.search_text = QStringLiteral("needle");
    query.search_fields = {QStringLiteral("thread_id")};

    EXPECT_EQ(m_history_service->count_entries(query), 0);
}

/**
 * @brief Verifies that regular-expression searches are not executed as FTS searches.
 */
TEST_F(LogHistoryServiceTest, RejectsRegexSearch)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("network failed"), QStringLiteral("first.log")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;
    query.search_text = QStringLiteral("network.*failed");
    query.search_fields = {LogField::Message};
    query.use_regex = true;

    EXPECT_EQ(m_history_service->count_entries(query), 0);
}

/**
 * @brief Verifies that pages are ordered by newest timestamp first.
 */
TEST_F(LogHistoryServiceTest, LoadsNewestEntriesFirstByDefault)
{
    const LogEntry oldest(
        QDateTime::fromString(QStringLiteral("2026-01-01T08:00:00.000Z"), Qt::ISODateWithMs),
        QStringLiteral("INFO"), QStringLiteral("oldest"),
        LogFileInfo(QStringLiteral("test.log"), QStringLiteral("TestApp")));

    const LogEntry newest(
        QDateTime::fromString(QStringLiteral("2026-01-01T12:00:00.000Z"), Qt::ISODateWithMs),
        QStringLiteral("INFO"), QStringLiteral("newest"),
        LogFileInfo(QStringLiteral("test.log"), QStringLiteral("TestApp")));

    const LogEntry middle(
        QDateTime::fromString(QStringLiteral("2026-01-01T10:00:00.000Z"), Qt::ISODateWithMs),
        QStringLiteral("INFO"), QStringLiteral("middle"),
        LogFileInfo(QStringLiteral("test.log"), QStringLiteral("TestApp")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, {oldest, newest, middle}));

    LogQuery query;
    query.view_id = m_view_id;

    const QVector<LogEntry> page = m_history_service->load_entries_page(query, 0, 2);

    ASSERT_EQ(page.size(), 2);
    EXPECT_EQ(page.at(0).get_message(), QStringLiteral("newest"));
    EXPECT_EQ(page.at(1).get_message(), QStringLiteral("middle"));
}

/**
 * @brief Verifies that offset and limit select the requested page.
 */
TEST_F(LogHistoryServiceTest, LoadsRequestedEntryPage)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("first"), QStringLiteral("test.log")));
    entries.append(create_entry(QStringLiteral("second"), QStringLiteral("test.log")));
    entries.append(create_entry(QStringLiteral("third"), QStringLiteral("test.log")));
    entries.append(create_entry(QStringLiteral("fourth"), QStringLiteral("test.log")));
    entries.append(create_entry(QStringLiteral("fifth"), QStringLiteral("test.log")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;

    const QVector<LogEntry> page = m_history_service->load_entries_page(query, 2, 2);

    ASSERT_EQ(page.size(), 2);
    EXPECT_EQ(page.at(0).get_message(), QStringLiteral("third"));
    EXPECT_EQ(page.at(1).get_message(), QStringLiteral("second"));
}

/**
 * @brief Verifies ascending timestamp sorting.
 */
TEST_F(LogHistoryServiceTest, LoadsEntriesByAscendingTimestamp)
{
    const LogEntry newest(
        QDateTime::fromString(QStringLiteral("2026-01-01T12:00:00.000Z"), Qt::ISODateWithMs),
        QStringLiteral("INFO"), QStringLiteral("newest"),
        LogFileInfo(QStringLiteral("test.log"), QStringLiteral("TestApp")));

    const LogEntry oldest(
        QDateTime::fromString(QStringLiteral("2026-01-01T08:00:00.000Z"), Qt::ISODateWithMs),
        QStringLiteral("INFO"), QStringLiteral("oldest"),
        LogFileInfo(QStringLiteral("test.log"), QStringLiteral("TestApp")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, {newest, oldest}));

    LogQuery query;
    query.view_id = m_view_id;
    query.sort_field = LogField::Timestamp;
    query.sort_order = Qt::AscendingOrder;

    const QVector<LogEntry> page = m_history_service->load_entries_page(query, 0, 10);

    ASSERT_EQ(page.size(), 2);
    EXPECT_EQ(page.at(0).get_message(), QStringLiteral("oldest"));
    EXPECT_EQ(page.at(1).get_message(), QStringLiteral("newest"));
}

/**
 * @brief Verifies that insertion order resolves equal sort values.
 */
TEST_F(LogHistoryServiceTest, UsesInsertionOrderAsSortTieBreaker)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("first"), QStringLiteral("test.log")));
    entries.append(create_entry(QStringLiteral("second"), QStringLiteral("test.log")));
    entries.append(create_entry(QStringLiteral("third"), QStringLiteral("test.log")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;

    const QVector<LogEntry> page = m_history_service->load_entries_page(query, 0, 10);

    ASSERT_EQ(page.size(), 3);
    EXPECT_EQ(page.at(0).get_message(), QStringLiteral("third"));
    EXPECT_EQ(page.at(1).get_message(), QStringLiteral("second"));
    EXPECT_EQ(page.at(2).get_message(), QStringLiteral("first"));
}

/**
 * @brief Verifies sorting by a selected text field.
 */
TEST_F(LogHistoryServiceTest, LoadsEntriesSortedByMessage)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("Charlie"), QStringLiteral("test.log")));
    entries.append(create_entry(QStringLiteral("alpha"), QStringLiteral("test.log")));
    entries.append(create_entry(QStringLiteral("Bravo"), QStringLiteral("test.log")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;
    query.sort_field = LogField::Message;
    query.sort_order = Qt::AscendingOrder;

    const QVector<LogEntry> page = m_history_service->load_entries_page(query, 0, 10);

    ASSERT_EQ(page.size(), 3);
    EXPECT_EQ(page.at(0).get_message(), QStringLiteral("alpha"));
    EXPECT_EQ(page.at(1).get_message(), QStringLiteral("Bravo"));
    EXPECT_EQ(page.at(2).get_message(), QStringLiteral("Charlie"));
}

/**
 * @brief Verifies that page loading applies the same filters as entry counting.
 */
TEST_F(LogHistoryServiceTest, LoadsEntriesMatchingQueryFilters)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("network failed"), QStringLiteral("backend.log"),
                                QStringLiteral("ERROR"), QStringLiteral("Backend")));
    entries.append(create_entry(QStringLiteral("network connected"), QStringLiteral("backend.log"),
                                QStringLiteral("INFO"), QStringLiteral("Backend")));
    entries.append(create_entry(QStringLiteral("network failed"), QStringLiteral("frontend.log"),
                                QStringLiteral("ERROR"), QStringLiteral("Frontend")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;
    query.app_name = QStringLiteral("Backend");
    query.log_levels = {QStringLiteral("ERROR")};
    query.search_text = QStringLiteral("network");
    query.search_fields = {LogField::Message};

    const QVector<LogEntry> page = m_history_service->load_entries_page(query, 0, 10);

    ASSERT_EQ(page.size(), 1);
    EXPECT_EQ(page.first().get_message(), QStringLiteral("network failed"));
    EXPECT_EQ(page.first().get_app_name(), QStringLiteral("Backend"));
}

/**
 * @brief Verifies that invalid page ranges return no entries.
 */
TEST_F(LogHistoryServiceTest, RejectsInvalidPageRange)
{
    LogQuery query;
    query.view_id = m_view_id;

    EXPECT_TRUE(m_history_service->load_entries_page(query, -1, 10).isEmpty());
    EXPECT_TRUE(m_history_service->load_entries_page(query, 0, 0).isEmpty());
    EXPECT_TRUE(m_history_service->load_entries_page(query, 0, -1).isEmpty());
}

/**
 * @brief Verifies that unsupported sort fields do not alter the SQL query.
 */
TEST_F(LogHistoryServiceTest, RejectsUnsupportedSortField)
{
    ASSERT_TRUE(m_history_service->add_entries(
        m_view_id, {create_entry(QStringLiteral("entry"), QStringLiteral("test.log"))}));

    LogQuery query;
    query.view_id = m_view_id;
    query.sort_field = QStringLiteral("duration_ms");

    EXPECT_TRUE(m_history_service->load_entries_page(query, 0, 10).isEmpty());
}
