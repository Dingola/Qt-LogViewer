/**
 * @file RecentItemPresentationMapper.cpp
 * @brief Implements Qt-LogViewer recent-record presentation mappings.
 */

#include "Qt-LogViewer/Models/RecentItemPresentationMapper.h"

#include <QCoreApplication>
#include <QFileInfo>

#include "QtCommonLib/FileSystem/LocalFileStatus.h"

using QtWidgetsCommonLib::RecentItemPresentation;

namespace
{
constexpr auto k_translation_context = "RecentListSchemas";
constexpr auto k_available_text = QT_TRANSLATE_NOOP("RecentListSchemas", "Available");
constexpr auto k_available_tooltip = QT_TRANSLATE_NOOP("RecentListSchemas", "Available: %1");
constexpr auto k_missing_text = QT_TRANSLATE_NOOP("RecentListSchemas", "File not found");
constexpr auto k_missing_tooltip = QT_TRANSLATE_NOOP("RecentListSchemas", "File not found: %1");
constexpr auto k_not_readable_text = QT_TRANSLATE_NOOP("RecentListSchemas", "File is not readable");
constexpr auto k_not_readable_tooltip =
    QT_TRANSLATE_NOOP("RecentListSchemas", "File is not readable: %1");
constexpr auto k_not_a_file_text = QT_TRANSLATE_NOOP("RecentListSchemas", "Path is not a file");
constexpr auto k_not_a_file_tooltip =
    QT_TRANSLATE_NOOP("RecentListSchemas", "Path is not a file: %1");

/**
 * @brief Translates presentation text for recent log-file states.
 * @param source_text Source text registered in the translation catalog.
 * @return Localized presentation text.
 */
[[nodiscard]] auto translate(const char* source_text) -> QString
{
    const QString result = QCoreApplication::translate(k_translation_context, source_text);
    return result;
}
}  // namespace

/**
 * @brief Maps a recent log-file record and its transient file status.
 * @param record Persisted recent log-file record.
 * @param file_status Current local filesystem status.
 * @return Presentation values for the recent log file.
 */
auto RecentItemPresentationMapper::from_recent_log_file(
    const RecentLogFileRecord& record,
    const QtCommonLib::LocalFileStatus& file_status) -> RecentItemPresentation
{
    RecentItemPresentation result;
    result.identity = record.file_path;
    result.title = QFileInfo(record.file_path).fileName();
    result.secondary_text = record.file_path;
    result.last_used = record.last_opened;

    switch (file_status.availability)
    {
    case QtCommonLib::LocalFileAvailability::Available:
        result.status_text = translate(k_available_text);
        result.tooltip = translate(k_available_tooltip).arg(file_status.normalized_path);
        result.enabled = true;
        break;
    case QtCommonLib::LocalFileAvailability::Missing:
        result.status_text = translate(k_missing_text);
        result.tooltip = translate(k_missing_tooltip).arg(file_status.normalized_path);
        result.enabled = false;
        break;
    case QtCommonLib::LocalFileAvailability::NotReadable:
        result.status_text = translate(k_not_readable_text);
        result.tooltip = translate(k_not_readable_tooltip).arg(file_status.normalized_path);
        result.enabled = false;
        break;
    case QtCommonLib::LocalFileAvailability::NotAFile:
        result.status_text = translate(k_not_a_file_text);
        result.tooltip = translate(k_not_a_file_tooltip).arg(file_status.normalized_path);
        result.enabled = false;
        break;
    }

    return result;
}

/**
 * @brief Maps a recent session record.
 * @param record Persisted recent session record.
 * @return Presentation values for the recent session.
 */
auto RecentItemPresentationMapper::from_recent_session(const RecentSessionRecord& record)
    -> RecentItemPresentation
{
    RecentItemPresentation result;
    result.identity = record.id;
    result.title = record.name;
    result.secondary_text = record.id;
    result.last_used = record.last_opened.isValid() ? record.last_opened : record.created_at;
    return result;
}
