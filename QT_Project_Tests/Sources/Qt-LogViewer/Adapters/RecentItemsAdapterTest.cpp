/**
 * @file RecentItemsAdapterTest.cpp
 * @brief Verifies mapping of SessionManager updates into recent-item models.
 */

#include "Qt-LogViewer/Adapters/RecentItemsAdapterTest.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>

#include "Qt-LogViewer/Adapters/RecentItemsAdapter.h"
#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Models/RecentRoles.h"
#include "Qt-LogViewer/Services/SessionManager.h"
#include "Qt-LogViewer/Services/SessionRepository.h"

/**
 * @brief Ensures both recent models follow records emitted by SessionManager.
 */
TEST_F(RecentItemsAdapterTest, SynchronizesModelsWhenRecentItemsChange)
{
    const QString marker_path =
        m_file_system.write_text_file(QStringLiteral("adapter.marker"), QString());
    SessionRepository repository(QFileInfo(marker_path).absolutePath());
    SessionManager session_manager(&repository);
    RecentItemsAdapter adapter(&session_manager);
    RecentItemsModel* files_model = adapter.get_recent_files_model();
    RecentItemsModel* sessions_model = adapter.get_recent_sessions_model();
    QSignalSpy files_reset_spy(files_model, &QAbstractItemModel::modelReset);
    QSignalSpy sessions_reset_spy(sessions_model, &QAbstractItemModel::modelReset);

    const QVector<RecentLogFileRecord> files{
        {QStringLiteral("C:/logs/application.log"), QStringLiteral("Application"),
         QDateTime::fromString(QStringLiteral("2026-09-28T10:00:00"), Qt::ISODate)}};
    const QVector<RecentSessionRecord> sessions{
        {QStringLiteral("session-id"), QStringLiteral("Session"),
         QDateTime::fromString(QStringLiteral("2026-09-28T09:00:00"), Qt::ISODate),
         QDateTime::fromString(QStringLiteral("2026-09-28T10:00:00"), Qt::ISODate)}};

    emit session_manager.recent_log_files_changed(files);
    emit session_manager.recent_sessions_changed(sessions);

    ASSERT_EQ(files_model->rowCount(), 1);
    ASSERT_EQ(sessions_model->rowCount(), 1);
    EXPECT_EQ(files_model->data(files_model->index(0, 0), to_role_id(RecentItemRole::Identity)),
              files.front().file_path);
    EXPECT_EQ(
        sessions_model->data(sessions_model->index(0, 0), to_role_id(RecentItemRole::Identity)),
        sessions.front().id);
    EXPECT_EQ(files_reset_spy.count(), 1);
    EXPECT_EQ(sessions_reset_spy.count(), 1);
}

/** @test Re-evaluates availability without persisting derived filesystem state. */
TEST_F(RecentItemsAdapterTest, RefreshesTransientRecentFileAvailability)
{
    const QString file_path =
        m_file_system.write_text_file(QStringLiteral("recent.log"), QStringLiteral("entry"));
    SessionRepository repository(QFileInfo(file_path).absolutePath());
    SessionManager session_manager(&repository);
    RecentItemsAdapter adapter(&session_manager);

    session_manager.add_recent_log_file(LogFileInfo(file_path));

    RecentItemsModel* model = adapter.get_recent_files_model();
    ASSERT_EQ(model->rowCount(), 1);
    const QModelIndex available_index = model->index(0, 0);
    EXPECT_TRUE(available_index.flags().testFlag(Qt::ItemIsEnabled));
    EXPECT_FALSE(
        model->data(available_index, to_role_id(RecentItemRole::StatusText)).toString().isEmpty());

    ASSERT_TRUE(QFile::remove(file_path));
    adapter.refresh_file_availability();

    const QModelIndex missing_index = model->index(0, 0);
    EXPECT_FALSE(missing_index.flags().testFlag(Qt::ItemIsEnabled));
    EXPECT_FALSE(
        model->data(missing_index, to_role_id(RecentItemRole::StatusText)).toString().isEmpty());
    EXPECT_FALSE(model->data(missing_index, Qt::ToolTipRole).toString().isEmpty());
    EXPECT_EQ(model->data(missing_index, Qt::AccessibleDescriptionRole),
              model->data(missing_index, Qt::ToolTipRole));
}
