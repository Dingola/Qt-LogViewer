/**
 * @file LogWorkflowIntegrationTest.cpp
 * @brief Implements facade-free integration tests for complete log workflows.
 */

#include "Qt-LogViewer/Controllers/LogWorkflowIntegrationTest.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTest>
#include <QTextStream>

#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogPageState.h"
#include "Qt-LogViewer/Services/LogCacheIdentity.h"
#include "Qt-LogViewer/Support/TestLogRuntime.h"

namespace
{
/**
 * @brief Processes Qt events until a predicate succeeds or its deadline expires.
 * @tparam Predicate Callable returning whether the awaited state has been reached.
 * @param predicate State predicate evaluated after each event-processing interval.
 * @param timeout_ms Maximum wait duration in milliseconds.
 * @return True when the predicate succeeded before the deadline.
 */
template<typename Predicate>
[[nodiscard]] auto wait_until(Predicate predicate, int timeout_ms = 5000) -> bool
{
    QElapsedTimer timer;
    timer.start();
    while (!predicate() && timer.elapsed() < timeout_ms)
    {
        QTest::qWait(10);
    }
    return predicate();
}
}  // namespace

/** @brief Creates the focused runtime graph used by each workflow test. */
void LogWorkflowIntegrationTest::SetUp()
{
    m_runtime = new TestLogRuntime(m_default_profile);
}

/** @brief Stops the runtime graph and removes temporary files. */
void LogWorkflowIntegrationTest::TearDown()
{
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
 * @brief Creates a persistent temporary log file.
 * @param records Complete records written in source order.
 * @return Created file, or nullptr when the file could not be opened.
 */
auto LogWorkflowIntegrationTest::create_log_file(const QVector<QString>& records) -> QTemporaryFile*
{
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
    }
    else
    {
        delete file;
        file = nullptr;
    }
    return file;
}

/** @test Verifies import, filtering, paging, and ordered view cleanup as one workflow. */
TEST_F(LogWorkflowIntegrationTest, ImportsFiltersAndClosesViewThroughFocusedComponents)
{
    QTemporaryFile* file =
        create_log_file({QStringLiteral("2024-01-01 12:00:00 INFO First WorkflowApp"),
                         QStringLiteral("2024-01-01 12:01:00 ERROR Second WorkflowApp")});
    ASSERT_NE(file, nullptr);

    const QUuid view_id = m_runtime->imports().import_file(file->fileName(), m_default_profile);
    const auto synchronous_identity = LogCacheIdentity::create(file->fileName(), m_default_profile);
    ASSERT_TRUE(synchronous_identity.has_value());
    const auto synchronous_generation =
        m_runtime->cache_catalog().find_complete_generation(synchronous_identity.value());
    EXPECT_TRUE(synchronous_generation.has_value());
    ASSERT_FALSE(view_id.isNull());
    ASSERT_TRUE(m_runtime->queries().set_log_levels(view_id, {QStringLiteral("ERROR")}));

    const LogPageState* page_state = m_runtime->pages().get_page_state(view_id);
    ASSERT_NE(page_state, nullptr);
    EXPECT_EQ(page_state->get_total_entries(), 1);

    LogViewContext* context = m_runtime->views().get_context(view_id);
    ASSERT_NE(context, nullptr);
    ASSERT_EQ(context->get_model()->rowCount(), 1);
    EXPECT_EQ(context->get_model()->get_entry(0).get_message(), QStringLiteral("Second"));

    ASSERT_TRUE(m_runtime->lifecycle().close_view(view_id));
    EXPECT_EQ(m_runtime->views().get_context(view_id), nullptr);
    EXPECT_EQ(m_runtime->pages().get_page_state(view_id), nullptr);
}

/** @test Verifies removing one file atomically rebinds a multi-file cache view. */
TEST_F(LogWorkflowIntegrationTest, RebindsCacheMappingAfterFileRemoval)
{
    QTemporaryFile* first_file =
        create_log_file({QStringLiteral("2024-01-01 12:00:00 INFO First WorkflowApp")});
    QTemporaryFile* second_file =
        create_log_file({QStringLiteral("2024-01-01 12:01:00 ERROR Second WorkflowApp")});
    ASSERT_NE(first_file, nullptr);
    ASSERT_NE(second_file, nullptr);

    const QUuid view_id = m_runtime->imports().import_files(
        {first_file->fileName(), second_file->fileName()}, m_default_profile);
    ASSERT_FALSE(view_id.isNull());
    ASSERT_EQ(m_runtime->cache_catalog().get_view_generations(view_id).size(), 2);

    const auto retained_identity =
        LogCacheIdentity::create(second_file->fileName(), m_default_profile);
    ASSERT_TRUE(retained_identity.has_value());
    const auto retained_generation =
        m_runtime->cache_catalog().find_complete_generation(retained_identity.value());
    ASSERT_TRUE(retained_generation.has_value());

    ASSERT_TRUE(m_runtime->lifecycle().remove_file(view_id, first_file->fileName()));
    EXPECT_EQ(m_runtime->cache_catalog().get_view_generations(view_id),
              QVector<qint64>{retained_generation->id});
}

/** @test Verifies one selected profile is shared by asynchronous import and live tailing. */
TEST_F(LogWorkflowIntegrationTest, StreamsAndTailsWithSelectedProfile)
{
    QTemporaryFile* file =
        create_log_file({QStringLiteral("INFO|initial custom message|CustomApp")});
    ASSERT_NE(file, nullptr);

    const LogParsingProfile profile = LogParsingProfile::create_default(
        QStringLiteral("{level}|{message}|{app_name}"), QStringLiteral("Pipe separated"));
    QSignalSpy finished_spy(&m_runtime->imports(), &LogImportCoordinator::finished);

    const QUuid view_id = m_runtime->imports().import_file_async(file->fileName(), profile, 1);
    ASSERT_FALSE(view_id.isNull());
    ASSERT_TRUE(wait_until([&finished_spy]() { return finished_spy.count() == 1; }));
    ASSERT_TRUE(wait_until(
        [this, &view_id]() { return m_runtime->pages().get_page_state(view_id) != nullptr; }));
    ASSERT_TRUE(wait_until([this, &view_id]() {
        const LogPageState* state = m_runtime->pages().get_page_state(view_id);
        return state != nullptr && state->get_total_entries() == 1;
    }));

    const auto cache_identity = LogCacheIdentity::create(file->fileName(), profile);
    ASSERT_TRUE(cache_identity.has_value());
    const auto cache_generation =
        m_runtime->cache_catalog().find_complete_generation(cache_identity.value());
    ASSERT_TRUE(cache_generation.has_value());
    EXPECT_EQ(cache_generation->entry_count, 1);
    EXPECT_TRUE(QFileInfo::exists(cache_generation->database_path));
    EXPECT_EQ(m_runtime->cache_catalog().get_view_generations(view_id),
              QVector<qint64>{cache_generation->id});

    m_runtime->live_tailing().set_enabled(view_id, false);
    m_runtime->live_tailing().set_enabled(view_id, true);

    QFile append_file(file->fileName());
    ASSERT_TRUE(append_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text));
    QTextStream output(&append_file);
    output << "ERROR|tailed custom message|CustomApp\n";
    output.flush();
    append_file.close();

    ASSERT_TRUE(wait_until([this, &view_id]() {
        const LogPageState* state = m_runtime->pages().get_page_state(view_id);
        return state != nullptr && state->get_total_entries() == 2;
    }));
    LogViewContext* context = m_runtime->views().get_context(view_id);
    ASSERT_NE(context, nullptr);
    ASSERT_EQ(context->get_model()->rowCount(), 2);
    EXPECT_EQ(context->get_model()->get_entry(0).get_message(),
              QStringLiteral("tailed custom message"));
}

/** @test Verifies asynchronous caching when the selected profile has no application field. */
TEST_F(LogWorkflowIntegrationTest, StreamsProfileWithoutApplicationField)
{
    QTemporaryFile* file =
        create_log_file({QStringLiteral("2024-01-01 12:00:00 INFO First message"),
                         QStringLiteral("2024-01-01 12:01:00 ERROR Second message")});
    ASSERT_NE(file, nullptr);

    const LogParsingProfile profile = LogParsingProfile::create_default(
        QStringLiteral("{timestamp} {level} {message}"), QStringLiteral("Without application"));
    QSignalSpy finished_spy(&m_runtime->imports(), &LogImportCoordinator::finished);
    QSignalSpy error_spy(&m_runtime->imports(), &LogImportCoordinator::error);

    const QUuid view_id = m_runtime->imports().import_file_async(file->fileName(), profile, 2);
    ASSERT_FALSE(view_id.isNull());
    ASSERT_TRUE(wait_until([&finished_spy]() { return finished_spy.count() == 1; }, 10000));
    EXPECT_EQ(error_spy.count(), 0);

    const auto identity = LogCacheIdentity::create(file->fileName(), profile);
    ASSERT_TRUE(identity.has_value());
    const auto generation = m_runtime->cache_catalog().find_complete_generation(identity.value());
    ASSERT_TRUE(generation.has_value());
    EXPECT_EQ(generation->entry_count, 2);
    EXPECT_EQ(m_runtime->cache_catalog().get_view_generations(view_id),
              QVector<qint64>{generation->id});
}

/** @test Verifies a cancelled import cannot leak its profile into a reopened file. */
TEST_F(LogWorkflowIntegrationTest, ReopensCancelledFileWithNewParsingProfile)
{
    constexpr int record_count = 1000;
    QVector<QString> records;
    const QString record = QStringLiteral(
        R"(2026-07-16 04:10:06.642 Info Application started - [default] D:\Projects\Qt-LogViewer\main.cpp:95, int main() Qt-LogViewer)");
    for (int index = 0; index < record_count; ++index)
    {
        records.append(record);
    }
    QTemporaryFile* file = create_log_file(records);
    ASSERT_NE(file, nullptr);

    const LogParsingProfile short_profile = LogParsingProfile::create_default(
        QStringLiteral("{timestamp} {level} {message}"), QStringLiteral("Short"));
    const LogParsingProfile complete_profile = LogParsingProfile::create_default(
        QStringLiteral(
            "{timestamp} {level} {message} - [{category}] {file}:{line}, {function} {app_name}"),
        QStringLiteral("Complete"));
    const auto cancelled_identity = LogCacheIdentity::create(file->fileName(), short_profile);
    ASSERT_TRUE(cancelled_identity.has_value());
    const QString cancelled_database_path =
        QDir(QDir(m_runtime->cache_catalog().get_cache_root()).filePath(QStringLiteral("files")))
            .filePath(cancelled_identity->cache_key + QStringLiteral(".sqlite"));
    QSignalSpy progress_spy(&m_runtime->imports(), &LogImportCoordinator::progress);

    const QUuid cancelled_view_id =
        m_runtime->imports().import_file_async(file->fileName(), short_profile, 1);
    ASSERT_FALSE(cancelled_view_id.isNull());
    ASSERT_TRUE(wait_until([&progress_spy]() { return progress_spy.count() > 0; }));
    ASSERT_TRUE(m_runtime->lifecycle().close_view(cancelled_view_id));
    ASSERT_TRUE(wait_until(
        [&cancelled_database_path]() { return !QFileInfo::exists(cancelled_database_path); },
        10000));
    EXPECT_FALSE(m_runtime->cache_catalog()
                     .find_complete_generation(cancelled_identity.value())
                     .has_value());

    QSignalSpy finished_spy(&m_runtime->imports(), &LogImportCoordinator::finished);
    const QUuid reopened_view_id =
        m_runtime->imports().import_file_async(file->fileName(), complete_profile, 1000);
    ASSERT_FALSE(reopened_view_id.isNull());
    ASSERT_TRUE(wait_until([&finished_spy]() { return finished_spy.count() == 1; }, 30000));

    QFile append_file(file->fileName());
    ASSERT_TRUE(append_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text));
    QTextStream output(&append_file);
    output << "2026-07-16 04:10:07.642 Info Application tailed - [runtime] "
              "D:\\Projects\\Qt-LogViewer\\tail.cpp:96, void tail() Qt-LogViewer\n";
    output.flush();
    append_file.close();

    ASSERT_TRUE(wait_until(
        [this, &reopened_view_id, record_count]() {
            const LogPageState* state = m_runtime->pages().get_page_state(reopened_view_id);
            return state != nullptr && state->get_total_entries() == record_count + 1;
        },
        30000));
    LogViewContext* context = m_runtime->views().get_context(reopened_view_id);
    ASSERT_NE(context, nullptr);
    ASSERT_FALSE(context->get_model()->get_entries().isEmpty());
    const LogEntry entry = context->get_model()->get_entries().constFirst();
    EXPECT_EQ(entry.get_message(), QStringLiteral("Application tailed"));
    EXPECT_EQ(entry.get_parsed_field(QStringLiteral("category")).toString(),
              QStringLiteral("runtime"));
    EXPECT_EQ(entry.get_parsed_field(QStringLiteral("line")).toLongLong(), 96);
    EXPECT_EQ(entry.get_app_name(), QStringLiteral("Qt-LogViewer"));
}
