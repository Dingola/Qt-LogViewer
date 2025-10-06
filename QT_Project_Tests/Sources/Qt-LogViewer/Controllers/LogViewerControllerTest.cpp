#include "Qt-LogViewer/Controllers/LogViewerControllerTest.h"

#include <QFile>
#include <QSet>
#include <QTextStream>

/**
 * @brief Constructs the test fixture.
 */
LogViewerControllerTest::LogViewerControllerTest(): m_controller(nullptr) {}

/**
 * @brief Destroys the test fixture.
 */
LogViewerControllerTest::~LogViewerControllerTest()
{
    for (auto* file: m_temp_files)
    {
        if (file)
        {
            QFile::remove(file->fileName());
            delete file;
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
auto LogViewerControllerTest::create_temp_file(const QVector<QString>& lines) -> QTemporaryFile*
{
    QTemporaryFile* temp_file = new QTemporaryFile();
    temp_file->setAutoRemove(false);
    if (!temp_file->open())
    {
        delete temp_file;
        return nullptr;
    }
    QTextStream out(temp_file);
    for (const auto& line: lines)
    {
        out << line << "\n";
    }
    out.flush();
    temp_file->close();
    m_temp_files.append(temp_file);
    m_temp_file_names.append(temp_file->fileName());
    return temp_file;
}

/**
 * @brief Sets up the test fixture before each test.
 */
void LogViewerControllerTest::SetUp()
{
    QString format = "{timestamp} {level} {message} {app_name}";
    m_controller = new LogViewerController(format);

    // Create two log files with different app names and levels
    QTemporaryFile* temp_file1 = create_temp_file(
        {"2024-01-01 10:00:00 INFO Startup AppA", "2024-01-01 10:01:00 ERROR Crash AppA"});
    QTemporaryFile* temp_file2 = create_temp_file(
        {"2024-01-01 10:02:00 DEBUG Debugging AppB", "2024-01-01 10:03:00 INFO UserLogin AppB"});

    ASSERT_NE(temp_file1, nullptr);
    ASSERT_NE(temp_file2, nullptr);

    QVector<QString> files = {temp_file1->fileName(), temp_file2->fileName()};
    m_view_id = m_controller->load_log_files(files);
    m_controller->set_current_view(m_view_id);
}

/**
 * @brief Tears down the test fixture after each test.
 */
void LogViewerControllerTest::TearDown()
{
    delete m_controller;
    m_controller = nullptr;

    for (auto* file: m_temp_files)
    {
        if (file != nullptr)
        {
            QFile::remove(file->fileName());
            delete file;
        }
    }

    m_temp_files.clear();
    m_temp_file_names.clear();
}

/**
 * @brief Tests that the controller loads all log entries and models/proxies are valid.
 */
TEST_F(LogViewerControllerTest, LoadsAllLogEntriesAndModels)
{
    auto* model = m_controller->get_log_model();
    auto* proxy = m_controller->get_sort_filter_proxy();
    auto* paging = m_controller->get_paging_proxy();

    ASSERT_NE(model, nullptr);
    ASSERT_NE(proxy, nullptr);
    ASSERT_NE(paging, nullptr);

    EXPECT_EQ(model->rowCount(), 4);
    EXPECT_EQ(proxy->rowCount(), 4);
    EXPECT_EQ(paging->rowCount(), 4);

    auto entries = m_controller->get_log_entries();
    EXPECT_EQ(entries.size(), 4);
}

/**
 * @brief Tests filtering by application name.
 */
TEST_F(LogViewerControllerTest, FilterByAppName)
{
    auto* proxy = m_controller->get_sort_filter_proxy();
    ASSERT_NE(proxy, nullptr);

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
    ASSERT_NE(proxy, nullptr);

    QSet<QString> levels;
    levels.insert("INFO");
    m_controller->set_log_level_filters(levels);
    EXPECT_EQ(proxy->rowCount(), 2);

    levels.insert("ERROR");
    m_controller->set_log_level_filters(levels);
    EXPECT_EQ(proxy->rowCount(), 3);

    levels.clear();
    m_controller->set_log_level_filters(levels);
    EXPECT_EQ(proxy->rowCount(), 4);
}

/**
 * @brief Tests filtering by search string (plain text).
 */
TEST_F(LogViewerControllerTest, FilterBySearchText)
{
    auto* proxy = m_controller->get_sort_filter_proxy();
    ASSERT_NE(proxy, nullptr);

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
    ASSERT_NE(proxy, nullptr);

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
 * @brief Tests view management: add, set, and remove views.
 */
TEST_F(LogViewerControllerTest, ViewManagement)
{
    QTemporaryFile* temp_file3 = create_temp_file({"2024-01-01 11:00:00 INFO NewEntry AppC"});
    ASSERT_NE(temp_file3, nullptr);

    QUuid new_view_id = m_controller->load_log_file(temp_file3->fileName());
    EXPECT_TRUE(m_controller->set_current_view(new_view_id));
    EXPECT_EQ(m_controller->get_current_view(), new_view_id);

    auto* model = m_controller->get_log_model();
    ASSERT_NE(model, nullptr);
    EXPECT_EQ(model->rowCount(), 1);

    EXPECT_TRUE(m_controller->remove_view(new_view_id));
    EXPECT_EQ(m_controller->get_log_model(new_view_id), nullptr);
    EXPECT_EQ(m_controller->get_sort_filter_proxy(new_view_id), nullptr);
    EXPECT_EQ(m_controller->get_paging_proxy(new_view_id), nullptr);
}

/**
 * @brief Tests is_file_loaded for global and per-view checks.
 */
TEST_F(LogViewerControllerTest, IsFileLoadedChecks)
{
    ASSERT_GE(m_temp_file_names.size(), 2);
    QString file1 = m_temp_file_names[0];
    QString file2 = m_temp_file_names[1];

    EXPECT_TRUE(m_controller->is_file_loaded(file1));
    EXPECT_TRUE(m_controller->is_file_loaded(file2));
    EXPECT_FALSE(m_controller->is_file_loaded("nonexistent.log"));

    EXPECT_TRUE(m_controller->is_file_loaded(m_view_id, file1));
    EXPECT_TRUE(m_controller->is_file_loaded(m_view_id, file2));
}

/**
 * @brief Tests removing a log file and its effect on views and models.
 */
TEST_F(LogViewerControllerTest, RemoveLogFile)
{
    ASSERT_GE(m_temp_file_names.size(), 1);
    QString file1 = m_temp_file_names[0];
    auto entries = m_controller->get_log_entries();
    LogFileInfo info;
    bool found = false;

    for (const auto& entry: entries)
    {
        if (!found && entry.get_file_info().get_file_path() == file1)
        {
            info = entry.get_file_info();
            found = true;
        }
    }

    ASSERT_FALSE(info.get_file_path().isEmpty());

    m_controller->remove_log_file(info);

    EXPECT_FALSE(m_controller->is_file_loaded(file1));
    auto* model = m_controller->get_log_model();
    ASSERT_NE(model, nullptr);
    EXPECT_EQ(model->rowCount(), 2);  // Only file2 entries remain
}

/**
 * @brief Tests that get_app_names returns correct set of application names.
 */
TEST_F(LogViewerControllerTest, GetAppNames)
{
    QSet<QString> app_names = m_controller->get_app_names();
    EXPECT_TRUE(app_names.contains("AppA"));
    EXPECT_TRUE(app_names.contains("AppB"));
    EXPECT_EQ(app_names.size(), 2);

    QSet<QString> app_names_view = m_controller->get_app_names(m_view_id);
    EXPECT_EQ(app_names, app_names_view);
}

/**
 * @brief Tests getter methods for filters and search.
 */
TEST_F(LogViewerControllerTest, FilterGetters)
{
    m_controller->set_app_name_filter("AppA");
    EXPECT_EQ(m_controller->get_app_name_filter(), "AppA");
    EXPECT_EQ(m_controller->get_app_name_filter(m_view_id), "AppA");

    QSet<QString> levels;
    levels.insert("INFO");
    m_controller->set_log_level_filters(levels);
    EXPECT_EQ(m_controller->get_log_level_filters(m_view_id), levels);

    m_controller->set_search_filter("Crash", "Message", false);
    EXPECT_EQ(m_controller->get_search_text(), "Crash");
    EXPECT_EQ(m_controller->get_search_text(m_view_id), "Crash");
    EXPECT_EQ(m_controller->get_search_field(), "Message");
    EXPECT_EQ(m_controller->get_search_field(m_view_id), "Message");
    EXPECT_FALSE(m_controller->is_search_regex());
    EXPECT_FALSE(m_controller->is_search_regex(m_view_id));
}

/**
 * @brief Tests get_log_file_tree_model returns a valid pointer.
 */
TEST_F(LogViewerControllerTest, LogFileTreeModelAccess)
{
    auto* tree_model = m_controller->get_log_file_tree_model();
    EXPECT_NE(tree_model, nullptr);
}

/**
 * @brief Tests get_log_entries and get_entries_for_file methods.
 */
TEST_F(LogViewerControllerTest, LogEntriesAccess)
{
    auto entries = m_controller->get_log_entries();
    EXPECT_EQ(entries.size(), 4);

    ASSERT_GE(m_temp_file_names.size(), 1);
    QString file1 = m_temp_file_names[0];
    LogFileInfo info;
    bool found = false;

    for (const auto& entry: entries)
    {
        if (!found && entry.get_file_info().get_file_path() == file1)
        {
            info = entry.get_file_info();
            found = true;
        }
    }
    ASSERT_FALSE(info.get_file_path().isEmpty());

    auto file_entries = m_controller->get_entries_for_file(info);
    EXPECT_EQ(file_entries.size(), 2);

    auto file_entries_view = m_controller->get_entries_for_file(m_view_id, info);
    ASSERT_EQ(file_entries.size(), file_entries_view.size());

    for (int i = 0; i < file_entries.size(); ++i)
    {
        EXPECT_EQ(file_entries[i].get_timestamp(), file_entries_view[i].get_timestamp());
        EXPECT_EQ(file_entries[i].get_level(), file_entries_view[i].get_level());
        EXPECT_EQ(file_entries[i].get_message(), file_entries_view[i].get_message());
        EXPECT_EQ(file_entries[i].get_app_name(), file_entries_view[i].get_app_name());
        EXPECT_EQ(file_entries[i].get_file_info().get_file_path(),
                  file_entries_view[i].get_file_info().get_file_path());
        EXPECT_EQ(file_entries[i].get_file_info().get_app_name(),
                  file_entries_view[i].get_file_info().get_app_name());
    }
}

/**
 * @brief Tests that model and proxy accessors return nullptr for invalid view.
 */
TEST_F(LogViewerControllerTest, InvalidViewReturnsNullptr)
{
    QUuid invalid_id = QUuid::createUuid();
    EXPECT_EQ(m_controller->get_log_model(invalid_id), nullptr);
    EXPECT_EQ(m_controller->get_sort_filter_proxy(invalid_id), nullptr);
    EXPECT_EQ(m_controller->get_paging_proxy(invalid_id), nullptr);
}
