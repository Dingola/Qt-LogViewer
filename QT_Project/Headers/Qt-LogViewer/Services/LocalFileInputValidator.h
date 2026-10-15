#pragma once

#include <QList>
#include <QSet>
#include <QStringList>
#include <QVector>

class QUrl;

/**
 * @file LocalFileInputValidator.h
 * @brief Declares transport-independent validation for local file URL candidates.
 */

/**
 * @brief Contains normalized local files and rejected input entries.
 */
struct LocalFileInputValidationResult {
        /** @brief Canonical readable local file paths accepted by generic validation. */
        QVector<QString> accepted_files;
        /** @brief Display values rejected as invalid, non-local, duplicate, or excluded. */
        QStringList rejected_entries;
};

/**
 * @class LocalFileInputValidator
 * @brief Validates local file URL candidates without application-specific format knowledge.
 */
class LocalFileInputValidator
{
    public:
        /**
         * @brief Validates local file URL candidates from any input source.
         * @param candidates URLs supplied by a drop, dialog, clipboard, or another source.
         * @param excluded_file_paths Files that must not be accepted again.
         * @return Canonical readable files and rejected input entries.
         */
        [[nodiscard]] static auto validate(const QList<QUrl>& candidates,
                                           const QSet<QString>& excluded_file_paths = {})
            -> LocalFileInputValidationResult;

    private:
        /**
         * @brief Resolves a readable local file to a stable canonical path.
         * @param path Candidate local path.
         * @return Canonical path, or an empty string when the file cannot be read.
         */
        [[nodiscard]] static auto normalize_readable_file(const QString& path) -> QString;
};
