#include "Qt-LogViewer/Services/SessionRestorePreflight.h"

#include <QDir>
#include <QFileInfo>
#include <QSet>

namespace
{
/**
 * @brief Returns a normalized absolute key for comparing local paths.
 * @param file_path Local path to normalize.
 * @return Clean absolute path used as a comparison key.
 */
[[nodiscard]] auto path_key(const QString& file_path) -> QString
{
    QString key = QDir::cleanPath(QFileInfo(file_path).absoluteFilePath());
#ifdef Q_OS_WIN
    key = key.toCaseFolded();
#endif
    return key;
}

/**
 * @brief Tests whether two local paths identify the same normalized path.
 * @param left First path.
 * @param right Second path.
 * @return True when the paths compare equal for the current platform.
 */
[[nodiscard]] auto paths_equal(const QString& left, const QString& right) -> bool
{
    const Qt::CaseSensitivity sensitivity =
#ifdef Q_OS_WIN
        Qt::CaseInsensitive;
#else
        Qt::CaseSensitive;
#endif
    return QString::compare(path_key(left), path_key(right), sensitivity) == 0;
}

/**
 * @brief Replaces matching paths in a list of file descriptions.
 * @param files File descriptions to update.
 * @param previous_path Path to replace.
 * @param replacement_path Replacement path.
 * @return True when a file description changed.
 */
[[nodiscard]] auto replace_file_infos(QList<LogFileInfo>& files, const QString& previous_path,
                                      const QString& replacement_path) -> bool
{
    bool changed = false;
    for (LogFileInfo& file_info: files)
    {
        if (paths_equal(file_info.get_file_path(), previous_path))
        {
            file_info = LogFileInfo(replacement_path, file_info.get_app_name());
            changed = true;
        }
    }
    return changed;
}
}  // namespace

/**
 * @brief Collects unique unavailable files referenced by a session.
 * @param state Saved session state to inspect.
 * @return Unavailable source paths in stable session order.
 */
auto SessionRestorePreflight::collect_file_issues(const SessionState& state)
    -> QVector<SessionFileIssue>
{
    QVector<SessionFileIssue> issues;
    QSet<QString> inspected_paths;
    QList<LogFileInfo> files = state.explorer_files;
    for (const SessionViewState& view: state.views)
    {
        files.append(view.loaded_files);
    }

    for (const LogFileInfo& file_info: files)
    {
        if (!file_info.get_file_path().isEmpty())
        {
            const QtCommonLib::LocalFileStatus status =
                QtCommonLib::inspect_local_file(file_info.get_file_path());
            const QString key = path_key(status.normalized_path);
            if (!inspected_paths.contains(key))
            {
                inspected_paths.insert(key);
                if (status.availability != QtCommonLib::LocalFileAvailability::Available)
                {
                    issues.append(SessionFileIssue{status.normalized_path, status.availability});
                }
            }
        }
    }
    return issues;
}

/**
 * @brief Removes one unavailable source from a session working copy.
 * @param state Session state to update.
 * @param file_path Source path to remove.
 */
auto SessionRestorePreflight::remove_file(SessionState& state, const QString& file_path) -> void
{
    state.explorer_files.removeIf([&file_path](const LogFileInfo& file_info) {
        return paths_equal(file_info.get_file_path(), file_path);
    });
    for (SessionViewState& view: state.views)
    {
        view.loaded_files.removeIf([&file_path](const LogFileInfo& file_info) {
            return paths_equal(file_info.get_file_path(), file_path);
        });
        const QList<QString> profile_paths = view.file_parsing_profile_ids.keys();
        for (const QString& profile_path: profile_paths)
        {
            if (paths_equal(profile_path, file_path))
            {
                view.file_parsing_profile_ids.remove(profile_path);
            }
        }
        const QSet<QString> hidden_files = view.filters.hidden_files;
        for (const QString& hidden_file: hidden_files)
        {
            if (paths_equal(hidden_file, file_path))
            {
                view.filters.hidden_files.remove(hidden_file);
            }
        }
        if (paths_equal(view.filters.show_only_file, file_path))
        {
            view.filters.show_only_file.clear();
        }
    }
}

/**
 * @brief Replaces one source path throughout a session working copy.
 * @param state Session state to update.
 * @param previous_path Persisted path to replace.
 * @param replacement_path Available replacement path.
 * @return True when at least one session reference changed.
 */
auto SessionRestorePreflight::replace_file(SessionState& state, const QString& previous_path,
                                           const QString& replacement_path) -> bool
{
    bool changed = replace_file_infos(state.explorer_files, previous_path, replacement_path);
    for (SessionViewState& view: state.views)
    {
        changed = replace_file_infos(view.loaded_files, previous_path, replacement_path) || changed;
        const QList<QString> profile_paths = view.file_parsing_profile_ids.keys();
        for (const QString& profile_path: profile_paths)
        {
            if (paths_equal(profile_path, previous_path))
            {
                const QUuid profile_id = view.file_parsing_profile_ids.take(profile_path);
                view.file_parsing_profile_ids.insert(replacement_path, profile_id);
                changed = true;
            }
        }
        const QSet<QString> hidden_files = view.filters.hidden_files;
        for (const QString& hidden_file: hidden_files)
        {
            if (paths_equal(hidden_file, previous_path))
            {
                view.filters.hidden_files.remove(hidden_file);
                view.filters.hidden_files.insert(replacement_path);
                changed = true;
            }
        }
        if (paths_equal(view.filters.show_only_file, previous_path))
        {
            view.filters.show_only_file = replacement_path;
            changed = true;
        }
    }
    return changed;
}
