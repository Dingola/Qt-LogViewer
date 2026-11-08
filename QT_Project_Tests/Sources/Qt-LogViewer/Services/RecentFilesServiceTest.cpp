/**
 * @file RecentFilesServiceTest.cpp
 * @brief Tests recent-file ordering, limits, compatibility, and repository isolation.
 */

#include "Qt-LogViewer/Services/RecentFilesServiceTest.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>

#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Services/RecentFilesService.h"
#include "Qt-LogViewer/Services/SessionRepository.h"

/**
 * @brief Creates an isolated repository and initialized recent-file service.
 */
void RecentFilesServiceTest::SetUp()
{
    const QString marker_path =
        m_file_system.write_text_file(QStringLiteral("repository.marker"), QString());
    m_repository = new SessionRepository(QFileInfo(marker_path).absolutePath());
    m_service = new RecentFilesService(m_repository);
    m_service->initialize_from_storage();
}

/**
 * @brief Destroys the service and repository owned by the fixture.
 */
void RecentFilesServiceTest::TearDown()
{
    delete m_service;
    m_service = nullptr;
    delete m_repository;
    m_repository = nullptr;
}

/**
 * @test Verifies that adding the same normalized file updates it and moves it to the MRU front.
 */
TEST_F(RecentFilesServiceTest, AddsUpdatesAndNormalizesRecentFiles)
{
    const QString first_path =
        m_file_system.write_text_file(QStringLiteral("first.log"), QStringLiteral("first"));
    const QString second_path =
        m_file_system.write_text_file(QStringLiteral("second.log"), QStringLiteral("second"));

    m_service->add_recent_log_file(LogFileInfo(first_path, QStringLiteral("First")));
    m_service->add_recent_log_file(LogFileInfo(second_path, QStringLiteral("Second")));
    m_service->add_recent_log_file(LogFileInfo(first_path, QStringLiteral("Updated")));

    const QVector<RecentLogFileRecord> records = m_service->get_recent_log_files();
    ASSERT_EQ(records.size(), 2);
    EXPECT_EQ(records.at(0).file_path, QFileInfo(first_path).canonicalFilePath());
    EXPECT_EQ(records.at(0).app_name, QStringLiteral("Updated"));
    EXPECT_EQ(records.at(1).file_path, QFileInfo(second_path).canonicalFilePath());
}

/**
 * @test Verifies that a configured maximum size evicts the least-recently-used file.
 */
TEST_F(RecentFilesServiceTest, EnforcesConfiguredMaximumSize)
{
    RecentFilesService limited_service(m_repository, 1);
    limited_service.initialize_from_storage();
    const QString first_path =
        m_file_system.write_text_file(QStringLiteral("limited-first.log"), QString());
    const QString second_path =
        m_file_system.write_text_file(QStringLiteral("limited-second.log"), QString());

    limited_service.add_recent_log_file(LogFileInfo(first_path));
    limited_service.add_recent_log_file(LogFileInfo(second_path));

    const QVector<RecentLogFileRecord> records = limited_service.get_recent_log_files();
    ASSERT_EQ(records.size(), 1);
    EXPECT_EQ(records.constFirst().file_path, QFileInfo(second_path).canonicalFilePath());
}

/**
 * @test Verifies that writes retain session fields in the shared index document.
 */
TEST_F(RecentFilesServiceTest, PreservesUnrelatedRootFieldsWhilePersisting)
{
    QJsonObject root = m_repository->load_all();
    root.insert(QStringLiteral("last_session_id"), QStringLiteral("session-id"));
    root.insert(QStringLiteral("recent_sessions"),
                QJsonArray{QJsonObject{{QStringLiteral("id"), QStringLiteral("session-id")}}});
    m_repository->save_all(root);
    const QString file_path =
        m_file_system.write_text_file(QStringLiteral("preserved.log"), QString());

    m_service->add_recent_log_file(LogFileInfo(file_path));

    const QJsonObject persisted = m_repository->load_all();
    EXPECT_EQ(persisted.value(QStringLiteral("last_session_id")).toString(),
              QStringLiteral("session-id"));
    ASSERT_EQ(persisted.value(QStringLiteral("recent_sessions")).toArray().size(), 1);
    EXPECT_EQ(persisted.value(QStringLiteral("recent_files")).toArray().size(), 1);
}

/**
 * @test Verifies that records in the existing index schema load without migration.
 */
TEST_F(RecentFilesServiceTest, LoadsExistingRecentFilesSchemaWithoutMigration)
{
    const QString timestamp = QStringLiteral("2026-11-08T10:15:30.000Z");
    QJsonObject root = m_repository->load_all();
    root.insert(QStringLiteral("recent_files"),
                QJsonArray{QJsonObject{
                    {QStringLiteral("file_path"), QStringLiteral("C:/logs/existing.log")},
                    {QStringLiteral("app_name"), QStringLiteral("Existing")},
                    {QStringLiteral("last_opened"), timestamp}}});
    m_repository->save_all(root);
    RecentFilesService reloaded_service(m_repository);

    reloaded_service.initialize_from_storage();

    const QVector<RecentLogFileRecord> records = reloaded_service.get_recent_log_files();
    ASSERT_EQ(records.size(), 1);
    EXPECT_EQ(records.constFirst().file_path, QStringLiteral("C:/logs/existing.log"));
    EXPECT_EQ(records.constFirst().app_name, QStringLiteral("Existing"));
    EXPECT_TRUE(records.constFirst().last_opened.isValid());
}
