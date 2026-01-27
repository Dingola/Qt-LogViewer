#include "Qt-LogViewer/Services/SessionRepositoryTest.h"

/**
 * @brief Sets up a fresh, isolated repository under a unique AppConfig subdirectory.
 */
void SessionRepositoryTest::SetUp()
{
    m_subdir = get_unique_subdir();
    m_repo = new SessionRepository(m_subdir);
}

/**
 * @brief Tears down and cleans the repository directory tree.
 */
void SessionRepositoryTest::TearDown()
{
    delete m_repo;
    m_repo = nullptr;

    // Best-effort cleanup of the unique repository directory
    const QString base_dir = get_base_dir();
    remove_dir_recursive(base_dir);
}

/**
 * @brief Returns a unique subdirectory name under AppConfigLocation.
 */
auto SessionRepositoryTest::get_unique_subdir() const -> QString
{
    const QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QString subdir = QStringLiteral("Qt-LogViewer-Test-%1").arg(uuid);
    return subdir;
}

/**
 * @brief Computes the absolute base directory path used by the current repository.
 */
auto SessionRepositoryTest::get_base_dir() const -> QString
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir d(base);
    const QString path = d.filePath(m_subdir);
    return path;
}

/**
 * @brief Recursively removes a directory tree.
 */
auto SessionRepositoryTest::remove_dir_recursive(const QString& dir_path) -> bool
{
    bool ok = true;
    QDir dir(dir_path);

    if (!dir.exists())
    {
        ok = true;
    }
    else
    {
        const QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries |
                                                        QDir::Hidden | QDir::System);
        for (const QFileInfo& fi: entries)
        {
            if (fi.isDir())
            {
                ok = remove_dir_recursive(fi.absoluteFilePath()) && ok;
            }
            else
            {
                ok = QFile::remove(fi.absoluteFilePath()) && ok;
            }
        }
        ok = dir.rmdir(dir.absolutePath()) && ok;
    }

    return ok;
}

/**
 * @brief Verifies path helpers produce expected locations.
 */
TEST_F(SessionRepositoryTest, PathComputations)
{
    ASSERT_NE(m_repo, nullptr);

    const QString base_dir = get_base_dir();
    const QString root_path = m_repo->get_root_file_path();
    const QString sessions_dir = m_repo->get_sessions_dir_path();

    EXPECT_TRUE(root_path.startsWith(base_dir));
    EXPECT_TRUE(root_path.endsWith(QStringLiteral("sessions.json")));

    EXPECT_TRUE(sessions_dir.startsWith(base_dir));
    EXPECT_TRUE(sessions_dir.endsWith(QStringLiteral("sessions")));

    const QString sid = QStringLiteral("abc123");
    const QString session_path = m_repo->get_session_file_path(sid);
    EXPECT_TRUE(session_path.contains(QStringLiteral("sessions")));
    EXPECT_TRUE(session_path.endsWith(QStringLiteral("abc123.json")));
}

/**
 * @brief load_all() returns defaults when root file is missing.
 */
TEST_F(SessionRepositoryTest, LoadAllReturnsDefaultsWhenMissing)
{
    ASSERT_NE(m_repo, nullptr);

    const QJsonObject root = m_repo->load_all();

    EXPECT_TRUE(root.contains(QStringLiteral("schema_version")));
    EXPECT_TRUE(root.value(QStringLiteral("schema_version")).isDouble());
    EXPECT_EQ(root.value(QStringLiteral("schema_version")).toInt(),
              SessionRepository::get_schema_version());

    EXPECT_TRUE(root.contains(QStringLiteral("recent_files")));
    EXPECT_TRUE(root.value(QStringLiteral("recent_files")).isArray());
    EXPECT_EQ(root.value(QStringLiteral("recent_files")).toArray().size(), 0);

    EXPECT_TRUE(root.contains(QStringLiteral("recent_sessions")));
    EXPECT_TRUE(root.value(QStringLiteral("recent_sessions")).isArray());
    EXPECT_EQ(root.value(QStringLiteral("recent_sessions")).toArray().size(), 0);
}

/**
 * @brief load_all() returns defaults when root file contains invalid JSON.
 */
TEST_F(SessionRepositoryTest, LoadAllReturnsDefaultsOnInvalidJson)
{
    ASSERT_NE(m_repo, nullptr);

    const QString root_path = m_repo->get_root_file_path();
    QDir().mkpath(QFileInfo(root_path).absolutePath());

    QFile f(root_path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Text));
    f.write("not-a-json");
    f.close();

    const QJsonObject root = m_repo->load_all();

    EXPECT_TRUE(root.contains(QStringLiteral("schema_version")));
    EXPECT_EQ(root.value(QStringLiteral("schema_version")).toInt(),
              SessionRepository::get_schema_version());

    EXPECT_TRUE(root.contains(QStringLiteral("recent_files")));
    EXPECT_TRUE(root.value(QStringLiteral("recent_files")).isArray());
    EXPECT_TRUE(root.contains(QStringLiteral("recent_sessions")));
    EXPECT_TRUE(root.value(QStringLiteral("recent_sessions")).isArray());
}

/**
 * @brief save_all() writes JSON and injects schema_version if missing.
 */
TEST_F(SessionRepositoryTest, SaveAllWritesAndInjectsSchema)
{
    ASSERT_NE(m_repo, nullptr);

    QJsonObject to_save;
    QJsonArray files;
    files.append(QStringLiteral("A.log"));
    files.append(QStringLiteral("B.log"));
    to_save.insert(QStringLiteral("recent_files"), files);
    to_save.insert(QStringLiteral("recent_sessions"), QJsonArray());

    m_repo->save_all(to_save);

    const QString root_path = m_repo->get_root_file_path();
    EXPECT_TRUE(QFile::exists(root_path));

    // Read back
    const QJsonObject loaded = m_repo->load_all();
    EXPECT_TRUE(loaded.contains(QStringLiteral("schema_version")));
    EXPECT_EQ(loaded.value(QStringLiteral("schema_version")).toInt(),
              SessionRepository::get_schema_version());

    const QJsonArray loaded_files = loaded.value(QStringLiteral("recent_files")).toArray();
    EXPECT_EQ(loaded_files.size(), 2);
    EXPECT_EQ(loaded_files.at(0).toString(), QStringLiteral("A.log"));
    EXPECT_EQ(loaded_files.at(1).toString(), QStringLiteral("B.log"));
}

/**
 * @brief save_all() preserves an explicitly provided schema_version.
 */
TEST_F(SessionRepositoryTest, SaveAllPreservesExplicitSchemaVersion)
{
    ASSERT_NE(m_repo, nullptr);

    QJsonObject to_save;
    to_save.insert(QStringLiteral("schema_version"), 123);  // custom version
    to_save.insert(QStringLiteral("recent_files"), QJsonArray());
    to_save.insert(QStringLiteral("recent_sessions"), QJsonArray());

    m_repo->save_all(to_save);

    const QJsonObject loaded = m_repo->load_all();
    EXPECT_EQ(loaded.value(QStringLiteral("schema_version")).toInt(), 123);
}

/**
 * @brief save_all() adds missing arrays when they are not provided explicitly.
 */
TEST_F(SessionRepositoryTest, SaveAllAddsMissingArrays)
{
    ASSERT_NE(m_repo, nullptr);

    // Save only schema_version; arrays should be defaulted on load
    QJsonObject to_save;
    to_save.insert(QStringLiteral("schema_version"), SessionRepository::get_schema_version());
    m_repo->save_all(to_save);

    const QJsonObject loaded = m_repo->load_all();
    EXPECT_TRUE(loaded.contains(QStringLiteral("recent_files")));
    EXPECT_TRUE(loaded.value(QStringLiteral("recent_files")).isArray());
    EXPECT_TRUE(loaded.contains(QStringLiteral("recent_sessions")));
    EXPECT_TRUE(loaded.value(QStringLiteral("recent_sessions")).isArray());
}

/**
 * @brief save_session() creates per-session file and injects schema; load_session() round-trips.
 */
TEST_F(SessionRepositoryTest, SaveAndLoadSessionRoundTrip)
{
    ASSERT_NE(m_repo, nullptr);

    const QString sid = QStringLiteral("S1");
    QJsonObject obj;
    obj.insert(QStringLiteral("foo"), 42);

    m_repo->save_session(sid, obj);

    const QString session_path = m_repo->get_session_file_path(sid);
    EXPECT_TRUE(QFile::exists(session_path));

    const QJsonObject loaded = m_repo->load_session(sid);
    EXPECT_FALSE(loaded.isEmpty());
    EXPECT_EQ(loaded.value(QStringLiteral("foo")).toInt(), 42);
    EXPECT_TRUE(loaded.contains(QStringLiteral("schema_version")));
}

/**
 * @brief save_session() injects schema when not present.
 */
TEST_F(SessionRepositoryTest, SaveSessionInjectsSchema)
{
    ASSERT_NE(m_repo, nullptr);

    const QString sid = QStringLiteral("S_only");
    QJsonObject obj;  // no schema_version
    m_repo->save_session(sid, obj);

    const QJsonObject loaded = m_repo->load_session(sid);
    EXPECT_TRUE(loaded.contains(QStringLiteral("schema_version")));
}

/**
 * @brief load_session() returns empty object when the file is missing.
 */
TEST_F(SessionRepositoryTest, LoadSessionReturnsEmptyWhenMissing)
{
    ASSERT_NE(m_repo, nullptr);

    const QString sid = QStringLiteral("missing_session");
    const QString session_path = m_repo->get_session_file_path(sid);
    QFile::remove(session_path);  // ensure it's missing

    const QJsonObject loaded = m_repo->load_session(sid);
    EXPECT_TRUE(loaded.isEmpty());
}

/**
 * @brief load_session() returns empty object when JSON is invalid.
 */
TEST_F(SessionRepositoryTest, LoadSessionReturnsEmptyOnInvalidJson)
{
    ASSERT_NE(m_repo, nullptr);

    const QString sid = QStringLiteral("invalid_json");
    const QString session_path = m_repo->get_session_file_path(sid);

    QDir().mkpath(QFileInfo(session_path).absolutePath());
    QFile f(session_path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Text));
    f.write("not-a-json");
    f.close();

    const QJsonObject loaded = m_repo->load_session(sid);
    EXPECT_TRUE(loaded.isEmpty());
}

/**
 * @brief delete_session() returns proper boolean and removes file.
 */
TEST_F(SessionRepositoryTest, DeleteSessionBehavior)
{
    ASSERT_NE(m_repo, nullptr);

    const QString sid = QStringLiteral("S_del");
    m_repo->save_session(sid, QJsonObject{{QStringLiteral("x"), 1}});

    const QString path = m_repo->get_session_file_path(sid);
    ASSERT_TRUE(QFile::exists(path));

    EXPECT_TRUE(m_repo->delete_session(sid));
    EXPECT_FALSE(QFile::exists(path));

    // Deleting again should return false
    EXPECT_FALSE(m_repo->delete_session(sid));
}

/**
 * @brief get_schema_version() returns a positive version.
 */
TEST_F(SessionRepositoryTest, SchemaVersionIsPositive)
{
    EXPECT_GT(SessionRepository::get_schema_version(), 0);
}

/**
 * @brief save_all() round-trip: recent_files and recent_sessions persist and load unchanged.
 */
TEST_F(SessionRepositoryTest, SaveAllRoundTripRecentFilesAndSessions)
{
    ASSERT_NE(m_repo, nullptr);

    // Prepare root with both arrays populated
    QJsonArray files;
    files.append(QStringLiteral("C:/logs/a.log"));
    files.append(QStringLiteral("C:/logs/b.log"));

    QJsonArray sessions;
    sessions.append(QStringLiteral("S-A"));
    sessions.append(QStringLiteral("S-B"));

    QJsonObject to_save;
    to_save.insert(QStringLiteral("recent_files"), files);
    to_save.insert(QStringLiteral("recent_sessions"), sessions);
    // Let repo inject schema_version
    m_repo->save_all(to_save);

    // Load and verify fidelity
    const QJsonObject loaded = m_repo->load_all();

    ASSERT_TRUE(loaded.contains(QStringLiteral("recent_files")));
    ASSERT_TRUE(loaded.contains(QStringLiteral("recent_sessions")));

    const QJsonArray loaded_files = loaded.value(QStringLiteral("recent_files")).toArray();
    const QJsonArray loaded_sessions = loaded.value(QStringLiteral("recent_sessions")).toArray();

    ASSERT_EQ(loaded_files.size(), 2);
    ASSERT_EQ(loaded_sessions.size(), 2);

    EXPECT_EQ(loaded_files.at(0).toString(), QStringLiteral("C:/logs/a.log"));
    EXPECT_EQ(loaded_files.at(1).toString(), QStringLiteral("C:/logs/b.log"));

    EXPECT_EQ(loaded_sessions.at(0).toString(), QStringLiteral("S-A"));
    EXPECT_EQ(loaded_sessions.at(1).toString(), QStringLiteral("S-B"));

    // Ensure schema_version exists
    EXPECT_TRUE(loaded.contains(QStringLiteral("schema_version")));
    EXPECT_GT(loaded.value(QStringLiteral("schema_version")).toInt(), 0);
}
