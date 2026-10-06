#include "Qt-LogViewer/Services/LogStreamWorkerTest.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTest>
#include <QTextStream>
#include <QThread>

#include "Qt-LogViewer/Services/LogParser.h"
#include "Qt-LogViewer/Services/LogStreamWorker.h"

/**
 * @brief Sets up the test fixture for each test.
 *
 * Initializes the LogStreamWorker with a simple format for easy test data creation.
 */
void LogStreamWorkerTest::SetUp()
{
    // Simple format: level message app_name
    // - message can contain spaces (greedy section), app_name has no spaces by design.
    m_format = "{level} {message} {app_name}";
    m_worker = new LogStreamWorker(LogParser(LogParsingProfile::create_default(m_format)));
}

/**
 * @brief Tears down the test fixture after each test.
 *
 * Deletes the worker and all created temporary files.
 */
void LogStreamWorkerTest::TearDown()
{
    delete m_worker;
    m_worker = nullptr;

    for (qsizetype i = 0; i < m_temp_files.size(); ++i)
    {
        QTemporaryFile* f = m_temp_files.at(i);
        if (f != nullptr)
        {
            if (f->isOpen())
            {
                f->close();
            }
            delete f;
        }
    }

    m_temp_files.clear();
    m_temp_file_names.clear();
}

/**
 * @brief Helper to create a QTemporaryFile with given log lines.
 * @param lines The log lines to write.
 * @return Pointer to the created QTemporaryFile (ownership transferred).
 */
auto LogStreamWorkerTest::create_temp_file(const QVector<QString>& lines) -> QTemporaryFile*
{
    QTemporaryFile* temp = new QTemporaryFile();
    bool opened = temp->open();

    if (opened)
    {
        QTextStream out(temp);
        for (qsizetype i = 0; i < lines.size(); ++i)
        {
            out << lines.at(i) << '\n';
        }
        out.flush();
        temp->close();
    }

    m_temp_files.append(temp);
    m_temp_file_names.append(temp->fileName());

    return temp;
}

/**
 * @test Verifies streaming emits batches, progress (including final), and finished, and that
 *       all valid lines are parsed and delivered.
 */
TEST_F(LogStreamWorkerTest, StreamsBatchesAndReportsProgress)
{
    // Generate 2500 valid lines (3 batches with batch_size=1000).
    QVector<QString> lines;
    lines.reserve(2500);
    for (int i = 0; i < 2500; ++i)
    {
        lines.append(QStringLiteral("Info message_%1 AppX").arg(i));
    }

    QTemporaryFile* file = create_temp_file(lines);

    // Observed values
    int batch_count = 0;
    int total_entries = 0;
    bool got_initial_progress = false;
    qint64 last_bytes_read = -1;
    qint64 total_bytes = -1;
    bool finished_called = false;

    QObject::connect(
        m_worker, &LogStreamWorker::entry_batch_parsed, m_worker,
        [&total_entries, &batch_count](const QString&, const QVector<LogEntry>& batch) {
            total_entries += batch.size();
            batch_count += 1;
        });

    QObject::connect(m_worker, &LogStreamWorker::progress, m_worker,
                     [&got_initial_progress, &last_bytes_read, &total_bytes](
                         const QString&, qint64 read, qint64 total) {
                         if (!got_initial_progress)
                         {
                             got_initial_progress = (read == 0);
                         }
                         last_bytes_read = read;
                         total_bytes = total;
                     });

    QObject::connect(m_worker, &LogStreamWorker::finished, m_worker,
                     [&finished_called](const QString&) { finished_called = true; });

    // Run synchronously in the current thread.
    m_worker->start(file->fileName(), 1000);

    // Expectations
    EXPECT_TRUE(finished_called);
    EXPECT_TRUE(got_initial_progress);
    EXPECT_GE(total_bytes, 0);
    EXPECT_EQ(last_bytes_read, total_bytes);  // final progress should equal file size
    EXPECT_EQ(total_entries, lines.size());
    EXPECT_EQ(batch_count, 3);
}

/** @test Verifies bounded streaming parses only the appended range with physical line numbers. */
TEST_F(LogStreamWorkerTest, StreamsOnlyBoundedSuffixRange)
{
    const QVector<QString> lines{QStringLiteral("Info first AppX"),
                                 QStringLiteral("Info second AppX"),
                                 QStringLiteral("Error third AppX")};
    QTemporaryFile* file = create_temp_file(lines);
    const QByteArray prefix = QByteArrayLiteral("Info first AppX\nInfo second AppX\n");
    QVector<LogEntry> entries;
    qint64 initial_progress = -1;
    qint64 final_progress = -1;
    qint64 total_bytes = -1;

    QObject::connect(
        m_worker, &LogStreamWorker::entry_batch_parsed, m_worker,
        [&entries](const QString&, const QVector<LogEntry>& batch) { entries.append(batch); });
    QObject::connect(m_worker, &LogStreamWorker::progress, m_worker,
                     [&initial_progress, &final_progress, &total_bytes](const QString&, qint64 read,
                                                                        qint64 total) {
                         if (initial_progress < 0)
                         {
                             initial_progress = read;
                         }
                         final_progress = read;
                         total_bytes = total;
                     });

    m_worker->start(file->fileName(), 10, prefix.size(), QFileInfo(file->fileName()).size());

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.constFirst().get_message(), QStringLiteral("third"));
    EXPECT_EQ(entries.constFirst().get_source_line(), 3);
    EXPECT_EQ(entries.constFirst().get_byte_offset(), prefix.size());
    EXPECT_EQ(initial_progress, prefix.size());
    EXPECT_EQ(final_progress, total_bytes);
}

/**
 * @test Verifies that an error signal is emitted when a file cannot be opened, and that
 *       finished is still emitted afterward. No progress is expected in this path.
 */
TEST_F(LogStreamWorkerTest, EmitsErrorOnOpenFail)
{
    // Use an obviously non-existent path in temp directory.
    QString missing_path = QDir::tempPath() + "/qt_logviewer_missing_file_1234567890.log";
    if (QFile::exists(missing_path))
    {
        bool removed = QFile::remove(missing_path);
        (void)removed;
    }

    bool error_called = false;
    bool finished_called = false;
    bool progress_called = false;

    QObject::connect(m_worker, &LogStreamWorker::error, m_worker,
                     [&error_called](const QString&, const QString&) { error_called = true; });
    QObject::connect(
        m_worker, &LogStreamWorker::progress, m_worker,
        [&progress_called](const QString&, qint64, qint64) { progress_called = true; });
    QObject::connect(m_worker, &LogStreamWorker::finished, m_worker,
                     [&finished_called](const QString&) { finished_called = true; });

    m_worker->start(missing_path, 500);

    EXPECT_TRUE(error_called);
    EXPECT_TRUE(finished_called);
    EXPECT_FALSE(progress_called);  // open failed path does not emit progress
}

/**
 * @test Verifies that cancellation is honored and stops processing.
 *       Cancels before the loop begins to deterministically ensure zero entries consumed.
 */
TEST_F(LogStreamWorkerTest, CancelHonorsRequest)
{
    // Large file to ensure that if cancel is ignored we'd see many entries.
    QVector<QString> lines;
    lines.reserve(10000);
    for (int i = 0; i < 10000; ++i)
    {
        lines.append(QStringLiteral("Debug payload_%1 AppY").arg(i));
    }

    QTemporaryFile* file = create_temp_file(lines);

    // Create a dedicated worker moved to a background QThread for this test.
    auto* worker = new LogStreamWorker(LogParser(LogParsingProfile::create_default(m_format)));
    QThread thread;

    int total_entries = 0;
    bool finished_called = false;

    QObject::connect(worker, &LogStreamWorker::entry_batch_parsed, worker,
                     [&total_entries](const QString&, const QVector<LogEntry>& batch) {
                         total_entries += batch.size();
                     });

    // Ensure thread quits when the worker signals finished (queued across threads).
    QObject::connect(worker, &LogStreamWorker::finished, &thread, &QThread::quit,
                     Qt::QueuedConnection);
    QObject::connect(worker, &LogStreamWorker::finished, worker,
                     [&finished_called](const QString&) { finished_called = true; });

    // Ensure safe deletion in the worker's thread once it has finished.
    QObject::connect(&thread, &QThread::finished, worker, &QObject::deleteLater);

    worker->moveToThread(&thread);

    // Pre-cancel to guarantee early exit in the loop.
    worker->cancel();

    QObject::connect(
        &thread, &QThread::started, worker,
        [worker, file]() { worker->start(file->fileName(), 200); }, Qt::QueuedConnection);

    thread.start();

    // Wait for the thread to finish; enforce shutdown if necessary to avoid leaks.
    bool joined = thread.wait(5000);
    if (!joined)
    {
        thread.requestInterruption();
        thread.quit();
        (void)thread.wait(5000);
    }

    EXPECT_TRUE(finished_called);
    EXPECT_EQ(total_entries, 0);
    EXPECT_FALSE(thread.isRunning());
}

/**
 * @test Verifies that cancel() during streaming stops further processing promptly.
 *       Cancels after the first emitted batch, ensuring we do not consume the entire file.
 */
TEST_F(LogStreamWorkerTest, CancelWhileStreamingStopsSoon)
{
    // Create a sufficiently large file to ensure streaming is ongoing when we cancel.
    QVector<QString> lines;
    lines.reserve(5000);
    for (int i = 0; i < 5000; ++i)
    {
        lines.append(QStringLiteral("Info midstream_%1 AppZ").arg(i));
    }

    QTemporaryFile* file = create_temp_file(lines);

    // Dedicated worker/thread for this test.
    auto* worker = new LogStreamWorker(LogParser(LogParsingProfile::create_default(m_format)));
    QThread thread;

    int total_entries = 0;
    int batch_count = 0;
    bool finished_called = false;
    bool cancel_requested = false;

    // Count received entries/batches (executed in worker thread).
    QObject::connect(
        worker, &LogStreamWorker::entry_batch_parsed, worker,
        [&total_entries, &batch_count](const QString&, const QVector<LogEntry>& batch) {
            total_entries += batch.size();
            batch_count += 1;
        },
        Qt::DirectConnection);

    // Trigger cancel from within the worker thread after the first batch arrives.
    QObject::connect(
        worker, &LogStreamWorker::entry_batch_parsed, worker,
        [worker, &cancel_requested](const QString&, const QVector<LogEntry>&) {
            if (!cancel_requested)
            {
                cancel_requested = true;
                worker->cancel();  // atomic; safe to call here
            }
        },
        Qt::DirectConnection);

    // Shutdown choreography and bookkeeping.
    QObject::connect(worker, &LogStreamWorker::finished, &thread, &QThread::quit,
                     Qt::QueuedConnection);
    QObject::connect(worker, &LogStreamWorker::finished, worker,
                     [&finished_called](const QString&) { finished_called = true; });
    QObject::connect(&thread, &QThread::finished, worker, &QObject::deleteLater);

    worker->moveToThread(&thread);
    QObject::connect(
        &thread, &QThread::started, worker,
        [worker, file]() { worker->start(file->fileName(), 100); }, Qt::QueuedConnection);

    thread.start();

    // Wait for clean thread termination; enforce shutdown if necessary.
    bool joined = thread.wait(5000);
    if (!joined)
    {
        thread.requestInterruption();
        thread.quit();
        (void)thread.wait(5000);
    }

    // Assertions: we cancelled during streaming, so not all entries should be consumed.
    EXPECT_TRUE(finished_called);
    EXPECT_TRUE(cancel_requested);
    EXPECT_GE(batch_count, 1);
    EXPECT_LT(total_entries, lines.size());
    EXPECT_FALSE(thread.isRunning());
}

/** @test Verifies parsing pauses when the downstream writer backlog reaches its bound. */
TEST_F(LogStreamWorkerTest, WaitsForWriterCapacityBeforeEmittingMoreBatches)
{
    QVector<QString> lines;
    const int total_batch_count = static_cast<int>(LogStreamWorker::maximum_in_flight_batches) + 2;
    for (int index = 0; index < total_batch_count; ++index)
    {
        lines.append(QStringLiteral("Info bounded_%1 AppB").arg(index));
    }

    QTemporaryFile* file = create_temp_file(lines);
    auto* worker = new LogStreamWorker(LogParser(LogParsingProfile::create_default(m_format)));
    QThread thread;
    QSignalSpy batch_spy(worker, &LogStreamWorker::entry_batch_parsed);
    QSignalSpy finished_spy(worker, &LogStreamWorker::finished);

    QObject::connect(worker, &LogStreamWorker::finished, &thread, &QThread::quit,
                     Qt::QueuedConnection);
    QObject::connect(&thread, &QThread::finished, worker, &QObject::deleteLater);
    worker->moveToThread(&thread);
    QObject::connect(
        &thread, &QThread::started, worker,
        [worker, file]() { worker->start(file->fileName(), 1); }, Qt::QueuedConnection);

    thread.start();

    QTRY_COMPARE_WITH_TIMEOUT(batch_spy.count(),
                              static_cast<int>(LogStreamWorker::maximum_in_flight_batches), 5000);
    QTest::qWait(50);
    EXPECT_EQ(batch_spy.count(), LogStreamWorker::maximum_in_flight_batches);
    EXPECT_EQ(finished_spy.count(), 0);

    worker->acknowledge_batches(2);

    QTRY_COMPARE_WITH_TIMEOUT(finished_spy.count(), 1, 5000);
    EXPECT_EQ(batch_spy.count(), total_batch_count);
    QTRY_VERIFY_WITH_TIMEOUT(!thread.isRunning(), 5000);
}

/** @test Verifies cancellation wakes a reader blocked by the downstream backlog limit. */
TEST_F(LogStreamWorkerTest, CancellationReleasesWriterCapacityWait)
{
    QVector<QString> lines;
    const int total_batch_count = static_cast<int>(LogStreamWorker::maximum_in_flight_batches) + 2;
    for (int index = 0; index < total_batch_count; ++index)
    {
        lines.append(QStringLiteral("Info cancel_wait_%1 AppC").arg(index));
    }

    QTemporaryFile* file = create_temp_file(lines);
    auto* worker = new LogStreamWorker(LogParser(LogParsingProfile::create_default(m_format)));
    QThread thread;
    QSignalSpy batch_spy(worker, &LogStreamWorker::entry_batch_parsed);
    QSignalSpy finished_spy(worker, &LogStreamWorker::finished);

    QObject::connect(worker, &LogStreamWorker::finished, &thread, &QThread::quit,
                     Qt::QueuedConnection);
    QObject::connect(&thread, &QThread::finished, worker, &QObject::deleteLater);
    worker->moveToThread(&thread);
    QObject::connect(
        &thread, &QThread::started, worker,
        [worker, file]() { worker->start(file->fileName(), 1); }, Qt::QueuedConnection);

    thread.start();

    QTRY_COMPARE_WITH_TIMEOUT(batch_spy.count(),
                              static_cast<int>(LogStreamWorker::maximum_in_flight_batches), 5000);
    worker->cancel();

    QTRY_COMPARE_WITH_TIMEOUT(finished_spy.count(), 1, 5000);
    EXPECT_EQ(batch_spy.count(), LogStreamWorker::maximum_in_flight_batches);
    QTRY_VERIFY_WITH_TIMEOUT(!thread.isRunning(), 5000);
}
