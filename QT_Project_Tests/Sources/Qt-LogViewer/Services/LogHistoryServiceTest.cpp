#include "Qt-LogViewer/Services/LogHistoryServiceTest.h"

#include <QDateTime>
#include <QElapsedTimer>
#include <iostream>

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
 * @brief Creates a deterministic batch with one shared timestamp.
 * @param entry_count Number of entries to create.
 * @return Generated entries ordered by their record number.
 */
auto LogHistoryServiceTest::create_large_entry_batch(qsizetype entry_count) const
    -> QVector<LogEntry>
{
    QVector<LogEntry> entries;

    if (entry_count > 0)
    {
        entries.reserve(entry_count);

        const QDateTime timestamp =
            QDateTime::fromString(QStringLiteral("2026-01-01T12:00:00.000Z"), Qt::ISODateWithMs);

        const LogFileInfo file_info(QStringLiteral("large-history.log"),
                                    QStringLiteral("LargeHistoryApp"));

        for (qsizetype index = 1; index <= entry_count; ++index)
        {
            const QString message = QStringLiteral("record_%1").arg(index, 6, 10, QLatin1Char('0'));

            const QString level = index % 2 == 0 ? QStringLiteral("INFO") : QStringLiteral("ERROR");

            entries.append(LogEntry(timestamp, level, message, file_info));
        }
    }

    return entries;
}

/**
 * @brief Verifies one exact message search against the archived view.
 * @param message Expected unique message.
 */
auto LogHistoryServiceTest::expect_single_message_result(const QString& message) const -> void
{
    LogQuery query;
    query.view_id = m_view_id;
    query.search_text = message;
    query.search_fields = {LogField::Message};

    EXPECT_EQ(m_history_service->count_entries(query), 1);

    const QVector<LogEntry> entries = m_history_service->load_entries_page(query, 0, 25);

    ASSERT_EQ(entries.size(), 1);

    EXPECT_EQ(entries.first().get_message(), message);
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
 * @brief Verifies case-insensitive regular-expression searching.
 */
TEST_F(LogHistoryServiceTest, CountsEntriesMatchingRegexSearch)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("Network timeout"), QStringLiteral("first.log")));
    entries.append(create_entry(QStringLiteral("network connected"), QStringLiteral("second.log")));
    entries.append(create_entry(QStringLiteral("database timeout"), QStringLiteral("third.log")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;
    query.search_text = QStringLiteral("^network\\s+(timeout|connected)$");
    query.search_fields = {LogField::Message};
    query.use_regex = true;

    EXPECT_EQ(m_history_service->count_entries(query), 2);
}

/**
 * @brief Verifies that regular-expression searching respects selected fields.
 */
TEST_F(LogHistoryServiceTest, AppliesRegexToSelectedSearchField)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("ordinary message"), QStringLiteral("first.log"),
                                QStringLiteral("INFO"), QStringLiteral("Backend-42")));
    entries.append(create_entry(QStringLiteral("Backend-21"), QStringLiteral("second.log"),
                                QStringLiteral("INFO"), QStringLiteral("Frontend")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;
    query.search_text = QStringLiteral("^backend-\\d+$");
    query.search_fields = {LogField::AppName};
    query.use_regex = true;

    EXPECT_EQ(m_history_service->count_entries(query), 1);

    const QVector<LogEntry> page = m_history_service->load_entries_page(query, 0, 25);

    ASSERT_EQ(page.size(), 1);
    EXPECT_EQ(page.first().get_app_name(), QStringLiteral("Backend-42"));
}

/**
 * @brief Verifies that regular-expression searching supports pagination.
 */
TEST_F(LogHistoryServiceTest, LoadsPageMatchingRegexSearch)
{
    QVector<LogEntry> entries;
    entries.append(
        create_entry(QStringLiteral("request-100 completed"), QStringLiteral("first.log")));
    entries.append(
        create_entry(QStringLiteral("request-200 failed"), QStringLiteral("second.log")));
    entries.append(create_entry(QStringLiteral("background task"), QStringLiteral("third.log")));
    entries.append(
        create_entry(QStringLiteral("request-300 completed"), QStringLiteral("fourth.log")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;
    query.search_text = QStringLiteral("^request-\\d+");
    query.search_fields = {LogField::Message};
    query.use_regex = true;

    const QVector<LogEntry> page = m_history_service->load_entries_page(query, 1, 1);

    ASSERT_EQ(page.size(), 1);
    EXPECT_EQ(page.first().get_message(), QStringLiteral("request-200 failed"));
}

/**
 * @brief Verifies that regular-expression searching affects level counts.
 */
TEST_F(LogHistoryServiceTest, AppliesRegexSearchToLogLevelCounts)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("network timeout"), QStringLiteral("first.log"),
                                QStringLiteral("ERROR")));
    entries.append(create_entry(QStringLiteral("network connected"), QStringLiteral("second.log"),
                                QStringLiteral("INFO")));
    entries.append(create_entry(QStringLiteral("database timeout"), QStringLiteral("third.log"),
                                QStringLiteral("ERROR")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;
    query.search_text = QStringLiteral("^network");
    query.search_fields = {LogField::Message};
    query.use_regex = true;

    const QMap<QString, qsizetype> counts = m_history_service->get_log_level_counts(query);

    EXPECT_EQ(counts.value(QStringLiteral("ERROR")), 1);
    EXPECT_EQ(counts.value(QStringLiteral("INFO")), 1);
}

/**
 * @brief Verifies that invalid regular expressions return no results.
 */
TEST_F(LogHistoryServiceTest, RejectsInvalidRegexSearch)
{
    ASSERT_TRUE(m_history_service->add_entries(
        m_view_id, {create_entry(QStringLiteral("network failed"), QStringLiteral("first.log"))}));

    LogQuery query;
    query.view_id = m_view_id;
    query.search_text = QStringLiteral("(");
    query.search_fields = {LogField::Message};
    query.use_regex = true;

    EXPECT_EQ(m_history_service->count_entries(query), 0);
    EXPECT_TRUE(m_history_service->load_entries_page(query, 0, 25).isEmpty());
    EXPECT_TRUE(m_history_service->get_log_level_counts(query).isEmpty());
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

/**
 * @brief Verifies that entries are counted by normalized log level.
 */
TEST_F(LogHistoryServiceTest, CountsEntriesGroupedByLogLevel)
{
    QVector<LogEntry> entries;
    entries.append(
        create_entry(QStringLiteral("first"), QStringLiteral("first.log"), QStringLiteral("INFO")));
    entries.append(create_entry(QStringLiteral("second"), QStringLiteral("second.log"),
                                QStringLiteral("info")));
    entries.append(create_entry(QStringLiteral("third"), QStringLiteral("third.log"),
                                QStringLiteral(" Warning ")));
    entries.append(create_entry(QStringLiteral("fourth"), QStringLiteral("fourth.log"),
                                QStringLiteral("CUSTOM")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;

    const QMap<QString, qsizetype> counts = m_history_service->get_log_level_counts(query);

    EXPECT_EQ(counts.value(QStringLiteral("INFO")), 2);
    EXPECT_EQ(counts.value(QStringLiteral("WARNING")), 1);
    EXPECT_EQ(counts.value(QStringLiteral("CUSTOM")), 1);
}

/**
 * @brief Verifies that entries from other views are excluded.
 */
TEST_F(LogHistoryServiceTest, CountsLogLevelsOnlyForRequestedView)
{
    const QUuid other_view_id = QUuid::createUuid();

    ASSERT_TRUE(m_history_service->add_entries(
        m_view_id, {create_entry(QStringLiteral("current"), QStringLiteral("current.log"),
                                 QStringLiteral("INFO"))}));

    ASSERT_TRUE(m_history_service->add_entries(
        other_view_id, {create_entry(QStringLiteral("other"), QStringLiteral("other.log"),
                                     QStringLiteral("ERROR"))}));

    LogQuery query;
    query.view_id = m_view_id;

    const QMap<QString, qsizetype> counts = m_history_service->get_log_level_counts(query);

    EXPECT_EQ(counts.value(QStringLiteral("INFO")), 1);
    EXPECT_FALSE(counts.contains(QStringLiteral("ERROR")));

    m_history_service->remove_view_entries(other_view_id);
}

/**
 * @brief Verifies that structured filters affect log level counts.
 */
TEST_F(LogHistoryServiceTest, AppliesStructuredFiltersToLogLevelCounts)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("backend error"), QStringLiteral("backend.log"),
                                QStringLiteral("ERROR"), QStringLiteral("Backend")));
    entries.append(create_entry(QStringLiteral("backend info"), QStringLiteral("backend.log"),
                                QStringLiteral("INFO"), QStringLiteral("Backend")));
    entries.append(create_entry(QStringLiteral("frontend error"), QStringLiteral("frontend.log"),
                                QStringLiteral("ERROR"), QStringLiteral("Frontend")));
    entries.append(create_entry(QStringLiteral("hidden warning"), QStringLiteral("hidden.log"),
                                QStringLiteral("WARNING"), QStringLiteral("Backend")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;
    query.app_name = QStringLiteral("Backend");
    query.hidden_files = {QStringLiteral("hidden.log")};

    const QMap<QString, qsizetype> counts = m_history_service->get_log_level_counts(query);

    EXPECT_EQ(counts.value(QStringLiteral("ERROR")), 1);
    EXPECT_EQ(counts.value(QStringLiteral("INFO")), 1);
    EXPECT_FALSE(counts.contains(QStringLiteral("WARNING")));
}

/**
 * @brief Verifies that text searching affects log level counts.
 */
TEST_F(LogHistoryServiceTest, AppliesTextSearchToLogLevelCounts)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("network failed"), QStringLiteral("first.log"),
                                QStringLiteral("ERROR")));
    entries.append(create_entry(QStringLiteral("network connected"), QStringLiteral("second.log"),
                                QStringLiteral("INFO")));
    entries.append(create_entry(QStringLiteral("database failed"), QStringLiteral("third.log"),
                                QStringLiteral("ERROR")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;
    query.search_text = QStringLiteral("network");
    query.search_fields = {LogField::Message};

    const QMap<QString, qsizetype> counts = m_history_service->get_log_level_counts(query);

    EXPECT_EQ(counts.value(QStringLiteral("ERROR")), 1);
    EXPECT_EQ(counts.value(QStringLiteral("INFO")), 1);
}

/**
 * @brief Verifies that selected levels do not hide other facet counts.
 */
TEST_F(LogHistoryServiceTest, IgnoresSelectedLevelsWhenCountingLevelFacets)
{
    QVector<LogEntry> entries;
    entries.append(create_entry(QStringLiteral("first"), QStringLiteral("first.log"),
                                QStringLiteral("ERROR")));
    entries.append(create_entry(QStringLiteral("second"), QStringLiteral("second.log"),
                                QStringLiteral("INFO")));
    entries.append(create_entry(QStringLiteral("third"), QStringLiteral("third.log"),
                                QStringLiteral("WARNING")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    LogQuery query;
    query.view_id = m_view_id;
    query.log_levels = {QStringLiteral("ERROR")};

    const QMap<QString, qsizetype> counts = m_history_service->get_log_level_counts(query);

    EXPECT_EQ(counts.value(QStringLiteral("ERROR")), 1);
    EXPECT_EQ(counts.value(QStringLiteral("INFO")), 1);
    EXPECT_EQ(counts.value(QStringLiteral("WARNING")), 1);
}

/**
 * @brief Verifies that distinct values include the complete archived view.
 */
TEST_F(LogHistoryServiceTest, LoadsDistinctValuesForStoredField)
{
    QVector<LogEntry> entries;

    entries.append(create_entry(QStringLiteral("first"), QStringLiteral("first.log"),
                                QStringLiteral("INFO"), QStringLiteral("Frontend")));

    entries.append(create_entry(QStringLiteral("second"), QStringLiteral("second.log"),
                                QStringLiteral("ERROR"), QStringLiteral("Backend")));

    entries.append(create_entry(QStringLiteral("third"), QStringLiteral("third.log"),
                                QStringLiteral("DEBUG"), QStringLiteral("Frontend")));

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    const QSet<QString> app_names =
        m_history_service->get_distinct_values(m_view_id, LogField::AppName);

    EXPECT_EQ(app_names, (QSet<QString>{QStringLiteral("Frontend"), QStringLiteral("Backend")}));
}

/**
 * @brief Verifies that unknown fields cannot become SQL expressions.
 */
TEST_F(LogHistoryServiceTest, RejectsUnsupportedDistinctValueField)
{
    const QSet<QString> values =
        m_history_service->get_distinct_values(m_view_id, QStringLiteral("unknown_field"));

    EXPECT_TRUE(values.isEmpty());
}

/**
 * @brief Verifies counting, paging and searching beyond 100,000 archived entries.
 */
TEST_F(LogHistoryServiceTest, HandlesMoreThanOneHundredThousandEntries)
{
    constexpr qsizetype entry_count = 100001;
    constexpr qsizetype page_size = 25;
    constexpr qsizetype middle_offset = 50000;
    constexpr qsizetype last_offset = 100000;

    const QVector<LogEntry> entries = create_large_entry_batch(entry_count);

    ASSERT_EQ(entries.size(), entry_count);

    QElapsedTimer insert_timer;
    insert_timer.start();

    ASSERT_TRUE(m_history_service->add_entries(m_view_id, entries));

    const qint64 insert_elapsed_ms = insert_timer.elapsed();

    LogQuery query;
    query.view_id = m_view_id;

    QElapsedTimer count_timer;
    count_timer.start();

    const qsizetype stored_entry_count = m_history_service->count_entries(query);

    const qint64 count_elapsed_ms = count_timer.elapsed();

    ASSERT_EQ(stored_entry_count, entry_count);

    QElapsedTimer first_page_timer;
    first_page_timer.start();

    const QVector<LogEntry> first_page = m_history_service->load_entries_page(query, 0, page_size);

    const qint64 first_page_elapsed_ms = first_page_timer.elapsed();

    ASSERT_EQ(first_page.size(), page_size);

    EXPECT_EQ(first_page.first().get_message(), QStringLiteral("record_100001"));

    EXPECT_EQ(first_page.last().get_message(), QStringLiteral("record_099977"));

    QElapsedTimer middle_page_timer;
    middle_page_timer.start();

    const QVector<LogEntry> middle_page =
        m_history_service->load_entries_page(query, middle_offset, page_size);

    const qint64 middle_page_elapsed_ms = middle_page_timer.elapsed();

    ASSERT_EQ(middle_page.size(), page_size);

    EXPECT_EQ(middle_page.first().get_message(), QStringLiteral("record_050001"));

    EXPECT_EQ(middle_page.last().get_message(), QStringLiteral("record_049977"));

    QElapsedTimer last_page_timer;
    last_page_timer.start();

    const QVector<LogEntry> last_page =
        m_history_service->load_entries_page(query, last_offset, page_size);

    const qint64 last_page_elapsed_ms = last_page_timer.elapsed();

    ASSERT_EQ(last_page.size(), 1);

    EXPECT_EQ(last_page.first().get_message(), QStringLiteral("record_000001"));

    QElapsedTimer search_timer;
    search_timer.start();

    expect_single_message_result(QStringLiteral("record_000001"));

    expect_single_message_result(QStringLiteral("record_050001"));

    expect_single_message_result(QStringLiteral("record_100001"));

    const qint64 search_elapsed_ms = search_timer.elapsed();

    std::cout << "[PERF] LogHistoryService 100001 entries:"
              << " insert=" << insert_elapsed_ms << " ms"
              << ", count=" << count_elapsed_ms << " ms"
              << ", first-page=" << first_page_elapsed_ms << " ms"
              << ", middle-page=" << middle_page_elapsed_ms << " ms"
              << ", last-page=" << last_page_elapsed_ms << " ms"
              << ", three-searches=" << search_elapsed_ms << " ms" << std::endl;
}
