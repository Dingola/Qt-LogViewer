/**
 * @file SqlTransactionTest.cpp
 * @brief Verifies scoped commit and rollback behavior for Qt SQL transactions.
 */

#include <gtest/gtest.h>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include "Qt-LogViewer/Sql/SqlTransaction.h"
#include "Qt-LogViewer/Sql/SqliteConnection.h"
#include "QtCommonLib/TestSupport/TestFileSystem.h"

using QtCommonLib::SqliteConnection;
using QtCommonLib::SqlTransaction;
using QtCommonLib::TestFileSystem;

/** @test Verifies an active transaction persists its changes after a successful commit. */
TEST(SqlTransactionTest, CommitsChanges)
{
    TestFileSystem file_system;
    ASSERT_TRUE(file_system.is_valid());
    const QString database_path = file_system.nonexistent_path(QStringLiteral("committed.sqlite"));
    ASSERT_FALSE(database_path.isEmpty());
    SqliteConnection connection(database_path);
    ASSERT_TRUE(connection.is_open());
    QSqlDatabase database = connection.database();
    QSqlQuery setup_query(database);
    ASSERT_TRUE(setup_query.exec(QStringLiteral("CREATE TABLE records(value TEXT NOT NULL)")));
    setup_query.finish();

    {
        SqlTransaction transaction(database);
        ASSERT_TRUE(transaction.is_active());
        QSqlQuery insert_query(database);
        ASSERT_TRUE(insert_query.exec(QStringLiteral("INSERT INTO records(value) VALUES('kept')")));
        insert_query.finish();
        ASSERT_TRUE(transaction.commit());
        EXPECT_FALSE(transaction.is_active());
    }

    QSqlQuery count_query(database);
    ASSERT_TRUE(count_query.exec(QStringLiteral("SELECT COUNT(*) FROM records")));
    ASSERT_TRUE(count_query.next());
    EXPECT_EQ(count_query.value(0).toInt(), 1);
}

/** @test Verifies destruction rolls back a transaction that was not completed explicitly. */
TEST(SqlTransactionTest, RollsBackActiveTransactionOnDestruction)
{
    TestFileSystem file_system;
    ASSERT_TRUE(file_system.is_valid());
    const QString database_path =
        file_system.nonexistent_path(QStringLiteral("automatic-rollback.sqlite"));
    ASSERT_FALSE(database_path.isEmpty());
    SqliteConnection connection(database_path);
    ASSERT_TRUE(connection.is_open());
    QSqlDatabase database = connection.database();
    QSqlQuery setup_query(database);
    ASSERT_TRUE(setup_query.exec(QStringLiteral("CREATE TABLE records(value TEXT NOT NULL)")));
    setup_query.finish();

    {
        SqlTransaction transaction(database);
        ASSERT_TRUE(transaction.is_active());
        QSqlQuery insert_query(database);
        ASSERT_TRUE(
            insert_query.exec(QStringLiteral("INSERT INTO records(value) VALUES('discarded')")));
    }

    QSqlQuery count_query(database);
    ASSERT_TRUE(count_query.exec(QStringLiteral("SELECT COUNT(*) FROM records")));
    ASSERT_TRUE(count_query.next());
    EXPECT_EQ(count_query.value(0).toInt(), 0);
}

/** @test Verifies an active transaction can be rolled back explicitly. */
TEST(SqlTransactionTest, RollsBackChangesExplicitly)
{
    TestFileSystem file_system;
    ASSERT_TRUE(file_system.is_valid());
    const QString database_path =
        file_system.nonexistent_path(QStringLiteral("explicit-rollback.sqlite"));
    ASSERT_FALSE(database_path.isEmpty());
    SqliteConnection connection(database_path);
    ASSERT_TRUE(connection.is_open());
    QSqlDatabase database = connection.database();
    QSqlQuery setup_query(database);
    ASSERT_TRUE(setup_query.exec(QStringLiteral("CREATE TABLE records(value TEXT NOT NULL)")));
    setup_query.finish();

    SqlTransaction transaction(database);
    ASSERT_TRUE(transaction.is_active());
    QSqlQuery insert_query(database);
    ASSERT_TRUE(
        insert_query.exec(QStringLiteral("INSERT INTO records(value) VALUES('discarded')")));
    insert_query.finish();
    ASSERT_TRUE(transaction.rollback());
    EXPECT_FALSE(transaction.is_active());

    QSqlQuery count_query(database);
    ASSERT_TRUE(count_query.exec(QStringLiteral("SELECT COUNT(*) FROM records")));
    ASSERT_TRUE(count_query.next());
    EXPECT_EQ(count_query.value(0).toInt(), 0);
}

/** @test Verifies a failed transaction start is inactive and exposes the database error. */
TEST(SqlTransactionTest, ReportsBeginFailure)
{
    TestFileSystem file_system;
    ASSERT_TRUE(file_system.is_valid());
    const QString database_path =
        file_system.nonexistent_path(QStringLiteral("failed-begin.sqlite"));
    ASSERT_FALSE(database_path.isEmpty());
    SqliteConnection connection(database_path);
    ASSERT_TRUE(connection.is_open());
    QSqlDatabase database = connection.database();
    ASSERT_TRUE(database.transaction());

    SqlTransaction transaction(database);

    EXPECT_FALSE(transaction.is_active());
    EXPECT_NE(transaction.last_error().type(), QSqlError::NoError);
    EXPECT_TRUE(database.rollback());
}
