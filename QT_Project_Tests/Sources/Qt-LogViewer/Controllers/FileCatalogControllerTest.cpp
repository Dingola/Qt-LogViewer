#include "Qt-LogViewer/Controllers/FileCatalogControllerTest.h"

#include <QDir>
#include <QModelIndex>
#include <QTemporaryFile>
#include <QTextStream>

#include "Qt-LogViewer/Models/LogFileInfo.h"

FileCatalogControllerTest::FileCatalogControllerTest()
    : m_ctrl(nullptr), m_ingest(nullptr), m_temp1(), m_temp2()
{}

FileCatalogControllerTest::~FileCatalogControllerTest() = default;

void FileCatalogControllerTest::SetUp()
{
    m_ingest = new LogIngestController(
        QStringLiteral("{timestamp} {level} {message} {app_name} [{file}:{line} ({function})]"));
    m_ctrl = new FileCatalogController(m_ingest);

    m_temp1 = make_temp_log_file(QStringLiteral("one"));
    m_temp2 = make_temp_log_file(QStringLiteral("two"));
}

void FileCatalogControllerTest::TearDown()
{
    delete m_ctrl;
    m_ctrl = nullptr;

    delete m_ingest;
    m_ingest = nullptr;

    if (!m_temp1.isEmpty())
    {
        QFile::remove(m_temp1);
    }
    if (!m_temp2.isEmpty())
    {
        QFile::remove(m_temp2);
    }
}

/**
 * @brief Creates a temporary log file with content matching the service format.
 * Format: "{timestamp} {level} {message} {app_name} [{file}:{line} ({function})]"
 *
 * @param stem Optional stem to include in the filename.
 * @return Absolute path to the temporary file (persists until tear down).
 */
auto FileCatalogControllerTest::make_temp_log_file(const QString& stem) -> QString
{
    QString path;

    QTemporaryFile tmp(QDir::temp().filePath(QStringLiteral("qt_fc_%1_XXXXXX.log").arg(stem)));
    tmp.setAutoRemove(false);
    if (tmp.open())
    {
        QTextStream out(&tmp);
        // Write a single line matching the known format with app_name "TestApp"
        out << "2024-01-01 12:34:56 Info Msg TestApp [file.cpp:1 (func)]\n";
        out.flush();
        path = tmp.fileName();
        tmp.close();
    }

    return path;
}

/**
 * @brief Returns a unique, non-existent file path in the temp directory.
 *
 * @param stem Optional stem to include in the filename.
 * @return Absolute path to a non-existent file.
 */
auto FileCatalogControllerTest::make_nonexistent_path(const QString& stem) const -> QString
{
    const QString base = QDir::tempPath();
    const QString name = QStringLiteral("qt_fc_%1_%2_missing.log")
                             .arg(stem, QUuid::createUuid().toString(QUuid::WithoutBraces));
    return QDir(base).filePath(name);
}

/**
 * @brief Returns the root row count of the tree model.
 *
 * @param model Target log file tree model.
 * @return Row count at root level.
 */
auto FileCatalogControllerTest::root_row_count(LogFileTreeModel* model) -> int
{
    return (model != nullptr) ? model->rowCount(QModelIndex()) : -1;
}

/**
 * @brief add_file should parse first entry via ingest and add one group to the session.
 */
TEST_F(FileCatalogControllerTest, AddFileUsesParsedAppNameAndAddsToModel)
{
    ASSERT_NE(m_ctrl, nullptr);
    auto* model = m_ctrl->get_model();
    ASSERT_NE(model, nullptr);

    // Create a session so that add_file (all-sessions overload) has a target
    ASSERT_TRUE(model->add_session("S", "S"));
    const QModelIndex s_index = model->get_session_index("S");
    ASSERT_TRUE(s_index.isValid());

    const int before_groups = model->rowCount(s_index);

    // add_file uses ingest to parse "TestApp" and adds under the only session
    m_ctrl->add_file(m_temp1);

    const int after_groups = model->rowCount(s_index);
    EXPECT_EQ(after_groups, before_groups + 1);
}

/**
 * @brief add_files should add both files under the same app group within the session.
 */
TEST_F(FileCatalogControllerTest, AddFilesAddsMultiple)
{
    ASSERT_NE(m_ctrl, nullptr);
    auto* model = m_ctrl->get_model();
    ASSERT_NE(model, nullptr);

    ASSERT_TRUE(model->add_session("S", "S"));
    const QModelIndex s_index = model->get_session_index("S");
    ASSERT_TRUE(s_index.isValid());

    const int before_groups = model->rowCount(s_index);

    QVector<QString> files;
    files << m_temp1 << m_temp2;
    m_ctrl->add_files(files);

    // Same app name "TestApp" -> exactly one new group in session
    const int after_groups = model->rowCount(s_index);
    EXPECT_EQ(after_groups, before_groups + 1);

    // Find the "TestApp" group and validate it has two files
    int testapp_row = -1;
    for (int i = 0; i < model->rowCount(s_index); ++i)
    {
        const auto idx = model->index(i, LogFileTreeModel::Column::Name, s_index);
        if (model->data(idx, Qt::DisplayRole).toString() == "TestApp")
        {
            testapp_row = i;
        }
    }
    ASSERT_NE(testapp_row, -1);
    const QModelIndex group_idx =
        model->index(testapp_row, LogFileTreeModel::Column::Name, s_index);
    EXPECT_EQ(model->rowCount(group_idx), 2);
}

/**
 * @brief remove_file should remove previously added file and drop its group when empty.
 */
TEST_F(FileCatalogControllerTest, RemoveFileRemovesFromModel)
{
    ASSERT_NE(m_ctrl, nullptr);
    auto* model = m_ctrl->get_model();
    ASSERT_NE(model, nullptr);

    ASSERT_TRUE(model->add_session("S", "S"));
    const QModelIndex s_index = model->get_session_index("S");
    ASSERT_TRUE(s_index.isValid());

    // Add one file via ingest-parsed entry ("TestApp" from temp content)
    m_ctrl->add_file(m_temp1);
    const int groups_after_add = model->rowCount(s_index);

    // Build matching LogFileInfo to remove (path + app name "TestApp")
    const LogFileInfo info(m_temp1, QStringLiteral("TestApp"));
    m_ctrl->remove_file(info);

    const int groups_after_remove = model->rowCount(s_index);
    EXPECT_EQ(groups_after_remove, groups_after_add - 1);
}

/**
 * @brief When ingest is null, controller should still add file (fallback identify_app).
 */
TEST_F(FileCatalogControllerTest, AddFileWithNullIngestFallsBackAndAdds)
{
    // Recreate controller with null ingest
    delete m_ctrl;
    m_ctrl = new FileCatalogController(nullptr);

    auto* model = m_ctrl->get_model();
    ASSERT_NE(model, nullptr);

    ASSERT_TRUE(model->add_session("S", "S"));
    const QModelIndex s_index = model->get_session_index("S");
    ASSERT_TRUE(s_index.isValid());

    const int before_groups = model->rowCount(s_index);

    const QString path = make_nonexistent_path(QStringLiteral("fb"));
    m_ctrl->add_file(path);

    const int after_groups = model->rowCount(s_index);
    EXPECT_EQ(after_groups, before_groups + 1);
}
