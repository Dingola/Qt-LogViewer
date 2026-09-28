/**
 * @file RecentItemsAdapterTest.cpp
 * @brief Verifies mapping of SessionManager updates into recent-item models.
 */

#include "Qt-LogViewer/Adapters/RecentItemsAdapterTest.h"

#include <QDateTime>
#include <QSignalSpy>

#include "Qt-LogViewer/Adapters/RecentItemsAdapter.h"
#include "Qt-LogViewer/Models/RecentListSchema.h"
#include "Qt-LogViewer/Services/SessionManager.h"
#include "Qt-LogViewer/Services/SessionRepository.h"

/**
 * @brief Ensures both recent models follow records emitted by SessionManager.
 */
TEST_F(RecentItemsAdapterTest, SynchronizesModelsWhenRecentItemsChange)
{
    SessionRepository repository(QStringLiteral("RecentItemsAdapterTest"));
    SessionManager session_manager(&repository);
    RecentItemsAdapter adapter(&session_manager);
    QSignalSpy changed_spy(&adapter, &RecentItemsAdapter::recent_items_changed);

    const QVector<RecentLogFileRecord> files{
        {QStringLiteral("C:/logs/application.log"), QStringLiteral("Application"),
         QDateTime::fromString(QStringLiteral("2026-09-28T10:00:00"), Qt::ISODate)}};
    const QVector<RecentSessionRecord> sessions{
        {QStringLiteral("session-id"), QStringLiteral("Session"),
         QDateTime::fromString(QStringLiteral("2026-09-28T09:00:00"), Qt::ISODate),
         QDateTime::fromString(QStringLiteral("2026-09-28T10:00:00"), Qt::ISODate)}};

    emit session_manager.recent_log_files_changed(files);
    emit session_manager.recent_sessions_changed(sessions);

    RecentItemsModel* files_model = adapter.get_recent_files_model();
    RecentItemsModel* sessions_model = adapter.get_recent_sessions_model();
    ASSERT_EQ(files_model->rowCount(), 1);
    ASSERT_EQ(sessions_model->rowCount(), 1);
    EXPECT_EQ(files_model->data(files_model->index(0, 0), to_role_id(RecentFileRole::FilePath)),
              files.front().file_path);
    EXPECT_EQ(sessions_model->data(sessions_model->index(0, 0), to_role_id(RecentSessionRole::Id)),
              sessions.front().id);
    EXPECT_EQ(changed_spy.count(), 2);
}
