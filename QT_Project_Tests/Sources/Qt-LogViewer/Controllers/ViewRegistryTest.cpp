#include "Qt-LogViewer/Controllers/ViewRegistryTest.h"

#include <QDateTime>

/**
 * @brief Constructs the test fixture.
 */
ViewRegistryTest::ViewRegistryTest(): m_registry(nullptr) {}

/**
 * @brief Destroys the test fixture.
 */
ViewRegistryTest::~ViewRegistryTest() = default;

/**
 * @brief Set up before each test.
 */
void ViewRegistryTest::SetUp()
{
    m_registry = new ViewRegistry();
}

/**
 * @brief Tear down after each test.
 */
void ViewRegistryTest::TearDown()
{
    delete m_registry;
    m_registry = nullptr;
}

/**
 * @brief Appends `count` sample entries to `ctx`, associated to `file_path`.
 */
auto ViewRegistryTest::append_sample_entries(LogViewContext* ctx, const QString& file_path,
                                             int count) -> void
{
    QVector<LogEntry> entries;
    entries.reserve(count);

    for (int i = 0; i < count; ++i)
    {
        const QDateTime ts = QDateTime::currentDateTime().addSecs(i);
        LogFileInfo info(file_path, QStringLiteral("AppX"));
        LogEntry e(ts, QStringLiteral("INFO"), QStringLiteral("Msg %1").arg(i), info);
        entries.append(e);
    }

    ctx->append_entries(entries);
}

/**
 * @brief create_view generates an id and creates a context; lookups reflect it.
 */
TEST_F(ViewRegistryTest, CreateViewGeneratesIdAndContextExists)
{
    ASSERT_NE(m_registry, nullptr);

    const QUuid id = m_registry->create_view();
    EXPECT_FALSE(id.isNull());

    LogViewContext* ctx = m_registry->get_context(id);
    EXPECT_NE(ctx, nullptr);

    const QVector<QUuid> ids = m_registry->get_all_view_ids();
    EXPECT_FALSE(ids.isEmpty());
    EXPECT_TRUE(ids.contains(id));
}

/**
 * @brief create_view with fixed id succeeds only once; ensure_view creates missing.
 */
TEST_F(ViewRegistryTest, CreateViewWithFixedIdAndEnsureView)
{
    ASSERT_NE(m_registry, nullptr);

    const QUuid fixed = QUuid::createUuid();

    const bool created_first = m_registry->create_view(fixed);
    EXPECT_TRUE(created_first);

    const bool created_duplicate = m_registry->create_view(fixed);
    EXPECT_FALSE(created_duplicate);

    // ensure_view is no-op if exists
    m_registry->ensure_view(fixed);
    EXPECT_NE(m_registry->get_context(fixed), nullptr);

    // ensure_view creates if missing
    const QUuid other = QUuid::createUuid();
    EXPECT_EQ(m_registry->get_context(other), nullptr);
    m_registry->ensure_view(other);
    EXPECT_NE(m_registry->get_context(other), nullptr);
}

/**
 * @brief remove_view removes context, clears current when matching, and does not emit view_removed
 * (class does not emit on remove).
 *
 * Note: Implementation does not emit view_removed; we assert removal and current reset behavior.
 */
TEST_F(ViewRegistryTest, RemoveViewClearsContextAndCurrentIfMatching)
{
    ASSERT_NE(m_registry, nullptr);

    const QUuid id1 = m_registry->create_view();
    const QUuid id2 = m_registry->create_view();

    // Set current to id2
    QSignalSpy spy_current(m_registry, &ViewRegistry::current_view_id_changed);
    const bool set_ok = m_registry->set_current_view(id2);
    EXPECT_TRUE(set_ok);
    EXPECT_EQ(m_registry->get_current_view(), id2);
    EXPECT_EQ(spy_current.count(), 1);

    // Remove id1: current unchanged
    const bool removed_id1 = m_registry->remove_view(id1);
    EXPECT_TRUE(removed_id1);
    EXPECT_EQ(m_registry->get_context(id1), nullptr);
    EXPECT_EQ(m_registry->get_current_view(), id2);

    // Remove id2: current becomes null
    const bool removed_id2 = m_registry->remove_view(id2);
    EXPECT_TRUE(removed_id2);
    EXPECT_EQ(m_registry->get_context(id2), nullptr);
    EXPECT_TRUE(m_registry->get_current_view().isNull());
}

/**
 * @brief set_current_view emits signal only when view exists and sets value.
 */
TEST_F(ViewRegistryTest, SetCurrentViewEmitsOnSuccessOnly)
{
    ASSERT_NE(m_registry, nullptr);

    const QUuid id = m_registry->create_view();

    QSignalSpy spy_current(m_registry, &ViewRegistry::current_view_id_changed);

    const bool ok = m_registry->set_current_view(id);
    EXPECT_TRUE(ok);
    EXPECT_EQ(spy_current.count(), 1);
    EXPECT_EQ(m_registry->get_current_view(), id);

    const QUuid missing = QUuid::createUuid();
    const bool fail = m_registry->set_current_view(missing);
    EXPECT_FALSE(fail);
    EXPECT_EQ(spy_current.count(), 1);
    EXPECT_EQ(m_registry->get_current_view(), id);
}

/**
 * @brief get_entries/get_file_paths return from context; initially empty.
 */
TEST_F(ViewRegistryTest, GetEntriesAndFilePaths)
{
    ASSERT_NE(m_registry, nullptr);

    const QUuid id = m_registry->create_view();
    LogViewContext* ctx = m_registry->get_context(id);
    ASSERT_NE(ctx, nullptr);

    // Initially empty
    EXPECT_TRUE(m_registry->get_entries(id).isEmpty());
    EXPECT_TRUE(m_registry->get_file_paths(id).isEmpty());

    // Append entries and set loaded files
    const QString fp = QStringLiteral("C:/tmp/a.log");
    append_sample_entries(ctx, fp, 3);

    QList<LogFileInfo> files;
    files.append(LogFileInfo(fp, QStringLiteral("AppA")));
    files.append(LogFileInfo(QStringLiteral("C:/tmp/b.log"), QStringLiteral("AppB")));

    m_registry->set_loaded_files(id, files);

    const QVector<LogEntry> entries = m_registry->get_entries(id);
    const QVector<QString> paths = m_registry->get_file_paths(id);

    EXPECT_EQ(entries.size(), 3);
    EXPECT_EQ(paths.size(), 2);
    EXPECT_TRUE(paths.contains(fp));
}

/**
 * @brief set_loaded_files and add_loaded_file emit view_file_paths_changed and reflect dedup
 * behavior from context.
 */
TEST_F(ViewRegistryTest, LoadedFilesSetAddAndSignalEmission)
{
    ASSERT_NE(m_registry, nullptr);

    const QUuid id = m_registry->create_view();
    QSignalSpy spy_paths(m_registry, &ViewRegistry::view_file_paths_changed);

    // Set list
    QList<LogFileInfo> files;
    files.append(LogFileInfo(QStringLiteral("C:/a.log"), QStringLiteral("AppA")));
    files.append(LogFileInfo(QStringLiteral("C:/b.log"), QStringLiteral("AppB")));

    m_registry->set_loaded_files(id, files);
    EXPECT_EQ(spy_paths.count(), 1);

    QVector<QString> paths = m_registry->get_file_paths(id);
    EXPECT_EQ(paths.size(), 2);

    // Add a new file
    m_registry->add_loaded_file(id,
                                LogFileInfo(QStringLiteral("C:/c.log"), QStringLiteral("AppC")));
    EXPECT_EQ(spy_paths.count(), 2);

    paths = m_registry->get_file_paths(id);
    EXPECT_EQ(paths.size(), 3);

    // Add duplicate should remain deduped by context; still emit from registry after add
    m_registry->add_loaded_file(id,
                                LogFileInfo(QStringLiteral("C:/a.log"), QStringLiteral("AppA")));
    EXPECT_EQ(spy_paths.count(), 3);
    paths = m_registry->get_file_paths(id);
    EXPECT_EQ(paths.size(), 3);
}

/**
 * @brief remove_entries_by_file removes entries associated with file_path and updates file paths;
 * emits signal.
 */
TEST_F(ViewRegistryTest, RemoveEntriesByFileRemovesEntriesAndUpdatesFilePaths)
{
    ASSERT_NE(m_registry, nullptr);

    const QUuid id = m_registry->create_view();
    LogViewContext* ctx = m_registry->get_context(id);
    ASSERT_NE(ctx, nullptr);

    const QString fa = QStringLiteral("C:/a.log");
    const QString fb = QStringLiteral("C:/b.log");

    // Seed entries for two files
    append_sample_entries(ctx, fa, 4);
    append_sample_entries(ctx, fb, 2);

    // Seed loaded files
    QList<LogFileInfo> files;
    files.append(LogFileInfo(fa, QStringLiteral("AppA")));
    files.append(LogFileInfo(fb, QStringLiteral("AppB")));
    m_registry->set_loaded_files(id, files);

    EXPECT_EQ(m_registry->get_entries(id).size(), 6);
    EXPECT_EQ(m_registry->get_file_paths(id).size(), 2);

    QSignalSpy spy_paths(m_registry, &ViewRegistry::view_file_paths_changed);

    // Remove entries for fa
    m_registry->remove_entries_by_file(id, fa);
    EXPECT_EQ(spy_paths.count(), 1);

    const QVector<LogEntry> remaining = m_registry->get_entries(id);
    const QVector<QString> paths = m_registry->get_file_paths(id);

    EXPECT_EQ(remaining.size(), 2);
    EXPECT_EQ(paths.size(), 1);
    EXPECT_TRUE(paths.contains(fb));
}
