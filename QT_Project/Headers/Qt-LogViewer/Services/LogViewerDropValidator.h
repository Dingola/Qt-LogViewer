#pragma once

#include <QSet>
#include <QStringList>
#include <QVector>

class QMimeData;

/**
 * @file LogViewerDropValidator.h
 * @brief Declares Qt-LogViewer validation and classification for dragged files.
 */

/**
 * @brief Describes the unambiguous action represented by accepted dropped files.
 */
enum class LogViewerDropContentType
{
    None,
    LogFiles,
    SessionFile
};

/**
 * @brief Contains normalized accepted paths and rejected drag entries.
 */
struct LogViewerDropValidationResult {
        /** @brief Actionable content type after validation. */
        LogViewerDropContentType content_type{LogViewerDropContentType::None};
        /** @brief Canonical local paths accepted for the classified action. */
        QVector<QString> accepted_paths;
        /** @brief Display values rejected as invalid, duplicate, loaded, or ambiguous. */
        QStringList rejected_entries;

        /**
         * @brief Reports whether the drag contains an actionable, unambiguous payload.
         * @return True when at least one accepted path can be processed.
         */
        [[nodiscard]] auto is_accepted() const -> bool;
};

/**
 * @class LogViewerDropValidator
 * @brief Applies application-specific log and session drop rules without depending on a widget.
 */
class LogViewerDropValidator final
{
    public:
        /**
         * @brief Validates and classifies URLs from drag-and-drop MIME data.
         * @param mime_data MIME data supplied by the drag operation.
         * @param loaded_file_paths Files already present in the active session.
         * @return Normalized accepted paths, content type, and rejected entries.
         */
        [[nodiscard]] static auto validate(const QMimeData& mime_data,
                                           const QSet<QString>& loaded_file_paths = {})
            -> LogViewerDropValidationResult;

    private:
        /**
         * @brief Checks whether a file uses a supported log-file suffix.
         * @param file_path Normalized file path.
         * @return True for .log and .txt files.
         */
        [[nodiscard]] static auto is_supported_log_file(const QString& file_path) -> bool;

        /**
         * @brief Checks whether a JSON file contains a supported session document.
         * @param file_path Normalized JSON file path.
         * @return True when the document can be recognized as a session.
         */
        [[nodiscard]] static auto is_session_file(const QString& file_path) -> bool;
};
