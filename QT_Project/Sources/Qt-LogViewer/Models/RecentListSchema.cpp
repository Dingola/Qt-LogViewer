/**
 * @file RecentListSchema.cpp
 * @brief Implements data-driven schemas and builders for recent lists (files and sessions).
 */

#include "Qt-LogViewer/Models/RecentListSchema.h"

#include "Qt-LogViewer/Models/RecentItemPresentationMapper.h"
#include "Qt-LogViewer/Services/LocalFileStatus.h"

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
    const RecentItemPresentation presentation =
        RecentItemPresentationMapper::from_recent_log_file(record, file_status);

    row_data.insert(to_role_id(RecentFileRole::FilePath), presentation.identity);
    row_data.insert(to_role_id(RecentFileRole::FileName), presentation.title);
    row_data.insert(to_role_id(RecentFileRole::AppName), record.app_name);
    row_data.insert(to_role_id(RecentFileRole::LastOpened), presentation.last_used);
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
    const RecentItemPresentation presentation =
        RecentItemPresentationMapper::from_recent_session(record);

    row_data.insert(to_role_id(RecentSessionRole::Name), presentation.title);
    row_data.insert(to_role_id(RecentSessionRole::LastOpened), presentation.last_used);
    row_data.insert(to_role_id(RecentSessionRole::Id), presentation.identity);

    return row_data;
}
