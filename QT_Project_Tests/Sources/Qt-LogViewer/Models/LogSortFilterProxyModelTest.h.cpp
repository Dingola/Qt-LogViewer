#include "Qt-LogViewer/Models/LogSortFilterProxyModelTest.h"

#include <QDateTime>
#include <QSet>
#include <QString>

/**k
 * @brief Sets up the test fixture for each test.
 */
void LogSortFilterProxyModelTest::SetUp()
{
    m_model = new LogModel();
    m_proxy = new LogSortFilterProxyModel();
    m_proxy->setSourceModel(m_model);

    // Add some test data
    m_model->add_entry(LogEntry(QDateTime::fromString("2024-01-01 10:00:00", "yyyy-MM-dd HH:mm:ss"),
                                "INFO", "Startup", LogFileInfo("fileA.log", "AppA")));
    m_model->add_entry(LogEntry(QDateTime::fromString("2024-01-01 10:01:00", "yyyy-MM-dd HH:mm:ss"),
                                "ERROR", "Crash", LogFileInfo("fileA.log", "AppA")));
    m_model->add_entry(LogEntry(QDateTime::fromString("2024-01-01 10:02:00", "yyyy-MM-dd HH:mm:ss"),
                                "DEBUG", "Debugging", LogFileInfo("fileB.log", "AppB")));
    m_model->add_entry(LogEntry(QDateTime::fromString("2024-01-01 10:03:00", "yyyy-MM-dd HH:mm:ss"),
                                "INFO", "User login", LogFileInfo("fileB.log", "AppB")));
}

/**
 * @brief Tears down the test fixture after each test.
 */
void LogSortFilterProxyModelTest::TearDown()
{
    delete m_proxy;
    delete m_model;
    m_proxy = nullptr;
    m_model = nullptr;
}

/**
 * @brief Tests that the proxy initially shows all entries.
 */
TEST_F(LogSortFilterProxyModelTest, ProxyInitiallyShowsAllEntries)
{
    EXPECT_EQ(m_proxy->rowCount(), 4);
}

/**
 * @brief Tests filtering by application name.
 */
TEST_F(LogSortFilterProxyModelTest, FilterByAppName)
{
    m_proxy->set_app_name_filter("AppA");
    EXPECT_EQ(m_proxy->rowCount(), 2);

    m_proxy->set_app_name_filter("AppB");
    EXPECT_EQ(m_proxy->rowCount(), 2);

    m_proxy->set_app_name_filter("NonExistentApp");
    EXPECT_EQ(m_proxy->rowCount(), 0);

    m_proxy->set_app_name_filter("");  // Reset filter
    EXPECT_EQ(m_proxy->rowCount(), 4);
}

/**
 * @brief Tests filtering by log level.
 */
TEST_F(LogSortFilterProxyModelTest, FilterByLogLevel)
{
    QSet<QString> levels;
    levels.insert("INFO");
    m_proxy->set_level_filter(levels);
    EXPECT_EQ(m_proxy->rowCount(), 2);

    levels.insert("ERROR");
    m_proxy->set_level_filter(levels);
    EXPECT_EQ(m_proxy->rowCount(), 3);

    levels.clear();
    m_proxy->set_level_filter(levels);
    EXPECT_EQ(m_proxy->rowCount(), 4);
}

/**
 * @brief Tests filtering by search string (plain text).
 */
TEST_F(LogSortFilterProxyModelTest, FilterBySearchText)
{
    m_proxy->set_search_filter("Debug", "Message", false);
    EXPECT_EQ(m_proxy->rowCount(), 1);

    m_proxy->set_search_filter("User", "Message", false);
    EXPECT_EQ(m_proxy->rowCount(), 1);

    m_proxy->set_search_filter("AppA", "AppName", false);
    EXPECT_EQ(m_proxy->rowCount(), 2);

    m_proxy->set_search_filter("Crash", "Message", false);
    EXPECT_EQ(m_proxy->rowCount(), 1);

    m_proxy->set_search_filter("NotFound", "Message", false);
    EXPECT_EQ(m_proxy->rowCount(), 0);

    m_proxy->set_search_filter("", "Message", false);  // Reset filter
    EXPECT_EQ(m_proxy->rowCount(), 4);
}

/**
 * @brief Tests filtering by search string (regex).
 */
TEST_F(LogSortFilterProxyModelTest, FilterBySearchRegex)
{
    m_proxy->set_search_filter("^User.*", "Message", true);
    EXPECT_EQ(m_proxy->rowCount(), 1);

    m_proxy->set_search_filter(".*ing$", "Message", true);
    EXPECT_EQ(m_proxy->rowCount(), 1);

    m_proxy->set_search_filter("Crash|Startup", "Message", true);
    EXPECT_EQ(m_proxy->rowCount(), 2);

    m_proxy->set_search_filter("NoMatch", "Message", true);
    EXPECT_EQ(m_proxy->rowCount(), 0);

    m_proxy->set_search_filter("", "Message", true);  // Reset filter
    EXPECT_EQ(m_proxy->rowCount(), 4);
}

/**
 * @brief Tests combined filtering by app name and log level.
 */
TEST_F(LogSortFilterProxyModelTest, CombinedAppNameAndLevelFilter)
{
    m_proxy->set_app_name_filter("AppA");
    QSet<QString> levels;
    levels.insert("ERROR");
    m_proxy->set_level_filter(levels);
    EXPECT_EQ(m_proxy->rowCount(), 1);

    m_proxy->set_search_filter("Crash", "Message", false);
    EXPECT_EQ(m_proxy->rowCount(), 1);

    m_proxy->set_search_filter("Startup", "Message", false);
    EXPECT_EQ(m_proxy->rowCount(), 0);

    m_proxy->set_level_filter(QSet<QString>());
    m_proxy->set_search_filter("", "Message", false);
    m_proxy->set_app_name_filter("");
    EXPECT_EQ(m_proxy->rowCount(), 4);
}
