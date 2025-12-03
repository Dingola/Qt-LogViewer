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
    EXPECT_FALSE(m_proxy->has_active_filters());
}

/**
 * @brief Tests filtering by application name.
 */
TEST_F(LogSortFilterProxyModelTest, FilterByAppName)
{
    m_proxy->set_app_name_filter("AppA");
    EXPECT_EQ(m_proxy->rowCount(), 2);
    EXPECT_TRUE(m_proxy->has_active_filters());
    EXPECT_EQ(m_proxy->get_app_name_filter(), QString("AppA"));

    m_proxy->set_app_name_filter("AppB");
    EXPECT_EQ(m_proxy->rowCount(), 2);

    m_proxy->set_app_name_filter("NonExistentApp");
    EXPECT_EQ(m_proxy->rowCount(), 0);

    m_proxy->set_app_name_filter("");  // Reset filter
    EXPECT_EQ(m_proxy->rowCount(), 4);
    EXPECT_FALSE(m_proxy->has_active_filters());
}

/**
 * @brief Tests filtering by log level.
 */
TEST_F(LogSortFilterProxyModelTest, FilterByLogLevel)
{
    QSet<QString> levels;
    levels.insert("INFO");
    m_proxy->set_log_level_filters(levels);
    EXPECT_EQ(m_proxy->rowCount(), 2);

    levels.insert("ERROR");
    m_proxy->set_log_level_filters(levels);
    EXPECT_EQ(m_proxy->rowCount(), 3);

    levels.clear();
    m_proxy->set_log_level_filters(levels);
    EXPECT_EQ(m_proxy->rowCount(), 4);
}

/**
 * @brief Tests that log level filter normalization (case and whitespace) works.
 */
TEST_F(LogSortFilterProxyModelTest, LogLevelNormalization)
{
    QSet<QString> levels;
    levels.insert(" info ");
    levels.insert("eRrOr");
    m_proxy->set_log_level_filters(levels);
    EXPECT_EQ(m_proxy->rowCount(), 3);

    const QSet<QString> normalized = m_proxy->get_log_level_filters();
    EXPECT_TRUE(normalized.contains(QString("info")));
    EXPECT_TRUE(normalized.contains(QString("error")));
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
    m_proxy->set_log_level_filters(levels);
    EXPECT_EQ(m_proxy->rowCount(), 1);

    m_proxy->set_search_filter("Crash", "Message", false);
    EXPECT_EQ(m_proxy->rowCount(), 1);

    m_proxy->set_search_filter("Startup", "Message", false);
    EXPECT_EQ(m_proxy->rowCount(), 0);

    m_proxy->set_log_level_filters(QSet<QString>());
    m_proxy->set_search_filter("", "Message", false);
    m_proxy->set_app_name_filter("");
    EXPECT_EQ(m_proxy->rowCount(), 4);
}

/**
 * @brief Tests search across all fields (plain text and regex) and case-insensitive behavior.
 */
TEST_F(LogSortFilterProxyModelTest, SearchAllFields)
{
    // Plain text, All Fields
    m_proxy->set_search_filter("AppB", "All Fields", false);
    EXPECT_EQ(m_proxy->rowCount(), 2);

    m_proxy->set_search_filter("startup", "All Fields", false);
    EXPECT_EQ(m_proxy->rowCount(), 1);

    // Regex, All Fields
    m_proxy->set_search_filter("^AppA$", "All Fields", true);
    EXPECT_EQ(m_proxy->rowCount(), 2);

    m_proxy->set_search_filter("(User|Crash)", "All Fields", true);
    EXPECT_EQ(m_proxy->rowCount(), 2);

    // Fallback when field not recognized should behave like All Fields
    m_proxy->set_search_filter("AppA", "UnknownField", false);
    EXPECT_EQ(m_proxy->rowCount(), 2);
}

/**
 * @brief Tests getters reflect the current state after setters.
 */
TEST_F(LogSortFilterProxyModelTest, GettersReflectState)
{
    m_proxy->set_app_name_filter("AppB");
    QSet<QString> levels;
    levels.insert("DEBUG");
    m_proxy->set_log_level_filters(levels);
    m_proxy->set_search_filter("login", "Message", false);

    EXPECT_EQ(m_proxy->get_app_name_filter(), QString("AppB"));
    EXPECT_TRUE(m_proxy->get_log_level_filters().contains(QString("debug")));
    EXPECT_EQ(m_proxy->get_search_text(), QString("login"));
    EXPECT_EQ(m_proxy->get_search_field(), QString("Message"));
    EXPECT_FALSE(m_proxy->is_search_regex());
    EXPECT_TRUE(m_proxy->has_active_filters());
}

/**
 * @brief Tests invalid regex handling.
 */
TEST_F(LogSortFilterProxyModelTest, InvalidRegex)
{
    // An obviously invalid regex. QRegularExpression should mark it invalid.
    m_proxy->set_search_filter("*", "Message", true);
    EXPECT_EQ(m_proxy->rowCount(), 0);

    // Reset
    m_proxy->set_search_filter("", "Message", true);
    EXPECT_EQ(m_proxy->rowCount(), 4);
}

/**
 * @brief Tests dynamic filter changes maintain correctness.
 */
TEST_F(LogSortFilterProxyModelTest, DynamicFilterChanges)
{
    EXPECT_EQ(m_proxy->rowCount(), 4);

    m_proxy->set_app_name_filter("AppA");
    EXPECT_EQ(m_proxy->rowCount(), 2);

    QSet<QString> levels;
    levels.insert("INFO");
    m_proxy->set_log_level_filters(levels);
    EXPECT_EQ(m_proxy->rowCount(), 1);

    m_proxy->set_search_filter("Startup", "Message", false);
    EXPECT_EQ(m_proxy->rowCount(), 1);

    // Remove app filter, keep others
    m_proxy->set_app_name_filter("");
    EXPECT_EQ(m_proxy->rowCount(), 1);

    // Clear search, keep level
    m_proxy->set_search_filter("", "Message", false);
    EXPECT_EQ(m_proxy->rowCount(), 2);

    // Clear level
    m_proxy->set_log_level_filters(QSet<QString>());
    EXPECT_EQ(m_proxy->rowCount(), 4);
    EXPECT_FALSE(m_proxy->has_active_filters());
}
