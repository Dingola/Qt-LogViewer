/**
 * @file RecentListSchema.cpp
 * @brief Implements data-driven schemas and builders for recent lists (files and sessions).
 */

#include "Qt-LogViewer/Models/RecentListSchema.h"

#include <QFileInfo>

/**
 * @brief Create a schema for recent log files.
 *        Roles: file_path, file_name, app_name, last_opened.
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
 * @brief Create a schema for recent sessions.
 *        Roles: name, last_opened, id.
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
 * @brief Convert a RecentLogFileRecord into a role->QVariant row per the files schema.
 * @param record The recent log file record to convert.
 * @return Role->QVariant map representing the row.
 */
auto RecentListSchemas::build_recent_file_row(const RecentLogFileRecord& record)
    -> QHash<int, QVariant>
{
    QHash<int, QVariant> row_data;
    const QString derived_file_name = QFileInfo(record.file_path).fileName();

    row_data.insert(to_role_id(RecentFileRole::FilePath), record.file_path);
    row_data.insert(to_role_id(RecentFileRole::FileName), derived_file_name);
    row_data.insert(to_role_id(RecentFileRole::AppName), record.app_name);
    row_data.insert(to_role_id(RecentFileRole::LastOpened), record.last_opened);

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
