/**
 * @file RecentItemPresentationMapperTest.cpp
 * @brief Tests Qt-LogViewer mappings onto the shared recent-item presentation contract.
 */

#include "Qt-LogViewer/Models/RecentItemPresentationMapperTest.h"

#include <QDateTime>
#include <QString>

#include "Qt-LogViewer/Models/RecentItemPresentationMapper.h"
#include "Qt-LogViewer/Services/LocalFileStatus.h"

/**
 * @brief Verifies that a log-file domain record maps to the shared presentation contract.
 */
TEST_F(RecentItemPresentationMapperTest, MapsRecentLogFileToPresentationItem)
{
    const RecentLogFileRecord record{
        QStringLiteral("C:/logs/app/test.log"), QStringLiteral("LogViewerApp"),
        QDateTime::fromString(QStringLiteral("2025-03-10T12:34:56"), Qt::ISODate)};
    const QtCommonLib::LocalFileStatus status{record.file_path, record.file_path,
                                              QtCommonLib::LocalFileAvailability::Missing};

    const RecentItemPresentation presentation =
        RecentItemPresentationMapper::from_recent_log_file(record, status);

    EXPECT_EQ(presentation.identity.toString(), record.file_path);
    EXPECT_EQ(presentation.title, QStringLiteral("test.log"));
    EXPECT_EQ(presentation.secondary_text, record.file_path);
    EXPECT_EQ(presentation.last_used, record.last_opened);
    EXPECT_FALSE(presentation.enabled);
    EXPECT_FALSE(presentation.status_text.isEmpty());
    EXPECT_TRUE(presentation.tooltip.contains(record.file_path));
}

/**
 * @brief Verifies that a session record maps to the same presentation contract.
 */
TEST_F(RecentItemPresentationMapperTest, MapsRecentSessionToPresentationItem)
{
    const QDateTime created_at =
        QDateTime::fromString(QStringLiteral("2025-03-11T18:00:00"), Qt::ISODate);
    const RecentSessionRecord record{QStringLiteral("sess-002"), QStringLiteral("Evening Session"),
                                     created_at, QDateTime()};

    const RecentItemPresentation presentation =
        RecentItemPresentationMapper::from_recent_session(record);

    EXPECT_EQ(presentation.identity.toString(), record.id);
    EXPECT_EQ(presentation.title, record.name);
    EXPECT_EQ(presentation.secondary_text, record.id);
    EXPECT_EQ(presentation.last_used, created_at);
    EXPECT_TRUE(presentation.enabled);
}
