#include "Qt-LogViewer/Controllers/LogViewerControllerTest.h"

#include <QDateTime>
#include <QSet>
#include <QTemporaryFile>
#include <QTextStream>

/**
 * @brief Sets up the test fixture for each test.
 */
void LogViewerControllerTest::SetUp()
{
    // Use a format string matching the test log lines
    QString format = "{timestamp} {level} {message} {app_name}";
    m_controller = new LogViewerController(format);

    // Prepare two temporary log files with different app names and levels
    QTemporaryFile* temp_file1 = new QTemporaryFile();
    QTemporaryFile* temp_file2 = new QTemporaryFile();
    ASSERT_TRUE(temp_file1->open());
    ASSERT_TRUE(temp_file2->open());

    QTextStream out1(temp_file1);
    QTextStream out2(temp_file2);

    out1 << "2024-01-01 10:00:00 INFO Startup AppA\n";
    out1 << "2024-01-01 10:01:00 ERROR Crash AppA\n";
    out1.flush();

    out2 << "2024-01-01 10:02:00 DEBUG Debugging AppB\n";
    out2 << "2024-01-01 10:03:00 INFO UserLogin AppB\n";
    out2.flush();

    temp_file1->close();
    temp_file2->close();

    QVector<QString> files = {temp_file1->fileName(), temp_file2->fileName()};
    m_controller->load_logs(files);

    // Clean up temp files after test
    delete temp_file1;
    delete temp_file2;
}

/**
 * @brief Tears down the test fixture after each test.
 */
void LogViewerControllerTest::TearDown()
{
    delete m_controller;
    m_controller = nullptr;
}

/**
 * @brief Tests that the controller loads all log entries.
 */
TEST_F(LogViewerControllerTest, LoadsAllLogEntries)
{
    auto* proxy = m_controller->get_sort_filter_proxy();
    EXPECT_EQ(proxy->rowCount(), 4);
}

/**
 * @brief Tests filtering by application name.
 */
TEST_F(LogViewerControllerTest, FilterByAppName)
{
    auto* proxy = m_controller->get_sort_filter_proxy();
    m_controller->set_app_name_filter("AppA");
    EXPECT_EQ(proxy->rowCount(), 2);

    m_controller->set_app_name_filter("AppB");
    EXPECT_EQ(proxy->rowCount(), 2);

    m_controller->set_app_name_filter("NonExistentApp");
    EXPECT_EQ(proxy->rowCount(), 0);

    m_controller->set_app_name_filter("");
    EXPECT_EQ(proxy->rowCount(), 4);
}

/**
 * @brief Tests filtering by log level.
 */
TEST_F(LogViewerControllerTest, FilterByLogLevel)
{
    auto* proxy = m_controller->get_sort_filter_proxy();
    QSet<QString> levels;
    levels.insert("INFO");
    m_controller->set_level_filter(levels);
    EXPECT_EQ(proxy->rowCount(), 2);

    levels.insert("ERROR");
    m_controller->set_level_filter(levels);
    EXPECT_EQ(proxy->rowCount(), 3);

    levels.clear();
    m_controller->set_level_filter(levels);
    EXPECT_EQ(proxy->rowCount(), 4);
}

/**
 * @brief Tests filtering by search string (plain text).
 */
TEST_F(LogViewerControllerTest, FilterBySearchText)
{
    auto* proxy = m_controller->get_sort_filter_proxy();
    m_controller->set_search_filter("Debug", "Message", false);
    EXPECT_EQ(proxy->rowCount(), 1);

    m_controller->set_search_filter("User", "Message", false);
    EXPECT_EQ(proxy->rowCount(), 1);

    m_controller->set_search_filter("AppA", "AppName", false);
    EXPECT_EQ(proxy->rowCount(), 2);

    m_controller->set_search_filter("Crash", "Message", false);
    EXPECT_EQ(proxy->rowCount(), 1);

    m_controller->set_search_filter("NotFound", "Message", false);
    EXPECT_EQ(proxy->rowCount(), 0);

    m_controller->set_search_filter("", "Message", false);
    EXPECT_EQ(proxy->rowCount(), 4);
}

/**
 * @brief Tests filtering by search string (regex).
 */
TEST_F(LogViewerControllerTest, FilterBySearchRegex)
{
    auto* proxy = m_controller->get_sort_filter_proxy();
    m_controller->set_search_filter("^User.*", "Message", true);
    EXPECT_EQ(proxy->rowCount(), 1);

    m_controller->set_search_filter(".*ing$", "Message", true);
    EXPECT_EQ(proxy->rowCount(), 1);

    m_controller->set_search_filter("Crash|Startup", "Message", true);
    EXPECT_EQ(proxy->rowCount(), 2);

    m_controller->set_search_filter("NoMatch", "Message", true);
    EXPECT_EQ(proxy->rowCount(), 0);

    m_controller->set_search_filter("", "Message", true);
    EXPECT_EQ(proxy->rowCount(), 4);
}

/**
 * @brief Tests that log_model() and proxy_model() return valid pointers.
 */
TEST_F(LogViewerControllerTest, ModelAccessorsReturnValidPointers)
{
    EXPECT_NE(m_controller->get_log_model(), nullptr);
    EXPECT_NE(m_controller->get_sort_filter_proxy(), nullptr);
}
