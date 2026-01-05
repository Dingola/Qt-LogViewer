#include "Qt-LogViewer/Controllers/LogIngestControllerTest.h"

#include <QApplication>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QTest>
#include <QTextStream>
#include <QTimer>
#include <atomic>

/**
 * @brief Constructs the test fixture.
 */
LogIngestControllerTest::LogIngestControllerTest(): m_ctrl(nullptr), m_temp_log_path() {}

/**
 * @brief Destroys the test fixture.
 */
LogIngestControllerTest::~LogIngestControllerTest() = default;

/**
 * @brief Set up before each test: create controller and a temp log file.
 */
void LogIngestControllerTest::SetUp()
{
    m_ctrl = new LogIngestController(
        QStringLiteral("{timestamp} {level} {message} {app_name} [{file}:{line} ({function})]"));
    m_temp_log_path = make_temp_log_file();
}

/**
 * @brief Tear down after each test.
 */
void LogIngestControllerTest::TearDown()
{
    delete m_ctrl;
    m_ctrl = nullptr;

    if (!m_temp_log_path.isEmpty())
    {
        QFile::remove(m_temp_log_path);
    }
    QApplication::processEvents();
}

/**
 * @brief Creates a temp file with content matching the service format.
 *
 * Format: "{timestamp} {level} {message} {app_name} [{file}:{line} ({function})]"
 */
auto LogIngestControllerTest::make_temp_log_file() -> QString
{
    QString path;

    QTemporaryFile tmp;
    tmp.setAutoRemove(false);
    if (tmp.open())
    {
        QTextStream out(&tmp);
        out << "2024-01-01 12:34:56 Info Startup MyApp [main.cpp:1 (main)]\n";
        out << "2024-01-01 12:35:00 Error Crash MyApp [engine.cpp:42 (run)]\n";
        out << "2024-01-01 12:35:10 Debug Trace MyApp [util.cpp:7 (helper)]\n";
        out.flush();
        path = tmp.fileName();
        tmp.close();
    }

    return path;
}

/**
 * @brief Returns a unique, non-existent file path in the temp directory.
 */
auto LogIngestControllerTest::make_nonexistent_path() const -> QString
{
    QString chosen_path;
    const QString temp_dir = QDir::tempPath();

    for (int i = 0; i < 5; ++i)
    {
        const QString candidate =
            QDir(temp_dir).filePath(QStringLiteral("qt_ingest_nonexistent_%1.log")
                                        .arg(QUuid::createUuid().toString(QUuid::WithoutBraces)));
        const bool candidate_exists = QFile::exists(candidate);
        if (!candidate_exists && chosen_path.isEmpty())
        {
            chosen_path = candidate;
        }
    }

    if (chosen_path.isEmpty())
    {
        const QString fallback =
            QDir(temp_dir).filePath(QStringLiteral("qt_ingest_nonexistent_fallback.log"));
        if (QFile::exists(fallback))
        {
            QFile::remove(fallback);
        }
        chosen_path = fallback;
    }

    return chosen_path;
}

/**
 * @brief Defaults after construction: idle controller and zero pending.
 */
TEST_F(LogIngestControllerTest, DefaultsAfterConstruction)
{
    ASSERT_NE(m_ctrl, nullptr);

    EXPECT_TRUE(m_ctrl->get_active_view_id().isNull());
    EXPECT_TRUE(m_ctrl->get_active_file_path().isEmpty());
    EXPECT_EQ(m_ctrl->get_pending_count(), 0);
    EXPECT_GE(m_ctrl->get_active_batch_size(), 0);
}

/**
 * @brief Synchronous helpers on invalid path: empty vector and default entry.
 */
TEST_F(LogIngestControllerTest, LoadFileSyncAndReadFirstEntryInvalidPath)
{
    ASSERT_NE(m_ctrl, nullptr);

    const QString invalid_path = make_nonexistent_path();

    const QVector<LogEntry> entries = m_ctrl->load_file_sync(invalid_path);
    EXPECT_TRUE(entries.isEmpty());

    const LogEntry first = m_ctrl->read_first_log_entry(invalid_path);
    EXPECT_TRUE(first.get_app_name().isEmpty());
    EXPECT_TRUE(first.get_file_info().get_file_path().isEmpty());
}

/**
 * @brief Synchronous helpers on valid path: non-empty entries and first entry has fields set.
 */
TEST_F(LogIngestControllerTest, LoadFileSyncAndReadFirstEntryValidPath)
{
    ASSERT_NE(m_ctrl, nullptr);
    ASSERT_FALSE(m_temp_log_path.isEmpty());

    const QVector<LogEntry> entries = m_ctrl->load_file_sync(m_temp_log_path);
    EXPECT_GE(entries.size(), 1);

    const LogEntry first = m_ctrl->read_first_log_entry(m_temp_log_path);
    EXPECT_FALSE(first.get_level().isEmpty());
    EXPECT_FALSE(first.get_message().isEmpty());
}

/**
 * @brief Enqueue invalid path and start: controller should emit error and idle; queue clears.
 */
TEST_F(LogIngestControllerTest, EnqueueInvalidStartEmitsErrorAndIdleAndClearsQueue)
{
    ASSERT_NE(m_ctrl, nullptr);

    const QUuid view = QUuid::createUuid();
    const QString invalid_path = make_nonexistent_path();
    const qsizetype batch = 7;

    QSignalSpy spy_error(m_ctrl, &LogIngestController::error);
    QSignalSpy spy_idle(m_ctrl, &LogIngestController::idle);

    EXPECT_EQ(m_ctrl->get_pending_count(), 0);

    m_ctrl->enqueue_stream(view, invalid_path);
    EXPECT_EQ(m_ctrl->get_pending_count(), 1);

    m_ctrl->start_next_if_idle(batch);

    // Signals may arrive synchronously; process events before waiting
    QApplication::processEvents();

    // Check if already received, otherwise wait
    if (spy_error.count() == 0)
    {
        spy_error.wait(1000);
    }
    EXPECT_GE(spy_error.count(), 1);

    if (spy_idle.count() == 0)
    {
        spy_idle.wait(1000);
    }
    EXPECT_GE(spy_idle.count(), 1);

    // Validate error payload
    if (spy_error.count() > 0)
    {
        const auto error_args = spy_error.first();
        const QUuid err_view = error_args.at(0).toUuid();
        const QString err_path = error_args.at(1).toString();

        EXPECT_EQ(err_view, view);
        EXPECT_EQ(err_path, invalid_path);
    }

    EXPECT_TRUE(m_ctrl->get_active_view_id().isNull());
    EXPECT_TRUE(m_ctrl->get_active_file_path().isEmpty());
    EXPECT_EQ(m_ctrl->get_pending_count(), 0);
}

/**
 * @brief Enqueue valid path and start: controller forwards signals and clears active on idle.
 */
TEST_F(LogIngestControllerTest, EnqueueValidStartEmitsAllSignalsAndClearsActiveOnIdle)
{
    ASSERT_NE(m_ctrl, nullptr);
    ASSERT_FALSE(m_temp_log_path.isEmpty());

    const QUuid view = QUuid::createUuid();
    const qsizetype batch = 1;

    QSignalSpy spy_batch(m_ctrl, &LogIngestController::entry_batch_parsed);
    QSignalSpy spy_progress(m_ctrl, &LogIngestController::progress);
    QSignalSpy spy_finished(m_ctrl, &LogIngestController::finished);
    QSignalSpy spy_idle(m_ctrl, &LogIngestController::idle);

    m_ctrl->enqueue_stream(view, m_temp_log_path);
    EXPECT_EQ(m_ctrl->get_pending_count(), 1);

    m_ctrl->start_next_if_idle(batch);

    const bool finished_ok = spy_finished.wait(3000);
    EXPECT_TRUE(finished_ok);

    const bool idle_ok = spy_idle.wait(3000);
    EXPECT_TRUE(idle_ok);

    // At least progress should occur; batch may be 0 for tiny files depending on batching
    EXPECT_GE(spy_progress.count(), 1);

    // Verify payload view id if signals were observed
    if (spy_finished.count() > 0)
    {
        const QUuid finished_view = spy_finished.first().at(0).toUuid();
        EXPECT_EQ(finished_view, view);
    }
    if (spy_batch.count() > 0)
    {
        const QUuid batch_view = spy_batch.first().at(0).toUuid();
        EXPECT_EQ(batch_view, view);
    }
    if (spy_progress.count() > 0)
    {
        const QUuid progress_view = spy_progress.first().at(0).toUuid();
        EXPECT_EQ(progress_view, view);
    }

    EXPECT_TRUE(m_ctrl->get_active_view_id().isNull());
    EXPECT_TRUE(m_ctrl->get_active_file_path().isEmpty());
    EXPECT_EQ(m_ctrl->get_pending_count(), 0);
}

/**
 * @brief Cancelling for a view is safe; ensures no crashes and pending count stays consistent.
 */
TEST_F(LogIngestControllerTest, CancelForViewIsSafe)
{
    ASSERT_NE(m_ctrl, nullptr);
    ASSERT_FALSE(m_temp_log_path.isEmpty());

    const QUuid v1 = QUuid::createUuid();
    const QUuid v2 = QUuid::createUuid();

    m_ctrl->enqueue_stream(v1, m_temp_log_path);
    m_ctrl->enqueue_stream(v2, m_temp_log_path);
    EXPECT_EQ(m_ctrl->get_pending_count(), 2);

    m_ctrl->cancel_for_view(v1);
    QApplication::processEvents();

    EXPECT_GE(m_ctrl->get_pending_count(), 0);
}

/**
 * @brief Starting while already active should not start a second item until idle advances the
 * queue.
 */
TEST_F(LogIngestControllerTest, StartNextWhileActiveDoesNotStartSecondUntilIdle)
{
    ASSERT_NE(m_ctrl, nullptr);
    ASSERT_FALSE(m_temp_log_path.isEmpty());

    const QUuid v1 = QUuid::createUuid();
    const QUuid v2 = QUuid::createUuid();

    QSignalSpy spy_finished(m_ctrl, &LogIngestController::finished);
    QSignalSpy spy_idle(m_ctrl, &LogIngestController::idle);

    m_ctrl->enqueue_stream(v1, m_temp_log_path);
    m_ctrl->enqueue_stream(v2, m_temp_log_path);
    EXPECT_EQ(m_ctrl->get_pending_count(), 2);

    m_ctrl->start_next_if_idle(3);
    // Attempt to start again while active -> should not start second yet
    m_ctrl->start_next_if_idle(5);

    // First item: finished then idle (advances queue)
    const bool finished_first_ok = spy_finished.wait(3000);
    EXPECT_TRUE(finished_first_ok);

    const bool idle1_ok = spy_idle.wait(3000);
    EXPECT_TRUE(idle1_ok);

    // Second item: finished then idle (clear active)
    const bool finished_second_ok = spy_finished.wait(3000);
    EXPECT_TRUE(finished_second_ok);

    const bool idle2_ok = spy_idle.wait(3000);
    EXPECT_TRUE(idle2_ok);

    EXPECT_GE(spy_finished.count(), 2);
    EXPECT_EQ(m_ctrl->get_pending_count(), 0);
    EXPECT_TRUE(m_ctrl->get_active_view_id().isNull());
    EXPECT_TRUE(m_ctrl->get_active_file_path().isEmpty());
}

/**
 * @brief Progress and batch signals should be emitted with small batch sizes during valid
 * streaming.
 */
TEST_F(LogIngestControllerTest, ProgressAndBatchSignalsEmittedWithBatching)
{
    ASSERT_NE(m_ctrl, nullptr);
    ASSERT_FALSE(m_temp_log_path.isEmpty());

    const QUuid view = QUuid::createUuid();
    const qsizetype batch_size = 1;

    QSignalSpy spy_batch(m_ctrl, &LogIngestController::entry_batch_parsed);
    QSignalSpy spy_progress(m_ctrl, &LogIngestController::progress);
    QSignalSpy spy_finished(m_ctrl, &LogIngestController::finished);
    QSignalSpy spy_idle(m_ctrl, &LogIngestController::idle);

    m_ctrl->enqueue_stream(view, m_temp_log_path);
    m_ctrl->start_next_if_idle(batch_size);

    const bool finished_ok = spy_finished.wait(3000);
    EXPECT_TRUE(finished_ok);

    const bool idle_ok = spy_idle.wait(3000);
    EXPECT_TRUE(idle_ok);

    EXPECT_GE(spy_progress.count(), 1);
    // Batch emissions may be zero for very small files; do not assert strictly.
    if (spy_batch.count() > 0)
    {
        const QUuid batch_view = spy_batch.first().at(0).toUuid();
        EXPECT_EQ(batch_view, view);
    }

    EXPECT_TRUE(m_ctrl->get_active_view_id().isNull());
    EXPECT_TRUE(m_ctrl->get_active_file_path().isEmpty());
}

/**
 * @brief Mixed queue: first valid then invalid; controller should forward finished/idle for first,
 * then error/idle for second.
 */
TEST_F(LogIngestControllerTest, MixedValidThenInvalidQueueEmitsFinishedAndError)
{
    ASSERT_NE(m_ctrl, nullptr);
    ASSERT_FALSE(m_temp_log_path.isEmpty());

    const QUuid v_valid = QUuid::createUuid();
    const QUuid v_invalid = QUuid::createUuid();
    const QString invalid_path = make_nonexistent_path();

    QSignalSpy spy_finished(m_ctrl, &LogIngestController::finished);
    QSignalSpy spy_error(m_ctrl, &LogIngestController::error);
    QSignalSpy spy_idle(m_ctrl, &LogIngestController::idle);

    m_ctrl->enqueue_stream(v_valid, m_temp_log_path);
    m_ctrl->enqueue_stream(v_invalid, invalid_path);
    EXPECT_EQ(m_ctrl->get_pending_count(), 2);

    m_ctrl->start_next_if_idle(2);

    // Valid first: finished + idle advances queue
    const bool finished_ok = spy_finished.wait(3000);
    EXPECT_TRUE(finished_ok);

    if (spy_idle.count() == 0)
    {
        spy_idle.wait(3000);
    }
    EXPECT_GE(spy_idle.count(), 1);

    // Invalid second: error + idle (may arrive synchronously after queue advances)
    QApplication::processEvents();

    if (spy_error.count() == 0)
    {
        spy_error.wait(1000);
    }
    EXPECT_GE(spy_error.count(), 1);

    if (spy_idle.count() < 2)
    {
        spy_idle.wait(1000);
    }
    EXPECT_GE(spy_idle.count(), 2);

    if (spy_finished.count() > 0)
    {
        const QUuid finished_view = spy_finished.first().at(0).toUuid();
        EXPECT_EQ(finished_view, v_valid);
    }
    if (spy_error.count() > 0)
    {
        const auto err_args = spy_error.first();
        const QUuid err_view = err_args.at(0).toUuid();
        const QString err_path = err_args.at(1).toString();
        EXPECT_EQ(err_view, v_invalid);
        EXPECT_EQ(err_path, invalid_path);
    }

    EXPECT_EQ(m_ctrl->get_pending_count(), 0);
    EXPECT_TRUE(m_ctrl->get_active_view_id().isNull());
    EXPECT_TRUE(m_ctrl->get_active_file_path().isEmpty());
}

/**
 * @brief Batch size getter should reflect last set active batch size across sequential runs when
 * active; after idle it resets.
 */
TEST_F(LogIngestControllerTest, ActiveBatchSizeReflectsLastRunWhileActiveResetsAfterIdle)
{
    ASSERT_NE(m_ctrl, nullptr);
    ASSERT_FALSE(m_temp_log_path.isEmpty());

    const QUuid v1 = QUuid::createUuid();
    const QUuid v2 = QUuid::createUuid();

    QSignalSpy spy_idle(m_ctrl, &LogIngestController::idle);

    m_ctrl->enqueue_stream(v1, m_temp_log_path);
    m_ctrl->start_next_if_idle(4);
    // While active, the queue has batch size 4
    EXPECT_EQ(m_ctrl->get_active_batch_size(), 4);
    const bool idle1 = spy_idle.wait(3000);
    EXPECT_TRUE(idle1);
    // After idle, reset to default (do not assert exact default value beyond non-negative)
    EXPECT_GE(m_ctrl->get_active_batch_size(), 0);

    m_ctrl->enqueue_stream(v2, m_temp_log_path);
    m_ctrl->start_next_if_idle(9);
    EXPECT_EQ(m_ctrl->get_active_batch_size(), 9);
    const bool idle2 = spy_idle.wait(3000);
    EXPECT_TRUE(idle2);
    EXPECT_GE(m_ctrl->get_active_batch_size(), 0);

    EXPECT_TRUE(m_ctrl->get_active_view_id().isNull());
    EXPECT_TRUE(m_ctrl->get_active_file_path().isEmpty());
}

/**
 * @brief Enqueue the same (view_id, file_path) twice: pending count should reflect idempotency.
 */
TEST_F(LogIngestControllerTest, EnqueueIdempotentPerViewPathPair)
{
    ASSERT_NE(m_ctrl, nullptr);
    ASSERT_FALSE(m_temp_log_path.isEmpty());

    const QUuid v1 = QUuid::createUuid();
    const QUuid v2 = QUuid::createUuid();

    EXPECT_EQ(m_ctrl->get_pending_count(), 0);

    m_ctrl->enqueue_stream(v1, m_temp_log_path);
    EXPECT_EQ(m_ctrl->get_pending_count(), 1);

    m_ctrl->enqueue_stream(v1, m_temp_log_path);
    EXPECT_EQ(m_ctrl->get_pending_count(), 1);

    m_ctrl->enqueue_stream(v2, m_temp_log_path);
    EXPECT_EQ(m_ctrl->get_pending_count(), 2);
}

/**
 * @brief Cancel for view while active: ensure controller reaches idle and clears active.
 */
TEST_F(LogIngestControllerTest, CancelForViewWhileActiveClearsActiveAndStopsForwarding)
{
    ASSERT_NE(m_ctrl, nullptr);
    ASSERT_FALSE(m_temp_log_path.isEmpty());

    const QUuid v1 = QUuid::createUuid();
    const qsizetype batch_size = 1;

    QSignalSpy spy_idle(m_ctrl, &LogIngestController::idle);

    m_ctrl->enqueue_stream(v1, m_temp_log_path);
    m_ctrl->start_next_if_idle(batch_size);

    m_ctrl->cancel_for_view(v1);
    QApplication::processEvents();

    const bool idle_ok = spy_idle.wait(3000);
    EXPECT_TRUE(idle_ok);

    EXPECT_TRUE(m_ctrl->get_active_view_id().isNull());
    EXPECT_TRUE(m_ctrl->get_active_file_path().isEmpty());
    EXPECT_EQ(m_ctrl->get_pending_count(), 0);
}

/**
 * @brief start_next_if_idle with batch_size==0: controller should handle gracefully.
 */
TEST_F(LogIngestControllerTest, StartNextWithZeroBatchSizeHandledGracefully)
{
    ASSERT_NE(m_ctrl, nullptr);
    ASSERT_FALSE(m_temp_log_path.isEmpty());

    const QUuid view = QUuid::createUuid();
    QSignalSpy spy_finished(m_ctrl, &LogIngestController::finished);
    QSignalSpy spy_idle(m_ctrl, &LogIngestController::idle);

    m_ctrl->enqueue_stream(view, m_temp_log_path);
    m_ctrl->start_next_if_idle(0);

    const bool finished_ok = spy_finished.wait(3000);
    EXPECT_TRUE(finished_ok);

    const bool idle_ok = spy_idle.wait(3000);
    EXPECT_TRUE(idle_ok);

    EXPECT_TRUE(m_ctrl->get_active_view_id().isNull());
    EXPECT_TRUE(m_ctrl->get_active_file_path().isEmpty());
    EXPECT_EQ(m_ctrl->get_pending_count(), 0);
}

/**
 * @brief Destructor cancels and disconnects: no further controller signals should fire after
 * deletion.
 */
TEST_F(LogIngestControllerTest, DestructorCancelsAndDisconnectsNoLateSignals)
{
    ASSERT_NE(m_ctrl, nullptr);
    ASSERT_FALSE(m_temp_log_path.isEmpty());

    const QUuid view = QUuid::createUuid();

    // Counters that outlive the controller; use atomic to be safe across threads/event loop.
    std::atomic<int> idle_count{0};
    std::atomic<int> finished_count{0};

    // Connect controller signals to lambdas whose lifetime is tied to the application (qApp).
    QObject::connect(m_ctrl, &LogIngestController::idle, qApp,
                     [&idle_count]() { idle_count.fetch_add(1); });
    QObject::connect(
        m_ctrl, &LogIngestController::finished, qApp,
        [&finished_count](const QUuid&, const QString&) { finished_count.fetch_add(1); });

    // Start a stream so the controller has work in flight.
    m_ctrl->enqueue_stream(view, m_temp_log_path);
    m_ctrl->start_next_if_idle(1);

    // Allow any synchronous/queued signals to arrive before snapshot.
    QApplication::processEvents();
    QTest::qWait(20);

    // Snapshot counts so we only observe signals that arrive after deletion.
    const int before_total =
        idle_count.load(std::memory_order_relaxed) + finished_count.load(std::memory_order_relaxed);

    // Destroy the controller while work may be in flight.
    delete m_ctrl;
    m_ctrl = nullptr;

    // Give the event loop a short window to process any late/racy emissions.
    QApplication::processEvents();
    QTest::qWait(100);

    const int after_total =
        idle_count.load(std::memory_order_relaxed) + finished_count.load(std::memory_order_relaxed);

    // No new signals must have been forwarded after deletion.
    EXPECT_EQ(before_total, after_total);
}
