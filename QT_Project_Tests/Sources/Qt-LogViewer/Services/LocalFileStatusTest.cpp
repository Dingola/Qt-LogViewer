#include "Qt-LogViewer/Services/LocalFileStatusTest.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "Qt-LogViewer/Services/LocalFileStatus.h"

/**
 * @file LocalFileStatusTest.cpp
 * @brief Implements focused tests for portable local file status inspection.
 */

/** @test Reports a readable file and exposes its canonical path. */
TEST_F(LocalFileStatusTest, ReportsReadableFileAsAvailable)
{
    const QString file_path =
        m_file_system.write_text_file(QStringLiteral("available.log"), QStringLiteral("entry"));

    const QtCommonLib::LocalFileStatus status = QtCommonLib::inspect_local_file(file_path);

    EXPECT_EQ(status.requested_path, file_path);
    EXPECT_EQ(status.normalized_path, QFileInfo(file_path).canonicalFilePath());
    EXPECT_EQ(status.availability, QtCommonLib::LocalFileAvailability::Available);
}

/** @test Reports a missing file while retaining a stable absolute fallback path. */
TEST_F(LocalFileStatusTest, ReportsMissingFileWithNormalizedFallback)
{
    const QString missing_path =
        QDir(QFileInfo(m_file_system.write_text_file(QStringLiteral("anchor"), QString())).path())
            .filePath(QStringLiteral("missing.log"));

    const QtCommonLib::LocalFileStatus status = QtCommonLib::inspect_local_file(missing_path);

    EXPECT_EQ(status.requested_path, missing_path);
    EXPECT_EQ(status.normalized_path, QDir::cleanPath(QFileInfo(missing_path).absoluteFilePath()));
    EXPECT_EQ(status.availability, QtCommonLib::LocalFileAvailability::Missing);
}

/** @test Distinguishes an existing directory from a consumable local file. */
TEST_F(LocalFileStatusTest, ReportsDirectoryAsNotAFile)
{
    const QString file_path =
        m_file_system.write_text_file(QStringLiteral("inside.log"), QStringLiteral("entry"));
    const QString directory_path = QFileInfo(file_path).absolutePath();

    const QtCommonLib::LocalFileStatus status = QtCommonLib::inspect_local_file(directory_path);

    EXPECT_EQ(status.normalized_path, QFileInfo(directory_path).canonicalFilePath());
    EXPECT_EQ(status.availability, QtCommonLib::LocalFileAvailability::NotAFile);
}

#ifndef Q_OS_WIN
/** @test Distinguishes an existing file without read permission from a readable file. */
TEST_F(LocalFileStatusTest, ReportsFileWithoutReadPermissionAsNotReadable)
{
    const QString file_path =
        m_file_system.write_text_file(QStringLiteral("unreadable.log"), QStringLiteral("entry"));
    const QFileDevice::Permissions original_permissions = QFile::permissions(file_path);
    ASSERT_TRUE(QFile::setPermissions(file_path, QFileDevice::WriteOwner));

    const QtCommonLib::LocalFileStatus status = QtCommonLib::inspect_local_file(file_path);

    EXPECT_EQ(status.availability, QtCommonLib::LocalFileAvailability::NotReadable);
    EXPECT_TRUE(QFile::setPermissions(file_path, original_permissions));
}
#endif
