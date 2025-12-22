#include "Qt-LogViewer/Models/LogSortFilterProxyModelTest.h"

#include <QDateTime>
#include <QSignalSpy>
#include <QVector>

/**
 * @brief Sets up the test fixture for each test.
 */
void LogSortFilterProxyModelTest::SetUp()
{
    m_model = new LogModel();
    m_proxy = new LogSortFilterProxyModel();
    m_proxy->setSourceModel(m_model);
    seed_data();
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
 * @brief Builds a small dataset with two files and mixed levels/messages/app names.
 */
auto LogSortFilterProxyModelTest::seed_data() -> void
{
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
 * @brief Proxy initially shows all entries; no filters active.
 */
TEST_F(LogSortFilterProxyModelTest, ProxyInitiallyShowsAllEntries)
{
    EXPECT_EQ(m_proxy->rowCount(), 4);
    EXPECT_FALSE(m_proxy->has_active_filters());
    EXPECT_EQ(m_proxy->get_app_name_filter(), QString());
    EXPECT_TRUE(m_proxy->get_log_level_filters().isEmpty());
    EXPECT_EQ(m_proxy->get_search_text(), QString());
    EXPECT_EQ(m_proxy->get_search_field(), QString());
    EXPECT_FALSE(m_proxy->is_search_regex());
    EXPECT_EQ(m_proxy->get_show_only_file_path(), QString());
    EXPECT_TRUE(m_proxy->get_hidden_file_paths().isEmpty());
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

    m_proxy->set_app_name_filter("");
    EXPECT_EQ(m_proxy->rowCount(), 4);
    EXPECT_FALSE(m_proxy->has_active_filters());
}

/**
 * @brief Tests filtering by log level and normalization.
 */
TEST_F(LogSortFilterProxyModelTest, FilterByLogLevelAndNormalization)
{
    QSet<QString> levels;
    levels.insert("INFO");
    m_proxy->set_log_level_filters(levels);
    EXPECT_EQ(m_proxy->rowCount(), 2);
    EXPECT_TRUE(m_proxy->has_active_filters());

    levels.insert("ERROR");
    m_proxy->set_log_level_filters(levels);
    EXPECT_EQ(m_proxy->rowCount(), 3);

    QSet<QString> sloppy;
    sloppy.insert(" info ");
    sloppy.insert("eRrOr");
    m_proxy->set_log_level_filters(sloppy);
    const QSet<QString> normalized = m_proxy->get_log_level_filters();
    EXPECT_TRUE(normalized.contains(QString("info")));
    EXPECT_TRUE(normalized.contains(QString("error")));
    EXPECT_EQ(m_proxy->rowCount(), 3);

    levels.clear();
    m_proxy->set_log_level_filters(levels);
    EXPECT_EQ(m_proxy->rowCount(), 4);
    EXPECT_FALSE(m_proxy->has_active_filters());
}

/**
 * @brief Tests filtering by search string (plain text).
 */
TEST_F(LogSortFilterProxyModelTest, FilterBySearchText)
{
    m_proxy->set_search_filter("Debug", "Message", false);
    EXPECT_EQ(m_proxy->rowCount(), 1);
    EXPECT_TRUE(m_proxy->has_active_filters());
    EXPECT_EQ(m_proxy->get_search_text(), QString("Debug"));
    EXPECT_EQ(m_proxy->get_search_field(), QString("Message"));
    EXPECT_FALSE(m_proxy->is_search_regex());

    m_proxy->set_search_filter("User", "Message", false);
    EXPECT_EQ(m_proxy->rowCount(), 1);

    m_proxy->set_search_filter("AppA", "AppName", false);
    EXPECT_EQ(m_proxy->rowCount(), 2);

    m_proxy->set_search_filter("Crash", "Message", false);
    EXPECT_EQ(m_proxy->rowCount(), 1);

    m_proxy->set_search_filter("NotFound", "Message", false);
    EXPECT_EQ(m_proxy->rowCount(), 0);

    m_proxy->set_search_filter("", "Message", false);
    EXPECT_EQ(m_proxy->rowCount(), 4);
    EXPECT_FALSE(m_proxy->has_active_filters());
}

/**
 * @brief Tests filtering by search string (regex), including invalid regex handling.
 */
TEST_F(LogSortFilterProxyModelTest, FilterBySearchRegexAndInvalidRegex)
{
    m_proxy->set_search_filter("^User.*", "Message", true);
    EXPECT_EQ(m_proxy->rowCount(), 1);
    EXPECT_TRUE(m_proxy->is_search_regex());

    m_proxy->set_search_filter(".*ing$", "Message", true);
    EXPECT_EQ(m_proxy->rowCount(), 1);

    m_proxy->set_search_filter("Crash|Startup", "Message", true);
    EXPECT_EQ(m_proxy->rowCount(), 2);

    m_proxy->set_search_filter("NoMatch", "Message", true);
    EXPECT_EQ(m_proxy->rowCount(), 0);

    // Invalid regex should yield zero rows
    m_proxy->set_search_filter("*", "Message", true);
    EXPECT_EQ(m_proxy->rowCount(), 0);

    m_proxy->set_search_filter("", "Message", true);
    EXPECT_EQ(m_proxy->rowCount(), 4);
    EXPECT_FALSE(m_proxy->has_active_filters());
}

/**
 * @brief Tests combined filtering by app name and log level and search.
 */
TEST_F(LogSortFilterProxyModelTest, CombinedAppNameLevelAndSearchFilter)
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
    EXPECT_FALSE(m_proxy->has_active_filters());
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

    // Unknown field should behave like All Fields
    m_proxy->set_search_filter("AppA", "UnknownField", false);
    EXPECT_EQ(m_proxy->rowCount(), 2);

    // Reset
    m_proxy->set_search_filter("", "All Fields", false);
    EXPECT_EQ(m_proxy->rowCount(), 4);
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
    m_proxy->set_show_only_file_path("fileB.log");
    m_proxy->hide_file("fileA.log");

    EXPECT_EQ(m_proxy->get_app_name_filter(), QString("AppB"));
    EXPECT_TRUE(m_proxy->get_log_level_filters().contains(QString("debug")));
    EXPECT_EQ(m_proxy->get_search_text(), QString("login"));
    EXPECT_EQ(m_proxy->get_search_field(), QString("Message"));
    EXPECT_FALSE(m_proxy->is_search_regex());
    EXPECT_EQ(m_proxy->get_show_only_file_path(), QString("fileB.log"));
    EXPECT_TRUE(m_proxy->get_hidden_file_paths().contains(QString("fileA.log")));
    EXPECT_TRUE(m_proxy->has_active_filters());
}

/**
 * @brief File visibility: show-only filter constrains to a single file and emits a signal.
 */
TEST_F(LogSortFilterProxyModelTest, ShowOnlyFilePathFiltersAndEmitsSignal)
{
    QSignalSpy spy(m_proxy, &LogSortFilterProxyModel::show_only_changed);

    EXPECT_EQ(m_proxy->rowCount(), 4);

    m_proxy->set_show_only_file_path("fileA.log");
    EXPECT_EQ(m_proxy->rowCount(), 2);
    EXPECT_EQ(m_proxy->get_show_only_file_path(), QString("fileA.log"));
    EXPECT_TRUE(m_proxy->has_active_filters());
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toString(), QString("fileA.log"));

    // Changing to same value should not emit again
    m_proxy->set_show_only_file_path("fileA.log");
    EXPECT_EQ(spy.count(), 0);

    // Switching to fileB
    m_proxy->set_show_only_file_path("fileB.log");
    EXPECT_EQ(m_proxy->rowCount(), 2);
    EXPECT_EQ(m_proxy->get_show_only_file_path(), QString("fileB.log"));
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toString(), QString("fileB.log"));

    // Clearing show-only
    m_proxy->set_show_only_file_path(QString());
    EXPECT_EQ(m_proxy->rowCount(), 4);
    EXPECT_EQ(m_proxy->get_show_only_file_path(), QString());
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toString(), QString());
}

/**
 * @brief File visibility: hide/unhide a file updates rows and emits per-change signal.
 */
TEST_F(LogSortFilterProxyModelTest, HideAndUnhideFileEmitsSignalAndUpdatesRows)
{
    QSignalSpy spy(m_proxy, &LogSortFilterProxyModel::file_visibility_changed);

    EXPECT_EQ(m_proxy->rowCount(), 4);

    m_proxy->hide_file("fileA.log");
    EXPECT_EQ(m_proxy->rowCount(), 2);
    EXPECT_TRUE(m_proxy->get_hidden_file_paths().contains(QString("fileA.log")));
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toString(), QString("fileA.log"));

    // Hiding again is idempotent (no signal, no change)
    m_proxy->hide_file("fileA.log");
    EXPECT_EQ(m_proxy->rowCount(), 2);
    EXPECT_EQ(spy.count(), 0);

    m_proxy->unhide_file("fileA.log");
    EXPECT_EQ(m_proxy->rowCount(), 4);
    EXPECT_FALSE(m_proxy->get_hidden_file_paths().contains(QString("fileA.log")));
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toString(), QString("fileA.log"));
}

/**
 * @brief File visibility: set/clear hidden set replaces completely and emits a broadcast signal.
 */
TEST_F(LogSortFilterProxyModelTest, SetAndClearHiddenFilePathsBroadcastsAndUpdatesRows)
{
    QSignalSpy spy(m_proxy, &LogSortFilterProxyModel::file_visibility_changed);

    // Hide both files via set
    QSet<QString> to_hide;
    to_hide.insert("fileA.log");
    to_hide.insert("fileB.log");
    m_proxy->set_hidden_file_paths(to_hide);
    EXPECT_EQ(m_proxy->rowCount(), 0);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(m_proxy->get_hidden_file_paths().contains(QString("fileA.log")));
    EXPECT_TRUE(m_proxy->get_hidden_file_paths().contains(QString("fileB.log")));
    EXPECT_TRUE(spy.takeFirst().at(0).toString().isEmpty());

    // Replace hidden set: only fileA
    QSet<QString> only_a;
    only_a.insert("fileA.log");
    m_proxy->set_hidden_file_paths(only_a);
    EXPECT_EQ(m_proxy->rowCount(), 2);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(m_proxy->get_hidden_file_paths().contains(QString("fileA.log")));
    EXPECT_FALSE(m_proxy->get_hidden_file_paths().contains(QString("fileB.log")));
    EXPECT_TRUE(spy.takeFirst().at(0).toString().isEmpty());

    // Clear via explicit clear method
    m_proxy->clear_hidden_files();
    EXPECT_EQ(m_proxy->rowCount(), 4);
    EXPECT_TRUE(m_proxy->get_hidden_file_paths().isEmpty());
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(spy.takeFirst().at(0).toString().isEmpty());

    // Clearing again is no-op (no broadcast)
    m_proxy->clear_hidden_files();
    EXPECT_EQ(spy.count(), 0);
}

/**
 * @brief Interaction between show-only and hidden: hidden takes effect after show-only selection.
 */
TEST_F(LogSortFilterProxyModelTest, ShowOnlyAndHiddenInteraction)
{
    // Show only fileA (2 rows), then hide fileA => 0 rows
    m_proxy->set_show_only_file_path("fileA.log");
    EXPECT_EQ(m_proxy->rowCount(), 2);
    m_proxy->hide_file("fileA.log");
    EXPECT_EQ(m_proxy->rowCount(), 0);

    // Switch show-only to fileB (2 rows), unhide fileA leaves hidden set with none affecting B
    m_proxy->set_show_only_file_path("fileB.log");
    EXPECT_EQ(m_proxy->rowCount(), 2);
    m_proxy->unhide_file("fileA.log");
    EXPECT_EQ(m_proxy->rowCount(), 2);

    // Clear show-only and hide fileB => only fileA rows visible
    m_proxy->set_show_only_file_path(QString());
    m_proxy->hide_file("fileB.log");
    EXPECT_EQ(m_proxy->rowCount(), 2);
}

/**
 * @brief Sorting: timestamp column uses QDateTime ordering; other columns use case-insensitive
 * collator.
 */
TEST_F(LogSortFilterProxyModelTest, SortingByTimestampAndStringCollation)
{
    // Ensure initial unsorted proxy can be sorted by timestamp ascending
    m_proxy->sort(LogModel::Timestamp, Qt::AscendingOrder);
    // Map proxy rows to source model timestamps
    QVector<QDateTime> ordered;
    for (int r = 0; r < m_proxy->rowCount(); ++r)
    {
        const QModelIndex proxy_index = m_proxy->index(r, LogModel::Timestamp);
        const QModelIndex source_index = m_proxy->mapToSource(proxy_index);
        const QDateTime ts = m_model->data(source_index, Qt::DisplayRole).toDateTime();
        ordered.append(ts);
    }
    EXPECT_TRUE(ordered[0] <= ordered[1]);
    EXPECT_TRUE(ordered[1] <= ordered[2]);
    EXPECT_TRUE(ordered[2] <= ordered[3]);

    // Sort by Message ascending (case-insensitive via collator)
    m_proxy->sort(LogModel::Message, Qt::AscendingOrder);
    QVector<QString> msgs;
    for (int r = 0; r < m_proxy->rowCount(); ++r)
    {
        const QModelIndex proxy_index = m_proxy->index(r, LogModel::Message);
        const QModelIndex source_index = m_proxy->mapToSource(proxy_index);
        const QString msg = m_model->data(source_index, Qt::DisplayRole).toString();
        msgs.append(msg);
    }
    // Expect alphabetical order ignoring case
    EXPECT_LE(m_proxy->get_collator().compare(msgs[0], msgs[1]), 0);
    EXPECT_LE(m_proxy->get_collator().compare(msgs[1], msgs[2]), 0);
    EXPECT_LE(m_proxy->get_collator().compare(msgs[2], msgs[3]), 0);
}

/**
 * @brief Dynamic filter changes maintain correctness with file filters in play.
 */
TEST_F(LogSortFilterProxyModelTest, DynamicFilterChangesWithFileFilters)
{
    EXPECT_EQ(m_proxy->rowCount(), 4);

    m_proxy->set_show_only_file_path("fileA.log");
    EXPECT_EQ(m_proxy->rowCount(), 2);

    QSet<QString> levels;
    levels.insert("INFO");
    m_proxy->set_log_level_filters(levels);
    EXPECT_EQ(m_proxy->rowCount(), 1);

    m_proxy->set_search_filter("Startup", "Message", false);
    EXPECT_EQ(m_proxy->rowCount(), 1);

    // Remove app filter, keep others (none set)
    m_proxy->set_app_name_filter("");
    EXPECT_EQ(m_proxy->rowCount(), 1);

    // Clear search, keep level
    m_proxy->set_search_filter("", "Message", false);
    EXPECT_EQ(m_proxy->rowCount(), 1);

    // Clear level; still constrained by show-only
    m_proxy->set_log_level_filters(QSet<QString>());
    EXPECT_EQ(m_proxy->rowCount(), 2);

    // Hide current file; show-only remains but hidden excludes it
    m_proxy->hide_file("fileA.log");
    EXPECT_EQ(m_proxy->rowCount(), 0);

    // Reset all file filters
    m_proxy->set_show_only_file_path(QString());
    m_proxy->clear_hidden_files();
    EXPECT_EQ(m_proxy->rowCount(), 4);
    EXPECT_FALSE(m_proxy->has_active_filters());
}