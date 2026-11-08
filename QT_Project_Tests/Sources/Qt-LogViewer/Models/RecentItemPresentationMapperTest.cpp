/**
 * @file RecentItemPresentationMapperTest.cpp
 * @brief Tests Qt-LogViewer mappings onto the shared recent-item presentation contract.
 */

#include "Qt-LogViewer/Models/RecentItemPresentationMapperTest.h"

#include <QDateTime>
#include <QString>

#include "Qt-LogViewer/Models/RecentItemPresentationMapper.h"

using QtWidgetsCommonLib::RecentItemPresentation;

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
