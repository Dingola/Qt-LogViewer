/**
 * @file LogHistoryService.cpp
 * @brief Implements the SQLite-backed full log history service.
 */

#include "Qt-LogViewer/Services/LogHistoryService.h"

#include <QCborMap>
#include <QCborParserError>
#include <QCborValue>
#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QPair>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QStringList>
#include <QVariant>

#include "Qt-LogViewer/Models/LogFieldDefinition.h"

namespace
{
struct SqlFilter {
        QString from_clause{QStringLiteral("log_entries AS entries")};
        QStringList predicates;
        QList<QPair<QString, QVariant>> bindings;
        QString error;
};

/**
 * @brief Converts a possibly null Qt string to non-null text accepted by SQLite constraints.
 * @param value String supplied by an optional parsed field.
 * @return Original value, or a non-null empty string when the input is null.
 */
[[nodiscard]] auto to_required_sql_text(const QString& value) -> QString
{
    return value.isNull() ? QStringLiteral("") : value;
}

/**
 * @brief Serializes dynamically parsed values while retaining supported QVariant types.
 * @param parsed_fields Converted values keyed by stable field identifier.
 * @return Compact CBOR representation suitable for SQLite BLOB storage.
 */
[[nodiscard]] auto serialize_parsed_fields(const LogEntry::ParsedFields& parsed_fields)
    -> QByteArray
{
    QCborMap serialized_fields;

    for (auto iterator = parsed_fields.cbegin(); iterator != parsed_fields.cend(); ++iterator)
    {
        serialized_fields.insert(iterator.key(), QCborValue::fromVariant(iterator.value()));
    }

    return QCborValue(serialized_fields).toCbor();
}

/**
 * @brief Restores dynamically parsed values from their CBOR representation.
 * @param serialized_fields CBOR data loaded from SQLite.
 * @return Restored values, or an empty map when the data is invalid.
 */
[[nodiscard]] auto deserialize_parsed_fields(const QByteArray& serialized_fields)
    -> LogEntry::ParsedFields
{
    LogEntry::ParsedFields parsed_fields;

    if (!serialized_fields.isEmpty())
    {
        QCborParserError parser_error;
        const QCborValue value = QCborValue::fromCbor(serialized_fields, &parser_error);

        if (parser_error.error == QCborError::NoError && value.isMap())
        {
            parsed_fields = value.toVariant().toMap();
        }
        else
        {
            qWarning() << "Restoring parsed log fields failed:" << parser_error.error;
        }
    }

    return parsed_fields;
}

/**
 * @brief Maps a log field identifier to an FTS5 column name.
 * @param field_id Stable log field identifier.
 * @return FTS5 column name, or an empty string for an unsupported field.
 */
[[nodiscard]] auto get_fts_column(const QString& field_id) -> QString
{
    QString column;

    if (field_id == LogField::Level)
    {
        column = QStringLiteral("level");
    }
    else if (field_id == LogField::Message)
    {
        column = QStringLiteral("message");
    }
    else if (field_id == LogField::AppName)
    {
        column = QStringLiteral("app_name");
    }
    else if (field_id == LogField::FilePath)
    {
        column = QStringLiteral("file_path");
    }

    return column;
}

/**
 * @brief Maps a filterable field identifier to a stored SQL column.
 * @param field_id Stable log field identifier.
 * @return SQL column name, or an empty string for an unsupported field.
 */
[[nodiscard]] auto get_distinct_value_column(const QString& field_id) -> QString
{
    QString column;

    if (field_id == LogField::Level)
    {
        column = QStringLiteral("level");
    }
    else if (field_id == LogField::AppName)
    {
        column = QStringLiteral("app_name");
    }
    else if (field_id == LogField::FilePath)
    {
        column = QStringLiteral("file_path");
    }

    return column;
}

/**
 * @brief Creates a SQL predicate for regular-expression searching.
 * @param search_fields Fields included in searching.
 * @param error Receives an error description for unsupported fields.
 * @return SQL predicate combining the selected fields.
 */
[[nodiscard]] auto create_regex_predicate(const QSet<QString>& search_fields,
                                          QString& error) -> QString
{
    QSet<QString> fields = search_fields;

    if (fields.isEmpty())
    {
        fields = {LogField::Level, LogField::Message, LogField::AppName, LogField::FilePath};
    }

    QStringList predicates;

    for (const QString& field_id: fields)
    {
        const QString column = get_fts_column(field_id);

        if (column.isEmpty())
        {
            error = QStringLiteral("Field is not available for regular-expression searching: %1")
                        .arg(field_id);
            break;
        }

        predicates.append(QStringLiteral("entries.%1 REGEXP :regex_pattern").arg(column));
    }

    QString predicate;

    if (error.isEmpty())
    {
        predicates.sort();

        predicate = QStringLiteral("(%1)").arg(predicates.join(QStringLiteral(" OR ")));
    }

    return predicate;
}

/**
 * @brief Creates a validated SQL ORDER BY expression.
 * @param log_query Query containing the requested sorting.
 * @param error Receives an error description for unsupported fields.
 * @return SQL ORDER BY expression.
 */
[[nodiscard]] auto create_order_by_expression(const LogQuery& log_query, QString& error) -> QString
{
    QString column;

    if (log_query.sort_field == LogField::Timestamp)
    {
        column = QStringLiteral("entries.timestamp_utc");
    }
    else if (log_query.sort_field == LogField::Level)
    {
        column = QStringLiteral("entries.level COLLATE NOCASE");
    }
    else if (log_query.sort_field == LogField::Message)
    {
        column = QStringLiteral("entries.message COLLATE NOCASE");
    }
    else if (log_query.sort_field == LogField::AppName)
    {
        column = QStringLiteral("entries.app_name COLLATE NOCASE");
    }
    else if (log_query.sort_field == LogField::FilePath)
    {
        column = QStringLiteral("entries.file_path COLLATE NOCASE");
    }
    else if (log_query.sort_field == LogField::InsertionOrder)
    {
        column = QStringLiteral("entries.id");
    }
    else
    {
        error = QStringLiteral("Field is not available for sorting: %1").arg(log_query.sort_field);
    }

    QString order_by;

    if (error.isEmpty())
    {
        const QString direction = log_query.sort_order == Qt::DescendingOrder
                                      ? QStringLiteral("DESC")
                                      : QStringLiteral("ASC");

        if (log_query.sort_field == LogField::InsertionOrder)
        {
            order_by = QStringLiteral("%1 %2").arg(column, direction);
        }
        else
        {
            order_by = QStringLiteral("%1 %2, entries.id %2").arg(column, direction);
        }
    }

    return order_by;
}

/**
 * @brief Creates an FTS5 match expression for selected fields.
 * @param search_fields Stable identifiers of fields included in searching.
 * @param search_expression Escaped FTS5 search expression.
 * @param error Receives an error description for unsupported fields.
 * @return FTS5 match expression.
 */
[[nodiscard]] auto create_match_expression(const QSet<QString>& search_fields,
                                           const QString& search_expression,
                                           QString& error) -> QString
{
    QString match_expression = search_expression;

    if (!search_fields.isEmpty())
    {
        QStringList columns;

        for (const QString& field_id: search_fields)
        {
            const QString column = get_fts_column(field_id);

            if (column.isEmpty())
            {
                error =
                    QStringLiteral("Field is not available for text searching: %1").arg(field_id);
                break;
            }

            columns.append(column);
        }

        if (error.isEmpty())
        {
            columns.sort();

            if (columns.size() == 1)
            {
                match_expression =
                    QStringLiteral("%1 : %2").arg(columns.first(), search_expression);
            }
            else
            {
                match_expression = QStringLiteral("{%1} : %2")
                                       .arg(columns.join(QLatin1Char(' ')), search_expression);
            }
        }
    }

    return match_expression;
}

/**
 * @brief Creates SQL predicates and bindings for a log query.
 * @param log_query Query containing the filter values.
 * @param search_expression Escaped FTS5 search expression.
 * @return SQL source, predicates, bindings, and validation result.
 */
[[nodiscard]] auto create_query_filter(const LogQuery& log_query) -> SqlFilter
{
    SqlFilter filter;

    filter.predicates.append(QStringLiteral("entries.view_id = :view_id"));
    filter.bindings.append(
        {QStringLiteral(":view_id"), log_query.view_id.toString(QUuid::WithoutBraces)});

    if (!log_query.app_name.isEmpty())
    {
        filter.predicates.append(QStringLiteral("entries.app_name = :app_name"));
        filter.bindings.append({QStringLiteral(":app_name"), log_query.app_name});
    }

    if (!log_query.log_levels.isEmpty())
    {
        QStringList placeholders;
        qsizetype index = 0;

        for (const QString& level: log_query.log_levels)
        {
            const QString placeholder = QStringLiteral(":log_level_%1").arg(index);

            placeholders.append(placeholder);
            filter.bindings.append({placeholder, level.trimmed().toLower()});

            ++index;
        }

        filter.predicates.append(QStringLiteral("LOWER(TRIM(entries.level)) IN (%1)")
                                     .arg(placeholders.join(QStringLiteral(", "))));
    }

    if (!log_query.show_only_file.isEmpty())
    {
        filter.predicates.append(QStringLiteral("entries.file_path = :show_only_file"));
        filter.bindings.append({QStringLiteral(":show_only_file"), log_query.show_only_file});
    }

    if (!log_query.hidden_files.isEmpty())
    {
        QStringList placeholders;
        qsizetype index = 0;

        for (const QString& file_path: log_query.hidden_files)
        {
            const QString placeholder = QStringLiteral(":hidden_file_%1").arg(index);

            placeholders.append(placeholder);
            filter.bindings.append({placeholder, file_path});

            ++index;
        }

        filter.predicates.append(QStringLiteral("entries.file_path NOT IN (%1)")
                                     .arg(placeholders.join(QStringLiteral(", "))));
    }

    if (!log_query.search_text.trimmed().isEmpty())
    {
        if (log_query.use_regex)
        {
            const QRegularExpression expression(log_query.search_text,
                                                QRegularExpression::CaseInsensitiveOption);

            if (!expression.isValid())
            {
                filter.error =
                    QStringLiteral("Invalid regular expression: %1").arg(expression.errorString());
            }
            else
            {
                const QString predicate =
                    create_regex_predicate(log_query.search_fields, filter.error);

                if (filter.error.isEmpty())
                {
                    filter.predicates.append(predicate);

                    filter.bindings.append({QStringLiteral(":regex_pattern"),
                                            QStringLiteral("(?i)") + log_query.search_text});
                }
            }
        }
        else
        {
            QSet<QString> fields = log_query.search_fields;

            if (fields.isEmpty())
            {
                fields = {LogField::Level, LogField::Message, LogField::AppName,
                          LogField::FilePath};
            }

            QStringList sorted_fields(fields.begin(), fields.end());
            sorted_fields.sort();

            QStringList search_predicates;
            qsizetype index = 0;

            for (const QString& field_id: sorted_fields)
            {
                const QString column = get_fts_column(field_id);

                if (column.isEmpty())
                {
                    filter.error = QStringLiteral("Field is not available for text searching: %1")
                                       .arg(field_id);
                    break;
                }

                const QString placeholder = QStringLiteral(":search_text_%1").arg(index);

                search_predicates.append(QStringLiteral("INSTR(LOWER(entries.%1), LOWER(%2)) > 0")
                                             .arg(column, placeholder));

                filter.bindings.append({placeholder, log_query.search_text.trimmed()});

                ++index;
            }

            if (filter.error.isEmpty())
            {
                filter.predicates.append(
                    QStringLiteral("(%1)").arg(search_predicates.join(QStringLiteral(" OR "))));
            }
        }
    }

    return filter;
}

/**
 * @brief Binds filter values to a prepared SQL query.
 * @param query Prepared SQL query.
 * @param bindings Placeholder and value pairs.
 */
auto bind_filter_values(QSqlQuery& query, const QList<QPair<QString, QVariant>>& bindings) -> void
{
    for (const auto& binding: bindings)
    {
        query.bindValue(binding.first, binding.second);
    }
}
}  // namespace

/**
 * @brief Constructs the history service and initializes the SQLite database.
 * @param parent Optional QObject parent.
 */
LogHistoryService::LogHistoryService(QObject* parent)
    : QObject(parent),
      m_connection_name(QStringLiteral("qt_log_viewer_history_%1")
                            .arg(QUuid::createUuid().toString(QUuid::WithoutBraces))),
      m_database_path(),
      m_is_available(false)
{
    m_is_available = initialize_database();
}

/**
 * @brief Closes and removes the private SQLite connection.
 */
LogHistoryService::~LogHistoryService()
{
    if (QSqlDatabase::contains(m_connection_name))
    {
        {
            QSqlDatabase database = QSqlDatabase::database(m_connection_name);
            database.close();
        }

        QSqlDatabase::removeDatabase(m_connection_name);
    }
}

/**
 * @brief Stores a parsed entry batch for a view.
 * @param view_id View that owns the entries.
 * @param entries Parsed entries to archive.
 * @return True when the transaction was committed successfully.
 */
auto LogHistoryService::add_entries(const QUuid& view_id, const QVector<LogEntry>& entries) -> bool
{
    bool added = false;

    if (m_is_available && !view_id.isNull() && !entries.isEmpty())
    {
        QSqlDatabase database = QSqlDatabase::database(m_connection_name);
        const bool transaction_started = database.transaction();

        if (transaction_started)
        {
            QSqlQuery query(database);
            query.prepare(QStringLiteral(
                "INSERT INTO log_entries "
                "(view_id, timestamp_utc, level, message, app_name, file_path, raw_record, "
                "source_line, parsed_fields_cbor) "
                "VALUES (:view_id, :timestamp_utc, :level, :message, :app_name, :file_path, "
                ":raw_record, :source_line, :parsed_fields_cbor)"));

            bool inserted = true;

            for (qsizetype index = 0; index < entries.size() && inserted; ++index)
            {
                const LogEntry& entry = entries.at(index);
                const QString timestamp_text =
                    entry.get_timestamp().isValid()
                        ? entry.get_timestamp().toUTC().toString(Qt::ISODateWithMs)
                        : QStringLiteral("");

                query.bindValue(QStringLiteral(":view_id"), view_id.toString(QUuid::WithoutBraces));
                query.bindValue(QStringLiteral(":timestamp_utc"), timestamp_text);
                query.bindValue(QStringLiteral(":level"), to_required_sql_text(entry.get_level()));
                query.bindValue(QStringLiteral(":message"),
                                to_required_sql_text(entry.get_message()));
                query.bindValue(QStringLiteral(":app_name"),
                                to_required_sql_text(entry.get_app_name()));
                query.bindValue(QStringLiteral(":file_path"),
                                to_required_sql_text(entry.get_file_info().get_file_path()));
                query.bindValue(QStringLiteral(":raw_record"),
                                to_required_sql_text(entry.get_raw_record()));
                query.bindValue(QStringLiteral(":source_line"),
                                static_cast<qlonglong>(entry.get_source_line()));
                query.bindValue(QStringLiteral(":parsed_fields_cbor"),
                                serialize_parsed_fields(entry.get_parsed_fields()));

                inserted = query.exec();

                if (!inserted)
                {
                    qWarning() << "Archiving log entry failed:" << query.lastError().text();
                }
            }

            added = inserted && database.commit();

            if (!added)
            {
                database.rollback();
            }
        }
    }

    return added;
}

/**
 * @brief Counts archived entries matching a log query.
 * @param log_query Query describing the requested result set.
 * @return Number of matching entries, or zero when the query cannot be executed.
 */
auto LogHistoryService::count_entries(const LogQuery& log_query) const -> qsizetype
{
    qsizetype entry_count = 0;

    if (m_is_available && !log_query.view_id.isNull())
    {
        const SqlFilter filter = create_query_filter(log_query);

        if (filter.error.isEmpty())
        {
            QSqlQuery query(QSqlDatabase::database(m_connection_name));

            query.prepare(
                QStringLiteral("SELECT COUNT(*) "
                               "FROM %1 "
                               "WHERE %2")
                    .arg(filter.from_clause, filter.predicates.join(QStringLiteral(" AND "))));

            bind_filter_values(query, filter.bindings);

            if (query.exec() && query.next())
            {
                entry_count = query.value(0).toLongLong();
            }
            else
            {
                qWarning() << "Counting log history entries failed:" << query.lastError().text();
            }
        }
        else
        {
            qWarning() << "Invalid log history query:" << filter.error;
        }
    }

    return entry_count;
}

/**
 * @brief Loads one page of archived entries matching a log query.
 * @param log_query Query describing the requested result set and sorting.
 * @param offset Zero-based offset within the complete result set.
 * @param limit Maximum number of entries returned.
 * @return Matching entries for the requested page.
 */
auto LogHistoryService::load_entries_page(const LogQuery& log_query, qsizetype offset,
                                          qsizetype limit) const -> QVector<LogEntry>
{
    QVector<LogEntry> entries;

    if (m_is_available && !log_query.view_id.isNull() && offset >= 0 && limit > 0)
    {
        const SqlFilter filter = create_query_filter(log_query);
        QString query_error = filter.error;
        const QString order_by = create_order_by_expression(log_query, query_error);

        if (query_error.isEmpty())
        {
            QSqlQuery query(QSqlDatabase::database(m_connection_name));

            query.prepare(
                QStringLiteral("SELECT entries.timestamp_utc, entries.level, entries.message, "
                               "entries.app_name, entries.file_path, entries.raw_record, "
                               "entries.source_line, entries.parsed_fields_cbor "
                               "FROM %1 "
                               "WHERE %2 "
                               "ORDER BY %3 "
                               "LIMIT :limit OFFSET :offset")
                    .arg(filter.from_clause, filter.predicates.join(QStringLiteral(" AND ")),
                         order_by));

            bind_filter_values(query, filter.bindings);
            query.bindValue(QStringLiteral(":limit"), static_cast<qlonglong>(limit));
            query.bindValue(QStringLiteral(":offset"), static_cast<qlonglong>(offset));

            if (query.exec())
            {
                while (query.next())
                {
                    entries.append(
                        create_log_entry(query.value(0).toString(), query.value(1).toString(),
                                         query.value(2).toString(), query.value(3).toString(),
                                         query.value(4).toString(), query.value(5).toString(),
                                         static_cast<qsizetype>(query.value(6).toLongLong()),
                                         query.value(7).toByteArray()));
                }
            }
            else
            {
                qWarning() << "Loading log history page failed:" << query.lastError().text();
            }
        }
        else
        {
            qWarning() << "Invalid log history query:" << query_error;
        }
    }

    return entries;
}

/**
 * @brief Counts matching entries grouped by normalized log level.
 * @param log_query Query describing the filtered result set.
 * @return Map of uppercase log level names to matching entry counts.
 */
auto LogHistoryService::get_log_level_counts(const LogQuery& log_query) const
    -> QMap<QString, qsizetype>
{
    QMap<QString, qsizetype> level_counts;

    if (m_is_available && !log_query.view_id.isNull())
    {
        LogQuery facet_query = log_query;
        facet_query.log_levels.clear();

        const SqlFilter filter = create_query_filter(facet_query);

        if (filter.error.isEmpty())
        {
            QSqlQuery query(QSqlDatabase::database(m_connection_name));

            query.prepare(
                QStringLiteral("SELECT UPPER(TRIM(entries.level)), COUNT(*) "
                               "FROM %1 "
                               "WHERE %2 "
                               "GROUP BY UPPER(TRIM(entries.level)) "
                               "ORDER BY UPPER(TRIM(entries.level)) ASC")
                    .arg(filter.from_clause, filter.predicates.join(QStringLiteral(" AND "))));

            bind_filter_values(query, filter.bindings);

            if (query.exec())
            {
                while (query.next())
                {
                    const QString level = query.value(0).toString();
                    const qsizetype count = query.value(1).toLongLong();

                    if (!level.isEmpty())
                    {
                        level_counts.insert(level, count);
                    }
                }
            }
            else
            {
                qWarning() << "Loading log level counts failed:" << query.lastError().text();
            }
        }
        else
        {
            qWarning() << "Invalid log history query:" << filter.error;
        }
    }

    return level_counts;
}

/**
 * @brief Returns distinct non-empty values stored for a filterable field.
 * @param view_id View whose archived values are queried.
 * @param field_id Stable field identifier.
 * @return Distinct values, or an empty set for invalid arguments or unsupported fields.
 */
auto LogHistoryService::get_distinct_values(const QUuid& view_id,
                                            const QString& field_id) const -> QSet<QString>
{
    QSet<QString> values;
    const QString column = get_distinct_value_column(field_id);

    const bool can_query = m_is_available && !view_id.isNull() && !column.isEmpty();

    if (can_query)
    {
        QSqlQuery query(QSqlDatabase::database(m_connection_name));

        query.prepare(QStringLiteral("SELECT DISTINCT %1 "
                                     "FROM log_entries "
                                     "WHERE view_id = :view_id "
                                     "AND %1 IS NOT NULL "
                                     "AND %1 <> '' "
                                     "ORDER BY %1 COLLATE NOCASE")
                          .arg(column));

        query.bindValue(QStringLiteral(":view_id"), view_id.toString(QUuid::WithoutBraces));

        if (query.exec())
        {
            while (query.next())
            {
                values.insert(query.value(0).toString());
            }
        }
        else
        {
            qWarning() << "Loading distinct log field values failed:" << query.lastError().text();
        }
    }
    else if (m_is_available && !view_id.isNull() && column.isEmpty())
    {
        qWarning() << "Field is not available for distinct values:" << field_id;
    }

    return values;
}

/**
 * @brief Searches every archived entry belonging to a view.
 * @param view_id View to search.
 * @param search_text Plain-text search expression.
 * @param search_field Entry field to search.
 * @param limit Maximum number of result entries.
 * @return Matching archived entries ordered by insertion order.
 */
auto LogHistoryService::search_entries(const QUuid& view_id, const QString& search_text,
                                       SearchField search_field,
                                       int limit) const -> QVector<LogEntry>
{
    QVector<LogEntry> entries;

    if (m_is_available && !view_id.isNull() && !search_text.trimmed().isEmpty() && limit > 0)
    {
        QString fts_column;

        if (search_field == SearchField::Message)
        {
            fts_column = QStringLiteral("message");
        }
        else if (search_field == SearchField::Level)
        {
            fts_column = QStringLiteral("level");
        }
        else if (search_field == SearchField::AppName)
        {
            fts_column = QStringLiteral("app_name");
        }

        const QString fts_query = create_fts_query(search_text);
        QString match_expression = fts_query;

        if (!fts_column.isEmpty())
        {
            match_expression = QStringLiteral("%1 : %2").arg(fts_column, fts_query);
        }

        QSqlDatabase database = QSqlDatabase::database(m_connection_name);
        QSqlQuery query(database);

        query.prepare(
            QStringLiteral("SELECT entries.timestamp_utc, entries.level, entries.message, "
                           "entries.app_name, entries.file_path, entries.raw_record, "
                           "entries.source_line, entries.parsed_fields_cbor "
                           "FROM log_entries AS entries "
                           "INNER JOIN log_entries_fts "
                           "ON log_entries_fts.rowid = entries.id "
                           "WHERE entries.view_id = :view_id "
                           "AND log_entries_fts MATCH :match_expression "
                           "ORDER BY entries.id ASC "
                           "LIMIT :limit"));

        query.bindValue(QStringLiteral(":view_id"), view_id.toString(QUuid::WithoutBraces));
        query.bindValue(QStringLiteral(":match_expression"), match_expression);
        query.bindValue(QStringLiteral(":limit"), limit);

        if (query.exec())
        {
            while (query.next())
            {
                entries.append(create_log_entry(
                    query.value(0).toString(), query.value(1).toString(), query.value(2).toString(),
                    query.value(3).toString(), query.value(4).toString(), query.value(5).toString(),
                    static_cast<qsizetype>(query.value(6).toLongLong()),
                    query.value(7).toByteArray()));
            }
        }
        else
        {
            qWarning() << "Log history search failed:" << query.lastError().text();
        }
    }

    return entries;
}

/**
 * @brief Removes all archived entries belonging to a view.
 * @param view_id View whose history should be removed.
 */
auto LogHistoryService::remove_view_entries(const QUuid& view_id) -> void
{
    if (m_is_available && !view_id.isNull())
    {
        QSqlQuery query(QSqlDatabase::database(m_connection_name));
        query.prepare(QStringLiteral("DELETE FROM log_entries WHERE view_id = :view_id"));
        query.bindValue(QStringLiteral(":view_id"), view_id.toString(QUuid::WithoutBraces));
        query.exec();
    }
}

/**
 * @brief Removes archived entries belonging to one file in a view.
 * @param view_id View that owns the file.
 * @param file_path Absolute file path.
 */
auto LogHistoryService::remove_file_entries(const QUuid& view_id, const QString& file_path) -> void
{
    if (m_is_available && !view_id.isNull() && !file_path.isEmpty())
    {
        QSqlQuery query(QSqlDatabase::database(m_connection_name));
        query.prepare(QStringLiteral(
            "DELETE FROM log_entries WHERE view_id = :view_id AND file_path = :file_path"));
        query.bindValue(QStringLiteral(":view_id"), view_id.toString(QUuid::WithoutBraces));
        query.bindValue(QStringLiteral(":file_path"), file_path);
        query.exec();
    }
}

/**
 * @brief Returns whether the SQLite database and required FTS tables are available.
 * @return True when history storage is ready.
 */
auto LogHistoryService::is_available() const -> bool
{
    return m_is_available;
}

/**
 * @brief Returns the absolute SQLite database path.
 * @return Database file path.
 */
auto LogHistoryService::get_database_path() const -> QString
{
    return m_database_path;
}

/**
 * @brief Opens the SQLite database and creates its schema.
 * @return True when initialization succeeds.
 */
auto LogHistoryService::initialize_database() -> bool
{
    bool initialized = false;

    const QString config_path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    const QString history_path = QDir(config_path).filePath(QStringLiteral("history"));

    if (QDir().mkpath(history_path))
    {
        m_database_path = QDir(history_path).filePath(QStringLiteral("log_history.sqlite"));

        QSqlDatabase database =
            QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connection_name);

        database.setDatabaseName(m_database_path);
        database.setConnectOptions(QStringLiteral("QSQLITE_ENABLE_REGEXP"));

        if (database.open())
        {
            initialized = create_schema();
        }

        if (database.open())
        {
            initialized = create_schema();
        }
    }

    return initialized;
}

/**
 * @brief Creates the archive table, FTS table, indexes, and synchronization triggers.
 * @return True when every statement succeeds.
 */
auto LogHistoryService::create_schema() -> bool
{
    QSqlQuery query(QSqlDatabase::database(m_connection_name));
    bool schema_created =
        query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS log_entries ("
                                  "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                  "view_id TEXT NOT NULL, "
                                  "timestamp_utc TEXT NOT NULL, "
                                  "level TEXT NOT NULL, "
                                  "message TEXT NOT NULL, "
                                  "app_name TEXT NOT NULL, "
                                  "file_path TEXT NOT NULL, "
                                  "raw_record TEXT NOT NULL DEFAULT '', "
                                  "source_line INTEGER NOT NULL DEFAULT -1, "
                                  "parsed_fields_cbor BLOB NOT NULL DEFAULT X'A0')"));

    if (schema_created)
    {
        schema_created = ensure_parse_metadata_columns();
    }

    const QStringList statements{
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_log_entries_view_id "
                       "ON log_entries(view_id, id)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_log_entries_view_timestamp "
                       "ON log_entries(view_id, timestamp_utc, id)"),
        QStringLiteral("CREATE VIRTUAL TABLE IF NOT EXISTS log_entries_fts "
                       "USING fts5(level, message, app_name, file_path, "
                       "content='log_entries', content_rowid='id')"),
        QStringLiteral(
            "CREATE TRIGGER IF NOT EXISTS log_entries_ai AFTER INSERT ON log_entries BEGIN "
            "INSERT INTO log_entries_fts(rowid, level, message, app_name, file_path) "
            "VALUES (new.id, new.level, new.message, new.app_name, new.file_path); END"),
        QStringLiteral(
            "CREATE TRIGGER IF NOT EXISTS log_entries_ad AFTER DELETE ON log_entries BEGIN "
            "INSERT INTO log_entries_fts(log_entries_fts, rowid, level, message, app_name, "
            "file_path) VALUES ('delete', old.id, old.level, old.message, old.app_name, "
            "old.file_path); END"),
        QStringLiteral(
            "CREATE TRIGGER IF NOT EXISTS log_entries_au AFTER UPDATE ON log_entries BEGIN "
            "INSERT INTO log_entries_fts(log_entries_fts, rowid, level, message, app_name, "
            "file_path) VALUES ('delete', old.id, old.level, old.message, old.app_name, "
            "old.file_path); "
            "INSERT INTO log_entries_fts(rowid, level, message, app_name, file_path) "
            "VALUES (new.id, new.level, new.message, new.app_name, new.file_path); END")};

    for (qsizetype index = 0; index < statements.size() && schema_created; ++index)
    {
        schema_created = query.exec(statements.at(index));
    }

    return schema_created;
}

/**
 * @brief Adds parser-metadata columns missing from an existing history database.
 * @return True when the table already is current or every migration statement succeeds.
 */
auto LogHistoryService::ensure_parse_metadata_columns() -> bool
{
    QSqlQuery query(QSqlDatabase::database(m_connection_name));
    bool migrated = query.exec(QStringLiteral("PRAGMA table_info(log_entries)"));
    QSet<QString> column_names;

    while (migrated && query.next())
    {
        column_names.insert(query.value(1).toString());
    }

    QStringList migration_statements;

    if (migrated && !column_names.contains(QStringLiteral("raw_record")))
    {
        migration_statements.append(QStringLiteral(
            "ALTER TABLE log_entries ADD COLUMN raw_record TEXT NOT NULL DEFAULT ''"));
    }

    if (migrated && !column_names.contains(QStringLiteral("source_line")))
    {
        migration_statements.append(QStringLiteral(
            "ALTER TABLE log_entries ADD COLUMN source_line INTEGER NOT NULL DEFAULT -1"));
    }

    if (migrated && !column_names.contains(QStringLiteral("parsed_fields_cbor")))
    {
        migration_statements.append(QStringLiteral(
            "ALTER TABLE log_entries ADD COLUMN parsed_fields_cbor BLOB NOT NULL DEFAULT X'A0'"));
    }

    for (qsizetype index = 0; index < migration_statements.size() && migrated; ++index)
    {
        migrated = query.exec(migration_statements.at(index));
    }

    return migrated;
}

/**
 * @brief Builds an FTS5 query expression from user-entered plain text.
 * @param search_text User-entered text.
 * @return Safe FTS5 query expression.
 */
auto LogHistoryService::create_fts_query(const QString& search_text) -> QString
{
    const QStringList terms =
        search_text.trimmed().split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);

    QStringList expressions;
    expressions.reserve(terms.size());

    for (QString term: terms)
    {
        term.replace(QLatin1Char('"'), QStringLiteral("\"\""));

        expressions.append(QStringLiteral("\"%1\"*").arg(term));
    }

    return expressions.join(QStringLiteral(" AND "));
}

/**
 * @brief Converts a database record into a LogEntry.
 * @param timestamp_text ISO timestamp text.
 * @param level Entry level.
 * @param message Entry message.
 * @param app_name Entry application name.
 * @param file_path Entry source file path.
 * @param raw_record Unmodified source record.
 * @param source_line One-based source line, or -1 when unknown.
 * @param parsed_fields_cbor CBOR-encoded dynamically parsed values.
 * @return Converted LogEntry.
 */
auto LogHistoryService::create_log_entry(const QString& timestamp_text, const QString& level,
                                         const QString& message, const QString& app_name,
                                         const QString& file_path, const QString& raw_record,
                                         qsizetype source_line,
                                         const QByteArray& parsed_fields_cbor) -> LogEntry
{
    const QDateTime timestamp = QDateTime::fromString(timestamp_text, Qt::ISODateWithMs);
    const LogFileInfo file_info(file_path, app_name);
    LogEntry entry(timestamp, level, message, file_info);
    entry.set_parse_metadata(raw_record, source_line,
                             deserialize_parsed_fields(parsed_fields_cbor));
    return entry;
}
