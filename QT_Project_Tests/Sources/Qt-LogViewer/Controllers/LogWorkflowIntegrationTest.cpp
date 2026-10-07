/**
 * @file LogWorkflowIntegrationTest.cpp
 * @brief Implements facade-free integration tests for complete log workflows.
 */

#include "Qt-LogViewer/Controllers/LogWorkflowIntegrationTest.h"

#include <QDir>
#include <QFileInfo>
#include <QSignalSpy>
#include <QUuid>
#include <chrono>

#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogPageState.h"
#include "Qt-LogViewer/Services/LogCacheIdentity.h"
#include "Qt-LogViewer/Support/TestLogRuntime.h"
#include "Qt-LogViewer/TestSupport/QtTestAwait.h"

/** @brief Creates the focused runtime graph used by each workflow test. */
void LogWorkflowIntegrationTest::SetUp()
{
    m_runtime = new TestLogRuntime(m_default_profile);
}

/** @brief Stops the runtime graph after each workflow test. */
void LogWorkflowIntegrationTest::TearDown()
{
    delete m_runtime;
    m_runtime = nullptr;
}

/**
 * @brief Creates an isolated UTF-8 log file.
 * @param records Complete records written in source order.
 * @return Absolute file path, or an empty string when creation failed.
 */
auto LogWorkflowIntegrationTest::create_log_file(const QVector<QString>& records) -> QString
{
    QString contents;
    for (const QString& record: records)
    {
        contents += record;
        contents += QLatin1Char('\n');
    }
    const QString file_name =
        QStringLiteral("workflow-%1.log").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    const QString file_path = m_file_system.write_text_file(file_name, contents);
    return file_path;
}

/** @test Verifies import, filtering, paging, and ordered view cleanup as one workflow. */
TEST_F(LogWorkflowIntegrationTest, ImportsFiltersAndClosesViewThroughFocusedComponents)
{
    const QString file_path =
        create_log_file({QStringLiteral("2024-01-01 12:00:00 INFO First WorkflowApp"),
                         QStringLiteral("2024-01-01 12:01:00 ERROR Second WorkflowApp")});
    ASSERT_FALSE(file_path.isEmpty());

    const QUuid view_id = m_runtime->imports().import_file(file_path, m_default_profile);
    const auto synchronous_identity = LogCacheIdentity::create(file_path, m_default_profile);
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
    const QString first_file_path =
        create_log_file({QStringLiteral("2024-01-01 12:00:00 INFO First WorkflowApp")});
    const QString second_file_path =
        create_log_file({QStringLiteral("2024-01-01 12:01:00 ERROR Second WorkflowApp")});
    ASSERT_FALSE(first_file_path.isEmpty());
    ASSERT_FALSE(second_file_path.isEmpty());

    const QUuid view_id =
        m_runtime->imports().import_files({first_file_path, second_file_path}, m_default_profile);
    ASSERT_FALSE(view_id.isNull());
    ASSERT_EQ(m_runtime->cache_catalog().get_view_generations(view_id).size(), 2);

    const auto retained_identity = LogCacheIdentity::create(second_file_path, m_default_profile);
    ASSERT_TRUE(retained_identity.has_value());
    const auto retained_generation =
        m_runtime->cache_catalog().find_complete_generation(retained_identity.value());
    ASSERT_TRUE(retained_generation.has_value());

    ASSERT_TRUE(m_runtime->lifecycle().remove_file(view_id, first_file_path));
    EXPECT_EQ(m_runtime->cache_catalog().get_view_generations(view_id),
              QVector<qint64>{retained_generation->id});
}

/** @test Verifies one selected profile is shared by asynchronous import and live tailing. */
TEST_F(LogWorkflowIntegrationTest, StreamsAndTailsWithSelectedProfile)
{
    const QString file_path =
        create_log_file({QStringLiteral("INFO|initial custom message|CustomApp")});
    ASSERT_FALSE(file_path.isEmpty());

    const LogParsingProfile profile = LogParsingProfile::create_default(
        QStringLiteral("{level}|{message}|{app_name}"), QStringLiteral("Pipe separated"));
    QSignalSpy finished_spy(&m_runtime->imports(), &LogImportCoordinator::finished);

    const QUuid view_id = m_runtime->imports().import_file_async(file_path, profile, 1);
    ASSERT_FALSE(view_id.isNull());
    ASSERT_TRUE(QtTestAwait::wait_until([&finished_spy]() { return finished_spy.count() == 1; }));
    ASSERT_TRUE(QtTestAwait::wait_until(
        [this, &view_id]() { return m_runtime->pages().get_page_state(view_id) != nullptr; }));
    ASSERT_TRUE(QtTestAwait::wait_until([this, &view_id]() {
        const LogPageState* state = m_runtime->pages().get_page_state(view_id);
        return state != nullptr && state->get_total_entries() == 1;
    }));

    const auto cache_identity = LogCacheIdentity::create(file_path, profile);
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

    ASSERT_TRUE(m_file_system.append_text(
        file_path, QStringLiteral("ERROR|tailed custom message|CustomApp\n")));

    ASSERT_TRUE(QtTestAwait::wait_until([this, &view_id]() {
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
    const QString file_path =
        create_log_file({QStringLiteral("2024-01-01 12:00:00 INFO First message"),
                         QStringLiteral("2024-01-01 12:01:00 ERROR Second message")});
    ASSERT_FALSE(file_path.isEmpty());

    const LogParsingProfile profile = LogParsingProfile::create_default(
        QStringLiteral("{timestamp} {level} {message}"), QStringLiteral("Without application"));
    QSignalSpy finished_spy(&m_runtime->imports(), &LogImportCoordinator::finished);
    QSignalSpy error_spy(&m_runtime->imports(), &LogImportCoordinator::error);

    const QUuid view_id = m_runtime->imports().import_file_async(file_path, profile, 2);
    ASSERT_FALSE(view_id.isNull());
    ASSERT_TRUE(QtTestAwait::wait_until([&finished_spy]() { return finished_spy.count() == 1; },
                                        std::chrono::seconds(10)));
    EXPECT_EQ(error_spy.count(), 0);

    const auto identity = LogCacheIdentity::create(file_path, profile);
    ASSERT_TRUE(identity.has_value());
    const auto generation = m_runtime->cache_catalog().find_complete_generation(identity.value());
    ASSERT_TRUE(generation.has_value());
    EXPECT_EQ(generation->entry_count, 2);
    EXPECT_EQ(m_runtime->cache_catalog().get_view_generations(view_id),
              QVector<qint64>{generation->id});
}

/**
 * @test Verifies an import cancelled during partial progress cannot leak its
 * profile into a reopened file.
 */
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
    const QString file_path = create_log_file(records);
    ASSERT_FALSE(file_path.isEmpty());

    const LogParsingProfile short_profile = LogParsingProfile::create_default(
        QStringLiteral("{timestamp} {level} {message}"), QStringLiteral("Short"));
    const LogParsingProfile complete_profile = LogParsingProfile::create_default(
        QStringLiteral(
            "{timestamp} {level} {message} - [{category}] {file}:{line}, {function} {app_name}"),
        QStringLiteral("Complete"));
    const auto cancelled_identity = LogCacheIdentity::create(file_path, short_profile);
    ASSERT_TRUE(cancelled_identity.has_value());
    const QString cancelled_database_path =
        QDir(QDir(m_runtime->cache_catalog().get_cache_root()).filePath(QStringLiteral("files")))
            .filePath(cancelled_identity->cache_key + QStringLiteral(".sqlite"));
    QUuid cancelled_view_id;
    bool cancellation_requested = false;
    bool cancelled_view_closed = false;
    const QMetaObject::Connection cancellation_connection = QObject::connect(
        &m_runtime->imports(), &LogImportCoordinator::progress, &m_runtime->imports(),
        [this, &cancelled_view_id, &cancellation_requested, &cancelled_view_closed](
            const QUuid& progress_view_id, qint64 bytes_read, qint64 total_bytes) {
            const bool is_partial_progress = total_bytes > 0 && bytes_read < total_bytes;
            if (!cancellation_requested && progress_view_id == cancelled_view_id &&
                is_partial_progress)
            {
                cancellation_requested = true;
                cancelled_view_closed = m_runtime->lifecycle().close_view(cancelled_view_id);
            }
        });

    cancelled_view_id = m_runtime->imports().import_file_async(file_path, short_profile, 1);
    const bool valid_cancelled_view = !cancelled_view_id.isNull();
    bool cancellation_observed = false;
    if (valid_cancelled_view)
    {
        cancellation_observed =
            QtTestAwait::wait_until([&cancellation_requested]() { return cancellation_requested; },
                                    std::chrono::seconds(10));
    }
    QObject::disconnect(cancellation_connection);

    ASSERT_TRUE(valid_cancelled_view);
    ASSERT_TRUE(cancellation_observed);
    ASSERT_TRUE(cancelled_view_closed);
    ASSERT_TRUE(QtTestAwait::wait_until(
        [&cancelled_database_path]() { return !QFileInfo::exists(cancelled_database_path); },
        std::chrono::seconds(10)));
    EXPECT_FALSE(m_runtime->cache_catalog()
                     .find_complete_generation(cancelled_identity.value())
                     .has_value());

    QSignalSpy finished_spy(&m_runtime->imports(), &LogImportCoordinator::finished);
    const QUuid reopened_view_id =
        m_runtime->imports().import_file_async(file_path, complete_profile, 1000);
    ASSERT_FALSE(reopened_view_id.isNull());
    ASSERT_TRUE(QtTestAwait::wait_until([&finished_spy]() { return finished_spy.count() == 1; },
                                        std::chrono::seconds(30)));

    ASSERT_TRUE(m_file_system.append_text(
        file_path,
        QStringLiteral("2026-07-16 04:10:07.642 Info Application tailed - [runtime] "
                       "D:\\Projects\\Qt-LogViewer\\tail.cpp:96, void tail() Qt-LogViewer\n")));

    ASSERT_TRUE(QtTestAwait::wait_until(
        [this, &reopened_view_id, record_count]() {
            const LogPageState* state = m_runtime->pages().get_page_state(reopened_view_id);
            return state != nullptr && state->get_total_entries() == record_count + 1;
        },
        std::chrono::seconds(30)));
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
