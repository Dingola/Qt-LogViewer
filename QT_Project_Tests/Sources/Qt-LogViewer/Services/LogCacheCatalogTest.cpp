/**
 * @file LogCacheCatalogTest.cpp
 * @brief Verifies cache fingerprints, catalog lifecycle rules and per-file
 * schemas.
 */

#include "Qt-LogViewer/Services/LogCacheCatalogTest.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTextStream>

#include "Qt-LogViewer/Services/LogCacheCatalog.h"
#include "Qt-LogViewer/Services/LogCacheIdentity.h"
#include "Qt-LogViewer/Services/LogFileCacheDatabase.h"
#include "Qt-LogViewer/Services/LogParser.h"
#include "Qt-LogViewer/Services/LogParsingProfile.h"

namespace
{
/**
 * @brief Replaces a UTF-8 text file with deterministic test contents.
 * @param path Destination file path.
 * @param contents Text written to the file.
 * @return True when the file was opened, written and flushed without stream
 * errors.
 */
auto write_file(const QString& path, const QString& contents) -> bool
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        return false;
    }
    QTextStream stream(&file);
    stream << contents;
    return stream.status() == QTextStream::Ok;
}
}  // namespace

/**
 * @test Verifies deterministic fingerprints, parser-configuration sensitivity
 * and source-change detection.
 */
TEST_F(LogCacheCatalogTest, FingerprintChangesWithFileOrParserConfiguration)
{
    const QString file_path = m_temporary_directory.filePath(QStringLiteral("source.log"));
    ASSERT_TRUE(write_file(file_path, QStringLiteral("INFO first App\n")));
    const LogParsingProfile first_profile =
        LogParsingProfile::create_default(QStringLiteral("{level} {message} {app_name}"));
    const LogParsingProfile second_profile =
        LogParsingProfile::create_default(QStringLiteral("{message} {level} {app_name}"));
    const LogParsingProfile equivalent_profile = LogParsingProfile::create_default(
        QStringLiteral("{level} {message} {app_name}"), QStringLiteral("Other name"));

    const auto first = LogCacheIdentity::create(file_path, first_profile);
    const auto same = LogCacheIdentity::create(file_path, first_profile);
    const auto other_profile = LogCacheIdentity::create(file_path, second_profile);
    const auto equivalent = LogCacheIdentity::create(file_path, equivalent_profile);
    ASSERT_TRUE(first.has_value());
    ASSERT_TRUE(same.has_value());
    ASSERT_TRUE(other_profile.has_value());
    ASSERT_TRUE(equivalent.has_value());
    EXPECT_TRUE(first->is_valid());
    EXPECT_EQ(first->cache_key, same->cache_key);
    EXPECT_EQ(first->parser_sha256, same->parser_sha256);
    EXPECT_EQ(first->parser_sha256, equivalent->parser_sha256);
    EXPECT_EQ(first->cache_key, equivalent->cache_key);
    EXPECT_NE(first->cache_key, other_profile->cache_key);

    ASSERT_TRUE(write_file(file_path, QStringLiteral("INFO changed App\n")));
    const auto changed_file = LogCacheIdentity::create(file_path, first_profile);
    ASSERT_TRUE(changed_file.has_value());
    EXPECT_NE(first->sample_sha256, changed_file->sample_sha256);
    EXPECT_NE(first->cache_key, changed_file->cache_key);
}

/**
 * @test Verifies an appended source retains its prior line-complete identity as a reusable prefix.
 */
TEST_F(LogCacheCatalogTest, FindsLargestUnchangedCompletePrefix)
{
    const QString file_path = m_temporary_directory.filePath(QStringLiteral("appended.log"));
    ASSERT_TRUE(write_file(file_path, QStringLiteral("INFO first App\n")));
    const LogParsingProfile profile =
        LogParsingProfile::create_default(QStringLiteral("{level} {message} {app_name}"));
    const auto prefix_identity = LogCacheIdentity::create(file_path, profile);
    ASSERT_TRUE(prefix_identity.has_value());

    LogCacheCatalog catalog(m_temporary_directory.filePath(QStringLiteral("prefix-cache")));
    const auto building = catalog.begin_generation(prefix_identity.value());
    ASSERT_TRUE(building.has_value());
    ASSERT_TRUE(catalog.mark_complete(building->id, prefix_identity->file_size, 1, 4096));

    QFile appended_file(file_path);
    ASSERT_TRUE(appended_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text));
    ASSERT_GT(appended_file.write("ERROR second App\n"), 0);
    appended_file.close();
    const auto current_identity = LogCacheIdentity::create(file_path, profile);
    ASSERT_TRUE(current_identity.has_value());

    const auto prefix = catalog.find_complete_prefix(current_identity.value());
    ASSERT_TRUE(prefix.has_value());
    EXPECT_EQ(prefix->id, building->id);
    EXPECT_TRUE(prefix_identity->matches_source_prefix(file_path));
}

/** @test Verifies changed bytes and incomplete final records reject prefix reuse. */
TEST_F(LogCacheCatalogTest, RejectsMutatedOrLineIncompletePrefixes)
{
    const QString file_path = m_temporary_directory.filePath(QStringLiteral("unsafe-prefix.log"));
    const LogParsingProfile profile =
        LogParsingProfile::create_default(QStringLiteral("{level} {message} {app_name}"));
    ASSERT_TRUE(write_file(file_path, QStringLiteral("INFO first App\n")));
    const auto original_identity = LogCacheIdentity::create(file_path, profile);
    ASSERT_TRUE(original_identity.has_value());
    ASSERT_TRUE(write_file(file_path, QStringLiteral("ERROR changed App\nINFO appended App\n")));
    EXPECT_FALSE(original_identity->matches_source_prefix(file_path));

    ASSERT_TRUE(write_file(file_path, QStringLiteral("INFO incomplete App")));
    const auto incomplete_identity = LogCacheIdentity::create(file_path, profile);
    ASSERT_TRUE(incomplete_identity.has_value());
    QFile appended_file(file_path);
    ASSERT_TRUE(appended_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text));
    ASSERT_GT(appended_file.write(" continued\n"), 0);
    appended_file.close();
    EXPECT_FALSE(incomplete_identity->matches_source_prefix(file_path));
}

/**
 * @test Verifies the building-to-complete transition and ordered, removable
 * view mappings.
 */
TEST_F(LogCacheCatalogTest, StoresCompleteGenerationAndOrderedViewMapping)
{
    const QString file_path = m_temporary_directory.filePath(QStringLiteral("source.log"));
    ASSERT_TRUE(write_file(file_path, QStringLiteral("INFO message App\n")));
    const LogParsingProfile profile =
        LogParsingProfile::create_default(QStringLiteral("{level} {message} {app_name}"));
    const auto identity = LogCacheIdentity::create(file_path, profile);
    ASSERT_TRUE(identity.has_value());

    LogCacheCatalog catalog(m_temporary_directory.filePath(QStringLiteral("cache")));
    ASSERT_TRUE(catalog.is_available());
    EXPECT_EQ(catalog.get_schema_version(), LogCacheCatalog::SchemaVersion);

    const auto building = catalog.begin_generation(identity.value());
    ASSERT_TRUE(building.has_value());
    EXPECT_EQ(building->state, LogCacheGenerationState::Building);
    EXPECT_TRUE(building->database_path.endsWith(identity->cache_key + QStringLiteral(".sqlite")));
    EXPECT_FALSE(catalog.find_complete_generation(identity.value()).has_value());

    ASSERT_TRUE(catalog.mark_complete(building->id, identity->file_size, 1, 4096));
    const auto complete = catalog.find_complete_generation(identity.value());
    ASSERT_TRUE(complete.has_value());
    EXPECT_EQ(complete->state, LogCacheGenerationState::Complete);
    EXPECT_EQ(complete->entry_count, 1);
    EXPECT_EQ(complete->storage_bytes, 4096);

    const QUuid view_id = QUuid::createUuid();
    ASSERT_TRUE(catalog.bind_view(view_id, {complete->id}));
    EXPECT_EQ(catalog.get_view_generations(view_id), QVector<qint64>({complete->id}));
    EXPECT_TRUE(catalog.remove_view(view_id));
    EXPECT_TRUE(catalog.get_view_generations(view_id).isEmpty());
    EXPECT_TRUE(catalog.find_complete_generation(identity.value()).has_value());
}

/**
 * @test Verifies that an incompatible catalog schema and its stale cache files
 * are rebuilt.
 */
TEST_F(LogCacheCatalogTest, RebuildsAnIncompatibleCatalogSchema)
{
    const QString cache_root = m_temporary_directory.filePath(QStringLiteral("cache"));
    ASSERT_TRUE(QDir().mkpath(cache_root));
    const QString database_path = QDir(cache_root).filePath(QStringLiteral("catalog.sqlite"));
    const QString stale_file_path = QDir(QDir(cache_root).filePath(QStringLiteral("files")))
                                        .filePath(QStringLiteral("stale.sqlite"));
    ASSERT_TRUE(QDir().mkpath(QFileInfo(stale_file_path).absolutePath()));
    ASSERT_TRUE(write_file(stale_file_path, QStringLiteral("obsolete")));
    const QString connection_name = QStringLiteral("old_cache_schema_test");
    {
        QSqlDatabase database =
            QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection_name);
        database.setDatabaseName(database_path);
        ASSERT_TRUE(database.open());
        QSqlQuery query(database);
        ASSERT_TRUE(query.exec(QStringLiteral("CREATE TABLE cache_sources(legacy TEXT)")));
        ASSERT_TRUE(query.exec(QStringLiteral("PRAGMA user_version=1")));
        database.close();
    }
    QSqlDatabase::removeDatabase(connection_name);

    LogCacheCatalog catalog(cache_root);
    ASSERT_TRUE(catalog.is_available());
    EXPECT_EQ(catalog.get_schema_version(), LogCacheCatalog::SchemaVersion);
    EXPECT_FALSE(QFileInfo::exists(stale_file_path));

    const QString verification_name = QStringLiteral("rebuilt_cache_schema_test");
    {
        QSqlDatabase database =
            QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), verification_name);
        database.setDatabaseName(database_path);
        ASSERT_TRUE(database.open());
        QSqlQuery query(database);
        ASSERT_TRUE(query.exec(QStringLiteral("PRAGMA table_info(cache_sources)")));
        QStringList columns;
        while (query.next())
        {
            columns.append(query.value(1).toString());
        }
        EXPECT_TRUE(columns.contains(QStringLiteral("canonical_path")));
        EXPECT_FALSE(columns.contains(QStringLiteral("legacy")));
        database.close();
    }
    QSqlDatabase::removeDatabase(verification_name);
}

/**
 * @test Verifies that building or failed generations cannot become visible
 * through a view.
 */
TEST_F(LogCacheCatalogTest, DoesNotBindIncompleteGenerationToView)
{
    const QString file_path = m_temporary_directory.filePath(QStringLiteral("source.log"));
    ASSERT_TRUE(write_file(file_path, QStringLiteral("INFO message App\n")));
    const LogParsingProfile profile =
        LogParsingProfile::create_default(QStringLiteral("{level} {message} {app_name}"));
    const auto identity = LogCacheIdentity::create(file_path, profile);
    ASSERT_TRUE(identity.has_value());
    LogCacheCatalog catalog(m_temporary_directory.filePath(QStringLiteral("cache")));
    const auto building = catalog.begin_generation(identity.value());
    ASSERT_TRUE(building.has_value());

    const QUuid view_id = QUuid::createUuid();
    EXPECT_FALSE(catalog.bind_view(view_id, {building->id}));
    EXPECT_TRUE(catalog.get_view_generations(view_id).isEmpty());
    EXPECT_TRUE(catalog.mark_failed(building->id, QStringLiteral("cancelled")));
    EXPECT_FALSE(catalog.find_complete_generation(identity.value()).has_value());
}

/**
 * @test Verifies the normalized per-file schema stores byte ranges instead of
 * duplicated text.
 */
TEST_F(LogCacheCatalogTest, CreatesNormalizedDisposableFileCacheSchema)
{
    const QString file_path = m_temporary_directory.filePath(QStringLiteral("source.log"));
    ASSERT_TRUE(write_file(file_path, QStringLiteral("INFO message App\n")));
    const LogParsingProfile profile =
        LogParsingProfile::create_default(QStringLiteral("{level} {message} {app_name}"));
    const auto identity = LogCacheIdentity::create(file_path, profile);
    ASSERT_TRUE(identity.has_value());
    const QString database_path =
        m_temporary_directory.filePath(QStringLiteral("file-cache.sqlite"));

    LogFileCacheDatabase cache(database_path, identity.value());
    ASSERT_TRUE(cache.is_available());
    EXPECT_EQ(cache.get_schema_version(), LogFileCacheDatabase::SchemaVersion);
    EXPECT_EQ(cache.get_identity().cache_key, identity->cache_key);

    const QString connection_name = QStringLiteral("file_cache_schema_test");
    {
        QSqlDatabase database =
            QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection_name);
        database.setDatabaseName(database_path);
        ASSERT_TRUE(database.open());
        QSqlQuery query(database);
        ASSERT_TRUE(query.exec(QStringLiteral("PRAGMA table_info(log_entries)")));
        QStringList columns;
        while (query.next())
        {
            columns.append(query.value(1).toString());
        }
        EXPECT_TRUE(columns.contains(QStringLiteral("byte_offset")));
        EXPECT_TRUE(columns.contains(QStringLiteral("byte_length")));
        EXPECT_TRUE(columns.contains(QStringLiteral("level_id")));
        EXPECT_TRUE(columns.contains(QStringLiteral("app_id")));
        EXPECT_TRUE(columns.contains(QStringLiteral("parsed_fields_cbor")));
        EXPECT_FALSE(columns.contains(QStringLiteral("view_id")));
        EXPECT_FALSE(columns.contains(QStringLiteral("file_path")));
        EXPECT_FALSE(columns.contains(QStringLiteral("raw_record")));
        EXPECT_FALSE(columns.contains(QStringLiteral("message")));
        database.close();
    }
    QSqlDatabase::removeDatabase(connection_name);
}

/**
 * @test Verifies parsed batches persist exact source ranges across native line
 * endings, normalized dimensions and searchable FTS metadata.
 */
TEST_F(LogCacheCatalogTest, WritesIndexedEntriesAndSearchMetadata)
{
    const QString file_path = m_temporary_directory.filePath(QStringLiteral("source.log"));
    ASSERT_TRUE(write_file(file_path, QStringLiteral("INFO first App\nERROR second App\n")));
    const LogParsingProfile profile =
        LogParsingProfile::create_default(QStringLiteral("{level} {message} {app_name}"));
    const auto identity = LogCacheIdentity::create(file_path, profile);
    ASSERT_TRUE(identity.has_value());
    QFile source_file(file_path);
    ASSERT_TRUE(source_file.open(QIODevice::ReadOnly));
    const QByteArray source_bytes = source_file.readAll();
    const qint64 second_entry_offset = source_bytes.indexOf(QByteArrayLiteral("ERROR second App"));
    ASSERT_GE(second_entry_offset, 0);

    const QVector<LogEntry> entries = LogParser(profile).parse_file(file_path);
    ASSERT_EQ(entries.size(), 2);
    EXPECT_EQ(entries.at(0).get_byte_offset(), 0);
    EXPECT_EQ(entries.at(0).get_byte_length(), 14);
    EXPECT_EQ(entries.at(1).get_byte_offset(), second_entry_offset);
    EXPECT_EQ(entries.at(1).get_byte_length(), 16);

    const QString database_path =
        m_temporary_directory.filePath(QStringLiteral("indexed-cache.sqlite"));
    {
        LogFileCacheDatabase cache(database_path, identity.value());
        ASSERT_TRUE(cache.is_available());
        ASSERT_TRUE(cache.reset_entries());
        ASSERT_TRUE(cache.append_entries(entries));
        ASSERT_TRUE(cache.finalize_writes());
        EXPECT_EQ(cache.get_entry_count(), 2);
        EXPECT_GT(cache.get_storage_bytes(), 0);
    }

    const QString connection_name = QStringLiteral("indexed_cache_content_test");
    {
        QSqlDatabase database =
            QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection_name);
        database.setDatabaseName(database_path);
        ASSERT_TRUE(database.open());
        QSqlQuery query(database);
        ASSERT_TRUE(
            query.exec(QStringLiteral("SELECT COUNT(*) FROM log_entries_fts WHERE "
                                      "log_entries_fts MATCH 'second'")));
        ASSERT_TRUE(query.next());
        EXPECT_EQ(query.value(0).toLongLong(), 1);
        database.close();
    }
    QSqlDatabase::removeDatabase(connection_name);
}

/**
 * @test Verifies that profiles without an application field store an empty,
 * non-null dimension.
 */
TEST_F(LogCacheCatalogTest, WritesEntriesWithoutApplicationField)
{
    const QString file_path =
        m_temporary_directory.filePath(QStringLiteral("source-without-application.log"));
    ASSERT_TRUE(write_file(file_path, QStringLiteral("INFO first\nERROR second\n")));
    const LogParsingProfile profile =
        LogParsingProfile::create_default(QStringLiteral("{level} {message}"));
    const auto identity = LogCacheIdentity::create(file_path, profile);
    ASSERT_TRUE(identity.has_value());
    const QVector<LogEntry> entries = LogParser(profile).parse_file(file_path);
    ASSERT_EQ(entries.size(), 2);
    EXPECT_TRUE(entries.at(0).get_app_name().isNull());

    const QString database_path =
        m_temporary_directory.filePath(QStringLiteral("cache-without-application.sqlite"));
    LogFileCacheDatabase cache(database_path, identity.value());
    ASSERT_TRUE(cache.is_available());
    ASSERT_TRUE(cache.reset_entries());
    EXPECT_TRUE(cache.append_entries(entries));
    EXPECT_TRUE(cache.finalize_writes());
    EXPECT_EQ(cache.get_entry_count(), 2);
}

/**
 * @test Verifies that a per-file database cannot be reused for a different
 * source identity.
 */
TEST_F(LogCacheCatalogTest, RejectsFileCacheOpenedWithDifferentIdentity)
{
    const QString first_path = m_temporary_directory.filePath(QStringLiteral("first.log"));
    const QString second_path = m_temporary_directory.filePath(QStringLiteral("second.log"));
    ASSERT_TRUE(write_file(first_path, QStringLiteral("INFO first App\n")));
    ASSERT_TRUE(write_file(second_path, QStringLiteral("INFO second App\n")));
    const LogParsingProfile profile =
        LogParsingProfile::create_default(QStringLiteral("{level} {message} {app_name}"));
    const auto first_identity = LogCacheIdentity::create(first_path, profile);
    const auto second_identity = LogCacheIdentity::create(second_path, profile);
    ASSERT_TRUE(first_identity.has_value());
    ASSERT_TRUE(second_identity.has_value());
    const QString database_path =
        m_temporary_directory.filePath(QStringLiteral("file-cache.sqlite"));

    {
        LogFileCacheDatabase cache(database_path, first_identity.value());
        ASSERT_TRUE(cache.is_available());
    }

    LogFileCacheDatabase mismatched_cache(database_path, second_identity.value());
    EXPECT_FALSE(mismatched_cache.is_available());
}

/** @test Verifies a finalized file cache can be cloned and extended without altering its prefix. */
TEST_F(LogCacheCatalogTest, ClonesCompleteGenerationAndAppendsSuffixEntries)
{
    const QString file_path = m_temporary_directory.filePath(QStringLiteral("clone-source.log"));
    const LogParsingProfile profile =
        LogParsingProfile::create_default(QStringLiteral("{level} {message} {app_name}"));
    ASSERT_TRUE(write_file(file_path, QStringLiteral("INFO first App\nERROR second App\n")));
    const auto prefix_identity = LogCacheIdentity::create(file_path, profile);
    ASSERT_TRUE(prefix_identity.has_value());
    const QVector<LogEntry> prefix_entries = LogParser(profile).parse_file(file_path);
    ASSERT_EQ(prefix_entries.size(), 2);
    const QString prefix_database = m_temporary_directory.filePath(QStringLiteral("prefix.sqlite"));
    {
        LogFileCacheDatabase cache(prefix_database, prefix_identity.value());
        ASSERT_TRUE(cache.is_available());
        ASSERT_TRUE(cache.reset_entries());
        ASSERT_TRUE(cache.append_entries(prefix_entries));
        ASSERT_TRUE(cache.finalize_writes());
    }

    QFile appended_file(file_path);
    ASSERT_TRUE(appended_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text));
    ASSERT_GT(appended_file.write("WARN third App\n"), 0);
    appended_file.close();
    const auto current_identity = LogCacheIdentity::create(file_path, profile);
    ASSERT_TRUE(current_identity.has_value());
    const QVector<LogEntry> all_entries = LogParser(profile).parse_file(file_path);
    ASSERT_EQ(all_entries.size(), 3);
    const QString destination_database =
        m_temporary_directory.filePath(QStringLiteral("destination.sqlite"));

    ASSERT_TRUE(LogFileCacheDatabase::clone_generation(prefix_database, destination_database,
                                                       current_identity.value()));
    {
        LogFileCacheDatabase cache(destination_database, current_identity.value());
        ASSERT_TRUE(cache.is_available());
        ASSERT_TRUE(cache.begin_append());
        ASSERT_TRUE(cache.append_entries({all_entries.constLast()}));
        ASSERT_TRUE(cache.finalize_writes());
        EXPECT_EQ(cache.get_entry_count(), 3);
    }
    LogFileCacheDatabase prefix_cache(prefix_database, prefix_identity.value());
    ASSERT_TRUE(prefix_cache.is_available());
    EXPECT_EQ(prefix_cache.get_entry_count(), 2);
}
