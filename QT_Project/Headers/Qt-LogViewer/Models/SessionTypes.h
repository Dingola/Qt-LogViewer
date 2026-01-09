#pragma once

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QSet>
#include <QString>
#include <QUuid>
#include <QVector>
#include <Qt>

#include "Qt-LogViewer/Controllers/FilterCoordinator.h"
#include "Qt-LogViewer/Models/LogFileInfo.h"

/**
 * @file SessionTypes.h
 * @brief Declares plain data contracts used for session persistence and restoration.
 *
 * These structs define the shape of session-related data stored in JSON and exchanged between
 * services and controllers. They are intentionally lightweight (value types) and independent
 * of UI.
 */

/**
 * @struct RecentLogFileRecord
 * @brief Describes a single recent log file item.
 *
 * Fields:
 * - file_path: Absolute path to the log file.
 * - app_name: Parsed or identified application name associated with the log.
 * - last_opened: Timestamp when the file was last opened by the application.
 */
struct RecentLogFileRecord {
        QString file_path;
        QString app_name;
        QDateTime last_opened;
};

/**
 * @struct RecentSessionRecord
 * @brief Describes a single recent session metadata item.
 *
 * Fields:
 * - id: Unique identifier of the session (string form).
 * - name: Human-readable session name.
 * - created_at: Timestamp when the session was first created/saved.
 * - last_opened: Timestamp when the session was last opened.
 */
struct RecentSessionRecord {
        QString id;
        QString name;
        QDateTime created_at;
        QDateTime last_opened;
};

/**
 * @struct FilterState
 * @brief Snapshot of filter configuration applied to a view.
 *
 * Fields:
 * - app_name: Current application-name filter (empty = all).
 * - log_levels: Set of included log levels (empty = all).
 * - search_text: Search text or regular expression.
 * - search_field: Field name to search in.
 * - use_regex: Whether search_text is interpreted as a regex.
 * - show_only_file: Absolute file path to exclusively show (empty = disabled).
 * - hidden_files: Set of absolute file paths hidden in the view.
 */
struct FilterState {
        QString app_name;
        QSet<QString> log_levels;
        QString search_text;
        QString search_field;
        bool use_regex{false};
        QString show_only_file;
        QSet<QString> hidden_files;
};

/**
 * @struct SessionViewState
 * @brief Captures the serialized state of a single view/tab in a session.
 *
 * Fields:
 * - id: Stable QUuid of the view.
 * - loaded_files: Files associated with this view.
 * - filters: FilterCoordinator state snapshot for this view.
 * - page_size: PagingProxyModel page size.
 * - current_page: Current page index in the paging proxy.
 * - sort_column: Column index used for sorting.
 * - sort_order: Sort order (ascending/descending).
 * - tab_title: Suggested title to use for the UI tab (e.g., first file name).
 */
struct SessionViewState {
        QUuid id;
        QList<LogFileInfo> loaded_files;
        FilterState filters;
        int page_size{0};
        int current_page{0};
        int sort_column{0};
        Qt::SortOrder sort_order{Qt::AscendingOrder};
        QString tab_title;
};

/**
 * @struct LogSessionDocument
 * @brief Root document representing persisted session-related data.
 *
 * Fields:
 * - schema_version: Version of the JSON schema to support migrations.
 * - active_session_id: Optional id of the currently active session.
 * - recent_files: Global list of recent log files (MRU across sessions).
 * - recent_sessions: Global list of recent session metadata (MRU of sessions).
 *
 * Note:
 * - Storing full session objects per session id is optional and typically handled
 *   as separate JSON blobs via SessionRepository (e.g., sessions/<id>.json).
 */
struct LogSessionDocument {
        int schema_version{1};
        QString active_session_id;
        QVector<RecentLogFileRecord> recent_files;
        QVector<RecentSessionRecord> recent_sessions;
};
