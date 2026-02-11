#include "Qt-LogViewer/Models/LogFileTreeModelTest.h"

#include <QIcon>

#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Models/LogFileTreeItem.h"

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
    EXPECT_EQ(m_model->headerData(0, Qt::Horizontal, Qt::DisplayRole).toString(), "Name");

    // Vertical headers should be empty
    EXPECT_EQ(m_model->headerData(0, Qt::Vertical, Qt::DisplayRole), QVariant());
}

/**
 * @brief Tests adding, checking, renaming, and removing sessions including signal emission.
 */
TEST_F(LogFileTreeModelTest, SessionLifecycleAndSignals)
{
    AllSessionsRemovedSpy spy;
    QObject::connect(m_model, &LogFileTreeModel::all_sessions_removed, &spy,
                     &AllSessionsRemovedSpy::on_all_sessions_removed);

    // Initially no sessions
    EXPECT_FALSE(m_model->has_session("s1"));
    EXPECT_EQ(m_model->get_session_count(), 0);

    // Add sessions
    EXPECT_TRUE(m_model->add_session("s1", "Session One"));
    EXPECT_TRUE(m_model->add_session("s2", "Session Two"));
    EXPECT_EQ(m_model->get_session_count(), 2);
    EXPECT_TRUE(m_model->has_session("s1"));
    EXPECT_TRUE(m_model->has_session("s2"));

    // get_session_index returns a valid index pointing to the session item
    const QModelIndex s1_index = m_model->get_session_index("s1");
    EXPECT_TRUE(s1_index.isValid());
    EXPECT_EQ(m_model->data(s1_index, Qt::DisplayRole).toString(), "Session One");

    // Rename session
    EXPECT_TRUE(m_model->rename_session("s1", "Renamed Session"));
    EXPECT_EQ(m_model->data(s1_index, Qt::DisplayRole).toString(), "Renamed Session");

    // Flags: session items are editable, groups/files are not
    auto flags = m_model->flags(s1_index);
    EXPECT_TRUE(flags.testFlag(Qt::ItemIsEnabled));
    EXPECT_TRUE(flags.testFlag(Qt::ItemIsSelectable));
    EXPECT_TRUE(flags.testFlag(Qt::ItemIsEditable));

    // Remove one session: signal should not fire yet
    EXPECT_TRUE(m_model->remove_session("s2"));
    EXPECT_EQ(m_model->get_session_count(), 1);
    EXPECT_EQ(spy.count, 0);

    // Remove last session: signal should fire
    EXPECT_TRUE(m_model->remove_session("s1"));
    EXPECT_EQ(m_model->get_session_count(), 0);
    EXPECT_EQ(spy.count, 1);

    // Removing non-existent session should return false
    EXPECT_FALSE(m_model->remove_session("missing"));
}

/**
 * @brief Tests setting log files within a single session and grouping by app name.
 */
TEST_F(LogFileTreeModelTest, SetLogFilesInSessionAndGrouping)
{
    // If session does not exist, it is created with id as name
    m_model->set_log_files("sessA", QList<LogFileInfo>{
                                        LogFileInfo("C:/logs/log_2024_06_01.txt", "MyApp"),
                                        LogFileInfo("C:/logs/log_2024_06_02.txt", "MyApp"),
                                        LogFileInfo("C:/logs/extern_error_01.log", ""),
                                        LogFileInfo("C:/logs/thirdparty_debug.log", "UnknownApp"),
                                    });

    EXPECT_EQ(m_model->get_session_count(), 1);

    const QModelIndex sess_index = m_model->get_session_index("sessA");
    EXPECT_TRUE(sess_index.isValid());
    EXPECT_EQ(m_model->data(sess_index, Qt::DisplayRole).toString(), "sessA");

    // Should have 3 groups: MyApp, Unknown, UnknownApp
    EXPECT_EQ(m_model->rowCount(sess_index), 3);

    // Check group names (order is stable by insertion from QMap, but validate set membership)
    for (int i = 0; i < m_model->rowCount(sess_index); ++i)
    {
        const QModelIndex group_index = m_model->index(i, 0, sess_index);
        const QString group_name = m_model->data(group_index, Qt::DisplayRole).toString();
        EXPECT_TRUE(group_name == "MyApp" || group_name == "Unknown" || group_name == "UnknownApp");
    }

    // Validate files within groups
    // Find MyApp group by scanning children
    int myapp_row = -1;
    for (int i = 0; i < m_model->rowCount(sess_index); ++i)
    {
        if (m_model->data(m_model->index(i, 0, sess_index), Qt::DisplayRole).toString() == "MyApp")
        {
            myapp_row = i;
        }
    }
    ASSERT_NE(myapp_row, -1);
    const QModelIndex myapp_group = m_model->index(myapp_row, 0, sess_index);
    EXPECT_EQ(m_model->rowCount(myapp_group), 2);

    const QModelIndex file1 = m_model->index(0, 0, myapp_group);
    const QModelIndex file2 = m_model->index(1, 0, myapp_group);
    EXPECT_EQ(m_model->data(file1, Qt::DisplayRole).toString(), "log_2024_06_01.txt");
    EXPECT_EQ(m_model->data(file2, Qt::DisplayRole).toString(), "log_2024_06_02.txt");
}

/**
 * @brief Tests adding log files incrementally per session and group creation.
 */
TEST_F(LogFileTreeModelTest, AddLogFileIncrementallyPerSession)
{
    EXPECT_TRUE(m_model->add_session("S", "Session S"));
    const QModelIndex sess_index = m_model->get_session_index("S");
    ASSERT_TRUE(sess_index.isValid());

    LogFileInfo file1("C:/logs/log_2024_06_01.txt", "AppA");
    LogFileInfo file2("C:/logs/log_2024_06_02.txt", "AppA");
    LogFileInfo file3("C:/logs/extern_error_01.log", "");
    LogFileInfo file4("C:/logs/thirdparty_debug.log", "AppB");

    // Add first file, should create group "AppA"
    EXPECT_TRUE(m_model->add_log_file("S", file1));
    EXPECT_EQ(m_model->rowCount(sess_index), 1);
    QModelIndex group_index = m_model->index(0, 0, sess_index);
    EXPECT_EQ(m_model->data(group_index, Qt::DisplayRole).toString(), "AppA");
    EXPECT_EQ(m_model->rowCount(group_index), 1);

    // Add second file to same group
    EXPECT_TRUE(m_model->add_log_file("S", file2));
    EXPECT_EQ(m_model->rowCount(sess_index), 1);
    EXPECT_EQ(m_model->rowCount(group_index), 2);

    // Add file with empty app name, should create "Unknown" group
    EXPECT_TRUE(m_model->add_log_file("S", file3));
    EXPECT_EQ(m_model->rowCount(sess_index), 2);

    // add file for new group
    EXPECT_TRUE(m_model->add_log_file("S", file4));
    EXPECT_EQ(m_model->rowCount(sess_index), 3);
}

/**
 * @brief Tests removing log files within a session and automatic group removal when empty.
 */
TEST_F(LogFileTreeModelTest, RemoveLogFileAndGroupRemovalInSession)
{
    const QString session_id = "S1";
    m_model->set_log_files(session_id, QList<LogFileInfo>{
                                           LogFileInfo("C:/logs/log_2024_06_01.txt", "AppA"),
                                           LogFileInfo("C:/logs/log_2024_06_02.txt", "AppA"),
                                           LogFileInfo("C:/logs/extern_error_01.log", ""),
                                           LogFileInfo("C:/logs/thirdparty_debug.log", "AppB"),
                                       });

    const QModelIndex sess_index = m_model->get_session_index(session_id);
    ASSERT_TRUE(sess_index.isValid());
    EXPECT_EQ(m_model->rowCount(sess_index), 3);

    // Remove one file from AppA, group should remain
    EXPECT_TRUE(
        m_model->remove_log_file(session_id, LogFileInfo("C:/logs/log_2024_06_01.txt", "AppA")));

    // Find AppA group and verify one file left
    int appa_row = -1;
    for (int i = 0; i < m_model->rowCount(sess_index); ++i)
    {
        if (m_model->data(m_model->index(i, 0, sess_index), Qt::DisplayRole).toString() == "AppA")
        {
            appa_row = i;
        }
    }
    ASSERT_NE(appa_row, -1);
    QModelIndex appa_group = m_model->index(appa_row, 0, sess_index);
    EXPECT_EQ(m_model->rowCount(appa_group), 1);
    EXPECT_EQ(m_model->rowCount(sess_index), 3);

    // Remove last file from AppA, group should be removed
    EXPECT_TRUE(
        m_model->remove_log_file(session_id, LogFileInfo("C:/logs/log_2024_06_02.txt", "AppA")));
    EXPECT_EQ(m_model->rowCount(sess_index), 2);

    // Remove file from "Unknown", group should be removed
    EXPECT_TRUE(
        m_model->remove_log_file(session_id, LogFileInfo("C:/logs/extern_error_01.log", "")));
    EXPECT_EQ(m_model->rowCount(sess_index), 1);

    // Remove last file, session should have 0 groups
    EXPECT_TRUE(
        m_model->remove_log_file(session_id, LogFileInfo("C:/logs/thirdparty_debug.log", "AppB")));
    EXPECT_EQ(m_model->rowCount(sess_index), 0);
}

/**
 * @brief Tests clearing session files keeps the session node intact.
 */
TEST_F(LogFileTreeModelTest, ClearSessionFilesKeepsSession)
{
    m_model->set_log_files("S", QList<LogFileInfo>{
                                    LogFileInfo("C:/logs/a.txt", "A"),
                                    LogFileInfo("C:/logs/b.txt", "B"),
                                });
    const QModelIndex s_index = m_model->get_session_index("S");
    ASSERT_TRUE(s_index.isValid());
    EXPECT_EQ(m_model->rowCount(s_index), 2);

    m_model->clear_session_files("S");
    EXPECT_EQ(m_model->rowCount(s_index), 0);
    EXPECT_EQ(m_model->get_session_count(), 1);  // session remains
}

/**
 * @brief Tests the "all sessions" overloads: set/add/remove propagate to all sessions.
 */
TEST_F(LogFileTreeModelTest, AllSessionsOperations)
{
    EXPECT_TRUE(m_model->add_session("A", "A"));
    EXPECT_TRUE(m_model->add_session("B", "B"));
    EXPECT_EQ(m_model->get_session_count(), 2);

    QList<LogFileInfo> files;
    files << LogFileInfo("C:/logs/x1.txt", "X") << LogFileInfo("C:/logs/x2.txt", "X");

    // Set files to all existing sessions
    m_model->set_log_files(files);

    const QModelIndex a_index = m_model->get_session_index("A");
    const QModelIndex b_index = m_model->get_session_index("B");
    ASSERT_TRUE(a_index.isValid());
    ASSERT_TRUE(b_index.isValid());
    EXPECT_EQ(m_model->rowCount(a_index), 1);
    EXPECT_EQ(m_model->rowCount(b_index), 1);

    // Add file to all sessions
    m_model->add_log_file(LogFileInfo("C:/logs/y.txt", "Y"));
    EXPECT_EQ(m_model->rowCount(a_index), 2);
    EXPECT_EQ(m_model->rowCount(b_index), 2);

    // Remove file from all sessions
    m_model->remove_log_file(LogFileInfo("C:/logs/y.txt", "Y"));
    EXPECT_EQ(m_model->rowCount(a_index), 1);
    EXPECT_EQ(m_model->rowCount(b_index), 1);
}

/**
 * @brief Tests custom roles: ItemTypeRole, SessionIdRole, FilePathRole, AppNameRole.
 */
TEST_F(LogFileTreeModelTest, CustomRolesDataAccess)
{
    m_model->set_log_files("S", QList<LogFileInfo>{
                                    LogFileInfo("C:/logs/a.txt", "AppA"),
                                    LogFileInfo("C:/logs/b.txt", ""),
                                });
    const QModelIndex s_index = m_model->get_session_index("S");
    ASSERT_TRUE(s_index.isValid());

    // SessionIdRole on session item
    EXPECT_EQ(m_model->data(s_index, LogFileTreeModel::SessionIdRole).toString(), "S");

    // Locate groups
    int appa_row = -1;
    int unknown_row = -1;
    for (int i = 0; i < m_model->rowCount(s_index); ++i)
    {
        const auto name = m_model->data(m_model->index(i, 0, s_index), Qt::DisplayRole).toString();
        if (name == "AppA")
        {
            appa_row = i;
        }
        else if (name == "Unknown")
        {
            unknown_row = i;
        }
    }
    ASSERT_NE(appa_row, -1);
    ASSERT_NE(unknown_row, -1);

    const QModelIndex appa_group = m_model->index(appa_row, 0, s_index);
    const QModelIndex unknown_group = m_model->index(unknown_row, 0, s_index);

    // AppNameRole on group should return group name
    EXPECT_EQ(m_model->data(appa_group, LogFileTreeModel::AppNameRole).toString(), "AppA");
    EXPECT_EQ(m_model->data(unknown_group, LogFileTreeModel::AppNameRole).toString(), "Unknown");

    // ItemTypeRole checks
    const auto type_session =
        m_model->data(s_index, LogFileTreeModel::ItemTypeRole).value<LogFileTreeItem::Type>();
    const auto type_group =
        m_model->data(appa_group, LogFileTreeModel::ItemTypeRole).value<LogFileTreeItem::Type>();
    const QModelIndex file_index = m_model->index(0, 0, appa_group);
    const auto type_file =
        m_model->data(file_index, LogFileTreeModel::ItemTypeRole).value<LogFileTreeItem::Type>();

    EXPECT_EQ(type_session, LogFileTreeItem::Type::Session);
    EXPECT_EQ(type_group, LogFileTreeItem::Type::Group);
    EXPECT_EQ(type_file, LogFileTreeItem::Type::File);

    // FilePathRole on file and AppNameRole on file
    EXPECT_EQ(m_model->data(file_index, LogFileTreeModel::FilePathRole).toString(),
              "C:/logs/a.txt");
    EXPECT_EQ(m_model->data(file_index, LogFileTreeModel::AppNameRole).toString(), "AppA");

    // SessionIdRole on nested items should resolve upward
    EXPECT_EQ(m_model->data(appa_group, LogFileTreeModel::SessionIdRole).toString(), "S");
    EXPECT_EQ(m_model->data(file_index, LogFileTreeModel::SessionIdRole).toString(), "S");
}

/**
 * @brief Tests setData and flags: only session names are editable with Qt::EditRole.
 */
TEST_F(LogFileTreeModelTest, SetDataEditSessionOnly)
{
    m_model->add_session("S", "Initial Name");
    const QModelIndex s_index = m_model->get_session_index("S");

    // Session is editable
    EXPECT_TRUE(m_model->flags(s_index).testFlag(Qt::ItemIsEditable));

    // Edit name via setData
    EXPECT_TRUE(m_model->setData(s_index, QVariant("Edited Name"), Qt::EditRole));
    EXPECT_EQ(m_model->data(s_index, Qt::DisplayRole).toString(), "Edited Name");

    // Add a group and file, then verify non-editable
    m_model->add_log_file("S", LogFileInfo("C:/logs/a.txt", "G1"));
    const QModelIndex g_index = m_model->index(0, 0, s_index);
    const QModelIndex f_index = m_model->index(0, 0, g_index);

    EXPECT_FALSE(m_model->flags(g_index).testFlag(Qt::ItemIsEditable));
    EXPECT_FALSE(m_model->flags(f_index).testFlag(Qt::ItemIsEditable));

    // setData on non-session should fail
    EXPECT_FALSE(m_model->setData(g_index, QVariant("New Group Name"), Qt::EditRole));
    EXPECT_FALSE(m_model->setData(f_index, QVariant("New File Name"), Qt::EditRole));
}

/**
 * @brief Tests parent-child relationships with sessions/groups/files hierarchy.
 */
TEST_F(LogFileTreeModelTest, ParentChildRelationships)
{
    m_model->set_log_files("S", QList<LogFileInfo>{
                                    LogFileInfo("C:/logs/log_2024_06_01.txt", "MyApp"),
                                    LogFileInfo("C:/logs/extern_error_01.log", ""),
                                });

    const QModelIndex s_index = m_model->get_session_index("S");
    ASSERT_TRUE(s_index.isValid());

    const QModelIndex group_index = m_model->index(0, 0, s_index);
    const QModelIndex file_index = m_model->index(0, 0, group_index);

    // Parent of file_index should be group_index
    EXPECT_EQ(m_model->parent(file_index), group_index);

    // Parent of group_index should be the session index
    EXPECT_EQ(m_model->parent(group_index), s_index);

    // Parent of session index should be invalid (root)
    EXPECT_FALSE(m_model->parent(s_index).isValid());
}

/**
 * @brief Tests edge cases: empty file list, invalid indices.
 */
TEST_F(LogFileTreeModelTest, EdgeCases)
{
    // Empty session files create session and then no groups
    m_model->set_log_files("S", QList<LogFileInfo>());
    const QModelIndex s_index = m_model->get_session_index("S");
    EXPECT_TRUE(s_index.isValid());
    EXPECT_EQ(m_model->rowCount(s_index), 0);

    // Invalid index behavior
    QModelIndex invalid_index;
    EXPECT_EQ(m_model->data(invalid_index, Qt::DisplayRole), QVariant());
    EXPECT_EQ(m_model->flags(invalid_index), Qt::NoItemFlags);
    EXPECT_EQ(m_model->headerData(0, Qt::Vertical, Qt::DisplayRole), QVariant());
}

/**
 * @brief Tests removing a file that does not exist (no-op).
 */
TEST_F(LogFileTreeModelTest, RemoveNonExistentFileIsNoOp)
{
    m_model->set_log_files("S", QList<LogFileInfo>{
                                    LogFileInfo("C:/logs/log_2024_06_01.txt", "AppA"),
                                });

    const QModelIndex s_index = m_model->get_session_index("S");
    ASSERT_TRUE(s_index.isValid());
    ASSERT_EQ(m_model->rowCount(s_index), 1);
    const QModelIndex appa_group = m_model->index(0, 0, s_index);
    ASSERT_EQ(m_model->rowCount(appa_group), 1);

    // Try to remove a file not in the model
    EXPECT_FALSE(m_model->remove_log_file("S", LogFileInfo("C:/logs/does_not_exist.txt", "AppA")));
    EXPECT_EQ(m_model->rowCount(s_index), 1);
    EXPECT_EQ(m_model->rowCount(appa_group), 1);
}

/**
 * @brief Tests adding multiple log files at once to a session returns correct count.
 */
TEST_F(LogFileTreeModelTest, AddLogFilesBatchCount)
{
    EXPECT_TRUE(m_model->add_session("S", "S"));
    const QList<LogFileInfo> files{LogFileInfo("C:/logs/a.txt", "A"),
                                   LogFileInfo("C:/logs/b.txt", "A"),
                                   LogFileInfo("C:/logs/c.txt", "")};
    const int added = m_model->add_log_files("S", files);
    EXPECT_EQ(added, 3);

    const QModelIndex s_index = m_model->get_session_index("S");
    ASSERT_TRUE(s_index.isValid());
    EXPECT_EQ(m_model->rowCount(s_index), 2);  // groups: A and Unknown
}

/**
 * @brief Tests that adding a duplicate file in a session is ignored (no duplicates).
 */
TEST_F(LogFileTreeModelTest, AddDuplicateFileIsIgnoredPerSession)
{
    EXPECT_TRUE(m_model->add_session("S", "S"));
    LogFileInfo file("C:/logs/log_2024_06_01.txt", "AppA");

    EXPECT_TRUE(m_model->add_log_file("S", file));
    // Second add should return false due to same file path check
    EXPECT_FALSE(m_model->add_log_file("S", file));

    const QModelIndex s_index = m_model->get_session_index("S");
    ASSERT_TRUE(s_index.isValid());
    const QModelIndex appa_group = m_model->index(0, 0, s_index);
    EXPECT_EQ(m_model->rowCount(appa_group), 1);
}

/**
 * @brief Tests decoration role returns non-null icons for session/group/file.
 */
TEST_F(LogFileTreeModelTest, DecorationRoleIconsNonNull)
{
    // Build one session with one group and one file
    m_model->add_session("S", "S");
    m_model->add_log_file("S", LogFileInfo("C:/logs/a.txt", "G"));

    const QModelIndex s_index = m_model->get_session_index("S");
    ASSERT_TRUE(s_index.isValid());
    const QModelIndex g_index = m_model->index(0, 0, s_index);
    ASSERT_TRUE(g_index.isValid());
    const QModelIndex f_index = m_model->index(0, 0, g_index);
    ASSERT_TRUE(f_index.isValid());

    const QIcon s_icon = m_model->data(s_index, Qt::DecorationRole).value<QIcon>();
    const QIcon g_icon = m_model->data(g_index, Qt::DecorationRole).value<QIcon>();
    const QIcon f_icon = m_model->data(f_index, Qt::DecorationRole).value<QIcon>();

    EXPECT_FALSE(s_icon.isNull());
    EXPECT_FALSE(g_icon.isNull());
    EXPECT_FALSE(f_icon.isNull());
}

/**
 * @brief Tests header data only returns text for section 0 horizontal; others are empty.
 */
TEST_F(LogFileTreeModelTest, HeaderDataOtherSectionsAndOrientationsEmpty)
{
    EXPECT_EQ(m_model->headerData(0, Qt::Horizontal, Qt::DisplayRole).toString(), "Name");
    EXPECT_EQ(m_model->headerData(1, Qt::Horizontal, Qt::DisplayRole), QVariant());
    EXPECT_EQ(m_model->headerData(0, Qt::Vertical, Qt::DisplayRole), QVariant());
}

/**
 * @brief Tests global add_log_file is a no-op when there are no sessions.
 */
TEST_F(LogFileTreeModelTest, AddLogFileAllSessionsNoSessionsNoOp)
{
    // No sessions created yet
    m_model->add_log_file(LogFileInfo("C:/logs/a.txt", "App"));
    // Root has 0 rows because there are no sessions
    EXPECT_EQ(m_model->rowCount(), 0);

    // Create a session and try again
    m_model->add_session("S", "S");
    const QModelIndex s_index = m_model->get_session_index("S");
    ASSERT_TRUE(s_index.isValid());
    EXPECT_EQ(m_model->rowCount(s_index), 0);

    m_model->add_log_file(LogFileInfo("C:/logs/a.txt", "App"));
    EXPECT_EQ(m_model->rowCount(s_index), 1);  // now it adds into existing sessions
}

/**
 * @brief Tests get_session_index returns invalid for unknown session id.
 */
TEST_F(LogFileTreeModelTest, GetSessionIndexInvalidForUnknownId)
{
    const QModelIndex missing = m_model->get_session_index("missing");
    EXPECT_FALSE(missing.isValid());
}
