#include "Qt-LogViewer/Controllers/SessionControllerTest.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSignalSpy>
#include <QTest>

#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/SessionController.h"
#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Models/LogFileTreeModel.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogPageState.h"
#include "Qt-LogViewer/Models/SessionTypes.h"
#include "Qt-LogViewer/Services/SessionCodec.h"
#include "Qt-LogViewer/Services/SessionManager.h"
#include "Qt-LogViewer/Services/SessionRepository.h"
#include "Qt-LogViewer/Support/TestLogRuntime.h"

/**
 * @file SessionControllerTest.cpp
 * @brief Implements integration tests for coordinated typed session restoration.
 */

/**
 * @brief Creates the controller graph and isolated session storage used by each test.
 */
void SessionControllerTest::SetUp()
{
    m_default_profile = LogParsingProfile::create_default(
        QStringLiteral("{timestamp} {level} {message} {app_name}"),
        QStringLiteral("Session test default"));
    const QString repository_marker =
        m_file_system.write_text_file(QStringLiteral("repository.marker"), QString());
    m_sessions_directory = QFileInfo(repository_marker).absolutePath();
    m_session_repository = new SessionRepository(m_sessions_directory);
    m_session_manager = new SessionManager(m_session_repository);
    m_runtime = new TestLogRuntime(m_default_profile);
    m_session_controller = new SessionController(
        m_session_manager, m_runtime->catalog().get_model(), m_default_profile,
        &m_runtime->catalog(), &m_runtime->views(), &m_runtime->filters(), &m_runtime->history(),
        &m_runtime->pages(), &m_runtime->queries(), &m_runtime->imports(), &m_runtime->lifecycle(),
        &m_runtime->live_tailing());
}

/**
 * @brief Destroys controllers and removes isolated session storage.
 */
void SessionControllerTest::TearDown()
{
    delete m_session_controller;
    m_session_controller = nullptr;
    delete m_runtime;
    m_runtime = nullptr;
    delete m_session_manager;
    m_session_manager = nullptr;
    delete m_session_repository;
    m_session_repository = nullptr;

    m_sessions_directory.clear();
}

/** @test Removes only requested recent metadata and persists the reduced list. */
TEST_F(SessionControllerTest, RemovesRecentFilesWithoutDeletingSourcesOrSessions)
{
    const QString first_file =
        m_file_system.write_text_file(QStringLiteral("first.log"), QStringLiteral("first"));
    const QString second_file =
        m_file_system.write_text_file(QStringLiteral("second.log"), QStringLiteral("second"));
    m_session_controller->add_recent_log_file(LogFileInfo(first_file, QStringLiteral("First")));
    m_session_controller->add_recent_log_file(LogFileInfo(second_file, QStringLiteral("Second")));
    m_session_manager->upsert_session_metadata(QStringLiteral("session-id"),
                                               QStringLiteral("Saved session"), false);
    m_session_manager->save_session(
        QStringLiteral("session-id"),
        QJsonObject{{QStringLiteral("name"), QStringLiteral("Saved session")}});
    const QString session_file_path =
        m_session_repository->get_session_file_path(QStringLiteral("session-id"));
    QSignalSpy recent_files_spy(m_session_manager, &SessionManager::recent_log_files_changed);

    const int removed_count =
        m_session_controller->remove_recent_log_files(QSet<QString>{first_file});

    EXPECT_EQ(removed_count, 1);
    EXPECT_EQ(recent_files_spy.count(), 1);
    const QVector<RecentLogFileRecord> remaining = m_session_manager->get_recent_log_files();
    ASSERT_EQ(remaining.size(), 1);
    EXPECT_EQ(remaining.constFirst().file_path, second_file);
    EXPECT_TRUE(QFileInfo::exists(first_file));
    EXPECT_TRUE(QFileInfo::exists(second_file));
    EXPECT_TRUE(QFileInfo::exists(session_file_path));
    ASSERT_EQ(m_session_manager->get_recent_sessions().size(), 1);

    SessionManager reloaded_manager(m_session_repository);
    reloaded_manager.initialize_from_storage();
    const QVector<RecentLogFileRecord> persisted = reloaded_manager.get_recent_log_files();
    ASSERT_EQ(persisted.size(), 1);
    EXPECT_EQ(persisted.constFirst().file_path, second_file);
    EXPECT_EQ(reloaded_manager.get_recent_sessions().size(), 1);
}

/** @test Relinks recent metadata while preserving its attributes and saved sessions. */
TEST_F(SessionControllerTest, RelinksRecentFileWithoutChangingSavedSessions)
{
    const QString missing_file =
        QDir(m_sessions_directory).filePath(QStringLiteral("old-location/moved.log"));
    const QString replacement_file =
        m_file_system.write_text_file(QStringLiteral("moved.log"), QStringLiteral("replacement"));
    m_session_controller->add_recent_log_file(
        LogFileInfo(missing_file, QStringLiteral("Original application")));
    m_session_manager->upsert_session_metadata(QStringLiteral("session-id"),
                                               QStringLiteral("Saved session"), false);
    m_session_manager->save_session(
        QStringLiteral("session-id"),
        QJsonObject{{QStringLiteral("name"), QStringLiteral("Saved session")}});
    const QString session_file_path =
        m_session_repository->get_session_file_path(QStringLiteral("session-id"));
    QSignalSpy recent_files_spy(m_session_manager, &SessionManager::recent_log_files_changed);

    const bool changed =
        m_session_controller->relink_recent_log_file(missing_file, replacement_file);

    EXPECT_TRUE(changed);
    EXPECT_EQ(recent_files_spy.count(), 1);
    const QVector<RecentLogFileRecord> recent_files = m_session_manager->get_recent_log_files();
    ASSERT_EQ(recent_files.size(), 1);
    EXPECT_EQ(recent_files.constFirst().file_path, replacement_file);
    EXPECT_EQ(recent_files.constFirst().app_name, QStringLiteral("Original application"));
    EXPECT_TRUE(QFileInfo::exists(session_file_path));

    SessionManager reloaded_manager(m_session_repository);
    reloaded_manager.initialize_from_storage();
    const QVector<RecentLogFileRecord> persisted_files = reloaded_manager.get_recent_log_files();
    ASSERT_EQ(persisted_files.size(), 1);
    EXPECT_EQ(persisted_files.constFirst().file_path, replacement_file);
    EXPECT_EQ(reloaded_manager.get_recent_sessions().size(), 1);
}

/**
 * @test Verifies that an external session is loaded by path, assigned a safe identifier, and
 * persisted as the active managed session.
 */
TEST_F(SessionControllerTest, ImportsExternalSessionFileIntoManagedStorage)
{
    SessionState source_state;
    source_state.id = QStringLiteral("../unsafe-source-id");
    source_state.name = QStringLiteral("Imported session");
    const QByteArray session_json =
        QJsonDocument(SessionCodec::to_json(source_state)).toJson(QJsonDocument::Compact);
    const QString session_file_path = m_file_system.write_text_file(
        QStringLiteral("external-session.json"), QString::fromUtf8(session_json));
    ASSERT_FALSE(session_file_path.isEmpty());

    QSignalSpy created_spy(m_session_controller, &SessionController::session_created);
    QSignalSpy current_spy(m_session_controller, &SessionController::current_session_changed);

    const std::optional<SessionState> imported_state =
        m_session_controller->import_session_file(session_file_path);

    ASSERT_TRUE(imported_state.has_value());
    EXPECT_FALSE(imported_state->id.isEmpty());
    EXPECT_NE(imported_state->id, source_state.id);
    EXPECT_EQ(imported_state->name, source_state.name);
    EXPECT_EQ(m_session_manager->get_current_session_id(), imported_state->id);
    EXPECT_EQ(m_session_manager->get_last_session_id(), imported_state->id);
    EXPECT_TRUE(m_runtime->catalog().get_model()->has_session(imported_state->id));
    EXPECT_EQ(created_spy.count(), 1);
    EXPECT_EQ(current_spy.count(), 1);

    const std::optional<SessionState> persisted_state = SessionCodec::from_json(
        m_session_manager->load_session(imported_state->id), imported_state->id);
    ASSERT_TRUE(persisted_state.has_value());
    EXPECT_EQ(persisted_state->id, imported_state->id);
    EXPECT_EQ(persisted_state->name, source_state.name);
}

/**
 * @test Verifies that opening a managed session document loads its existing identifier instead of
 * creating a duplicate session.
 */
TEST_F(SessionControllerTest, OpensManagedSessionFileWithoutDuplicatingIt)
{
    const QString session_id =
        m_session_controller->ensure_current_session(QStringLiteral("Managed session"));
    ASSERT_FALSE(session_id.isEmpty());
    m_session_controller->save_current_session();

    const QString session_file_path = m_session_repository->get_session_file_path(session_id);
    ASSERT_TRUE(QFileInfo::exists(session_file_path));
    const int session_count_before = m_session_manager->get_recent_sessions().size();

    QSignalSpy created_spy(m_session_controller, &SessionController::session_created);
    const std::optional<SessionState> opened_state =
        m_session_controller->open_session_file(session_file_path);

    ASSERT_TRUE(opened_state.has_value());
    EXPECT_EQ(opened_state->id, session_id);
    EXPECT_EQ(m_session_manager->get_current_session_id(), session_id);
    EXPECT_EQ(m_session_manager->get_recent_sessions().size(), session_count_before);
    EXPECT_EQ(created_spy.count(), 0);
}

/**
 * @test Verifies that importing a session replaces every previously displayed session while
 * preserving the previous session in managed storage.
 */
TEST_F(SessionControllerTest, ImportReplacesDisplayedSessionWithoutDeletingPersistence)
{
    const QString previous_session_id =
        m_session_controller->ensure_current_session(QStringLiteral("Previous session"));
    ASSERT_FALSE(previous_session_id.isEmpty());
    ASSERT_TRUE(m_runtime->catalog().get_model()->has_session(previous_session_id));

    SessionState source_state;
    source_state.name = QStringLiteral("Dropped session");
    const QByteArray session_json =
        QJsonDocument(SessionCodec::to_json(source_state)).toJson(QJsonDocument::Compact);
    const QString session_file_path = m_file_system.write_text_file(
        QStringLiteral("dropped-session.json"), QString::fromUtf8(session_json));
    ASSERT_FALSE(session_file_path.isEmpty());

    const std::optional<SessionState> imported_state =
        m_session_controller->import_session_file(session_file_path);

    ASSERT_TRUE(imported_state.has_value());
    EXPECT_EQ(m_runtime->catalog().get_model()->get_session_count(), 1);
    EXPECT_FALSE(m_runtime->catalog().get_model()->has_session(previous_session_id));
    EXPECT_TRUE(m_runtime->catalog().get_model()->has_session(imported_state->id));
    EXPECT_TRUE(
        SessionCodec::is_session_document(m_session_manager->load_session(previous_session_id)));
}

/**
 * @test Verifies that a valid JSON root document without session payload is rejected.
 */
TEST_F(SessionControllerTest, RejectsNonSessionJsonDocument)
{
    const QString root_json =
        QStringLiteral(R"({"schema_version":1,"recent_files":[],"recent_sessions":[]})");
    const QString session_file_path =
        m_file_system.write_text_file(QStringLiteral("root.json"), root_json);
    ASSERT_FALSE(session_file_path.isEmpty());

    const std::optional<SessionState> imported_state =
        m_session_controller->import_session_file(session_file_path);

    EXPECT_FALSE(imported_state.has_value());
    EXPECT_FALSE(m_session_manager->has_current_session());
}

/**
 * @test Verifies that a session document using an unsupported schema version is rejected.
 */
TEST_F(SessionControllerTest, RejectsUnsupportedSessionSchema)
{
    const QString session_json =
        QStringLiteral(R"({"schema_version":999,"name":"Future session","views":[]})");
    const QString session_file_path =
        m_file_system.write_text_file(QStringLiteral("future-session.json"), session_json);
    ASSERT_FALSE(session_file_path.isEmpty());

    const std::optional<SessionState> imported_state =
        m_session_controller->import_session_file(session_file_path);

    EXPECT_FALSE(imported_state.has_value());
    EXPECT_FALSE(m_session_manager->has_current_session());
}

/**
 * @test Verifies that saved sorting and page two are applied after restored entries exist.
 */
TEST_F(SessionControllerTest, AppliesSavedQueryStateAfterImportCompletion)
{
    const QString file_path = m_file_system.write_text_file(
        QStringLiteral("paged.log"), QStringLiteral("2024-01-01 12:01:00 INFO A RestoreApp\n"
                                                    "2024-01-01 12:02:00 INFO B RestoreApp\n"
                                                    "2024-01-01 12:03:00 INFO C RestoreApp\n"
                                                    "2024-01-01 12:04:00 INFO D RestoreApp\n"
                                                    "2024-01-01 12:05:00 INFO E RestoreApp\n"
                                                    "2024-01-01 12:06:00 INFO F RestoreApp\n"));
    ASSERT_FALSE(file_path.isEmpty());

    SessionViewState view_state;
    view_state.id = QUuid::createUuid();
    view_state.loaded_files = {LogFileInfo(file_path, QStringLiteral("RestoreApp"))};
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
    const QString first_file_path = m_file_system.write_text_file(
        QStringLiteral("first.log"),
        QStringLiteral("2024-01-01 12:00:00 INFO FirstInfo FirstApp\n"
                       "2024-01-01 12:01:00 ERROR FirstError FirstApp\n"));
    const QString second_file_path = m_file_system.write_text_file(
        QStringLiteral("second.log"),
        QStringLiteral("2024-01-01 13:00:00 DEBUG SecondDebug SecondApp\n"
                       "2024-01-01 13:01:00 ERROR SecondError SecondApp\n"));
    ASSERT_FALSE(first_file_path.isEmpty());
    ASSERT_FALSE(second_file_path.isEmpty());

    SessionViewState first_state;
    first_state.id = QUuid::createUuid();
    first_state.loaded_files = {LogFileInfo(first_file_path, QStringLiteral("FirstApp"))};
    first_state.filters.log_levels = {QStringLiteral("INFO")};
    first_state.filters.live_tailing_enabled = false;

    SessionViewState second_state;
    second_state.id = QUuid::createUuid();
    second_state.loaded_files = {LogFileInfo(second_file_path, QStringLiteral("SecondApp"))};
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
    QTRY_COMPARE(restored_view_count, 2);
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
    const QString file_path = m_file_system.write_text_file(
        QStringLiteral("fallback.log"),
        QStringLiteral("2024-01-01 12:00:00 INFO FallbackEntry FallbackApp\n"));
    ASSERT_FALSE(file_path.isEmpty());
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
    const QString file_path = m_file_system.write_text_file(
        QStringLiteral("duplicate-restore.log"),
        QStringLiteral("2024-01-01 12:00:00 INFO First RestoreApp\n"
                       "2024-01-01 12:01:00 ERROR Second RestoreApp\n"));
    ASSERT_FALSE(file_path.isEmpty());

    SessionViewState view_state;
    view_state.id = QUuid::createUuid();
    view_state.loaded_files = {LogFileInfo(file_path, QStringLiteral("RestoreApp"))};
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

/**
 * @test Verifies a restored view is exposed without stale history before presentation releases
 * its deferred imports.
 */
TEST_F(SessionControllerTest, ExposesEmptyRestoredViewBeforeReplacingStaleHistory)
{
    const QString file_path = m_file_system.write_text_file(
        QStringLiteral("fresh.log"),
        QStringLiteral("2024-01-01 12:00:00 INFO FreshEntry RestoreApp\n"));
    ASSERT_FALSE(file_path.isEmpty());
    const QUuid view_id = QUuid::createUuid();
    const LogEntry stale_entry(
        QDateTime::fromString(QStringLiteral("2024-01-01T11:00:00.000Z"), Qt::ISODateWithMs),
        QStringLiteral("INFO"), QStringLiteral("StalePartialEntry"),
        LogFileInfo(file_path, QStringLiteral("RestoreApp")));
    ASSERT_TRUE(m_runtime->history().add_entries(view_id, {stale_entry}));

    SessionViewState view_state;
    view_state.id = view_id;
    view_state.loaded_files = {LogFileInfo(file_path, QStringLiteral("RestoreApp"))};
    view_state.filters.live_tailing_enabled = false;

    SessionState state;
    state.id = QStringLiteral("restore-with-stale-partial-history");
    state.views = {view_state};

    QSignalSpy view_restored_spy(m_session_controller, &SessionController::view_restored);
    QSignalSpy views_registered_spy(m_session_controller,
                                    &SessionController::session_views_registered);
    QSignalSpy session_restored_spy(m_session_controller, &SessionController::session_restored);
    QObject::connect(
        m_session_controller, &SessionController::session_views_registered, m_session_controller,
        [this](const QString&) { m_session_controller->defer_restored_imports_until_presented(); });

    ASSERT_TRUE(m_session_controller->restore_session(state));
    ASSERT_EQ(view_restored_spy.count(), 1);
    ASSERT_EQ(views_registered_spy.count(), 1);
    EXPECT_EQ(views_registered_spy.constFirst().at(0).toString(), state.id);
    EXPECT_EQ(session_restored_spy.count(), 0);
    const LogPageState* initial_page_state = m_runtime->pages().get_page_state(view_id);
    ASSERT_NE(initial_page_state, nullptr);
    EXPECT_EQ(initial_page_state->get_total_entries(), 0);
    LogModel* initial_model = m_runtime->views().get_context(view_id)->get_model();
    ASSERT_NE(initial_model, nullptr);
    EXPECT_EQ(initial_model->rowCount(), 0);

    QTest::qWait(50);
    EXPECT_EQ(session_restored_spy.count(), 0);
    m_session_controller->start_deferred_restore_imports();
    QTRY_COMPARE_WITH_TIMEOUT(session_restored_spy.count(), 1, 5000);
    ASSERT_EQ(view_restored_spy.count(), 1);

    const LogPageState* page_state = m_runtime->pages().get_page_state(view_id);
    ASSERT_NE(page_state, nullptr);
    EXPECT_EQ(page_state->get_total_entries(), 1);

    LogModel* model = m_runtime->views().get_context(view_id)->get_model();
    ASSERT_NE(model, nullptr);
    ASSERT_EQ(model->rowCount(), 1);
    EXPECT_EQ(model->get_entry(0).get_message(), QStringLiteral("FreshEntry"));
}
