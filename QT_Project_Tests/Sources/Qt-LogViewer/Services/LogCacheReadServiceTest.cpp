/**
 * @file LogCacheReadServiceTest.cpp
 * @brief Verifies cache-hit reopening, querying and legacy-storage avoidance.
 */

#include "Qt-LogViewer/Services/LogCacheReadServiceTest.h"

#include <QElapsedTimer>
#include <QFile>
#include <QSignalSpy>
#include <QTest>
#include <QTextStream>

#include "Qt-LogViewer/Controllers/LogImportCoordinator.h"
#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogPageState.h"
#include "Qt-LogViewer/Models/LogQuery.h"
#include "Qt-LogViewer/Models/SearchFields.h"
#include "Qt-LogViewer/Services/LogHistoryService.h"
#include "Qt-LogViewer/Support/TestLogRuntime.h"

namespace
{
/**
 * @brief Processes Qt events until a predicate succeeds or its deadline expires.
 * @tparam Predicate Callable returning the awaited state.
 * @param predicate State predicate evaluated after every event-processing interval.
 * @param timeout_ms Maximum wait duration in milliseconds.
 * @return True when the predicate succeeded before the deadline.
 */
template<typename Predicate>
[[nodiscard]] auto wait_until(Predicate predicate, int timeout_ms = 10000) -> bool
{
    QElapsedTimer timer;
    timer.start();
    while (!predicate() && timer.elapsed() < timeout_ms)
    {
        QTest::qWait(10);
    }
    const bool completed = predicate();
    return completed;
}
}  // namespace

/** @brief Creates an isolated production component graph before each cache-read test. */
auto LogCacheReadServiceTest::SetUp() -> void
{
    m_runtime = new TestLogRuntime(m_profile);
}

/** @brief Stops asynchronous services and removes persistent temporary source files. */
auto LogCacheReadServiceTest::TearDown() -> void
{
    delete m_runtime;
    m_runtime = nullptr;
    for (QTemporaryFile* file: m_files)
    {
        if (file != nullptr)
        {
            QFile::remove(file->fileName());
            delete file;
        }
    }
    m_files.clear();
}

/**
 * @brief Writes deterministic records to a persistent temporary source file.
 * @param records Complete UTF-8 log records in their source order.
 * @return Created file retained by the fixture, or nullptr when opening failed.
 */
auto LogCacheReadServiceTest::create_log_file(const QVector<QString>& records) -> QTemporaryFile*
{
    auto* file = new QTemporaryFile();
    const bool opened = file->open();
    if (opened)
    {
        file->setAutoRemove(false);
        QTextStream stream(file);
        for (const QString& record: records)
        {
            stream << record << '\n';
        }
        stream.flush();
        file->close();
        m_files.append(file);
    }
    else
    {
        delete file;
        file = nullptr;
    }
    return file;
}

/**
 * @test Verifies an exact cache hit reopens without parsing or copying rows into legacy history.
 */
TEST_F(LogCacheReadServiceTest, ReopensExactCacheWithoutParsingOrLegacyCopy)
{
    QTemporaryFile* file =
        create_log_file({QStringLiteral("2026-01-01 10:00:00 INFO First CacheApp"),
                         QStringLiteral("2026-01-01 10:01:00 ERROR Second CacheApp")});
    ASSERT_NE(file, nullptr);

    const QUuid first_view = m_runtime->imports().import_file(file->fileName(), m_profile);
    ASSERT_FALSE(first_view.isNull());

    LogQuery first_query;
    first_query.view_id = first_view;
    LogHistoryService legacy_history(m_runtime->history().get_database_path());
    ASSERT_TRUE(legacy_history.is_available());
    EXPECT_EQ(legacy_history.count_entries(first_query), 0);
    ASSERT_TRUE(m_runtime->lifecycle().close_view(first_view));

    QSignalSpy progress_spy(&m_runtime->imports(), &LogImportCoordinator::progress);
    QSignalSpy finished_spy(&m_runtime->imports(), &LogImportCoordinator::finished);
    const QUuid reopened_view =
        m_runtime->imports().import_file_async(file->fileName(), m_profile, 1);
    ASSERT_FALSE(reopened_view.isNull());
    m_runtime->live_tailing().set_enabled(reopened_view, false);

    EXPECT_EQ(progress_spy.count(), 0);
    ASSERT_EQ(finished_spy.count(), 1);
    const LogPageState* page_state = m_runtime->pages().get_page_state(reopened_view);
    ASSERT_NE(page_state, nullptr);
    EXPECT_EQ(page_state->get_total_entries(), 2);

    LogQuery reopened_query;
    reopened_query.view_id = reopened_view;
    EXPECT_EQ(legacy_history.count_entries(reopened_query), 0);
}

/** @test Verifies filters and searches use the rebound per-file cache data source. */
TEST_F(LogCacheReadServiceTest, FiltersAndSearchesReboundCacheEntries)
{
    QTemporaryFile* file =
        create_log_file({QStringLiteral("2026-01-01 10:00:00 INFO First CacheApp"),
                         QStringLiteral("2026-01-01 10:01:00 ERROR Second CacheApp")});
    ASSERT_NE(file, nullptr);

    const QUuid initial_view = m_runtime->imports().import_file(file->fileName(), m_profile);
    ASSERT_FALSE(initial_view.isNull());
    ASSERT_TRUE(m_runtime->lifecycle().close_view(initial_view));
    const QUuid view_id = m_runtime->imports().import_file_async(file->fileName(), m_profile, 1);
    ASSERT_FALSE(view_id.isNull());
    m_runtime->live_tailing().set_enabled(view_id, false);

    ASSERT_TRUE(m_runtime->queries().set_log_levels(view_id, {QStringLiteral("ERROR")}));
    const LogPageState* filtered_state = m_runtime->pages().get_page_state(view_id);
    ASSERT_NE(filtered_state, nullptr);
    EXPECT_EQ(filtered_state->get_total_entries(), 1);

    ASSERT_TRUE(m_runtime->queries().set_log_levels(view_id, {}));
    ASSERT_TRUE(m_runtime->queries().set_search(view_id, QStringLiteral("Second"),
                                                SearchField::Message, false));
    LogViewContext* context = m_runtime->views().get_context(view_id);
    ASSERT_NE(context, nullptr);
    ASSERT_EQ(context->get_model()->rowCount(), 1);
    EXPECT_EQ(context->get_model()->get_entry(0).get_message(), QStringLiteral("Second"));
}

/**
 * @test Verifies bounded SQL pages retain global ordering and facets across cache databases.
 */
TEST_F(LogCacheReadServiceTest, MergesBoundedPagesAcrossMultipleCacheDatabases)
{
    QTemporaryFile* first_file =
        create_log_file({QStringLiteral("2026-01-01 10:00:00 INFO First AppA"),
                         QStringLiteral("2026-01-01 10:03:00 ERROR Fourth AppA")});
    QTemporaryFile* second_file =
        create_log_file({QStringLiteral("2026-01-01 10:01:00 WARN Second AppB"),
                         QStringLiteral("2026-01-01 10:02:00 ERROR Third AppB")});
    ASSERT_NE(first_file, nullptr);
    ASSERT_NE(second_file, nullptr);

    const QUuid view_id = m_runtime->imports().import_files(
        {first_file->fileName(), second_file->fileName()}, m_profile);
    ASSERT_FALSE(view_id.isNull());
    m_runtime->live_tailing().set_enabled(view_id, false);

    LogQuery query;
    query.view_id = view_id;
    query.sort_field = LogField::Timestamp;
    query.sort_order = Qt::DescendingOrder;

    EXPECT_EQ(m_runtime->history().count_entries(query), 4);
    const QVector<LogEntry> page = m_runtime->history().load_entries_page(query, 1, 2);
    ASSERT_EQ(page.size(), 2);
    EXPECT_EQ(page.at(0).get_message(), QStringLiteral("Third"));
    EXPECT_EQ(page.at(1).get_message(), QStringLiteral("Second"));

    query.log_levels = {QStringLiteral("error")};
    EXPECT_EQ(m_runtime->history().count_entries(query), 2);
    const QMap<QString, qsizetype> level_counts = m_runtime->history().get_log_level_counts(query);
    EXPECT_EQ(level_counts.value(QStringLiteral("INFO")), 1);
    EXPECT_EQ(level_counts.value(QStringLiteral("WARN")), 1);
    EXPECT_EQ(level_counts.value(QStringLiteral("ERROR")), 2);

    const QSet<QString> applications =
        m_runtime->history().get_distinct_values(view_id, LogField::AppName);
    EXPECT_EQ(applications, (QSet<QString>{QStringLiteral("AppA"), QStringLiteral("AppB")}));
}

/** @test Verifies an appended source reuses its cache prefix and imports only the suffix. */
TEST_F(LogCacheReadServiceTest, ReusesCachedPrefixForAppendedSource)
{
    QTemporaryFile* file =
        create_log_file({QStringLiteral("2026-01-01 10:00:00 INFO First CacheApp"),
                         QStringLiteral("2026-01-01 10:01:00 ERROR Second CacheApp")});
    ASSERT_NE(file, nullptr);

    const QUuid initial_view = m_runtime->imports().import_file(file->fileName(), m_profile);
    ASSERT_FALSE(initial_view.isNull());
    const auto prefix_identity = LogCacheIdentity::create(file->fileName(), m_profile);
    ASSERT_TRUE(prefix_identity.has_value());
    const auto prefix_generation =
        m_runtime->cache_catalog().find_complete_generation(prefix_identity.value());
    ASSERT_TRUE(prefix_generation.has_value());
    ASSERT_TRUE(m_runtime->lifecycle().close_view(initial_view));

    QFile changed_file(file->fileName());
    ASSERT_TRUE(changed_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text));
    QTextStream output(&changed_file);
    output << "2026-01-01 10:02:00 WARN Third CacheApp\n";
    output.flush();
    changed_file.close();

    QSignalSpy progress_spy(&m_runtime->imports(), &LogImportCoordinator::progress);
    QSignalSpy finished_spy(&m_runtime->imports(), &LogImportCoordinator::finished);
    const QUuid view_id = m_runtime->imports().import_file_async(file->fileName(), m_profile, 1);
    ASSERT_FALSE(view_id.isNull());
    const LogPageState* prefix_page_state = m_runtime->pages().get_page_state(view_id);
    ASSERT_NE(prefix_page_state, nullptr);
    EXPECT_EQ(prefix_page_state->get_total_entries(), 2);
    ASSERT_TRUE(wait_until([&finished_spy]() { return finished_spy.count() == 1; }));
    m_runtime->live_tailing().set_enabled(view_id, false);

    EXPECT_GT(progress_spy.count(), 0);
    EXPECT_EQ(progress_spy.constFirst().at(1).toLongLong(), prefix_identity->file_size);
    const LogPageState* page_state = m_runtime->pages().get_page_state(view_id);
    ASSERT_NE(page_state, nullptr);
    EXPECT_EQ(page_state->get_total_entries(), 3);
    const auto current_identity = LogCacheIdentity::create(file->fileName(), m_profile);
    ASSERT_TRUE(current_identity.has_value());
    const auto current_generation =
        m_runtime->cache_catalog().find_complete_generation(current_identity.value());
    ASSERT_TRUE(current_generation.has_value());
    EXPECT_EQ(current_generation->entry_count, 3);
    EXPECT_NE(current_generation->id, prefix_generation->id);
}
