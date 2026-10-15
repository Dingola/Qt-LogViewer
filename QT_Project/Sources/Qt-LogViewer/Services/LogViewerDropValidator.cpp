#include "Qt-LogViewer/Services/LogViewerDropValidator.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QMimeData>

#include "Qt-LogViewer/Services/LocalFileInputValidator.h"
#include "Qt-LogViewer/Services/SessionCodec.h"

/**
 * @file LogViewerDropValidator.cpp
 * @brief Implements Qt-LogViewer validation and classification for dragged files.
 */

/**
 * @brief Reports whether the validation result contains actionable content.
 * @return True when at least one accepted path can be processed.
 */
auto LogViewerDropValidationResult::is_accepted() const -> bool
{
    const bool result = content_type != LogViewerDropContentType::None && !accepted_paths.isEmpty();
    return result;
}

/**
 * @brief Validates and classifies URLs from drag-and-drop MIME data.
 * @param mime_data MIME data supplied by the drag operation.
 * @param loaded_file_paths Files already present in the active session.
 * @return Normalized accepted paths, content type, and rejected entries.
 */
auto LogViewerDropValidator::validate(const QMimeData& mime_data,
                                      const QSet<QString>& loaded_file_paths)
    -> LogViewerDropValidationResult
{
    LogViewerDropValidationResult result;
    QVector<QString> log_files;
    QVector<QString> session_files;
    const LocalFileInputValidationResult file_validation =
        LocalFileInputValidator::validate(mime_data.urls(), loaded_file_paths);
    result.rejected_entries = file_validation.rejected_entries;

    for (const QString& file_path: file_validation.accepted_files)
    {
        if (is_supported_log_file(file_path))
        {
            log_files.append(file_path);
        }
        else if (is_session_file(file_path))
        {
            session_files.append(file_path);
        }
        else
        {
            result.rejected_entries.append(file_path);
        }
    }

    const bool mixes_content_types = !log_files.isEmpty() && !session_files.isEmpty();
    const bool has_multiple_sessions = session_files.size() > 1;
    if (mixes_content_types || has_multiple_sessions)
    {
        result.rejected_entries.append(log_files);
        result.rejected_entries.append(session_files);
    }
    else if (!session_files.isEmpty())
    {
        result.content_type = LogViewerDropContentType::SessionFile;
        result.accepted_paths = session_files;
    }
    else if (!log_files.isEmpty())
    {
        result.content_type = LogViewerDropContentType::LogFiles;
        result.accepted_paths = log_files;
    }

    return result;
}

/**
 * @brief Checks whether a file uses a supported log-file suffix.
 * @param file_path Normalized file path.
 * @return True for .log and .txt files.
 */
auto LogViewerDropValidator::is_supported_log_file(const QString& file_path) -> bool
{
    const QString suffix = QFileInfo(file_path).suffix();
    const bool result = suffix.compare(QStringLiteral("log"), Qt::CaseInsensitive) == 0 ||
                        suffix.compare(QStringLiteral("txt"), Qt::CaseInsensitive) == 0;
    return result;
}

/**
 * @brief Checks whether a JSON file contains a supported session document.
 * @param file_path Normalized JSON file path.
 * @return True when the document can be recognized as a session.
 */
auto LogViewerDropValidator::is_session_file(const QString& file_path) -> bool
{
    const bool has_json_suffix =
        QFileInfo(file_path).suffix().compare(QStringLiteral("json"), Qt::CaseInsensitive) == 0;
    QFile file(file_path);
    bool result = false;
    if (has_json_suffix && file.open(QIODevice::ReadOnly))
    {
        QJsonParseError parse_error;
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parse_error);
        result = parse_error.error == QJsonParseError::NoError && document.isObject() &&
                 SessionCodec::is_session_document(document.object());
    }
    return result;
}
