#include "Qt-LogViewer/Models/RecentListSchemaTest.h"

#include <QFileInfo>

/**
 * @brief Verifies recent files schema role names and columns.
 */
TEST_F(RecentListSchemaTest, MakeRecentFilesSchemaDefinesRolesAndColumns)
{
    const auto schema = RecentListSchemas::make_recent_files_schema();

    // Roles
    ASSERT_EQ(schema.role_names.size(), 4);
    EXPECT_EQ(schema.role_names.value(to_role_id(RecentFileRole::FilePath)),
              QByteArray("file_path"));
    EXPECT_EQ(schema.role_names.value(to_role_id(RecentFileRole::FileName)),
              QByteArray("file_name"));
    EXPECT_EQ(schema.role_names.value(to_role_id(RecentFileRole::AppName)), QByteArray("app_name"));
    EXPECT_EQ(schema.role_names.value(to_role_id(RecentFileRole::LastOpened)),
              QByteArray("last_opened"));

    // Columns: File Name, File Path, Last Opened
    ASSERT_EQ(schema.columns.size(), 3);
    EXPECT_EQ(schema.columns.at(0).display_role, to_role_id(RecentFileRole::FileName));
    EXPECT_EQ(schema.columns.at(0).header_title, QStringLiteral("File Name"));

    EXPECT_EQ(schema.columns.at(1).display_role, to_role_id(RecentFileRole::FilePath));
    EXPECT_EQ(schema.columns.at(1).header_title, QStringLiteral("File Path"));

    EXPECT_EQ(schema.columns.at(2).display_role, to_role_id(RecentFileRole::LastOpened));
    EXPECT_EQ(schema.columns.at(2).header_title, QStringLiteral("Last Opened"));
}

/**
 * @brief Verifies recent sessions schema role names and columns.
 */
TEST_F(RecentListSchemaTest, MakeRecentSessionsSchemaDefinesRolesAndColumns)
{
    const auto schema = RecentListSchemas::make_recent_sessions_schema();

    // Roles
    ASSERT_EQ(schema.role_names.size(), 3);
    EXPECT_EQ(schema.role_names.value(to_role_id(RecentSessionRole::Name)), QByteArray("name"));
    EXPECT_EQ(schema.role_names.value(to_role_id(RecentSessionRole::LastOpened)),
              QByteArray("last_opened"));
    EXPECT_EQ(schema.role_names.value(to_role_id(RecentSessionRole::Id)), QByteArray("id"));

    // Columns: Session Name, Last Opened, Session Id
    ASSERT_EQ(schema.columns.size(), 3);
    EXPECT_EQ(schema.columns.at(0).display_role, to_role_id(RecentSessionRole::Name));
    EXPECT_EQ(schema.columns.at(0).header_title, QStringLiteral("Session Name"));

    EXPECT_EQ(schema.columns.at(1).display_role, to_role_id(RecentSessionRole::LastOpened));
    EXPECT_EQ(schema.columns.at(1).header_title, QStringLiteral("Last Opened"));

    EXPECT_EQ(schema.columns.at(2).display_role, to_role_id(RecentSessionRole::Id));
    EXPECT_EQ(schema.columns.at(2).header_title, QStringLiteral("Session Id"));
}

/**
 * @brief Ensures file row builder fills all roles and derives `file_name` from `file_path`.
 */
TEST_F(RecentListSchemaTest, BuildRecentFileRowPopulatesAllRolesAndDerivesFileName)
{
    const RecentLogFileRecord rec{
        QStringLiteral("C:/logs/app/test.log"), QStringLiteral("LogViewerApp"),
        QDateTime::fromString(QStringLiteral("2025-03-10T12:34:56"), Qt::ISODate)};

    const auto row = RecentListSchemas::build_recent_file_row(rec);

    ASSERT_EQ(row.size(), 4);
    EXPECT_EQ(row.value(to_role_id(RecentFileRole::FilePath)).toString(), rec.file_path);
    EXPECT_EQ(row.value(to_role_id(RecentFileRole::AppName)).toString(), rec.app_name);
    EXPECT_EQ(row.value(to_role_id(RecentFileRole::LastOpened)).toDateTime(), rec.last_opened);

    const QString expected_name = QFileInfo(rec.file_path).fileName();
    EXPECT_EQ(row.value(to_role_id(RecentFileRole::FileName)).toString(), expected_name);
}

/**
 * @brief Ensures session row builder uses `last_opened` when valid; otherwise falls back to
 * `created_at`.
 */
TEST_F(RecentListSchemaTest, BuildRecentSessionRowUsesEffectiveLastOpened)
{
    // Case 1: last_opened valid -> use it
    const RecentSessionRecord rec_valid{
        QStringLiteral("sess-001"), QStringLiteral("Morning Session"),
        QDateTime::fromString(QStringLiteral("2025-03-10T08:00:00"), Qt::ISODate),
        QDateTime::fromString(QStringLiteral("2025-03-10T09:00:00"), Qt::ISODate)};

    const auto row_valid = RecentListSchemas::build_recent_session_row(rec_valid);
    ASSERT_EQ(row_valid.size(), 3);
    EXPECT_EQ(row_valid.value(to_role_id(RecentSessionRole::Name)).toString(), rec_valid.name);
    EXPECT_EQ(row_valid.value(to_role_id(RecentSessionRole::Id)).toString(), rec_valid.id);
    EXPECT_EQ(row_valid.value(to_role_id(RecentSessionRole::LastOpened)).toDateTime(),
              rec_valid.last_opened);

    // Case 2: last_opened invalid -> fallback to created_at
    const RecentSessionRecord rec_fallback{
        QStringLiteral("sess-002"), QStringLiteral("Evening Session"),
        QDateTime::fromString(QStringLiteral("2025-03-11T18:00:00"), Qt::ISODate),
        QDateTime()  // invalid
    };

    const auto row_fallback = RecentListSchemas::build_recent_session_row(rec_fallback);
    ASSERT_EQ(row_fallback.size(), 3);
    EXPECT_EQ(row_fallback.value(to_role_id(RecentSessionRole::Name)).toString(),
              rec_fallback.name);
    EXPECT_EQ(row_fallback.value(to_role_id(RecentSessionRole::Id)).toString(), rec_fallback.id);
    EXPECT_EQ(row_fallback.value(to_role_id(RecentSessionRole::LastOpened)).toDateTime(),
              rec_fallback.created_at);
}
