#include "Qt-LogViewer/Services/LocalFileInputValidator.h"

#include <QFileInfo>
#include <QUrl>

/**
 * @file LocalFileInputValidator.cpp
 * @brief Implements transport-independent validation for local file URL candidates.
 */

/**
 * @brief Validates local file URL candidates from any input source.
 * @param candidates URLs supplied by a drop, dialog, clipboard, or another source.
 * @param excluded_file_paths Files that must not be accepted again.
 * @return Canonical readable files and rejected input entries.
 */
auto LocalFileInputValidator::validate(const QList<QUrl>& candidates,
                                       const QSet<QString>& excluded_file_paths)
    -> LocalFileInputValidationResult
{
    LocalFileInputValidationResult result;
    QSet<QString> normalized_excluded_paths;
    QSet<QString> seen_paths;

    for (const QString& excluded_path: excluded_file_paths)
    {
        const QString normalized_path = normalize_readable_file(excluded_path);
        if (!normalized_path.isEmpty())
        {
            normalized_excluded_paths.insert(normalized_path);
        }
    }

    for (const QUrl& candidate: candidates)
    {
        const QString display_value = candidate.toDisplayString();
        const QString normalized_path =
            candidate.isLocalFile() ? normalize_readable_file(candidate.toLocalFile()) : QString();
        const bool accepted = !normalized_path.isEmpty() &&
                              !normalized_excluded_paths.contains(normalized_path) &&
                              !seen_paths.contains(normalized_path);
        if (accepted)
        {
            seen_paths.insert(normalized_path);
            result.accepted_files.append(normalized_path);
        }
        else
        {
            result.rejected_entries.append(display_value);
        }
    }

    return result;
}

/**
 * @brief Resolves a readable local file to a stable canonical path.
 * @param path Candidate local path.
 * @return Canonical path, or an empty string when the file cannot be read.
 */
auto LocalFileInputValidator::normalize_readable_file(const QString& path) -> QString
{
    const QFileInfo file_info(path);
    QString result;
    if (file_info.exists() && file_info.isFile() && file_info.isReadable())
    {
        result = file_info.canonicalFilePath();
    }
    return result;
}
