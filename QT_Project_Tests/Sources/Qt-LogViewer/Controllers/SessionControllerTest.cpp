#include "Qt-LogViewer/Controllers/SessionControllerTest.h"

#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTest>
#include <QTextStream>

#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/SessionController.h"
#include "Qt-LogViewer/Models/LogFileTreeModel.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogPageState.h"
#include "Qt-LogViewer/Models/SessionTypes.h"
#include "Qt-LogViewer/Support/TestLogRuntime.h"

/**
 * @file SessionControllerTest.cpp
 * @brief Implements integration tests for coordinated typed session restoration.
 */

/**
 * @brief Creates the controller graph used by each restoration test.
 */
void SessionControllerTest::SetUp()
{
    m_default_profile = LogParsingProfile::create_default(
        QStringLiteral("{timestamp} {level} {message} {app_name}"),
        QStringLiteral("Session test default"));
    m_runtime = new TestLogRuntime(m_default_profile);
    m_session_controller = new SessionController(
        nullptr, m_runtime->catalog().get_model(), m_default_profile, &m_runtime->catalog(),
        &m_runtime->views(), &m_runtime->filters(), &m_runtime->history(), &m_runtime->pages(),
        &m_runtime->queries(), &m_runtime->imports(), &m_runtime->lifecycle(),
        &m_runtime->live_tailing());
}

/**
 * @brief Destroys controllers and removes temporary files.
 */
void SessionControllerTest::TearDown()
{
    delete m_session_controller;
    m_session_controller = nullptr;
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
 * @brief Creates a temporary log file containing the supplied records.
 * @param lines Records written in source order.
 * @return Created temporary file, or nullptr when creation fails.
 */
auto SessionControllerTest::create_temp_file(const QVector<QString>& lines) -> QTemporaryFile*
{
    QTemporaryFile* file = new QTemporaryFile();
    const bool opened = file->open();

    if (opened)
    {
        file->setAutoRemove(false);
        QTextStream stream(file);

        for (const QString& line: lines)
        {
            stream << line << '\n';
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

/**
 * @test Verifies that saved sorting and page two are applied after restored entries exist.
 */
TEST_F(SessionControllerTest, AppliesSavedQueryStateAfterImportCompletion)
{
    QTemporaryFile* file =
        create_temp_file({QStringLiteral("2024-01-01 12:01:00 INFO A RestoreApp"),
                          QStringLiteral("2024-01-01 12:02:00 INFO B RestoreApp"),
                          QStringLiteral("2024-01-01 12:03:00 INFO C RestoreApp"),
                          QStringLiteral("2024-01-01 12:04:00 INFO D RestoreApp"),
                          QStringLiteral("2024-01-01 12:05:00 INFO E RestoreApp"),
                          QStringLiteral("2024-01-01 12:06:00 INFO F RestoreApp")});
    ASSERT_NE(file, nullptr);

    SessionViewState view_state;
    view_state.id = QUuid::createUuid();
    view_state.loaded_files = {LogFileInfo(file->fileName(), QStringLiteral("RestoreApp"))};
    view_state.page_size = 2;
    view_state.current_page = 2;
    view_state.sort_column = LogModel::Message;
    view_state.sort_order = Qt::DescendingOrder;
    view_state.filters.live_tailing_enabled = false;

    SessionState state;
    state.id = QStringLiteral("restore-query-state");
    state.views = {view_state};

    QSignalSpy restored_spy(m_session_controller, &SessionController::session_restored);

    ASSERT_TRUE(m_session_controller->restore_session(state));
    QTRY_COMPARE(restored_spy.count(), 1);

    const LogPageState* page_state = m_runtime->pages().get_page_state(view_state.id);
    ASSERT_NE(page_state, nullptr);
    EXPECT_EQ(page_state->get_current_page(), 2);
    EXPECT_EQ(page_state->get_page_size(), 2);
    EXPECT_EQ(page_state->get_total_pages(), 3);
    EXPECT_EQ(page_state->get_query().sort_field, LogField::Message);
    EXPECT_EQ(page_state->get_query().sort_order, Qt::DescendingOrder);

    LogModel* model = m_runtime->views().get_context(view_state.id)->get_model();
    ASSERT_NE(model, nullptr);
    ASSERT_EQ(model->rowCount(), 2);
    EXPECT_EQ(model->get_entry(0).get_message(), QStringLiteral("D"));
    EXPECT_EQ(model->get_entry(1).get_message(), QStringLiteral("C"));
}

/**
 * @test Verifies that multiple restored views complete independently with their saved filters.
 */
TEST_F(SessionControllerTest, RestoresMultipleViewsIndependently)
{
    QTemporaryFile* first_file =
        create_temp_file({QStringLiteral("2024-01-01 12:00:00 INFO FirstInfo FirstApp"),
                          QStringLiteral("2024-01-01 12:01:00 ERROR FirstError FirstApp")});
    QTemporaryFile* second_file =
        create_temp_file({QStringLiteral("2024-01-01 13:00:00 DEBUG SecondDebug SecondApp"),
                          QStringLiteral("2024-01-01 13:01:00 ERROR SecondError SecondApp")});
    ASSERT_NE(first_file, nullptr);
    ASSERT_NE(second_file, nullptr);

    SessionViewState first_state;
    first_state.id = QUuid::createUuid();
    first_state.loaded_files = {LogFileInfo(first_file->fileName(), QStringLiteral("FirstApp"))};
    first_state.filters.log_levels = {QStringLiteral("INFO")};
    first_state.filters.live_tailing_enabled = false;

    SessionViewState second_state;
    second_state.id = QUuid::createUuid();
    second_state.loaded_files = {LogFileInfo(second_file->fileName(), QStringLiteral("SecondApp"))};
    second_state.filters.log_levels = {QStringLiteral("ERROR")};
    second_state.filters.live_tailing_enabled = false;

    SessionState state;
    state.id = QStringLiteral("restore-multiple-views");
    state.views = {first_state, second_state};

    int restored_view_count = 0;
    QObject::connect(
        m_session_controller, &SessionController::view_restored, m_session_controller,
        [&restored_view_count](const QUuid&, const SessionViewState&) { ++restored_view_count; });
    QSignalSpy restored_spy(m_session_controller, &SessionController::session_restored);

    ASSERT_TRUE(m_session_controller->restore_session(state));
    EXPECT_EQ(restored_view_count, 2);
    QTRY_COMPARE(restored_spy.count(), 1);

    const LogPageState* first_page = m_runtime->pages().get_page_state(first_state.id);
    const LogPageState* second_page = m_runtime->pages().get_page_state(second_state.id);
    ASSERT_NE(first_page, nullptr);
    ASSERT_NE(second_page, nullptr);
    EXPECT_EQ(first_page->get_total_entries(), 1);
    EXPECT_EQ(second_page->get_total_entries(), 1);

    LogModel* first_model = m_runtime->views().get_context(first_state.id)->get_model();
    LogModel* second_model = m_runtime->views().get_context(second_state.id)->get_model();
    ASSERT_NE(first_model, nullptr);
    ASSERT_NE(second_model, nullptr);
    ASSERT_EQ(first_model->rowCount(), 1);
    ASSERT_EQ(second_model->rowCount(), 1);
    EXPECT_EQ(first_model->get_entry(0).get_message(), QStringLiteral("FirstInfo"));
    EXPECT_EQ(second_model->get_entry(0).get_message(), QStringLiteral("SecondError"));
}

/**
 * @test Verifies that an unavailable persisted profile falls back to the controller default.
 */
TEST_F(SessionControllerTest, FallsBackWhenPersistedProfileIsUnavailable)
{
    QTemporaryFile* file =
        create_temp_file({QStringLiteral("2024-01-01 12:00:00 INFO FallbackEntry FallbackApp")});
    ASSERT_NE(file, nullptr);

    const QString file_path = QFileInfo(file->fileName()).absoluteFilePath();
    SessionViewState view_state;
    view_state.id = QUuid::createUuid();
    view_state.loaded_files = {LogFileInfo(file_path, QStringLiteral("FallbackApp"))};
    view_state.file_parsing_profile_ids.insert(file_path, QUuid::createUuid());
    view_state.filters.live_tailing_enabled = false;

    SessionState state;
    state.id = QStringLiteral("restore-missing-profile");
    state.views = {view_state};

    QSignalSpy restored_spy(m_session_controller, &SessionController::session_restored);

    ASSERT_TRUE(m_session_controller->restore_session(state));
    QTRY_COMPARE(restored_spy.count(), 1);

    LogModel* model = m_runtime->views().get_context(view_state.id)->get_model();
    ASSERT_NE(model, nullptr);
    ASSERT_EQ(model->rowCount(), 1);
    EXPECT_EQ(model->get_entry(0).get_message(), QStringLiteral("FallbackEntry"));

    const SessionState exported_session = m_session_controller->export_session_state();
    ASSERT_EQ(exported_session.views.size(), 1);
    const SessionViewState exported = exported_session.views.constFirst();
    const auto profile_id = exported.file_parsing_profile_ids.constFind(file_path);
    ASSERT_NE(profile_id, exported.file_parsing_profile_ids.cend());
    EXPECT_EQ(profile_id.value(), m_default_profile.get_id());
}

/**
 * @test Verifies that restoring the same snapshot again replaces views and imported entries.
 */
TEST_F(SessionControllerTest, RestoresSameSessionWithoutDuplicatingViewsOrEntries)
{
    QTemporaryFile* file =
        create_temp_file({QStringLiteral("2024-01-01 12:00:00 INFO First RestoreApp"),
                          QStringLiteral("2024-01-01 12:01:00 ERROR Second RestoreApp")});
    ASSERT_NE(file, nullptr);

    SessionViewState view_state;
    view_state.id = QUuid::createUuid();
    view_state.loaded_files = {LogFileInfo(file->fileName(), QStringLiteral("RestoreApp"))};
    view_state.filters.live_tailing_enabled = false;

    SessionState state;
    state.id = QStringLiteral("restore-without-duplicates");
    state.views = {view_state};

    QSignalSpy restored_spy(m_session_controller, &SessionController::session_restored);

    ASSERT_TRUE(m_session_controller->restore_session(state));
    QTRY_COMPARE(restored_spy.count(), 1);
    ASSERT_TRUE(m_session_controller->restore_session(state));
    QTRY_COMPARE(restored_spy.count(), 2);

    const QVector<QUuid> view_ids = m_runtime->views().get_all_view_ids();
    ASSERT_EQ(view_ids.size(), 1);
    EXPECT_EQ(view_ids.constFirst(), view_state.id);

    const LogPageState* page_state = m_runtime->pages().get_page_state(view_state.id);
    ASSERT_NE(page_state, nullptr);
    EXPECT_EQ(page_state->get_total_entries(), 2);

    LogModel* model = m_runtime->views().get_context(view_state.id)->get_model();
    ASSERT_NE(model, nullptr);
    EXPECT_EQ(model->rowCount(), 2);
}
