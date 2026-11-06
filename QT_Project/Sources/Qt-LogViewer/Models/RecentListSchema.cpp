/**
 * @file RecentListSchema.cpp
 * @brief Implements data-driven schemas and builders for recent lists (files and sessions).
 */

#include "Qt-LogViewer/Models/RecentListSchema.h"

#include <QCoreApplication>
#include <QFileInfo>

#include "Qt-LogViewer/Services/LocalFileStatus.h"

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
 * @struct FileAvailabilityPresentation
 * @brief Contains localized presentation values derived from a local file status.
 */
struct FileAvailabilityPresentation {
        QString status_text;
        QString tooltip;
        bool enabled{false};
};

/**
 * @brief Translates presentation text for recent-item schemas.
 * @param source_text Source text registered in the translation catalog.
 * @return Localized text.
 */
[[nodiscard]] auto translate(const char* source_text) -> QString
{
    const QString result = QCoreApplication::translate(k_translation_context, source_text);
    return result;
}

/**
 * @brief Maps a filesystem status to localized, presentation-only values.
 * @param file_status Current status of the recent file.
 * @return Status text, tooltip, and activation state.
 */
[[nodiscard]] auto make_availability_presentation(const QtCommonLib::LocalFileStatus& file_status)
    -> FileAvailabilityPresentation
{
    FileAvailabilityPresentation result;
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
        break;
    case QtCommonLib::LocalFileAvailability::NotReadable:
        result.status_text = translate(k_not_readable_text);
        result.tooltip = translate(k_not_readable_tooltip).arg(file_status.normalized_path);
        break;
    case QtCommonLib::LocalFileAvailability::NotAFile:
        result.status_text = translate(k_not_a_file_text);
        result.tooltip = translate(k_not_a_file_tooltip).arg(file_status.normalized_path);
        break;
    }
    return result;
}
}  // namespace

/**
 * @brief Create a schema for recent log files. Roles: file_path, file_name, app_name, last_opened.
 * @return The recent files schema.
 */
auto RecentListSchemas::make_recent_files_schema() -> RecentListSchema
{
    RecentListSchema files_schema;

    files_schema.role_names.insert(to_role_id(RecentFileRole::FilePath), QByteArray("file_path"));
    files_schema.role_names.insert(to_role_id(RecentFileRole::FileName), QByteArray("file_name"));
    files_schema.role_names.insert(to_role_id(RecentFileRole::AppName), QByteArray("app_name"));
    files_schema.role_names.insert(to_role_id(RecentFileRole::LastOpened),
                                   QByteArray("last_opened"));
    files_schema.role_names.insert(to_role_id(RecentFileRole::Availability),
                                   QByteArray("availability"));
    files_schema.role_names.insert(to_role_id(RecentFileRole::StatusText),
                                   QByteArray("status_text"));
    files_schema.role_names.insert(to_role_id(RecentFileRole::Enabled), QByteArray("enabled"));
    files_schema.enabled_role = to_role_id(RecentFileRole::Enabled);

    // Present a compact list view (name, path, date)
    files_schema.columns = {
        RecentListSchemaColumn{to_role_id(RecentFileRole::FileName), QStringLiteral("File Name")},
        RecentListSchemaColumn{to_role_id(RecentFileRole::FilePath), QStringLiteral("File Path")},
        RecentListSchemaColumn{to_role_id(RecentFileRole::LastOpened),
                               QStringLiteral("Last Opened")},
    };

    return files_schema;
}

/**
 * @brief Create a schema for recent sessions. Roles: name, last_opened, id.
 * @return The recent sessions schema.
 */
auto RecentListSchemas::make_recent_sessions_schema() -> RecentListSchema
{
    RecentListSchema sessions_schema;

    sessions_schema.role_names.insert(to_role_id(RecentSessionRole::Name), QByteArray("name"));
    sessions_schema.role_names.insert(to_role_id(RecentSessionRole::LastOpened),
                                      QByteArray("last_opened"));
    sessions_schema.role_names.insert(to_role_id(RecentSessionRole::Id), QByteArray("id"));

    sessions_schema.columns = {
        RecentListSchemaColumn{to_role_id(RecentSessionRole::Name), QStringLiteral("Session Name")},
        RecentListSchemaColumn{to_role_id(RecentSessionRole::LastOpened),
                               QStringLiteral("Last Opened")},
        RecentListSchemaColumn{to_role_id(RecentSessionRole::Id), QStringLiteral("Session Id")},
    };

    return sessions_schema;
}

/**
 * @brief Convert a recent-file record and runtime status into a presentation row.
 * @param record The recent log file record to convert.
 * @param file_status Current, non-persisted local file status.
 * @return Role->QVariant map representing the row.
 */
auto RecentListSchemas::build_recent_file_row(const RecentLogFileRecord& record,
                                              const QtCommonLib::LocalFileStatus& file_status)
    -> QHash<int, QVariant>
{
    QHash<int, QVariant> row_data;
    const QString derived_file_name = QFileInfo(record.file_path).fileName();
    const FileAvailabilityPresentation presentation = make_availability_presentation(file_status);

    row_data.insert(to_role_id(RecentFileRole::FilePath), record.file_path);
    row_data.insert(to_role_id(RecentFileRole::FileName), derived_file_name);
    row_data.insert(to_role_id(RecentFileRole::AppName), record.app_name);
    row_data.insert(to_role_id(RecentFileRole::LastOpened), record.last_opened);
    row_data.insert(to_role_id(RecentFileRole::Availability),
                    static_cast<int>(file_status.availability));
    row_data.insert(to_role_id(RecentFileRole::StatusText), presentation.status_text);
    row_data.insert(to_role_id(RecentFileRole::Enabled), presentation.enabled);
    row_data.insert(Qt::ToolTipRole, presentation.tooltip);
    row_data.insert(Qt::AccessibleDescriptionRole, presentation.tooltip);

    return row_data;
}

/**
 * @brief Convert a RecentSessionRecord into a role->QVariant row per the sessions schema.
 * @param record The recent session record to convert.
 * @return Role->QVariant map representing the row.
 */
auto RecentListSchemas::build_recent_session_row(const RecentSessionRecord& record)
    -> QHash<int, QVariant>
{
    QHash<int, QVariant> row_data;
    const QDateTime effective_last_opened =
        record.last_opened.isValid() ? record.last_opened : record.created_at;

    row_data.insert(to_role_id(RecentSessionRole::Name), record.name);
    row_data.insert(to_role_id(RecentSessionRole::LastOpened), effective_last_opened);
    row_data.insert(to_role_id(RecentSessionRole::Id), record.id);

    return row_data;
}
