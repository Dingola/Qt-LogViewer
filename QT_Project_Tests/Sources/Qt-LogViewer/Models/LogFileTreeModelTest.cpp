#include "Qt-LogViewer/Models/LogFileTreeModelTest.h"

/**
 * @brief Sets up the test fixture for each test.
 */
void LogFileTreeModelTest::SetUp()
{
    m_model = new LogFileTreeModel();
}

/**
 * @brief Tears down the test fixture after each test.
 */
void LogFileTreeModelTest::TearDown()
{
    delete m_model;
    m_model = nullptr;
}

/**
 * @brief Tests default construction and header data.
 */
TEST_F(LogFileTreeModelTest, DefaultConstructionAndHeader)
{
    EXPECT_EQ(m_model->columnCount(), LogFileTreeModel::ColumnCount);
    EXPECT_EQ(m_model->rowCount(), 0);
    EXPECT_EQ(m_model->headerData(0, Qt::Horizontal, Qt::DisplayRole).toString(),
              "Application / File");
}

/**
 * @brief Tests setting log files and grouping by application name.
 */
TEST_F(LogFileTreeModelTest, SetLogFilesAndGrouping)
{
    QList<LogFileInfo> files;
    files << LogFileInfo("C:/logs/log_2024_06_01.txt", "MyApp");
    files << LogFileInfo("C:/logs/log_2024_06_02.txt", "MyApp");
    files << LogFileInfo("C:/logs/extern_error_01.log", "");
    files << LogFileInfo("C:/logs/thirdparty_debug.log", "UnknownApp");

    m_model->set_log_files(files);

    // Should have 3 groups: MyApp, Unknown, UnknownApp
    EXPECT_EQ(m_model->rowCount(), 3);

    // Check group names
    for (int i = 0; i < m_model->rowCount(); ++i)
    {
        QModelIndex group_index = m_model->index(i, 0, QModelIndex());
        QString group_name = m_model->data(group_index, Qt::DisplayRole).toString();
        EXPECT_TRUE(group_name == "MyApp" || group_name == "Unknown" || group_name == "UnknownApp");
    }
}

/**
 * @brief Tests adding log files incrementally and group creation.
 */
TEST_F(LogFileTreeModelTest, AddLogFileIncrementally)
{
    LogFileInfo file1("C:/logs/log_2024_06_01.txt", "AppA");
    LogFileInfo file2("C:/logs/log_2024_06_02.txt", "AppA");
    LogFileInfo file3("C:/logs/extern_error_01.log", "");
    LogFileInfo file4("C:/logs/thirdparty_debug.log", "AppB");

    // Add first file, should create group "AppA"
    m_model->add_log_file(file1);
    EXPECT_EQ(m_model->rowCount(), 1);
    QModelIndex group_index = m_model->index(0, 0, QModelIndex());
    EXPECT_EQ(m_model->data(group_index, Qt::DisplayRole).toString(), "AppA");
    EXPECT_EQ(m_model->rowCount(group_index), 1);

    // Add second file to same group
    m_model->add_log_file(file2);
    EXPECT_EQ(m_model->rowCount(), 1);
    EXPECT_EQ(m_model->rowCount(group_index), 2);

    // Add file with empty app name, should create "Unknown" group
    m_model->add_log_file(file3);
    EXPECT_EQ(m_model->rowCount(), 2);
    QModelIndex unknown_group = m_model->index(1, 0, QModelIndex());
    EXPECT_EQ(m_model->data(unknown_group, Qt::DisplayRole).toString(), "Unknown");
    EXPECT_EQ(m_model->rowCount(unknown_group), 1);

    // Add file for new group
    m_model->add_log_file(file4);
    EXPECT_EQ(m_model->rowCount(), 3);
    QModelIndex appb_group = m_model->index(2, 0, QModelIndex());
    EXPECT_EQ(m_model->data(appb_group, Qt::DisplayRole).toString(), "AppB");
    EXPECT_EQ(m_model->rowCount(appb_group), 1);
}

/**
 * @brief Tests adding a duplicate file (model allows duplicates).
 */
TEST_F(LogFileTreeModelTest, AddDuplicateFile)
{
    LogFileInfo file("C:/logs/log_2024_06_01.txt", "AppA");
    m_model->add_log_file(file);
    m_model->add_log_file(file);

    EXPECT_EQ(m_model->rowCount(), 1);
    QModelIndex appa_group = m_model->index(0, 0, QModelIndex());
    EXPECT_EQ(m_model->rowCount(appa_group), 2);

    // Both entries should have the same file name
    QModelIndex file1 = m_model->index(0, 0, appa_group);
    QModelIndex file2 = m_model->index(1, 0, appa_group);
    EXPECT_EQ(m_model->data(file1, Qt::DisplayRole).toString(), "log_2024_06_01.txt");
    EXPECT_EQ(m_model->data(file2, Qt::DisplayRole).toString(), "log_2024_06_01.txt");
}

/**
 * @brief Tests removing log files and automatic group removal when empty.
 */
TEST_F(LogFileTreeModelTest, RemoveLogFileAndGroupRemoval)
{
    QList<LogFileInfo> files;
    files << LogFileInfo("C:/logs/log_2024_06_01.txt", "AppA");
    files << LogFileInfo("C:/logs/log_2024_06_02.txt", "AppA");
    files << LogFileInfo("C:/logs/extern_error_01.log", "");
    files << LogFileInfo("C:/logs/thirdparty_debug.log", "AppB");
    m_model->set_log_files(files);

    // Remove one file from AppA, group should remain
    m_model->remove_log_file(LogFileInfo("C:/logs/log_2024_06_01.txt", "AppA"));
    QModelIndex appa_group = m_model->index(0, 0, QModelIndex());
    EXPECT_EQ(m_model->rowCount(appa_group), 1);
    EXPECT_EQ(m_model->rowCount(), 3);

    // Remove last file from AppA, group should be removed
    m_model->remove_log_file(LogFileInfo("C:/logs/log_2024_06_02.txt", "AppA"));
    EXPECT_EQ(m_model->rowCount(), 2);

    // Remove file from "Unknown", group should be removed
    m_model->remove_log_file(LogFileInfo("C:/logs/extern_error_01.log", ""));
    EXPECT_EQ(m_model->rowCount(), 1);

    // Remove last file, model should be empty
    m_model->remove_log_file(LogFileInfo("C:/logs/thirdparty_debug.log", "AppB"));
    EXPECT_EQ(m_model->rowCount(), 0);
}

/**
 * @brief Tests removing a file that does not exist (should not crash or change model).
 */
TEST_F(LogFileTreeModelTest, RemoveNonExistentFile)
{
    QList<LogFileInfo> files;
    files << LogFileInfo("C:/logs/log_2024_06_01.txt", "AppA");
    m_model->set_log_files(files);

    // Try to remove a file not in the model
    m_model->remove_log_file(LogFileInfo("C:/logs/does_not_exist.txt", "AppA"));
    EXPECT_EQ(m_model->rowCount(), 1);
    QModelIndex appa_group = m_model->index(0, 0, QModelIndex());
    EXPECT_EQ(m_model->rowCount(appa_group), 1);
}

/**
 * @brief Tests file entries under groups and data access.
 */
TEST_F(LogFileTreeModelTest, FileEntriesAndDataAccess)
{
    QList<LogFileInfo> files;
    files << LogFileInfo("C:/logs/log_2024_06_01.txt", "MyApp");
    files << LogFileInfo("C:/logs/log_2024_06_02.txt", "MyApp");
    files << LogFileInfo("C:/logs/extern_error_01.log", "");
    m_model->set_log_files(files);

    // MyApp group
    QModelIndex myapp_group = m_model->index(0, 0, QModelIndex());
    EXPECT_EQ(m_model->data(myapp_group, Qt::DisplayRole).toString(), "MyApp");
    EXPECT_EQ(m_model->rowCount(myapp_group), 2);

    QModelIndex file1 = m_model->index(0, 0, myapp_group);
    QModelIndex file2 = m_model->index(1, 0, myapp_group);
    EXPECT_EQ(m_model->data(file1, Qt::DisplayRole).toString(), "log_2024_06_01.txt");
    EXPECT_EQ(m_model->data(file2, Qt::DisplayRole).toString(), "log_2024_06_02.txt");

    // Unknown group
    QModelIndex unknown_group = m_model->index(1, 0, QModelIndex());
    EXPECT_EQ(m_model->data(unknown_group, Qt::DisplayRole).toString(), "Unknown");
    EXPECT_EQ(m_model->rowCount(unknown_group), 1);

    QModelIndex unknown_file = m_model->index(0, 0, unknown_group);
    EXPECT_EQ(m_model->data(unknown_file, Qt::DisplayRole).toString(), "extern_error_01.log");
}

/**
 * @brief Tests parent and child relationships.
 */
TEST_F(LogFileTreeModelTest, ParentChildRelationships)
{
    QList<LogFileInfo> files;
    files << LogFileInfo("C:/logs/log_2024_06_01.txt", "MyApp");
    files << LogFileInfo("C:/logs/extern_error_01.log", "");
    m_model->set_log_files(files);

    QModelIndex group_index = m_model->index(0, 0, QModelIndex());
    QModelIndex file_index = m_model->index(0, 0, group_index);

    // Parent of file_index should be group_index
    EXPECT_EQ(m_model->parent(file_index), group_index);

    // Parent of group_index should be invalid (root)
    EXPECT_FALSE(m_model->parent(group_index).isValid());
}

/**
 * @brief Tests edge cases: empty file list, invalid indices.
 */
TEST_F(LogFileTreeModelTest, EdgeCases)
{
    m_model->set_log_files(QList<LogFileInfo>());
    EXPECT_EQ(m_model->rowCount(), 0);

    QModelIndex invalid_index;
    EXPECT_EQ(m_model->data(invalid_index, Qt::DisplayRole), QVariant());
    EXPECT_EQ(m_model->flags(invalid_index), Qt::NoItemFlags);
    EXPECT_EQ(m_model->headerData(0, Qt::Vertical, Qt::DisplayRole), QVariant());
}
