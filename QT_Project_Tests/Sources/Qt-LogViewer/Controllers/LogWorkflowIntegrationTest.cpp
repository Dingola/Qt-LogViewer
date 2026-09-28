/**
 * @file LogWorkflowIntegrationTest.cpp
 * @brief Implements facade-free integration tests for complete log workflows.
 */

#include "Qt-LogViewer/Controllers/LogWorkflowIntegrationTest.h"

#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTest>
#include <QTextStream>

#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogPageState.h"
#include "Qt-LogViewer/Support/TestLogRuntime.h"

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
    QTRY_COMPARE(finished_spy.count(), 1);
    QTRY_VERIFY(m_runtime->pages().get_page_state(view_id) != nullptr);
    QTRY_COMPARE(m_runtime->pages().get_page_state(view_id)->get_total_entries(),
                 static_cast<qsizetype>(1));

    m_runtime->live_tailing().set_enabled(view_id, false);
    m_runtime->live_tailing().set_enabled(view_id, true);

    QFile append_file(file->fileName());
    ASSERT_TRUE(append_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text));
    QTextStream output(&append_file);
    output << "ERROR|tailed custom message|CustomApp\n";
    output.flush();
    append_file.close();

    QTRY_COMPARE(m_runtime->pages().get_page_state(view_id)->get_total_entries(),
                 static_cast<qsizetype>(2));
    LogViewContext* context = m_runtime->views().get_context(view_id);
    ASSERT_NE(context, nullptr);
    ASSERT_EQ(context->get_model()->rowCount(), 2);
    EXPECT_EQ(context->get_model()->get_entry(0).get_message(),
              QStringLiteral("tailed custom message"));
}

/** @test Verifies a cancelled import cannot leak its profile into a reopened file. */
TEST_F(LogWorkflowIntegrationTest, ReopensCancelledFileWithNewParsingProfile)
{
    QVector<QString> records;
    const QString record = QStringLiteral(
        R"(2026-07-16 04:10:06.642 Info Application started - [default] D:\Projects\Qt-LogViewer\main.cpp:95, int main() Qt-LogViewer)");
    for (int index = 0; index < 10000; ++index)
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
    QSignalSpy progress_spy(&m_runtime->imports(), &LogImportCoordinator::progress);

    const QUuid cancelled_view_id =
        m_runtime->imports().import_file_async(file->fileName(), short_profile, 1);
    ASSERT_FALSE(cancelled_view_id.isNull());
    QTRY_VERIFY(progress_spy.count() > 0);
    ASSERT_TRUE(m_runtime->lifecycle().close_view(cancelled_view_id));

    QSignalSpy finished_spy(&m_runtime->imports(), &LogImportCoordinator::finished);
    const QUuid reopened_view_id =
        m_runtime->imports().import_file_async(file->fileName(), complete_profile, 1000);
    ASSERT_FALSE(reopened_view_id.isNull());
    QTRY_COMPARE_WITH_TIMEOUT(finished_spy.count(), 1, 30000);

    QFile append_file(file->fileName());
    ASSERT_TRUE(append_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text));
    QTextStream output(&append_file);
    output << "2026-07-16 04:10:07.642 Info Application tailed - [runtime] "
              "D:\\Projects\\Qt-LogViewer\\tail.cpp:96, void tail() Qt-LogViewer\n";
    output.flush();
    append_file.close();

    QTRY_COMPARE_WITH_TIMEOUT(
        m_runtime->pages().get_page_state(reopened_view_id)->get_total_entries(),
        static_cast<qsizetype>(10001), 30000);
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
