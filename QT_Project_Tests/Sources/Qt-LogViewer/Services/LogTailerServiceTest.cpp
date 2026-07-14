#include "Qt-LogViewer/Services/LogTailerServiceTest.h"

#include <QFile>
#include <QSignalSpy>
#include <QTest>

/**
 * @brief Creates a temporary file system location and tailer instance.
 */
void LogTailerServiceTest::SetUp()
{
    ASSERT_TRUE(m_temporary_directory.isValid());

    m_file_path = m_temporary_directory.filePath(QStringLiteral("tail.log"));

    QFile file(m_file_path);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("2026-01-01 12:00:00 [INFO] initial\n");
    file.close();

    m_tailer_service = new LogTailerService(QStringLiteral("{timestamp} [{level}] {message}"));
    m_tailer_service->set_debounce_interval_ms(10);
    m_view_id = QUuid::createUuid();
}

/**
 * @brief Releases the tailer and temporary resources.
 */
void LogTailerServiceTest::TearDown()
{
    delete m_tailer_service;
    m_tailer_service = nullptr;
}

/**
 * @brief Processes watcher events until a signal spy receives the requested count.
 * @param spy Signal spy collecting entries_available emissions.
 * @param expected_count Required signal count.
 * @return True when the count was observed before timeout.
 */
auto LogTailerServiceTest::wait_for_entries(QSignalSpy& spy, int expected_count) const -> bool
{
    constexpr int timeout_ms = 3000;
    constexpr int poll_interval_ms = 25;

    int elapsed_ms = 0;

    while (spy.count() < expected_count && elapsed_ms < timeout_ms)
    {
        QTest::qWait(poll_interval_ms);
        elapsed_ms += poll_interval_ms;
    }

    const bool received = spy.count() >= expected_count;
    return received;
}

/**
 * @brief Appends raw UTF-8 text to the temporary log file.
 * @param text Text to append.
 */
auto LogTailerServiceTest::append_text(const QString& text) const -> void
{
    QFile file(m_file_path);

    if (file.open(QIODevice::Append | QIODevice::Text))
    {
        file.write(text.toUtf8());
        file.close();
    }
}

/**
 * @brief Verifies appended complete records are emitted.
 */
TEST_F(LogTailerServiceTest, EmitsEntriesForAppendedCompleteLines)
{
    QSignalSpy spy(m_tailer_service, &LogTailerService::entries_available);

    m_tailer_service->start_tailing(m_view_id, m_file_path);
    append_text(QStringLiteral("2026-01-01 12:00:01 [INFO] appended\n"));

    ASSERT_TRUE(wait_for_entries(spy, 1));

    const QList<QVariant> arguments = spy.takeFirst();
    const QVector<LogEntry> entries = qvariant_cast<QVector<LogEntry>>(arguments.at(2));

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.first().get_message(), QStringLiteral("appended"));
}

/**
 * @brief Verifies incomplete lines are buffered until a line terminator arrives.
 */
TEST_F(LogTailerServiceTest, BuffersPartialLineUntilNewline)
{
    QSignalSpy spy(m_tailer_service, &LogTailerService::entries_available);

    m_tailer_service->start_tailing(m_view_id, m_file_path);
    append_text(QStringLiteral("2026-01-01 12:00:01 [INFO] partial"));

    QTest::qWait(250);
    EXPECT_EQ(spy.count(), 0);

    append_text(QStringLiteral(" message\n"));

    ASSERT_TRUE(wait_for_entries(spy, 1));

    const QList<QVariant> arguments = spy.takeFirst();
    const QVector<LogEntry> entries = qvariant_cast<QVector<LogEntry>>(arguments.at(2));

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.first().get_message(), QStringLiteral("partial message"));
}

/**
 * @brief Verifies truncation resets the byte offset and processes replacement content.
 */
TEST_F(LogTailerServiceTest, ReadsNewContentAfterTruncation)
{
    QSignalSpy spy(m_tailer_service, &LogTailerService::entries_available);

    m_tailer_service->start_tailing(m_view_id, m_file_path);

    QFile file(m_file_path);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
    file.write("2026-01-01 12:01:00 [INFO] after truncation\n");
    file.close();

    ASSERT_TRUE(wait_for_entries(spy, 1));

    const QList<QVariant> arguments = spy.takeFirst();
    const QVector<LogEntry> entries = qvariant_cast<QVector<LogEntry>>(arguments.at(2));

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.first().get_message(), QStringLiteral("after truncation"));
}

/**
 * @brief Verifies replacement recovery reads a same-size-or-larger replacement from its beginning.
 */
TEST_F(LogTailerServiceTest, ReadsReplacementFileFromBeginning)
{
    QSignalSpy spy(m_tailer_service, &LogTailerService::entries_available);

    m_tailer_service->start_tailing(m_view_id, m_file_path);

    const QString rotated_path = m_temporary_directory.filePath(QStringLiteral("tail.log.1"));
    ASSERT_TRUE(QFile::rename(m_file_path, rotated_path));

    QFile replacement_file(m_file_path);
    ASSERT_TRUE(replacement_file.open(QIODevice::WriteOnly | QIODevice::Text));
    replacement_file.write("2026-01-01 13:00:00 [INFO] replacement record with more bytes\n");
    replacement_file.close();

    ASSERT_TRUE(wait_for_entries(spy, 1));

    const QList<QVariant> arguments = spy.takeFirst();
    const QVector<LogEntry> entries = qvariant_cast<QVector<LogEntry>>(arguments.at(2));

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.first().get_message(), QStringLiteral("replacement record with more bytes"));
}
