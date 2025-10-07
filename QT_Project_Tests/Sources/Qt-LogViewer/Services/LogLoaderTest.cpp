#include "Qt-LogViewer/Services/LogLoaderTest.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QTextStream>

/**
 * @brief Sets up the test fixture for each test.
 */
void LogLoaderTest::SetUp()
{
    // Use a format string matching the test log lines
    QString format = "{timestamp} {level} {message} {app_name} [{file}:{line} ({function})]";
    m_loader = new LogLoader(format);
}

/**
 * @brief Tears down the test fixture after each test.
 */
void LogLoaderTest::TearDown()
{
    delete m_loader;
    m_loader = nullptr;
}

/**
 * @brief Tests that LogLoader can be constructed.
 */
TEST_F(LogLoaderTest, Constructor)
{
    EXPECT_NE(m_loader, nullptr);
}

/**
 * @brief Tests loading a single valid log file.
 */
TEST_F(LogLoaderTest, LoadSingleLogFile)
{
    QTemporaryFile temp_file;
    ASSERT_TRUE(temp_file.open());
    QTextStream out(&temp_file);
    out << "2024-01-01 12:34:56 Info Test message MyApp [file.cpp:42 (main)]\n";
    out.flush();
    temp_file.close();

    QVector<LogEntry> entries = m_loader->load_log_file(temp_file.fileName());
    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries[0].get_timestamp(),
              QDateTime::fromString("2024-01-01 12:34:56", "yyyy-MM-dd HH:mm:ss"));
    EXPECT_EQ(entries[0].get_level(), "Info");
    EXPECT_EQ(entries[0].get_message(), "Test message");
    EXPECT_EQ(entries[0].get_app_name(), "MyApp");
}

/**
 * @brief Tests loading a log file with no valid entries.
 */
TEST_F(LogLoaderTest, LoadInvalidLogFile)
{
    QTemporaryFile temp_file;
    ASSERT_TRUE(temp_file.open());
    QTextStream out(&temp_file);
    out << "not a valid log line\n";
    out.flush();
    temp_file.close();

    QVector<LogEntry> entries = m_loader->load_log_file(temp_file.fileName());
    EXPECT_TRUE(entries.isEmpty());
}

/**
 * @brief Tests loading multiple log files and grouping by app name.
 */
TEST_F(LogLoaderTest, LoadLogsByApp)
{
    QTemporaryFile temp_file1;
    QTemporaryFile temp_file2;
    ASSERT_TRUE(temp_file1.open());
    ASSERT_TRUE(temp_file2.open());
    QTextStream out1(&temp_file1);
    QTextStream out2(&temp_file2);
    out1 << "2024-01-01 12:34:56 Info Message1 AppA [file1.cpp:10 (main)]\n";
    out2 << "2024-01-01 12:35:00 Error Message2 AppB [file2.cpp:20 (foo)]\n";
    out1.flush();
    out2.flush();
    temp_file1.close();
    temp_file2.close();

    QVector<QString> files = {temp_file1.fileName(), temp_file2.fileName()};
    QMap<QString, QVector<LogEntry>> app_logs = m_loader->load_logs_by_app(files);

    ASSERT_EQ(app_logs.size(), 2);
    EXPECT_TRUE(app_logs.contains("AppA"));
    EXPECT_TRUE(app_logs.contains("AppB"));
    EXPECT_EQ(app_logs["AppA"].size(), 1);
    EXPECT_EQ(app_logs["AppB"].size(), 1);
    EXPECT_EQ(app_logs["AppA"][0].get_message(), "Message1");
    EXPECT_EQ(app_logs["AppB"][0].get_message(), "Message2");
}

/**
 * @brief Tests that load_logs_by_app uses identify_app when no entries are found.
 */
TEST_F(LogLoaderTest, LoadLogsByApp_UsesIdentifyAppWhenNoEntries)
{
    QTemporaryFile temp_file;
    temp_file.setFileTemplate(QDir::tempPath() + "/FakeAppXXXXXX.log");
    ASSERT_TRUE(temp_file.open());
    QTextStream out(&temp_file);
    out << "not a valid log line\n";
    out.flush();
    temp_file.close();

    QVector<QString> files = {temp_file.fileName()};
    QMap<QString, QVector<LogEntry>> app_logs = m_loader->load_logs_by_app(files);

    QString expected_app_name = QFileInfo(temp_file.fileName()).baseName();
    ASSERT_EQ(app_logs.size(), 1);
    EXPECT_TRUE(app_logs.contains(expected_app_name));
    EXPECT_TRUE(app_logs[expected_app_name].isEmpty());
}

/**
 * @brief Tests that load_logs_by_app does not add entries when app_name is empty.
 */
TEST_F(LogLoaderTest, LoadLogsByApp_AppNameIsEmpty)
{
    QTemporaryFile temp_file;
    temp_file.setFileTemplate(QDir::tempPath() + "/.log");
    ASSERT_TRUE(temp_file.open());
    QTextStream out(&temp_file);
    out << "not a valid log line\n";
    out.flush();
    temp_file.close();

    QVector<QString> files = {temp_file.fileName()};
    QMap<QString, QVector<LogEntry>> app_logs = m_loader->load_logs_by_app(files);

    EXPECT_TRUE(app_logs.isEmpty());
}

/**
 * @brief Tests that loading a non-existent file returns an empty vector.
 */
TEST_F(LogLoaderTest, LoadNonExistentFile)
{
    QVector<LogEntry> entries = m_loader->load_log_file("nonexistent_file.log");
    EXPECT_TRUE(entries.isEmpty());
}

/**
 * @brief Tests the identify_app static method.
 */
TEST_F(LogLoaderTest, IdentifyAppFromFilePath)
{
    QString file_path = "/tmp/MyApp.log";
    QString app_name = LogLoader::identify_app(file_path);
    EXPECT_EQ(app_name, "MyApp");
}

/**
 * @brief Tests that read_first_log_entry returns the first valid log entry from a file with
 * multiple valid lines.
 */
TEST_F(LogLoaderTest, ReadFirstLogEntry_ReturnsFirstValidEntry)
{
    QTemporaryFile temp_file;
    ASSERT_TRUE(temp_file.open());
    QTextStream out(&temp_file);
    out << "2024-01-01 12:34:56 Info FirstMessage MyApp [file.cpp:42 (main)]\n";
    out << "2024-01-01 12:35:00 Error SecondMessage MyApp [file.cpp:43 (main)]\n";
    out.flush();
    temp_file.close();

    LogEntry entry = m_loader->read_first_log_entry(temp_file.fileName());
    EXPECT_EQ(entry.get_message(), "FirstMessage");
    EXPECT_EQ(entry.get_level(), "Info");
    EXPECT_EQ(entry.get_app_name(), "MyApp");
    EXPECT_EQ(entry.get_timestamp(),
              QDateTime::fromString("2024-01-01 12:34:56", "yyyy-MM-dd HH:mm:ss"));
}

/**
 * @brief Tests that read_first_log_entry skips invalid lines and returns the first valid log entry.
 */
TEST_F(LogLoaderTest, ReadFirstLogEntry_SkipsInvalidLines)
{
    QTemporaryFile temp_file;
    ASSERT_TRUE(temp_file.open());
    QTextStream out(&temp_file);
    out << "invalid log line\n";
    out << "2024-01-01 12:34:56 Info ValidMessage MyApp [file.cpp:42 (main)]\n";
    out.flush();
    temp_file.close();

    LogEntry entry = m_loader->read_first_log_entry(temp_file.fileName());
    EXPECT_EQ(entry.get_message(), "ValidMessage");
    EXPECT_EQ(entry.get_level(), "Info");
    EXPECT_EQ(entry.get_app_name(), "MyApp");
    EXPECT_EQ(entry.get_timestamp(),
              QDateTime::fromString("2024-01-01 12:34:56", "yyyy-MM-dd HH:mm:ss"));
}

/**
 * @brief Tests that read_first_log_entry returns a default LogEntry if no valid lines are found.
 */
TEST_F(LogLoaderTest, ReadFirstLogEntry_NoValidLines)
{
    QTemporaryFile temp_file;
    ASSERT_TRUE(temp_file.open());
    QTextStream out(&temp_file);
    out << "invalid log line\n";
    out << "another invalid line\n";
    out.flush();
    temp_file.close();

    LogEntry entry = m_loader->read_first_log_entry(temp_file.fileName());
    EXPECT_TRUE(entry.get_app_name().isEmpty());
    EXPECT_TRUE(entry.get_message().isEmpty());
    EXPECT_TRUE(entry.get_level().isEmpty());
    EXPECT_FALSE(entry.get_timestamp().isValid());
}

/**
 * @brief Tests that read_first_log_entry returns a default LogEntry if the file does not exist.
 */
TEST_F(LogLoaderTest, ReadFirstLogEntry_FileDoesNotExist)
{
    LogEntry entry = m_loader->read_first_log_entry("nonexistent_file.log");
    EXPECT_TRUE(entry.get_app_name().isEmpty());
    EXPECT_TRUE(entry.get_message().isEmpty());
    EXPECT_TRUE(entry.get_level().isEmpty());
    EXPECT_FALSE(entry.get_timestamp().isValid());
}
