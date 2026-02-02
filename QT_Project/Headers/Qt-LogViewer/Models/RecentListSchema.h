#pragma once

#include <QHash>
#include <QVariant>
#include <QVector>

#include "Qt-LogViewer/Models/RecentRoles.h"
#include "Qt-LogViewer/Models/SessionTypes.h"

/**
 * @file RecentListSchema.h
 * @brief Declares a generic, data-driven schema for recent lists and row builders.
 *
 * A `RecentListSchema` describes:
 *  - which roles are exported (`role_names`),
 *  - the model columns (which role is displayed, column header).
 *
 * Builders convert typed records to role->QVariant rows that `RecentItemsModel` can consume.
 */

/**
 * @struct RecentListSchemaColumn
 * @brief Describes a single column in the model (display role + header title).
 */
struct RecentListSchemaColumn {
        int display_role;      ///< Role id whose value is shown in this column.
        QString header_title;  ///< Header text for this column.
};

/**
 * @struct RecentListSchema
 * @brief Complete schema for a recent list (roles and columns).
 */
struct RecentListSchema {
        QHash<int, QByteArray> role_names;        ///< Exported role ids to role names.
        QVector<RecentListSchemaColumn> columns;  ///< Ordered columns for view presentation.
};

/**
 * @namespace RecentListSchemas
 * @brief Provides ready-made schemas and row builders for recent files and sessions.
 */
namespace RecentListSchemas
{
/**
 * @brief Create a schema for recent log files.
 *        Roles: file_path, file_name, app_name, last_opened.
 * @return The recent files schema.
 */
[[nodiscard]] auto make_recent_files_schema() -> RecentListSchema;

/**
 * @brief Create a schema for recent sessions.
 *        Roles: name, last_opened, id.
 * @return The recent sessions schema.
 */
[[nodiscard]] auto make_recent_sessions_schema() -> RecentListSchema;

/**
 * @brief Convert a RecentLogFileRecord into a role->QVariant row per the files schema.
 * @param record The recent log file record to convert.
 * @return Role->QVariant map representing the row.
 */
[[nodiscard]] auto build_recent_file_row(const RecentLogFileRecord& record) -> QHash<int, QVariant>;

/**
 * @brief Convert a RecentSessionRecord into a role->QVariant row per the sessions schema.
 * @param record The recent session record to convert.
 * @return Role->QVariant map representing the row.
 */
[[nodiscard]] auto build_recent_session_row(const RecentSessionRecord& record)
    -> QHash<int, QVariant>;
}  // namespace RecentListSchemas
