/**
 * @file HistoryWriteServiceTest.cpp
 * @brief Verifies write ordering, failures, cleanup, and deterministic shutdown.
 */

#include "Qt-LogViewer/Services/HistoryWriteServiceTest.h"

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTest>

#include "Qt-LogViewer/Models/LogQuery.h"
#include "Qt-LogViewer/Services/HistoryWriteService.h"
#include "Qt-LogViewer/Services/LogHistoryService.h"

/**
 * @brief Creates a deterministic log entry.
 * @param index Sequence number included in the timestamp and message.
 * @param file_path Source file path stored in the entry.
 * @return Constructed log entry.
 */
auto HistoryWriteServiceTest::create_entry(int index, const QString& file_path) const -> LogEntry
{
    return LogEntry(
        QDateTime::fromString(
            QStringLiteral("2026-01-01T12:%1:00.000Z").arg(index, 2, 10, QLatin1Char('0')),
            Qt::ISODateWithMs),
        QStringLiteral("INFO"), QStringLiteral("entry-%1").arg(index),
        LogFileInfo(file_path, QStringLiteral("TestApp")));
}

/**
 * @brief Returns the isolated SQLite database path for this test.
 * @return Absolute path below the temporary directory.
 */
auto HistoryWriteServiceTest::get_database_path() const -> QString
{
    return QDir(m_temporary_directory.path()).filePath(QStringLiteral("history.sqlite"));
}

/**
 * @brief Verifies that batches and their completion marker are processed in submission order.
 */
TEST_F(HistoryWriteServiceTest, StoresBatchesBeforeCompletingImport)
{
    const QUuid operation_id = QUuid::createUuid();
    const QUuid view_id = QUuid::createUuid();
    const QString file_path =
        QDir(m_temporary_directory.path()).filePath(QStringLiteral("ordered.log"));
    LogHistoryService history(get_database_path());
    HistoryWriteService writer(get_database_path());
    QSignalSpy finished_spy(&writer, &HistoryWriteService::import_write_finished);

    ASSERT_TRUE(writer.begin_import(operation_id, view_id, file_path));
    ASSERT_TRUE(writer.store_batch(operation_id, view_id, file_path, {create_entry(1, file_path)}));
    ASSERT_TRUE(writer.store_batch(operation_id, view_id, file_path, {create_entry(2, file_path)}));
    ASSERT_TRUE(writer.finish_import(operation_id, view_id, file_path));

    QTRY_COMPARE_WITH_TIMEOUT(finished_spy.count(), 1, 5000);
    ASSERT_EQ(finished_spy.first().size(), 5);
    EXPECT_EQ(finished_spy.first().at(0).toUuid(), operation_id);
    EXPECT_TRUE(finished_spy.first().at(3).toBool());
    EXPECT_TRUE(finished_spy.first().at(4).toString().isEmpty());

    LogQuery query;
    query.view_id = view_id;
    EXPECT_EQ(history.count_entries(query), 2);
}

/**
 * @brief Verifies that an unavailable database is reported as an import failure.
 */
TEST_F(HistoryWriteServiceTest, ReportsStorageFailure)
{
    const QString blocking_file_path =
        QDir(m_temporary_directory.path()).filePath(QStringLiteral("blocking-file"));
    QFile blocking_file(blocking_file_path);
    ASSERT_TRUE(blocking_file.open(QIODevice::WriteOnly));
    blocking_file.close();

    const QString invalid_path =
        QDir(blocking_file_path).filePath(QStringLiteral("history.sqlite"));
    const QUuid operation_id = QUuid::createUuid();
    const QUuid view_id = QUuid::createUuid();
    const QString file_path =
        QDir(m_temporary_directory.path()).filePath(QStringLiteral("failed.log"));
    HistoryWriteService writer(invalid_path);
    QSignalSpy finished_spy(&writer, &HistoryWriteService::import_write_finished);

    ASSERT_TRUE(writer.begin_import(operation_id, view_id, file_path));
    ASSERT_TRUE(writer.store_batch(operation_id, view_id, file_path, {create_entry(1, file_path)}));
    ASSERT_TRUE(writer.finish_import(operation_id, view_id, file_path));

    QTRY_COMPARE_WITH_TIMEOUT(finished_spy.count(), 1, 5000);
    EXPECT_EQ(finished_spy.first().at(0).toUuid(), operation_id);
    EXPECT_FALSE(finished_spy.first().at(3).toBool());
    EXPECT_FALSE(finished_spy.first().at(4).toString().isEmpty());
}

/**
 * @brief Verifies that cleanup queued after a write removes the discarded file's entries.
 */
TEST_F(HistoryWriteServiceTest, DiscardsFileAfterEarlierWrites)
{
    const QUuid operation_id = QUuid::createUuid();
    const QUuid view_id = QUuid::createUuid();
    const QString file_path =
        QDir(m_temporary_directory.path()).filePath(QStringLiteral("discarded.log"));
    LogHistoryService history(get_database_path());
    HistoryWriteService writer(get_database_path());

    ASSERT_TRUE(writer.begin_import(operation_id, view_id, file_path));
    ASSERT_TRUE(writer.store_batch(operation_id, view_id, file_path, {create_entry(1, file_path)}));
    ASSERT_TRUE(writer.discard_file(view_id, file_path));
    EXPECT_FALSE(writer.finish_import(operation_id, view_id, file_path));
    writer.shutdown();

    LogQuery query;
    query.view_id = view_id;
    EXPECT_EQ(history.count_entries(query), 0);
}

/**
 * @brief Verifies that shutdown completes already accepted writes and rejects later work.
 */
TEST_F(HistoryWriteServiceTest, ShutdownDrainsPendingWrites)
{
    const QUuid operation_id = QUuid::createUuid();
    const QUuid view_id = QUuid::createUuid();
    const QString file_path =
        QDir(m_temporary_directory.path()).filePath(QStringLiteral("shutdown.log"));
    HistoryWriteService writer(get_database_path());

    ASSERT_TRUE(writer.begin_import(operation_id, view_id, file_path));
    for (int index = 1; index <= 20; ++index)
    {
        ASSERT_TRUE(
            writer.store_batch(operation_id, view_id, file_path, {create_entry(index, file_path)}));
    }

    writer.shutdown();

    EXPECT_FALSE(writer.is_running());
    EXPECT_FALSE(
        writer.store_batch(operation_id, view_id, file_path, {create_entry(21, file_path)}));

    LogHistoryService history(get_database_path());
    LogQuery query;
    query.view_id = view_id;
    EXPECT_EQ(history.count_entries(query), 20);
}

/**
 * @brief Verifies that cancelling one import does not cancel a later import of the same view.
 */
TEST_F(HistoryWriteServiceTest, ReopenedViewUsesIndependentImportOperation)
{
    const QUuid reopened_operation_id = QUuid::createUuid();
    const QUuid view_id = QUuid::createUuid();
    const QString file_path =
        QDir(m_temporary_directory.path()).filePath(QStringLiteral("reopened.log"));
    HistoryWriteService writer(get_database_path());
    QSignalSpy finished_spy(&writer, &HistoryWriteService::import_write_finished);

    for (int attempt = 1; attempt <= 3; ++attempt)
    {
        const QUuid cancelled_operation_id = QUuid::createUuid();
        ASSERT_TRUE(writer.begin_import(cancelled_operation_id, view_id, file_path));

        for (int batch = 1; batch <= 100; ++batch)
        {
            ASSERT_TRUE(writer.store_batch(cancelled_operation_id, view_id, file_path,
                                           {create_entry(batch, file_path)}));
        }

        ASSERT_TRUE(writer.discard_view(view_id));
        EXPECT_FALSE(writer.store_batch(cancelled_operation_id, view_id, file_path,
                                        {create_entry(attempt, file_path)}));
    }

    ASSERT_TRUE(writer.begin_import(reopened_operation_id, view_id, file_path));
    ASSERT_TRUE(writer.store_batch(reopened_operation_id, view_id, file_path,
                                   {create_entry(3, file_path)}));
    ASSERT_TRUE(writer.finish_import(reopened_operation_id, view_id, file_path));

    QTRY_COMPARE_WITH_TIMEOUT(finished_spy.count(), 1, 5000);
    writer.shutdown();

    LogHistoryService history(get_database_path());
    LogQuery query;
    query.view_id = view_id;
    EXPECT_EQ(history.count_entries(query), 1);
}
