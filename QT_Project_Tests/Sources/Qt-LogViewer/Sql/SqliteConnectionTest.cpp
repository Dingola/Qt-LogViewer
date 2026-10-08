/**
 * @file SqliteConnectionTest.cpp
 * @brief Verifies reusable SQLite connection ownership and initialization behavior.
 */

#include <gtest/gtest.h>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include "Qt-LogViewer/Sql/SqliteConnection.h"
#include "QtCommonLib/TestSupport/TestFileSystem.h"

using QtCommonLib::SqliteConnection;
using QtCommonLib::SqliteConnectionOptions;
using QtCommonLib::TestFileSystem;

/** @test Verifies caller-provided options and initialization statements are applied. */
TEST(SqliteConnectionTest, OpensAndInitializesDatabase)
{
    TestFileSystem file_system;
    ASSERT_TRUE(file_system.is_valid());
    const QString database_path = file_system.nonexistent_path(QStringLiteral("configured.sqlite"));
    ASSERT_FALSE(database_path.isEmpty());
    SqliteConnectionOptions options;
    options.connection_name_prefix = QStringLiteral("configured_connection");
    options.connect_options = QStringLiteral("QSQLITE_BUSY_TIMEOUT=2500");
    options.initialization_statements = {QStringLiteral("PRAGMA foreign_keys=ON"),
                                         QStringLiteral("PRAGMA busy_timeout=2500")};

    SqliteConnection connection(database_path, options);

    ASSERT_TRUE(connection.is_open());
    EXPECT_TRUE(connection.connection_name().startsWith(QStringLiteral("configured_connection_")));
    EXPECT_EQ(connection.last_error().type(), QSqlError::NoError);
    QSqlDatabase database = connection.database();
    ASSERT_TRUE(database.isValid());
    EXPECT_EQ(database.connectionName(), connection.connection_name());

    QSqlQuery query(database);
    ASSERT_TRUE(query.exec(QStringLiteral("PRAGMA foreign_keys")));
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 1);
    query.finish();
    ASSERT_TRUE(query.exec(QStringLiteral("PRAGMA busy_timeout")));
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 2500);
}

/** @test Verifies separate owners receive distinct connection names for one database. */
TEST(SqliteConnectionTest, GeneratesUniqueConnectionNames)
{
    TestFileSystem file_system;
    ASSERT_TRUE(file_system.is_valid());
    const QString database_path = file_system.nonexistent_path(QStringLiteral("shared.sqlite"));
    ASSERT_FALSE(database_path.isEmpty());

    SqliteConnection first_connection(database_path);
    SqliteConnection second_connection(database_path);

    ASSERT_TRUE(first_connection.is_open());
    ASSERT_TRUE(second_connection.is_open());
    EXPECT_NE(first_connection.connection_name(), second_connection.connection_name());
}

/** @test Verifies destruction closes and unregisters the owned Qt SQL connection. */
TEST(SqliteConnectionTest, UnregistersConnectionOnDestruction)
{
    TestFileSystem file_system;
    ASSERT_TRUE(file_system.is_valid());
    const QString database_path = file_system.nonexistent_path(QStringLiteral("lifetime.sqlite"));
    ASSERT_FALSE(database_path.isEmpty());
    QString connection_name;
    {
        SqliteConnection connection(database_path);
        ASSERT_TRUE(connection.is_open());
        connection_name = connection.connection_name();
        EXPECT_TRUE(QSqlDatabase::contains(connection_name));
    }
    EXPECT_FALSE(QSqlDatabase::contains(connection_name));
}

/** @test Verifies a failed initialization statement closes the connection and reports its error. */
TEST(SqliteConnectionTest, ReportsInitializationFailure)
{
    TestFileSystem file_system;
    ASSERT_TRUE(file_system.is_valid());
    const QString database_path =
        file_system.nonexistent_path(QStringLiteral("invalid-initialization.sqlite"));
    ASSERT_FALSE(database_path.isEmpty());
    SqliteConnectionOptions options;
    options.initialization_statements = {QStringLiteral("INVALID SQL")};

    SqliteConnection connection(database_path, options);

    EXPECT_FALSE(connection.is_open());
    EXPECT_NE(connection.last_error().type(), QSqlError::NoError);
    EXPECT_TRUE(QSqlDatabase::contains(connection.connection_name()));
}

/** @test Verifies an SQLite open failure is exposed and cleaned up with the owner. */
TEST(SqliteConnectionTest, ReportsOpenFailure)
{
    TestFileSystem file_system;
    ASSERT_TRUE(file_system.is_valid());
    QString connection_name;
    {
        SqliteConnection connection(file_system.root_path());
        connection_name = connection.connection_name();
        EXPECT_FALSE(connection.is_open());
        EXPECT_NE(connection.last_error().type(), QSqlError::NoError);
        EXPECT_TRUE(QSqlDatabase::contains(connection_name));
    }
    EXPECT_FALSE(QSqlDatabase::contains(connection_name));
}
