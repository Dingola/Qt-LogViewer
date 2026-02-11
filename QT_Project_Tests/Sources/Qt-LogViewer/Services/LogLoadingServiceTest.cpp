#include "Qt-LogViewer/Services/LogLoadingServiceTest.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryFile>
#include <QTextStream>
#include <QUuid>
#include <QVector>

/**
 * @brief Constructs the test fixture.
 */
LogLoadingServiceTest::LogLoadingServiceTest(): m_service(nullptr) {}

/**
 * @brief Destroys the test fixture.
 */
LogLoadingServiceTest::~LogLoadingServiceTest() = default;

/**
 * @brief Set up before each test: create service with a format matching test lines.
 */
void LogLoadingServiceTest::SetUp()
{
    // Use the same format as in LogLoaderTest
    const QString format =
        QStringLiteral("{timestamp} {level} {message} {app_name} [{file}:{line} ({function})]");
    m_service = new LogLoadingService(format);
}

/**
 * @brief Tear down after each test.
 */
void LogLoadingServiceTest::TearDown()
{
    delete m_service;
    m_service = nullptr;
}

/**
 * @brief Helper: connect to error signal and capture last payload.
 */
auto LogLoadingServiceTest::connect_error(LogLoadingService* service, QString& out_file_path,
                                          QString& out_message) -> QMetaObject::Connection
{
    return QObject::connect(service, &LogLoadingService::error, service,
                            [&](const QString& fp, const QString& msg) {
                                out_file_path = fp;
                                out_message = msg;
                            });
}

/**
 * @brief Returns a unique, non-existent file path in the temp directory (cross-platform).
 *
 * Tries a handful of UUID-based filenames and picks the first non-existing candidate.
 * Falls back to a fixed filename in the temp directory (ensuring it is absent).
 */
auto LogLoadingServiceTest::make_nonexistent_path() const -> QString
{
    QString chosen_path;
    const QString temp_dir = QDir::tempPath();

    for (int i = 0; i < 5; ++i)
    {
        const QString candidate =
            QDir(temp_dir).filePath(QStringLiteral("qt_lv_nonexistent_%1.log")
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
            QDir(temp_dir).filePath(QStringLiteral("qt_lv_nonexistent_fallback.log"));
        if (QFile::exists(fallback))
        {
            QFile::remove(fallback);
        }
        chosen_path = fallback;
    }

    return chosen_path;
}

/**
 * @brief load_log_file on non-existent path returns empty and emits error.
 */
TEST_F(LogLoadingServiceTest, LoadLogFileInvalidPathEmitsErrorAndReturnsEmpty)
{
    ASSERT_NE(m_service, nullptr);

    QString captured_path;
    QString captured_msg;
    auto connection = connect_error(m_service, captured_path, captured_msg);

    const QString path = make_nonexistent_path();
    const auto entries = m_service->load_log_file(path);

    EXPECT_TRUE(entries.isEmpty());
    EXPECT_EQ(captured_path, path);
    EXPECT_FALSE(captured_msg.isEmpty());

    QObject::disconnect(connection);
}

/**
 * @brief read_first_log_entry on non-existent path returns default-constructed entry.
 */
TEST_F(LogLoadingServiceTest, ReadFirstLogEntryInvalidPathReturnsDefault)
{
    ASSERT_NE(m_service, nullptr);

    const QString path = make_nonexistent_path();
    const auto entry = m_service->read_first_log_entry(path);

    EXPECT_TRUE(entry.get_app_name().isEmpty());
    EXPECT_TRUE(entry.get_file_info().get_file_path().isEmpty());
}

/**
 * @brief load_log_file_async on non-existent path emits error then streaming_idle.
 *
 * Note: streaming_idle is emitted synchronously on validation failure, so assert count directly.
 */
TEST_F(LogLoadingServiceTest, LoadLogFileAsyncInvalidPathEmitsErrorThenIdle)
{
    ASSERT_NE(m_service, nullptr);

    QString captured_path;
    QString captured_msg;
    auto connection = connect_error(m_service, captured_path, captured_msg);

    QSignalSpy idle_spy(m_service, &LogLoadingService::streaming_idle);

    const QString path = make_nonexistent_path();
    m_service->load_log_file_async(path, 10);

    // Synchronous emission expected; allow one cycle for safety in different environments
    if (idle_spy.count() == 0)
    {
        QEventLoop loop;
        QTimer::singleShot(0, &loop, &QEventLoop::quit);
        loop.exec();
    }

    EXPECT_GE(idle_spy.count(), 1);
    EXPECT_EQ(captured_path, path);
    EXPECT_FALSE(captured_msg.isEmpty());

    QObject::disconnect(connection);
}

/**
 * @brief set_max_retries/get_max_retries clamp negatives to 0 and reflect set values.
 */
TEST_F(LogLoadingServiceTest, RetryMaxGetSetClampsNegativeToZero)
{
    ASSERT_NE(m_service, nullptr);

    m_service->set_max_retries(-5);
    EXPECT_EQ(m_service->get_max_retries(), 0);

    m_service->set_max_retries(3);
    EXPECT_EQ(m_service->get_max_retries(), 3);
}

/**
 * @brief set_retry_delay_ms/get_retry_delay_ms clamp negatives to 0 and reflect set values.
 */
TEST_F(LogLoadingServiceTest, RetryDelayGetSetClampsNegativeToZero)
{
    ASSERT_NE(m_service, nullptr);

    m_service->set_retry_delay_ms(-100);
    EXPECT_EQ(m_service->get_retry_delay_ms(), 0);

    m_service->set_retry_delay_ms(250);
    EXPECT_EQ(m_service->get_retry_delay_ms(), 250);
}

/**
 * @brief cancel_async is callable and does not crash or deadlock when no stream is active.
 */
TEST_F(LogLoadingServiceTest, CancelAsyncNoActiveStreamDoesNotCrash)
{
    ASSERT_NE(m_service, nullptr);

    m_service->cancel_async();
    SUCCEED();
}

/**
 * @brief Async invalid path: no retries scheduled; only one idle emission expected and no finished.
 */
TEST_F(LogLoadingServiceTest, AsyncInvalidPathEmitsIdleOnceNoRetries)
{
    ASSERT_NE(m_service, nullptr);

    m_service->set_max_retries(5);      // Ignored when validation fails
    m_service->set_retry_delay_ms(10);  // Ignored too

    QSignalSpy idle_spy(m_service, &LogLoadingService::streaming_idle);
    QSignalSpy finished_spy(m_service, &LogLoadingService::finished);

    const QString path = make_nonexistent_path();
    m_service->load_log_file_async(path, 5);

    if (idle_spy.count() == 0)
    {
        QEventLoop loop;
        QTimer::singleShot(0, &loop, &QEventLoop::quit);
        loop.exec();
    }

    EXPECT_EQ(idle_spy.count(), 1);
    EXPECT_EQ(finished_spy.count(), 0);
}

/**
 * @brief Synchronous valid file: returns parsed entries with correct fields.
 */
TEST_F(LogLoadingServiceTest, LoadLogFileValidPathParsesEntries)
{
    ASSERT_NE(m_service, nullptr);

    QTemporaryFile temp_file;
    ASSERT_TRUE(temp_file.open());
    QTextStream out(&temp_file);
    out << "2024-01-01 12:34:56 Info TestMessage MyApp [file.cpp:42 (main)]\n";
    out.flush();
    temp_file.close();

    const auto entries = m_service->load_log_file(temp_file.fileName());
    ASSERT_EQ(entries.size(), 1);

    EXPECT_EQ(entries[0].get_message(), "TestMessage");
    EXPECT_EQ(entries[0].get_level(), "Info");
    EXPECT_EQ(entries[0].get_app_name(), "MyApp");
    EXPECT_EQ(entries[0].get_timestamp(),
              QDateTime::fromString("2024-01-01 12:34:56", "yyyy-MM-dd HH:mm:ss"));
}

/**
 * @brief read_first_log_entry should return the first valid entry from a valid file.
 */
TEST_F(LogLoadingServiceTest, ReadFirstLogEntryValidPathReturnsFirst)
{
    ASSERT_NE(m_service, nullptr);

    QTemporaryFile temp_file;
    ASSERT_TRUE(temp_file.open());
    QTextStream out(&temp_file);
    out << "2024-01-01 12:34:56 Info FirstMessage MyApp [f.cpp:1 (f)]\n";
    out << "2024-01-01 12:35:00 Error SecondMessage MyApp [f.cpp:2 (g)]\n";
    out.flush();
    temp_file.close();

    const auto entry = m_service->read_first_log_entry(temp_file.fileName());
    EXPECT_EQ(entry.get_message(), "FirstMessage");
    EXPECT_EQ(entry.get_level(), "Info");
    EXPECT_EQ(entry.get_app_name(), "MyApp");
    EXPECT_EQ(entry.get_timestamp(),
              QDateTime::fromString("2024-01-01 12:34:56", "yyyy-MM-dd HH:mm:ss"));
}

/**
 * @brief Async valid file: emits finished and then streaming_idle; batches and progress observed;
 * no error.
 */
TEST_F(LogLoadingServiceTest, LoadLogFileAsyncValidEmitsFinishedAndIdle)
{
    ASSERT_NE(m_service, nullptr);

    QTemporaryFile temp_file;
    ASSERT_TRUE(temp_file.open());
    QTextStream out(&temp_file);
    out << "2024-01-01 12:34:56 Info M1 MyApp [a.cpp:1 (a)]\n";
    out << "2024-01-01 12:35:00 Info M2 MyApp [a.cpp:2 (b)]\n";
    out << "2024-01-01 12:35:10 Error M3 MyApp [a.cpp:3 (c)]\n";
    out.flush();
    temp_file.close();

    QSignalSpy finished_spy(m_service, &LogLoadingService::finished);
    QSignalSpy idle_spy(m_service, &LogLoadingService::streaming_idle);
    QSignalSpy error_spy(m_service, &LogLoadingService::error);

    // Count batch and progress emissions with lambdas to avoid metatype issues.
    int batch_count = 0;
    bool saw_progress = false;

    const auto c_batch =
        QObject::connect(m_service, &LogLoadingService::entry_batch_parsed, m_service,
                         [&](const QString& file_path, const QVector<LogEntry>& batch) {
                             Q_UNUSED(file_path);
                             if (!batch.isEmpty())
                             {
                                 batch_count += 1;
                             }
                         });
    const auto c_progress =
        QObject::connect(m_service, &LogLoadingService::progress, m_service,
                         [&](const QString& file_path, qint64 bytes_read, qint64 total_bytes) {
                             Q_UNUSED(file_path);
                             Q_UNUSED(bytes_read);
                             Q_UNUSED(total_bytes);
                             saw_progress = true;
                         });

    m_service->load_log_file_async(temp_file.fileName(), /*batch_size*/ 1);

    // Wait for finished then for streaming_idle
    const bool finished_ok = finished_spy.wait(3000);
    EXPECT_TRUE(finished_ok);

    const bool idle_ok = idle_spy.wait(3000);
    EXPECT_TRUE(idle_ok);

    EXPECT_EQ(error_spy.count(), 0);
    EXPECT_GE(batch_count, 1);
    EXPECT_TRUE(saw_progress);

    QObject::disconnect(c_batch);
    QObject::disconnect(c_progress);
}
