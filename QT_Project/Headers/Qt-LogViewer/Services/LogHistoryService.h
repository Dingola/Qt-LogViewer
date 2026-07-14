#pragma once

#include <QObject>
#include <QString>
#include <QUuid>
#include <QVector>

#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Models/LogQuery.h"
#include "Qt-LogViewer/Models/SearchFields.h"

/**
 * @file LogHistoryService.h
 * @brief Declares the SQLite-backed full log history service.
 */

/**
 * @class LogHistoryService
 * @brief Stores complete parsed log history independently from the bounded live view model.
 *
 * The service stores every parsed entry in SQLite and maintains an FTS5 index for efficient
 * full-history searches. All methods must be called from the thread that owns this QObject.
 */
class LogHistoryService final: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs the history service and opens its SQLite database.
         * @param parent Optional QObject parent.
         */
        explicit LogHistoryService(QObject* parent = nullptr);

        /**
         * @brief Closes the SQLite database connection.
         */
        ~LogHistoryService() override;

        /**
         * @brief Stores a parsed entry batch for a view.
         * @param view_id View that owns the entries.
         * @param entries Parsed entries to archive.
         * @return True when the transaction was committed successfully.
         */
        auto add_entries(const QUuid& view_id, const QVector<LogEntry>& entries) -> bool;

        /**
         * @brief Counts archived entries matching a log query.
         *
         * @param log_query Query describing the requested result set.
         * @return Number of matching entries, or zero when the query cannot be
         * executed.
         */
        [[nodiscard]] auto count_entries(const LogQuery& log_query) const -> qsizetype;

        /**
         * @brief Loads one page of archived entries matching a log query.
         * @param log_query Query describing the requested result set and sorting.
         * @param offset Zero-based offset within the complete result set.
         * @param limit Maximum number of entries returned.
         * @return Matching entries for the requested page.
         */
        [[nodiscard]] auto load_entries_page(const LogQuery& log_query, qsizetype offset,
                                             qsizetype limit) const -> QVector<LogEntry>;

        /**
         * @brief Searches every archived entry belonging to a view.
         * @param view_id View to search.
         * @param search_text Plain-text search expression.
         * @param search_field Entry field to search.
         * @param limit Maximum number of result entries.
         * @return Matching archived entries ordered by insertion order.
         */
        [[nodiscard]] auto search_entries(const QUuid& view_id, const QString& search_text,
                                          SearchField search_field,
                                          int limit = 100000) const -> QVector<LogEntry>;

        /**
         * @brief Removes all archived entries belonging to a view.
         * @param view_id View whose history should be removed.
         */
        auto remove_view_entries(const QUuid& view_id) -> void;

        /**
         * @brief Removes archived entries belonging to one file in a view.
         * @param view_id View that owns the file.
         * @param file_path Absolute file path.
         */
        auto remove_file_entries(const QUuid& view_id, const QString& file_path) -> void;

        /**
         * @brief Returns whether the SQLite database and required FTS tables are available.
         * @return True when history storage is ready.
         */
        [[nodiscard]] auto is_available() const -> bool;

        /**
         * @brief Returns the absolute SQLite database path.
         * @return Database file path.
         */
        [[nodiscard]] auto get_database_path() const -> QString;

    private:
        /**
         * @brief Opens the SQLite database and creates its schema.
         * @return True when initialization succeeds.
         */
        auto initialize_database() -> bool;

        /**
         * @brief Creates the archive table, FTS table, indexes, and synchronization triggers.
         * @return True when every statement succeeds.
         */
        auto create_schema() -> bool;

        /**
         * @brief Builds an FTS5 query expression from user-entered plain text.
         * @param search_text User-entered text.
         * @return Safe FTS5 query expression.
         */
        [[nodiscard]] static auto create_fts_query(const QString& search_text) -> QString;

        /**
         * @brief Converts a database record into a LogEntry.
         * @param timestamp_text ISO timestamp text.
         * @param level Entry level.
         * @param message Entry message.
         * @param app_name Entry application name.
         * @param file_path Entry source file path.
         * @return Converted LogEntry.
         */
        [[nodiscard]] static auto create_log_entry(const QString& timestamp_text,
                                                   const QString& level, const QString& message,
                                                   const QString& app_name,
                                                   const QString& file_path) -> LogEntry;

    private:
        QString m_connection_name;
        QString m_database_path;
        bool m_is_available{false};
};
