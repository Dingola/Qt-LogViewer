#include "Qt-LogViewer/Controllers/LogViewContextTest.h"

#include <QDateTime>

#include "Qt-LogViewer/Models/LogModel.h"

/**
 * @brief Constructs the test fixture.
 */
LogViewContextTest::LogViewContextTest(): m_ctx(nullptr) {}

/**
 * @brief Destroys the test fixture.
 */
LogViewContextTest::~LogViewContextTest() = default;

/**
 * @brief Set up before each test.
 */
void LogViewContextTest::SetUp()
{
    m_ctx = new LogViewContext();
}

/**
 * @brief Tear down after each test.
 */
void LogViewContextTest::TearDown()
{
    delete m_ctx;
    m_ctx = nullptr;
}

/**
 * @brief Helper: build sample entries for a file/app.
 */
auto LogViewContextTest::make_entries(const QString& file_path,
                                      const QString& app_name) -> QVector<LogEntry>
{
    QVector<LogEntry> entries;

    LogFileInfo info(file_path, app_name);

    LogEntry e1(QDateTime::currentDateTimeUtc(), "INFO", "Startup complete", info);
    LogEntry e2(QDateTime::currentDateTimeUtc(), "ERROR", "Crash detected", info);

    entries.append(e1);
    entries.append(e2);

    return entries;
}

/**
 * @brief Tests that the model and default filter state are initialized.
 */
TEST_F(LogViewContextTest, ModelAndDefaultFilterStateExist)
{
    ASSERT_NE(m_ctx->get_model(), nullptr);

    const FilterState state = m_ctx->get_filter_state();

    EXPECT_TRUE(state.app_name.isEmpty());
    EXPECT_TRUE(state.log_levels.isEmpty());
    EXPECT_TRUE(state.search_text.isEmpty());
    EXPECT_EQ(state.search_field, SearchField::AllFields);
    EXPECT_FALSE(state.use_regex);
    EXPECT_TRUE(state.show_only_file.isEmpty());
    EXPECT_TRUE(state.hidden_files.isEmpty());
}

/**
 * @brief Tests append_entries adds rows to the model.
 */
TEST_F(LogViewContextTest, AppendEntriesAddsRows)
{
    ASSERT_NE(m_ctx, nullptr);

    auto entries = make_entries("C:/logs/appA.log", "AppA");
    m_ctx->append_entries(entries);

    auto* model = m_ctx->get_model();
    ASSERT_NE(model, nullptr);

    EXPECT_EQ(model->rowCount(), entries.size());

    const auto stored = m_ctx->get_entries();
    EXPECT_EQ(stored.size(), entries.size());
}

/**
 * @brief Tests remove_entries_by_file_path removes only matching rows.
 */
TEST_F(LogViewContextTest, RemoveEntriesByFilePathRemovesMatchingRows)
{
    ASSERT_NE(m_ctx, nullptr);

    auto a_entries = make_entries("C:/logs/appA.log", "AppA");
    auto b_entries = make_entries("C:/logs/appB.log", "AppB");

    m_ctx->append_entries(a_entries);
    m_ctx->append_entries(b_entries);

    auto* model = m_ctx->get_model();
    ASSERT_NE(model, nullptr);
    EXPECT_EQ(model->rowCount(), a_entries.size() + b_entries.size());

    m_ctx->remove_entries_by_file_path("C:/logs/appA.log");
    EXPECT_EQ(model->rowCount(), b_entries.size());

    const auto stored = m_ctx->get_entries();
    EXPECT_EQ(stored.size(), b_entries.size());
    for (const auto& e: stored)
    {
        EXPECT_EQ(e.get_file_info().get_file_path(), QString("C:/logs/appB.log"));
        EXPECT_EQ(e.get_file_info().get_app_name(), QString("AppB"));
    }
}

/**
 * @brief Tests set_loaded_files replaces the loaded list and get_file_paths reflects it.
 */
TEST_F(LogViewContextTest, SetLoadedFilesAndGetFilePaths)
{
    ASSERT_NE(m_ctx, nullptr);

    QList<LogFileInfo> files;
    files.append(LogFileInfo("C:/logs/appA.log", "AppA"));
    files.append(LogFileInfo("C:/logs/appB.log", "AppB"));

    m_ctx->set_loaded_files(files);

    const auto paths = m_ctx->get_file_paths();
    ASSERT_EQ(paths.size(), 2);
    EXPECT_EQ(paths[0], "C:/logs/appA.log");
    EXPECT_EQ(paths[1], "C:/logs/appB.log");

    const auto infos = m_ctx->get_loaded_files();
    ASSERT_EQ(infos.size(), 2);
    EXPECT_EQ(infos[0].get_app_name(), "AppA");
    EXPECT_EQ(infos[1].get_app_name(), "AppB");
}

/**
 * @brief Tests add_loaded_file avoids duplicates by file path.
 */
TEST_F(LogViewContextTest, AddLoadedFileNoDuplicates)
{
    ASSERT_NE(m_ctx, nullptr);

    m_ctx->set_loaded_files(QList<LogFileInfo>{LogFileInfo("C:/logs/appA.log", "AppA")});
    m_ctx->add_loaded_file(LogFileInfo("C:/logs/appA.log", "AppA"));  // duplicate
    m_ctx->add_loaded_file(LogFileInfo("C:/logs/appB.log", "AppB"));  // new

    const auto infos = m_ctx->get_loaded_files();
    ASSERT_EQ(infos.size(), 2);

    const auto paths = m_ctx->get_file_paths();
    ASSERT_EQ(paths.size(), 2);
    EXPECT_EQ(paths[0], "C:/logs/appA.log");
    EXPECT_EQ(paths[1], "C:/logs/appB.log");
}

/**
 * @brief Tests clear_loaded_files resets the list and get_file_paths returns empty.
 */
TEST_F(LogViewContextTest, ClearLoadedFiles)
{
    ASSERT_NE(m_ctx, nullptr);

    m_ctx->set_loaded_files(QList<LogFileInfo>{LogFileInfo("C:/logs/appA.log", "AppA"),
                                               LogFileInfo("C:/logs/appB.log", "AppB")});

    m_ctx->clear_loaded_files();

    const auto infos = m_ctx->get_loaded_files();
    EXPECT_EQ(infos.size(), 0);

    const auto paths = m_ctx->get_file_paths();
    EXPECT_EQ(paths.size(), 0);
}

/**
 * @brief remove_entries_by_file_path with non-existent path should not change model.
 */
TEST_F(LogViewContextTest, RemoveEntriesByFilePathNonExistentNoChange)
{
    ASSERT_NE(m_ctx, nullptr);

    auto entries = make_entries("C:/logs/appA.log", "AppA");
    m_ctx->append_entries(entries);

    auto* model = m_ctx->get_model();
    ASSERT_NE(model, nullptr);
    const int before = model->rowCount();

    m_ctx->remove_entries_by_file_path("C:/logs/not_there.log");
    const int after = model->rowCount();

    EXPECT_EQ(before, after);

    const auto stored = m_ctx->get_entries();
    EXPECT_EQ(stored.size(), entries.size());
}

/**
 * @brief Dedup by path even if app_name differs.
 */
TEST_F(LogViewContextTest, AddLoadedFileDuplicatePathDifferentAppNameNoDuplicate)
{
    ASSERT_NE(m_ctx, nullptr);

    m_ctx->set_loaded_files(QList<LogFileInfo>{LogFileInfo("C:/logs/appA.log", "AppA")});
    m_ctx->add_loaded_file(LogFileInfo("C:/logs/appA.log", "OtherApp"));  // same path

    const auto infos = m_ctx->get_loaded_files();
    ASSERT_EQ(infos.size(), 1);
    EXPECT_EQ(infos[0].get_file_path(), "C:/logs/appA.log");
}

/**
 * @brief set_loaded_files replaces, not appends.
 */
TEST_F(LogViewContextTest, SetLoadedFilesReplacesNotAppends)
{
    ASSERT_NE(m_ctx, nullptr);

    m_ctx->set_loaded_files(QList<LogFileInfo>{LogFileInfo("C:/logs/appA.log", "AppA")});
    m_ctx->set_loaded_files(QList<LogFileInfo>{LogFileInfo("C:/logs/appB.log", "AppB")});

    const auto infos = m_ctx->get_loaded_files();
    ASSERT_EQ(infos.size(), 1);
    EXPECT_EQ(infos[0].get_file_path(), "C:/logs/appB.log");

    const auto paths = m_ctx->get_file_paths();
    ASSERT_EQ(paths.size(), 1);
    EXPECT_EQ(paths[0], "C:/logs/appB.log");
}

/**
 * @brief get_file_paths order matches insertion and remains stable after add.
 */
TEST_F(LogViewContextTest, GetFilePathsOrderStability)
{
    ASSERT_NE(m_ctx, nullptr);

    m_ctx->set_loaded_files(QList<LogFileInfo>{
        LogFileInfo("C:/logs/1.log", "A"),
        LogFileInfo("C:/logs/2.log", "B"),
    });

    m_ctx->add_loaded_file(LogFileInfo("C:/logs/3.log", "C"));

    const auto paths = m_ctx->get_file_paths();
    ASSERT_EQ(paths.size(), 3);
    EXPECT_EQ(paths[0], "C:/logs/1.log");
    EXPECT_EQ(paths[1], "C:/logs/2.log");
    EXPECT_EQ(paths[2], "C:/logs/3.log");
}

/**
 * @brief clear_loaded_files followed by add_loaded_file repopulates cleanly.
 */
TEST_F(LogViewContextTest, ClearThenAddLoadedFile)
{
    ASSERT_NE(m_ctx, nullptr);

    m_ctx->set_loaded_files(QList<LogFileInfo>{LogFileInfo("C:/logs/appA.log", "AppA")});
    m_ctx->clear_loaded_files();
    EXPECT_TRUE(m_ctx->get_loaded_files().isEmpty());

    m_ctx->add_loaded_file(LogFileInfo("C:/logs/appB.log", "AppB"));
    const auto infos = m_ctx->get_loaded_files();
    ASSERT_EQ(infos.size(), 1);
    EXPECT_EQ(infos[0].get_file_path(), "C:/logs/appB.log");
}

/**
 * @brief append_entries with empty vector does not change rowCount.
 */
TEST_F(LogViewContextTest, AppendEntriesEmptyNoChange)
{
    ASSERT_NE(m_ctx, nullptr);

    auto* model = m_ctx->get_model();
    ASSERT_NE(model, nullptr);
    const int before = model->rowCount();

    m_ctx->append_entries(QVector<LogEntry>{});
    const int after = model->rowCount();

    EXPECT_EQ(before, after);
}

/**
 * @brief get_entries content integrity after removals.
 */
TEST_F(LogViewContextTest, GetEntriesIntegrityAfterRemoval)
{
    ASSERT_NE(m_ctx, nullptr);

    const auto a_entries = make_entries("C:/logs/a.log", "A");
    const auto b_entries = make_entries("C:/logs/b.log", "B");
    m_ctx->append_entries(a_entries);
    m_ctx->append_entries(b_entries);

    m_ctx->remove_entries_by_file_path("C:/logs/a.log");

    const auto stored = m_ctx->get_entries();
    ASSERT_EQ(stored.size(), b_entries.size());
    for (int i = 0; i < stored.size(); ++i)
    {
        EXPECT_EQ(stored[i].get_file_info().get_file_path(), QString("C:/logs/b.log"));
        EXPECT_EQ(stored[i].get_app_name(), QString("B"));
    }
}
