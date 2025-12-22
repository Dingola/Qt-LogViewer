#include "Qt-LogViewer/Controllers/LogViewLoadQueueTest.h"

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QUuid>

/**
 * @brief Constructs the test fixture.
 */
LogViewLoadQueueTest::LogViewLoadQueueTest()
    : m_queue(), m_loader(nullptr), m_view_a(), m_view_b(), m_view_c()
{}

/**
 * @brief Destroys the test fixture.
 */
LogViewLoadQueueTest::~LogViewLoadQueueTest() = default;

/**
 * @brief Set up before each test: create a loader service and random view ids.
 */
void LogViewLoadQueueTest::SetUp()
{
    const QString format =
        QStringLiteral("{timestamp} {level} {message} {app_name} [{file}:{line} ({function})]");
    m_loader = new LogLoadingService(format);

    m_view_a = QUuid::createUuid();
    m_view_b = QUuid::createUuid();
    m_view_c = QUuid::createUuid();
}

/**
 * @brief Tear down after each test.
 */
void LogViewLoadQueueTest::TearDown()
{
    delete m_loader;
    m_loader = nullptr;
}

/**
 * @brief Returns a unique, non-existent file path in the temp directory (cross-platform).
 */
auto LogViewLoadQueueTest::make_nonexistent_path() const -> QString
{
    QString chosen_path;
    const QString temp_dir = QDir::tempPath();

    for (int i = 0; i < 5; ++i)
    {
        const QString candidate =
            QDir(temp_dir).filePath(QStringLiteral("qt_lvq_nonexistent_%1.log")
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
            QDir(temp_dir).filePath(QStringLiteral("qt_lvq_nonexistent_fallback.log"));
        if (QFile::exists(fallback))
        {
            QFile::remove(fallback);
        }
        chosen_path = fallback;
    }

    return chosen_path;
}

/**
 * @brief Initial state: idle with defaults.
 */
TEST_F(LogViewLoadQueueTest, InitialStateIsIdleAndDefaults)
{
    EXPECT_TRUE(m_queue.get_active_view_id().isNull());
    EXPECT_TRUE(m_queue.get_active_file_path().isEmpty());
    EXPECT_EQ(m_queue.get_pending_count(), 0);
    EXPECT_EQ(m_queue.get_active_batch_size(), 1000);
}

/**
 * @brief enqueue adds items and skips duplicates (pending and active).
 */
TEST_F(LogViewLoadQueueTest, EnqueueAddsAndSkipsDuplicates)
{
    const QString path1 = make_nonexistent_path();
    const QString path2 = make_nonexistent_path();

    EXPECT_EQ(m_queue.get_pending_count(), 0);

    m_queue.enqueue(m_view_a, path1);
    EXPECT_EQ(m_queue.get_pending_count(), 1);

    // Duplicate pending pair -> skipped
    m_queue.enqueue(m_view_a, path1);
    EXPECT_EQ(m_queue.get_pending_count(), 1);

    // Different path same view -> allowed
    m_queue.enqueue(m_view_a, path2);
    EXPECT_EQ(m_queue.get_pending_count(), 2);

    // Same path different view -> allowed
    m_queue.enqueue(m_view_b, path1);
    EXPECT_EQ(m_queue.get_pending_count(), 3);

    // Start first to make it active, then attempt to enqueue same active pair -> skipped
    const bool started = m_queue.try_start_next(m_loader, 50);
    EXPECT_TRUE(started);
    EXPECT_EQ(m_queue.get_active_view_id(), m_view_a);
    EXPECT_EQ(m_queue.get_active_file_path(), path1);
    EXPECT_EQ(m_queue.get_active_batch_size(), 50);
    EXPECT_EQ(m_queue.get_pending_count(), 2);

    m_queue.enqueue(m_view_a, path1);
    EXPECT_EQ(m_queue.get_pending_count(), 2);
}

/**
 * @brief try_start_next requires non-null loader and idle state.
 */
TEST_F(LogViewLoadQueueTest, TryStartNextRequiresLoaderAndIdle)
{
    const QString path1 = make_nonexistent_path();
    const QString path2 = make_nonexistent_path();

    m_queue.enqueue(m_view_a, path1);
    m_queue.enqueue(m_view_b, path2);

    // Without loader -> no start
    const bool started_null = m_queue.try_start_next(nullptr, 25);
    EXPECT_FALSE(started_null);
    EXPECT_EQ(m_queue.get_pending_count(), 2);
    EXPECT_TRUE(m_queue.get_active_view_id().isNull());

    // Start once with loader
    const bool started_once = m_queue.try_start_next(m_loader, 25);
    EXPECT_TRUE(started_once);
    EXPECT_EQ(m_queue.get_pending_count(), 1);

    // Attempt to start again while active -> no start, pending unchanged
    const bool started_twice = m_queue.try_start_next(m_loader, 25);
    EXPECT_FALSE(started_twice);
    EXPECT_EQ(m_queue.get_pending_count(), 1);
}

/**
 * @brief FIFO order preserved across multiple starts when clearing active between runs.
 */
TEST_F(LogViewLoadQueueTest, FifoOrderPreserved)
{
    const QString p1 = make_nonexistent_path();
    const QString p2 = make_nonexistent_path();
    const QString p3 = make_nonexistent_path();

    m_queue.enqueue(m_view_a, p1);
    m_queue.enqueue(m_view_b, p2);
    m_queue.enqueue(m_view_c, p3);

    EXPECT_EQ(m_queue.get_pending_count(), 3);

    bool started = m_queue.try_start_next(m_loader, 10);
    EXPECT_TRUE(started);
    EXPECT_EQ(m_queue.get_active_file_path(), p1);
    EXPECT_EQ(m_queue.get_pending_count(), 2);

    m_queue.clear_active();

    started = m_queue.try_start_next(m_loader, 10);
    EXPECT_TRUE(started);
    EXPECT_EQ(m_queue.get_active_file_path(), p2);
    EXPECT_EQ(m_queue.get_pending_count(), 1);

    m_queue.clear_active();

    started = m_queue.try_start_next(m_loader, 10);
    EXPECT_TRUE(started);
    EXPECT_EQ(m_queue.get_active_file_path(), p3);
    EXPECT_EQ(m_queue.get_pending_count(), 0);
}

/**
 * @brief clear_pending_for_view removes only that view's items.
 */
TEST_F(LogViewLoadQueueTest, ClearPendingForViewRemovesOnlyTargets)
{
    const QString pa1 = make_nonexistent_path();
    const QString pa2 = make_nonexistent_path();
    const QString pb1 = make_nonexistent_path();

    m_queue.enqueue(m_view_a, pa1);
    m_queue.enqueue(m_view_a, pa2);
    m_queue.enqueue(m_view_b, pb1);

    EXPECT_EQ(m_queue.get_pending_count(), 3);

    m_queue.clear_pending_for_view(m_view_a);
    EXPECT_EQ(m_queue.get_pending_count(), 1);

    m_queue.clear_pending_for_view(m_view_b);
    EXPECT_EQ(m_queue.get_pending_count(), 0);
}

/**
 * @brief cancel_if_active clears active and pending for that view and resets batch size (with
 * loader).
 */
TEST_F(LogViewLoadQueueTest, CancelIfActiveClearsActiveAndPendingWithLoader)
{
    const QString pa1 = make_nonexistent_path();
    const QString pa2 = make_nonexistent_path();
    const QString pb1 = make_nonexistent_path();

    m_queue.enqueue(m_view_a, pa1);
    m_queue.enqueue(m_view_a, pa2);
    m_queue.enqueue(m_view_b, pb1);

    const bool started = m_queue.try_start_next(m_loader, 77);
    EXPECT_TRUE(started);
    EXPECT_EQ(m_queue.get_active_view_id(), m_view_a);
    EXPECT_EQ(m_queue.get_active_file_path(), pa1);
    EXPECT_EQ(m_queue.get_active_batch_size(), 77);
    EXPECT_EQ(m_queue.get_pending_count(), 2);

    m_queue.cancel_if_active(m_loader, m_view_a);

    EXPECT_TRUE(m_queue.get_active_view_id().isNull());
    EXPECT_TRUE(m_queue.get_active_file_path().isEmpty());
    EXPECT_EQ(m_queue.get_active_batch_size(), 1000);
    EXPECT_EQ(m_queue.get_pending_count(), 1);
}

/**
 * @brief cancel_if_active with nullptr loader does not clear active but does clear pending for that
 * view.
 */
TEST_F(LogViewLoadQueueTest, CancelIfActiveWithNullLoaderKeepsActiveButClearsPendings)
{
    const QString pa1 = make_nonexistent_path();
    const QString pa2 = make_nonexistent_path();
    const QString pb1 = make_nonexistent_path();

    m_queue.enqueue(m_view_a, pa1);
    m_queue.enqueue(m_view_a, pa2);
    m_queue.enqueue(m_view_b, pb1);

    const bool started = m_queue.try_start_next(m_loader, 13);
    EXPECT_TRUE(started);
    EXPECT_EQ(m_queue.get_active_view_id(), m_view_a);
    EXPECT_EQ(m_queue.get_pending_count(), 2);

    m_queue.cancel_if_active(nullptr, m_view_a);

    // Active remains because loader was null; pendings for view A removed
    EXPECT_EQ(m_queue.get_active_view_id(), m_view_a);
    EXPECT_EQ(m_queue.get_pending_count(), 1);
}

/**
 * @brief clear_active_if only clears when file paths match; otherwise no-op.
 */
TEST_F(LogViewLoadQueueTest, ClearActiveIfMatchesPathOnly)
{
    const QString pa1 = make_nonexistent_path();
    const QString pb1 = make_nonexistent_path();

    m_queue.enqueue(m_view_a, pa1);
    m_queue.enqueue(m_view_b, pb1);

    const bool started = m_queue.try_start_next(m_loader, 21);
    EXPECT_TRUE(started);
    EXPECT_EQ(m_queue.get_active_file_path(), pa1);
    EXPECT_EQ(m_queue.get_pending_count(), 1);

    // No-op on non-matching path
    m_queue.clear_active_if(pb1);
    EXPECT_EQ(m_queue.get_active_file_path(), pa1);

    // Match clears
    m_queue.clear_active_if(pa1);
    EXPECT_TRUE(m_queue.get_active_view_id().isNull());
    EXPECT_TRUE(m_queue.get_active_file_path().isEmpty());
    EXPECT_EQ(m_queue.get_active_batch_size(), 1000);
}

/**
 * @brief clear_active always resets to idle and default batch size.
 */
TEST_F(LogViewLoadQueueTest, ClearActiveAlwaysResetsToIdle)
{
    const QString p1 = make_nonexistent_path();

    m_queue.enqueue(m_view_a, p1);
    const bool started = m_queue.try_start_next(m_loader, 5);
    EXPECT_TRUE(started);

    EXPECT_FALSE(m_queue.get_active_view_id().isNull());
    EXPECT_FALSE(m_queue.get_active_file_path().isEmpty());
    EXPECT_EQ(m_queue.get_active_batch_size(), 5);

    m_queue.clear_active();

    EXPECT_TRUE(m_queue.get_active_view_id().isNull());
    EXPECT_TRUE(m_queue.get_active_file_path().isEmpty());
    EXPECT_EQ(m_queue.get_active_batch_size(), 1000);
}

/**
 * @brief Accessors reflect active vs idle state correctly during lifecycle.
 */
TEST_F(LogViewLoadQueueTest, AccessorsReflectState)
{
    const QString p1 = make_nonexistent_path();
    const QString p2 = make_nonexistent_path();

    EXPECT_TRUE(m_queue.get_active_view_id().isNull());
    EXPECT_TRUE(m_queue.get_active_file_path().isEmpty());
    EXPECT_EQ(m_queue.get_pending_count(), 0);

    m_queue.enqueue(m_view_a, p1);
    m_queue.enqueue(m_view_b, p2);
    EXPECT_EQ(m_queue.get_pending_count(), 2);

    const bool started = m_queue.try_start_next(m_loader, 64);
    EXPECT_TRUE(started);
    EXPECT_EQ(m_queue.get_active_view_id(), m_view_a);
    EXPECT_EQ(m_queue.get_active_file_path(), p1);
    EXPECT_EQ(m_queue.get_active_batch_size(), 64);
    EXPECT_EQ(m_queue.get_pending_count(), 1);

    m_queue.clear_active();
    EXPECT_TRUE(m_queue.get_active_view_id().isNull());
    EXPECT_TRUE(m_queue.get_active_file_path().isEmpty());
    EXPECT_EQ(m_queue.get_active_batch_size(), 1000);
    EXPECT_EQ(m_queue.get_pending_count(), 1);
}
