/**
 * @file TestFileSystemTest.cpp
 * @brief Implements tests for the isolated test filesystem helper.
 */

#include "Qt-LogViewer/TestSupport/TestFileSystemTest.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>

#include "Qt-LogViewer/TestSupport/TestFileSystem.h"

/** @test Verifies that every helper instance owns a different temporary root. */
TEST_F(TestFileSystemTest, CreatesIsolatedRoot)
{
    TestFileSystem first;
    TestFileSystem second;

    ASSERT_TRUE(first.is_valid());
    ASSERT_TRUE(second.is_valid());
    EXPECT_NE(first.root_path(), second.root_path());
    EXPECT_TRUE(QDir(first.root_path()).exists());
    EXPECT_TRUE(QDir(second.root_path()).exists());
}

/** @test Verifies UTF-8 text writing and automatic creation of parent directories. */
TEST_F(TestFileSystemTest, WritesTextFileBelowRoot)
{
    TestFileSystem file_system;
    const QString contents = QStringLiteral("first line\nGrüße aus dem Test\n");

    const QString file_path =
        file_system.write_text_file(QStringLiteral("logs/nested/sample.log"), contents);

    ASSERT_FALSE(file_path.isEmpty());
    QFile file(file_path);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    EXPECT_EQ(QString::fromUtf8(file.readAll()), contents);
    EXPECT_EQ(QDir(file_system.root_path()).relativeFilePath(file_path),
              QStringLiteral("logs/nested/sample.log"));
}

/** @test Verifies that text is appended to an existing owned file. */
TEST_F(TestFileSystemTest, AppendsTextToOwnedFile)
{
    TestFileSystem file_system;
    const QString file_path =
        file_system.write_text_file(QStringLiteral("append.log"), QStringLiteral("first\n"));
    ASSERT_FALSE(file_path.isEmpty());

    ASSERT_TRUE(file_system.append_text(file_path, QStringLiteral("second\n")));

    QFile file(file_path);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    EXPECT_EQ(QString::fromUtf8(file.readAll()), QStringLiteral("first\nsecond\n"));
}

/** @test Verifies that appending outside the isolated root is rejected. */
TEST_F(TestFileSystemTest, RejectsAppendOutsideRoot)
{
    TestFileSystem file_system;
    const QString outside_path =
        QFileInfo(QDir::temp().filePath(QStringLiteral("outside-test-file.log")))
            .absoluteFilePath();

    EXPECT_FALSE(file_system.append_text(outside_path, QStringLiteral("content")));
}

/** @test Verifies creation of unique paths without creating filesystem entries. */
TEST_F(TestFileSystemTest, ProvidesUniqueNonexistentPaths)
{
    TestFileSystem file_system;

    const QString first_path = file_system.nonexistent_path(QStringLiteral("missing/input.log"));
    const QString second_path = file_system.nonexistent_path(QStringLiteral("missing/input.log"));

    ASSERT_FALSE(first_path.isEmpty());
    ASSERT_FALSE(second_path.isEmpty());
    EXPECT_NE(first_path, second_path);
    EXPECT_FALSE(QFileInfo::exists(first_path));
    EXPECT_FALSE(QFileInfo::exists(second_path));
    EXPECT_EQ(QFileInfo(first_path).suffix(), QStringLiteral("log"));
}

/** @test Verifies that absolute and parent-traversal paths cannot escape the root. */
TEST_F(TestFileSystemTest, RejectsPathsOutsideRoot)
{
    TestFileSystem file_system;

    EXPECT_TRUE(file_system
                    .write_text_file(QDir::temp().filePath(QStringLiteral("absolute.log")),
                                     QStringLiteral("content"))
                    .isEmpty());
    EXPECT_TRUE(
        file_system.write_text_file(QStringLiteral("../escaped.log"), QStringLiteral("content"))
            .isEmpty());
    EXPECT_TRUE(file_system.nonexistent_path(QStringLiteral("../missing.log")).isEmpty());
}

/** @test Verifies that destruction removes the complete isolated directory tree. */
TEST_F(TestFileSystemTest, RemovesRootOnDestruction)
{
    QString root_path;
    {
        TestFileSystem file_system;
        root_path = file_system.root_path();
        ASSERT_FALSE(
            file_system
                .write_text_file(QStringLiteral("nested/file.log"), QStringLiteral("temporary"))
                .isEmpty());
        ASSERT_TRUE(QDir(root_path).exists());
    }

    EXPECT_FALSE(QFileInfo::exists(root_path));
}
