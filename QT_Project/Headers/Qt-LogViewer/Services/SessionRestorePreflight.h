#pragma once

#include <QString>
#include <QVector>

#include "Qt-LogViewer/Models/SessionTypes.h"
#include "QtCommonLib/FileSystem/LocalFileStatus.h"

/**
 * @file SessionRestorePreflight.h
 * @brief Declares non-mutating preparation helpers for restoring saved sessions.
 */

/**
 * @struct SessionFileIssue
 * @brief Describes one unavailable source referenced by a saved session.
 */
struct SessionFileIssue {
        QString file_path;
        QtCommonLib::LocalFileAvailability availability{
            QtCommonLib::LocalFileAvailability::Missing};
};

namespace SessionRestorePreflight
{
/**
 * @brief Collects unique unavailable files referenced by a session.
 * @param state Saved session state to inspect.
 * @return Unavailable source paths in stable session order.
 */
[[nodiscard]] auto collect_file_issues(const SessionState& state) -> QVector<SessionFileIssue>;

/**
 * @brief Removes one unavailable source from a session working copy.
 * @param state Session state to update.
 * @param file_path Source path to remove.
 */
auto remove_file(SessionState& state, const QString& file_path) -> void;

/**
 * @brief Replaces one source path throughout a session working copy.
 * @param state Session state to update.
 * @param previous_path Persisted path to replace.
 * @param replacement_path Available replacement path.
 * @return True when at least one session reference changed.
 */
[[nodiscard]] auto replace_file(SessionState& state, const QString& previous_path,
                                const QString& replacement_path) -> bool;
}  // namespace SessionRestorePreflight
