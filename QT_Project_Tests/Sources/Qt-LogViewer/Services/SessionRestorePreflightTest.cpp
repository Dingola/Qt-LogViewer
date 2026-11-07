#include <gtest/gtest.h>

#include <QDir>
#include <QFileInfo>

#include "Qt-LogViewer/Models/SessionTypes.h"
#include "Qt-LogViewer/Services/SessionRestorePreflight.h"
#include "QtCommonLib/TestSupport/TestFileSystem.h"

/**
 * @file SessionRestorePreflightTest.cpp
 * @brief Verifies unavailable-file preparation for saved session restoration.
 */

/** @test Collects a duplicated unavailable path only once across explorer and view state. */
TEST(SessionRestorePreflightTest, CollectsUniqueUnavailableFiles)
{
    QtCommonLib::TestFileSystem file_system;
    const QString marker =
        file_system.write_text_file(QStringLiteral("marker.txt"), QStringLiteral("marker"));
    const QString missing_path =
        QDir(QFileInfo(marker).absolutePath()).filePath(QStringLiteral("missing.log"));
    SessionViewState view;
    view.loaded_files = {LogFileInfo(missing_path, QStringLiteral("View application"))};
    SessionState state;
    state.explorer_files = {LogFileInfo(missing_path, QStringLiteral("Explorer application"))};
    state.views = {view};

    const QVector<SessionFileIssue> issues = SessionRestorePreflight::collect_file_issues(state);

    ASSERT_EQ(issues.size(), 1);
    EXPECT_EQ(issues.constFirst().file_path, QFileInfo(missing_path).absoluteFilePath());
    EXPECT_EQ(issues.constFirst().availability, QtCommonLib::LocalFileAvailability::Missing);
}

/** @test Replaces a located path consistently without mutating unrelated session data. */
TEST(SessionRestorePreflightTest, ReplacesLocatedFileThroughoutWorkingCopy)
{
    QtCommonLib::TestFileSystem file_system;
    const QString replacement_path =
        file_system.write_text_file(QStringLiteral("replacement.log"), QStringLiteral("log"));
    const QString missing_path =
        QDir(QFileInfo(replacement_path).absolutePath()).filePath(QStringLiteral("missing.log"));
    const QUuid profile_id = QUuid::createUuid();
    SessionViewState view;
    view.loaded_files = {LogFileInfo(missing_path, QStringLiteral("Application"))};
    view.file_parsing_profile_ids.insert(missing_path, profile_id);
    view.filters.show_only_file = missing_path;
    view.filters.hidden_files.insert(missing_path);
    SessionState state;
    state.explorer_files = {LogFileInfo(missing_path, QStringLiteral("Application"))};
    state.views = {view};

    const bool changed =
        SessionRestorePreflight::replace_file(state, missing_path, replacement_path);

    ASSERT_TRUE(changed);
    ASSERT_EQ(state.explorer_files.size(), 1);
    EXPECT_EQ(state.explorer_files.constFirst().get_file_path(), replacement_path);
    ASSERT_EQ(state.views.size(), 1);
    ASSERT_EQ(state.views.constFirst().loaded_files.size(), 1);
    EXPECT_EQ(state.views.constFirst().loaded_files.constFirst().get_file_path(), replacement_path);
    EXPECT_EQ(state.views.constFirst().file_parsing_profile_ids.value(replacement_path),
              profile_id);
    EXPECT_EQ(state.views.constFirst().filters.show_only_file, replacement_path);
    EXPECT_TRUE(state.views.constFirst().filters.hidden_files.contains(replacement_path));
    EXPECT_TRUE(SessionRestorePreflight::collect_file_issues(state).isEmpty());
}

/** @test Removes unavailable paths while leaving the surrounding view restorable. */
TEST(SessionRestorePreflightTest, RemovesUnavailableFileFromWorkingCopy)
{
    QtCommonLib::TestFileSystem file_system;
    const QString available_path =
        file_system.write_text_file(QStringLiteral("available.log"), QStringLiteral("log"));
    const QString missing_path =
        QDir(QFileInfo(available_path).absolutePath()).filePath(QStringLiteral("missing.log"));
    SessionViewState view;
    view.loaded_files = {LogFileInfo(missing_path, QStringLiteral("Missing")),
                         LogFileInfo(available_path, QStringLiteral("Available"))};
    view.file_parsing_profile_ids.insert(missing_path, QUuid::createUuid());
    SessionState state;
    state.explorer_files = view.loaded_files;
    state.views = {view};

    SessionRestorePreflight::remove_file(state, missing_path);

    ASSERT_EQ(state.explorer_files.size(), 1);
    EXPECT_EQ(state.explorer_files.constFirst().get_file_path(), available_path);
    ASSERT_EQ(state.views.constFirst().loaded_files.size(), 1);
    EXPECT_EQ(state.views.constFirst().loaded_files.constFirst().get_file_path(), available_path);
    EXPECT_FALSE(state.views.constFirst().file_parsing_profile_ids.contains(missing_path));
}
